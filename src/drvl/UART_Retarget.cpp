#include <stdio.h>
#include "main.h"

extern UART_HandleTypeDef huart1;

/* ==================== 重 定 向 printf / scanf ==================== */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar(void)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    uint8_t ch8 = (uint8_t)ch;
    HAL_UART_Transmit_DMA(&huart1, &ch8, 1);
    return ch;
}

GETCHAR_PROTOTYPE
{
    uint8_t ch8;
    HAL_UART_Receive(&huart1, &ch8, 1, HAL_MAX_DELAY);
    return (int)ch8;
}

/* ==================== 重 定 向 cout / cin ==================== */

extern "C" int _write(int fd, char* buf, int len)
{
    if(fd == 1 || fd == 2){
        HAL_UART_Transmit(&huart1, (uint8_t*)buf, len, HAL_MAX_DELAY);
        return len;
    }
    return -1;
}

extern "C" int _read(int fd, char* buf, int len)
{
    if(fd == 0){
        HAL_UART_Receive(&huart1, (uint8_t*)buf, len, HAL_MAX_DELAY);
        return len;
    }
    return -1;
}
