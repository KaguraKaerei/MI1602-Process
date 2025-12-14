#include "d_led.h"

#include "main.h"

// ! ========================= 变 量 声 明 ========================= ! //



// ! ========================= 私 有 函 数 声 明 ========================= ! //



// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief  点亮指定LED
 * @param  led: LED编号 (0或1)
 */
void LED_ON(LED_Color_te color)
{
    if(color == RED) HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_RESET);
    else if(color == GREEN) HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  熄灭指定LED
 * @param  led: LED编号 (0或1)
 */
void LED_OFF(LED_Color_te color)
{
    if(color == RED) HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
    else if(color == GREEN) HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //


