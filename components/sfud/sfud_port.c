/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2016-04-23
 */

#include <sfud.h>
#include <stdarg.h>

#include "luat_spi.h"
#include "luat_rtos.h"
// static char log_buf[256];

// void sfud_log_debug(const char *file, const long line, const char *format, ...);

/**
 * SPI write data then read data
 */
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
        size_t read_size) {
    sfud_err result = SFUD_SUCCESS;
    if (write_size) {
        SFUD_ASSERT(write_buf);
    }
    if (read_size) {
        SFUD_ASSERT(read_buf);
    }
    int type = (*(luat_sfud_flash_t*)(spi->user_data)).luat_spi;
    if ( type == LUAT_TYPE_SPI ) {
        luat_spi_t* spi_flash = (luat_spi_t*) ((*(luat_sfud_flash_t*)(spi->user_data)).user_data);
        luat_spi_lock(spi_flash -> id);
        if (write_size && read_size) {
            if (luat_spi_transfer(spi_flash -> id, (const char*)write_buf, write_size, (char*)read_buf, read_size) <= 0) {
                result = SFUD_ERR_TIMEOUT;
            }
        } else if (write_size) {
            if (luat_spi_send(spi_flash -> id,  (const char*)write_buf, write_size) <= 0) {
                result = SFUD_ERR_WRITE;
            }
        } else {
            if (luat_spi_recv(spi_flash -> id, (char*)read_buf, read_size) <= 0) {
                result = SFUD_ERR_READ;
            }
        }
        luat_spi_unlock(spi_flash -> id);
    }
    else if ( type == LUAT_TYPE_SPI_DEVICE ) {
        luat_spi_device_t* spi_dev = (luat_spi_device_t*) ((*(luat_sfud_flash_t*)(spi->user_data)).user_data);
        if (write_size && read_size) {
            if (luat_spi_device_transfer(spi_dev , (const char*)write_buf, write_size, (char*)read_buf, read_size) <= 0) {
                result = SFUD_ERR_TIMEOUT;
            }
        } else if (write_size) {
            if (luat_spi_device_send(spi_dev ,  (const char*)write_buf, write_size) <= 0) {
                result = SFUD_ERR_WRITE;
            }
        } else {
            if (luat_spi_device_recv(spi_dev , (char*)read_buf, read_size) <= 0) {
                result = SFUD_ERR_READ;
            }
        }
    }
    return result;
}

#ifdef SFUD_USING_QSPI
/**
 * read flash data by QSPI
 */
static sfud_err qspi_read(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
        uint8_t *read_buf, size_t read_size) {
    sfud_err result = SFUD_SUCCESS;

    /**
     * add your qspi read flash data code
     */

    return result;
}
#endif /* SFUD_USING_QSPI */

/* about 100 microsecond delay */
static void retry_delay_1ms(void) {
    luat_rtos_task_sleep(1);
}
static void retry_delay_10ms(void) {
    luat_rtos_task_sleep(10);
}

static void luat_sfud_lock(const sfud_spi *spi)
{
    luat_sfud_flash_t *sfud_flash = (luat_sfud_flash_t*)(spi->user_data);
    luat_mutex_lock(sfud_flash->sem);
}

static void luat_sfud_unlock(const sfud_spi *spi)
{
    luat_sfud_flash_t *sfud_flash = (luat_sfud_flash_t*)(spi->user_data);
    luat_mutex_unlock(sfud_flash->sem);
}
sfud_err sfud_spi_port_init(sfud_flash *flash) {
    sfud_err result = SFUD_SUCCESS;

    /* port SPI device interface */
    flash->spi.wr = spi_write_read;
    flash->spi.user_data = &(flash->luat_sfud);
    flash->luat_sfud.sem = luat_mutex_create();
    flash->spi.lock = luat_sfud_lock;
    flash->spi.unlock = luat_sfud_unlock;
    flash->retry.delay = retry_delay_1ms;
    flash->retry.long_delay = retry_delay_10ms;
    flash->retry.times = 20;     /* write操作 20ms足够了 */
    return result;
}

// /**
//  * This function is print debug info.
//  *
//  * @param file the file which has call this function
//  * @param line the line number which has call this function
//  * @param format output format
//  * @param ... args
//  */
// void sfud_log_debug(const char *file, const long line, const char *format, ...) {
//     va_list args;

//     /* args point to the first variable parameter */
//     va_start(args, format);
//     printf("[SFUD](%s:%ld) ", file, line);
//     /* must use vprintf to print */
//     vsnprintf(log_buf, sizeof(log_buf), format, args);
//     printf("%s\n", log_buf);
//     va_end(args);
// }

// /**
//  * This function is print routine info.
//  *
//  * @param format output format
//  * @param ... args
//  */
// void sfud_log_info(const char *format, ...) {
//     va_list args;

//     /* args point to the first variable parameter */
//     va_start(args, format);
//     printf("[SFUD]");
//     /* must use vprintf to print */
//     vsnprintf(log_buf, sizeof(log_buf), format, args);
//     printf("%s\n", log_buf);
//     va_end(args);
// }
