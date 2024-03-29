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
        printf("Invocation: sudo ./gs_test.out <TTY device>\n\n");
        return 0;
    }
    eprintf("Test beginning.");

    uhf_modem_t fd = uhf_init(argv[1]);

    if (fd < 3) {
        eprintf("FAILURE: fd is only %d.", fd);
        return -1;
    }

    char wr_buf[UHF_MAX_PAYLOAD_SIZE];
    char rd_buf[UHF_MAX_PAYLOAD_SIZE];

    memset(wr_buf, 0x0, UHF_MAX_PAYLOAD_SIZE);
    memset(rd_buf, 0x0, UHF_MAX_PAYLOAD_SIZE);

    char payload[] = "One two three four five six seven eight nine ten";

    eprintf("Attempting to write...");
    while (uhf_write(fd, payload, sizeof(payload)) != UHF_SUCCESS);

    eprintf("Attempting to read...");
    while(uhf_read(fd, rd_buf, UHF_MAX_PAYLOAD_SIZE) != UHF_SUCCESS);
    
    eprintf("What we sent vs what we got:");
    memprintl_hex(payload, sizeof(payload));
    memprintl_hex(rd_buf, UHF_MAX_PAYLOAD_SIZE);

    if (memcmp(payload, rd_buf, sizeof(payload)) == 0){
        eprintf("SUCCESS: Buffers are EQUAL.");
    } else {
        eprintf("FAILURE: Buffers are UNEQUAL");
    }

    eprintf("Test complete.");

    return 1;
}