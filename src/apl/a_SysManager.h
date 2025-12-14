#ifndef _A_SYSMANAGER_H_
#define _A_SYSMANAGER_H_

// c/c++标准库头文件
#include <stdio.h>
// hal库头文件
#include "main.h"
// 驱动层头文件
#include "d_led.h"
#include "d_thermal_image.h"
// 服务层头文件
#include "s_LOG.hpp"
#include "s_DefineTools.h"
// 应用层头文件



/* ==================== 全 局 变 量 ==================== */



/* ==================== 初 始 化 与 轮 询 进 程 ==================== */

/**
 * @brief 系统管理模块初始化函数
 * @note 该函数在系统启动时(hal初始化后, 轮询前)调用一次, 用于初始化各个模块
 */
static inline void SysManager_Init(void)
{
    /* ===== 驱动层初始化部分 ===== */

    /* ===== 服务层初始化部分 ===== */

    /* ===== 应用层初始化部分 ===== */

    /* ===== 轮询前执行部分 ===== */

    LED_OFF(GREEN);
    LED_OFF(RED);
    HAL_Delay(500);
}

/**
 * @brief 系统管理模块轮询进程函数
 * @note 该函数在系统启动后的主循环中调用
 */
static inline void SysManager_Process(void)
{
    /* ===== 状态机驱动部分 ===== */



    /* ===== 周期运行部分 ===== */

    // 40ms -- 25帧处理热成像图像数据
    PERIODIC_TASK(40, {
        IrTriggerOneFrame();
    });

    PERIODIC_TASK(1000, {
        printf("原始数据: ");
        for(uint32_t i = 0; i < 64; i++){
            printf("%02X ", ir_raw_buf[i]);
        }
        printf("\r\n");
    });

    /* ===== 事件驱动部分 ===== */

    if(ir_acquire_state == 2)
        LED_ON(RED);
    else
        LED_OFF(RED);

}

#endif
