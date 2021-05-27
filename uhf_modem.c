#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "uhf_modem.h"

unsigned char uhf_max_retries = 0x10;
volatile bool uhf_done = false;
int uhf_sleep_time = 5; // 0.5 seconds of block on read by default

#define UHF_GUID 0x6f35
#define UHF_TERMINATION 0x0d0a // CRLF
typedef struct __attribute__((packed))
{
    uint16_t guid;
    uint16_t crc;
    uint8_t payload[UHF_MAX_PAYLOAD_SIZE];
    uint16_t crc1;
    uint16_t termination;
} uhf_frame_t;
#define UHF_MAX_FRAME_SIZE sizeof(uhf_frame_t)

uhf_modem_t uhf_init(const char *sername)
{
    int fd = -1;
    // check device name
    if (sername == NULL)
    {
        eprintf("Serial device name pointer is null");
        goto ret;
    }
    // open device
    fd = open(sername, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd <= 0)
    {
        eprintf("Could not open %s", sername);
        goto ret;
    }
    // set attributes
    struct termios tty[1];
    if (tcgetattr(fd, tty))
    {
        eprintf("Error %d getting TTY attributes", errno);
        goto cleanup;
    }
    cfsetospeed(tty, B9600);
    cfsetispeed(tty, B9600);
    tty->c_cflag = (tty->c_cflag & ~CSIZE) | CS8; // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty->c_iflag = 0;                  // disable break processing
    tty->c_lflag = 0;                  // no signaling chars, no echo,
                                       // no canonical processing
    tty->c_oflag = 0;                  // no remapping, no delays
    tty->c_cc[VMIN] = 63;              // read is blocking call
    tty->c_cc[VTIME] = uhf_sleep_time; // 0.1 * GS_SLEEP_TIME seconds read timeout

    tty->c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

    tty->c_cflag |= (CLOCAL | CREAD);   // ignore modem controls,
                                        // enable reading
    tty->c_cflag &= ~(PARENB | PARODD); // shut off parity
    // tty->c_cflag |= parity;
    tty->c_cflag &= ~CSTOPB;
    tty->c_cflag &= ~CRTSCTS;
    if (tcsetattr(fd, TCSANOW, tty) != 0)
    {
        eprintf("Error %d from tcsetattr", errno);
        goto cleanup;
    }
    else
        goto ret; // success
cleanup:
    close(fd);
ret:
    if (uhf_max_retries == 0)
        uhf_max_retries = 0x40;
    return (uhf_modem_t)fd;
}

void uhf_destroy(uhf_modem_t dev)
{
    close(dev);
    return;
}

int uhf_read(uhf_modem_t dev, char *buf, ssize_t len)
{
    if (len < UHF_MAX_PAYLOAD_SIZE)
    {
        eprintf("Payload buffer has to have the size %d", UHF_MAX_PAYLOAD_SIZE);
        return UHF_ERROR;
    }
    if (buf == NULL)
    {
        eprintf("Output buffer is NULL");
        return UHF_ERROR;
    }
    uhf_done = false;
    int retval;
    char *tmp;
    uhf_frame_t frame[1];
    ssize_t rd_sz;
    unsigned char tries;
retry:
    memset(frame, 0x0, 64);
    retval = UHF_ERROR;
    tmp = (char *)frame;
    tries = 0;
    rd_sz = 0;
    // start reading from serial
    while ((rd_sz < UHF_MAX_FRAME_SIZE) && (tries++ < uhf_max_retries) && (!uhf_done))
    {
        if ((rd_sz == 3) && (tmp[0] == 'O') && (tmp[1] == 'K') && (tmp[2] == '+')) // check for OK+ which would indicate PIPE mode, in that case reset
        {
            eprintf("PIPE command received, resetting to read data!");
            rd_sz = 0;
            tries = 0;
        }
        ssize_t _rd_sz = read(dev, tmp + rd_sz, UHF_MAX_FRAME_SIZE - rd_sz);
        if (_rd_sz == -1) // Error
        {
            eprintf("Error reading data");
            perror(__func__);
            goto ret;
        }
        rd_sz += _rd_sz;
        if ((rd_sz == 4) && (frame->guid != UHF_GUID))
        { // What if we read 64 bytes of garbage? Do we retry forever?
            // received wrong GUID, reset to top
            eprintf("Received GUID 0x%04x, not SPACE HAUC GUID", frame->guid);
            goto retry;
        }
    }
    if (uhf_done)
    {
        eprintf("UHF loop aborted");
        retval = -UHF_ABORT;
        goto ret;
    }
    if ((tries >= uhf_max_retries) && (rd_sz < UHF_MAX_FRAME_SIZE))
    {
        eprintf("Received %d/%d bytes, retry count %u expired", rd_sz, UHF_MAX_FRAME_SIZE, tries);
        retval = UHF_TOUT;
        goto ret;
    }
    if (rd_sz < UHF_MAX_FRAME_SIZE)
    { // When would we get here? We read less than the max size, but didnt keep trying until we ran out of tries...
        eprintf("Received %d/%d bytes, not full frame", rd_sz, UHF_MAX_FRAME_SIZE);
        retval = -UHF_NOT_FULL_FRAME;
        goto ret;
    }
    if (frame->termination != UHF_TERMINATION)
    {
        eprintf("Received wrong termination 0x%04x", frame->termination); // not considered fatal as long as CRCs are equal
    }
    if (frame->crc != frame->crc1)
    {
        // "Header and footer CRCs are not equivalent (0x%04x vs. 0x%04x)." <--
        eprintf("Received wrong CRC in header (0x%04x) and footer (0x%04x)", frame->crc, frame->crc1);
        goto retry;
    }
    if (frame->crc == 0x660a)
    {
        eprintf("Payload is a string of zeros");
        goto retry;
    }
    if (frame->crc != crc16(frame->payload, UHF_MAX_PAYLOAD_SIZE))
    {
        eprintf("Invalid payload CRC");
        retval = -UHF_CRC_INVALID;
    }
    // everything good
    memcpy(buf, frame->payload, UHF_MAX_PAYLOAD_SIZE);
    retval = UHF_SUCCESS;
ret:
    return retval;
}

// Return: Negative on failure, # bytes written on success
ssize_t uhf_write(uhf_modem_t dev, char *buf, ssize_t len)
{
    if (len > UHF_MAX_PAYLOAD_SIZE)
    {
        eprintf("Payload buffer has to be %d bytes.", UHF_MAX_PAYLOAD_SIZE);
        return UHF_ERROR;
    }

    if (buf == NULL)
    {
        eprintf("Input buffer is NULL.");
        return UHF_ERROR;
    }

    uhf_frame_t frame[1];
    memset(frame, 0x0, 64);
    char *tmp = (char *)frame;
    // Set frame's values prior to writing.
    frame->guid = UHF_GUID;
    memcpy(frame->payload, buf, len);
    frame->termination = UHF_TERMINATION;
    frame->crc = crc16(frame->payload, UHF_MAX_PAYLOAD_SIZE);
    frame->crc1 = frame->crc;

    unsigned char tries = 0;
    int retval = 0;
    ssize_t wr_sz = 0;

    // Start writing to serial.
    while ((wr_sz < UHF_MAX_FRAME_SIZE) && (tries++ < uhf_max_retries) && (!uhf_done))
    {
        retval = write(dev, tmp + wr_sz, UHF_MAX_FRAME_SIZE - wr_sz);

        if (retval >= 0)
        {
            wr_sz += retval;
        }
        else
        {
            eprintf("Error writing data.");
            break;
        }
    }

    tcflush(dev, TCOFLUSH);

    if (uhf_done)
    {
        eprintf("UHF loop aborted");
        return -UHF_ABORT;
    }

    if ((tries >= uhf_max_retries) && (wr_sz < UHF_MAX_FRAME_SIZE))
    {
        eprintf("Maxxed out retries (%d) and did not write entire frame (%d/%d).", tries, wr_sz, UHF_MAX_FRAME_SIZE);
        return UHF_ERROR;
    }

    return UHF_SUCCESS;
}