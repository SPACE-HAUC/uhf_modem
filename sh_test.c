/**
 * @file gs_test.c
 * @author Mit Bailey (mitbailey99@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "uhf_modem.h"
#include "debugheader.h"

int main (int argc, char *argv[]){
    if (argc != 2)
    {
        printf("Invocation: sudo ./sh_test.out <TTY device>\n\n");
        return 0;
    }
    eprintf("Test beginning.");

    uhf_modem_t fd = uhf_init(argv[1], B9600);

    if (fd < 3) {
        eprintf("FAILURE: fd is only %d.", fd);
        return -1;
    }

    char rd_buf[UHF_MAX_PAYLOAD_SIZE];

    memset(rd_buf, 0x0, UHF_MAX_PAYLOAD_SIZE);

    eprintf("Attempting to read...");
    while (uhf_read(fd, rd_buf, UHF_MAX_PAYLOAD_SIZE) != 1);

    eprintf("Attempting to write...");
    ssize_t out_sz = uhf_write(fd, rd_buf, UHF_MAX_PAYLOAD_SIZE);
    eprintf("Sent %d/%d bytes.", out_sz, UHF_MAX_PAYLOAD_SIZE);
    
    eprintf("Test complete.");

    return 1;
}