#include "uhf_modem.h"
#include <stdio.h>
/**
 * @brief Prints n bytes starting at address pointed to by m in hexadecimal with prefix and newline.
 * 
 * @param m 
 * @param n 
 */
static void memprintl_hex(void *m, ssize_t n)
{
    eprintlf(NOTIFY "(%p) ", m);
    for (int i = 0; i < n; i++)
    {
        printf("%02hhx", *((unsigned char *)m + i));
    }
    printf("(END)\n");
    fflush(stdout);
}

