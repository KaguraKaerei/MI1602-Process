#ifndef _S_DEFINETOOLS_H_
#define _S_DEFINETOOLS_H_

#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

/* ==================== 宏 定 义 所 需 函 数 声 明 ==================== */

bool _DT_Delay_Check_Timeout(uint32_t* start_timer, uint32_t ms);

/* ==================== 宏 定 义 工 具 ==================== */

/**
 * @brief 定义周期任务执行宏
 * @param period_ms 周期时间, 单位为毫秒
 * @param code_block 代码块, 即需要周期执行的代码, 需要用花括号括起来
 */
#define PERIODIC_TASK(period_ms, code_block)    _DT_PERIODIC_TASK(__LINE__, period_ms, code_block)                           
#define _DT_PERIODIC_TASK(line, period_ms, code_block)                          \
            do{                                                                 \
                static uint32_t timer##line = 0;                                \
                if(timer##line == 0) timer##line = HAL_GetTick();               \
                if((uint32_t)(HAL_GetTick() - (timer##line)) >= (period_ms)){   \
                    (timer##line) = HAL_GetTick();                              \
                    code_block                                                  \
                }                                                               \
            } while(0)

#define NB_DELAY_MS(ms, ...)    _DT_NB_DELAY_MS(__LINE__, ms, ##__VA_ARGS__)
#define _DT_NB_DELAY_MS(line, ms, ...)                                      \
            do{                                                             \
                static uint32_t delayer##line = 0;                          \
                if(_DT_Delay_Check_Timeout(&delayer##line, ms) == false)    \
                    return __VA_ARGS__;                                     \
            } while(0)

#endif
