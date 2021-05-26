/**
 * @file uhf_modem.h
 * @author Sunip K. Mukherjee (sunipkmukherjee@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _UHF_MODEM_H
#define _UHF_MODEM_H

#ifdef __cplusplus
// extern "C" {
#endif // __cplusplus
#include <stdio.h>
#include <stdint.h>

#ifndef eprintf
#define NOTIFY "\x1b[32m"
#define WARN "\x1b[33m"
#define ERR "\x1b[31m"
#define CLR "\x1b[0m"
#define eprintf(str, ...) \
    { \
        fprintf(stderr, "%s, %d: " str CLR "\n", __func__, __LINE__, ##__VA_ARGS__); \
        fflush(stderr); \
    }
#define eprintlf(str, ...) \
    { \
        fprintf(stderr, "%s, %d: " str CLR, __func__, __LINE__, ##__VA_ARGS__); \
        fflush(stderr); \
    }
#endif // eprintf

#define UHF_GUID 0x6f35
#define UHF_TERMINATION 0xaaaa
/**
 * @brief 
 * 
 */
#define UHF_MAX_PAYLOAD_SIZE 56
/**
 * @brief 
 * 
 */
typedef int uhf_modem_t;
/**
 * @brief 
 * 
 * @return typedef struct 
 */
typedef struct __attribute__((packed))
{
    uint16_t guid;
    uint16_t crc;
    uint8_t payload[UHF_MAX_PAYLOAD_SIZE];
    uint16_t crc1;
    uint16_t termination;
} uhf_frame_t;
/**
 * @brief 
 * 
 */
extern unsigned char uhf_max_retries;
/**
 * @brief 
 * 
 */
extern int UHF_SLEEP_TIME;

/**
 * @brief 
 * 
 */
#define UHF_MAX_FRAME_SIZE sizeof(uhf_frame_t)
enum UHF_ERROR
{
    UHF_ERROR = -1,
    UHF_TOUT = 0,
    UHF_SUCCESS = 1,
    UHF_CRC_INVALID = 2,
    UHF_NOT_FULL_FRAME = 3
};
/**
 * @brief 
 * 
 * @param sername 
 * @param baud 
 * @return uhf_modem_t 
 */
uhf_modem_t uhf_init(const char *sername, int baud);
/**
 * @brief 
 * 
 * @param dev 
 * @param buf 
 * @param len 
 * @return int 
 */
int uhf_read(uhf_modem_t dev, char *buf, ssize_t len);
/**
 * @brief 
 * 
 * @param dev 
 * @param buf 
 * @param len 
 * @return int 
 */
ssize_t uhf_write(uhf_modem_t dev, char *buf, ssize_t len);
/**
 * @brief 
 * 
 * @param dev 
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
// }
#endif //__cplusplus

#endif // _UHF_MODEM_H