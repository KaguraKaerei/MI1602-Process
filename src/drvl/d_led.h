#ifndef _D_LED_H_
#define _D_LED_H_

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

typedef enum{
    RED = 0,
    GREEN = 1
} LED_Color_te;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void LED_ON(LED_Color_te color);
void LED_OFF(LED_Color_te color);

#endif
