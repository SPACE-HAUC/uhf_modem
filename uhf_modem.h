/**
 * @file uhf_modem.h
 * @author Sunip K. Mukherjee (sunipkmukherjee@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2021-05-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _UHF_MODEM_H
#define _UHF_MODEM_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#include <stdio.h>
#include <stdint.h>

#ifndef eprintf
#define NOTIFY "\x1b[32m"
#define WARN "\x1b[33m"
#define ERR "\x1b[31m"
#define CLR "\x1b[0m"
#define eprintf(str, ...)                                                            \
    {                                                                                \
        fprintf(stderr, "%s, %d: " str CLR "\n", __func__, __LINE__, ##__VA_ARGS__); \
        fflush(stderr);                                                              \
    }
#define eprintlf(str, ...)                                                      \
    {                                                                           \
        fprintf(stderr, "%s, %d: " str CLR, __func__, __LINE__, ##__VA_ARGS__); \
        fflush(stderr);                                                         \
    }
#endif // eprintf

/**
 * @brief Maximum size of UHF frame payload
 * 
 */
#define UHF_MAX_PAYLOAD_SIZE 56
/**
 * @brief UHF modem device (file descriptor)
 * 
 */
typedef int uhf_modem_t;
/**
 * @brief Maximum number of retries for UHF operation (0x0 -- 0xff)
 * 
 */
extern unsigned char uhf_max_retries;
/**
 * @brief Read/write blocking time in units of 100 ms.
 * 
 */
extern int uhf_sleep_time;
/**
 * @brief This variable can be set to abort an UHF operation. Operation returns -UHF_ABORT.
 * 
 */
extern volatile bool uhf_done;
/**
 * @brief Enumeration of UHF device return values
 * 
 */
enum UHF_RETVALS
{
    UHF_ERROR = -1,         //!< UHF operation error
    UHF_TOUT = 0,           //!< UHF operation timeout
    UHF_SUCCESS = 1,        //!< UHF operation success
    UHF_CRC_INVALID = 2,    //!< UHF CRC invalid
    UHF_NOT_FULL_FRAME = 3, //!< UHF full frame not received
    UHF_ABORT = 4           //!< uhf_done was set to abort the operation
};
/**
 * @brief Open a serial device as UHF modem
 * 
 * @param sername Pointer to serial device name string. The serial device is opened at baud 9600.
 * @return uhf_modem_t 
 */
uhf_modem_t uhf_init(const char *sername);
/**
 * @brief Read data from UHF serial
 * 
 * @param dev UHF device
 * @param buf Input buffer
 * @param len Length of input buffer (has to be equal or greater than UHF_MAX_PAYLOAD_SIZE)
 * @return int UHF_SUCCESS (1) on success, UHF_TOUT (0) on timeout, negative on error
 */
int uhf_read(uhf_modem_t dev, char *buf, ssize_t len);
/**
 * @brief Write data to UHF serial
 * 
 * @param dev UHF device
 * @param buf Output buffer
 * @param len Length (has to be smaller or equal to UHF_MAX_PAYLOAD_SIZE)
 * @return int UHF_SUCCESS (1) on success, UHF_TOUT (0) on timeout, negative on error
 */
ssize_t uhf_write(uhf_modem_t dev, char *buf, ssize_t len);
/**
 * @brief Close a UHF device
 * 
 * @param dev UHF device
 */
void uhf_destroy(uhf_modem_t dev);

/*
 * this is the CCITT CRC 16 polynomial X^16  + X^12  + X^5  + 1.
 * This works out to be 0x1021, but the way the algorithm works
 * lets us use 0x8408 (the reverse of the bit pattern).  The high
 * bit is always assumed to be set, thus we only use 16 bits to
 * represent the 17 bit value.
 */
static inline uint16_t crc16(unsigned char *data_p, uint16_t length)
{
#define CRC16_POLY 0x8408
    unsigned char i;
    unsigned int data;
    unsigned int crc = 0xffff;

    if (length == 0)
        return (~crc);

    do
    {
        for (i = 0, data = (unsigned int)0xff & *data_p++;
             i < 8;
             i++, data >>= 1)
        {
            if ((crc & 0x0001) ^ (data & 0x0001))
                crc = (crc >> 1) ^ CRC16_POLY;
            else
                crc >>= 1;
        }
    } while (--length);

    crc = ~crc;
    data = crc;
    crc = (crc << 8) | (data >> 8 & 0xff);

    return (crc);
}
#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _UHF_MODEM_H