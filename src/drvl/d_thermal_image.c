#include "d_thermal_image.h"

#include "usbh_cdc.h"
#include <stdio.h>

#include "d_led.h"
#include "d_lcd.h"

// ! ========================= 变 量 声 明 ========================= ! //

extern USBH_HandleTypeDef hUsbHostFS;

// 接收缓冲区，多留空间防溢出
uint8_t ir_raw_buf[FRAME_SIZE + FRAME_HEADER_SIZE + 256];
// 解析后的热成像图像数据
uint16_t ir_frame[FRAME_HEIGHT][FRAME_WIDTH];

volatile uint32_t ir_received_bytes = 0;
// 0: 空闲  1: 等待发送完成  2: 正在接收一帧数据
volatile uint8_t ir_acquire_state = 0;
// 0: 正在处理数据 1: 可以开始绘制 
volatile uint8_t ir_draw_ready = 0;

// 获取单帧数据触发命令
const uint8_t ir_trigger_cmd[] = "#000CWREGB10102DC";

/**
 * @brief 伪彩色映射表
 */
static const uint8_t lut_r[256] = {
    255,253,251,249,247,245,243,241,239,237,235,233,231,229,227,225,223,221,219,217,215,213,211,209,207,205,203,201,199,197,195,193,191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,161,159,157,155,153,151,149,147,145,143,141,139,137,135,133,131,129,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,0,2,4,6,8,10,12,14,17,19,21,23,25,27,29,31,36,41,46,51,56,61,66,71,76,81,86,91,96,101,106,111,116,121,125,130,135,139,144,149,153,158,163,167,172,177,181,186,189,191,194,196,198,200,203,205,207,209,212,214,216,218,221,223,224,225,226,227,228,229,230,231,231,232,233,234,235,236,237,238,239,240,240,241,241,242,242,243,243,244,244,245,245,246,246,247,247,248,248,249,249,250,250,251,251,252,252,253,253,254,254,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};
static const uint8_t lut_g[256] = {
    255,253,251,249,247,245,243,241,239,237,235,233,231,229,227,225,223,221,219,217,215,213,211,209,207,205,203,201,199,197,195,193,191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,161,159,157,155,153,151,149,147,145,143,141,139,137,135,133,131,129,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,3,3,3,4,4,5,5,5,6,6,7,7,10,13,16,19,22,25,28,31,34,37,40,43,46,49,52,55,57,60,64,67,71,74,78,81,85,88,92,95,99,102,106,109,112,116,119,123,127,130,134,138,141,145,149,152,156,160,163,167,171,175,178,182,185,189,192,196,199,203,206,210,213,217,220,224,227,229,231,233,234,236,238,240,242,244,246,248,249,251,253,255
};
static const uint8_t lut_b[256] = {
    255,253,251,249,247,245,243,241,239,237,235,233,231,229,227,225,223,221,219,217,215,213,211,209,207,205,203,201,199,197,195,193,191,189,187,185,183,181,179,177,175,173,171,169,167,165,163,161,159,157,155,153,151,149,147,145,143,141,139,137,135,133,131,129,126,124,122,120,118,116,114,112,110,108,106,104,102,100,98,96,94,92,90,88,86,84,82,80,78,76,74,72,70,68,66,64,62,60,58,56,54,52,50,48,46,44,42,40,38,36,34,32,30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,9,16,24,31,38,45,53,60,67,74,82,89,96,103,111,118,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,136,137,137,137,138,138,138,139,139,139,140,140,140,141,141,137,132,127,121,116,111,106,101,95,90,85,80,75,69,64,59,49,47,44,42,39,37,34,32,29,27,24,22,19,17,14,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,14,15,16,18,19,20,21,22,23,24,25,27,28,29,30,39,53,67,81,95,109,123,137,151,165,179,193,207,221,235,249
};

// ! ========================= 私 有 函 数 声 明 ========================= ! //

static void ParseIRFrame(void);
static uint16_t LutToRGB565(uint8_t gray);

// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief 触发获取一帧热成像图像数据
 * @note 该函数会发送触发命令, 并在接收完成后调用 ParseIRFrame 进行数据解析
 */
void IrTriggerOneFrame(void)
{
    if(ir_acquire_state == 0 && hUsbHostFS.gState == HOST_CLASS){
        ir_acquire_state = 1;

        USBH_CDC_Transmit(&hUsbHostFS,
            (uint8_t*)ir_trigger_cmd,
            sizeof(ir_trigger_cmd) - 1);
    }
}

/**
 * @brief 绘制热成像图像到LCD屏幕
 */
void IrDrawFrameToLCD(void)
{
    const int LCD_W = lcddev.width;
    const int LCD_H = lcddev.height;
    const int scale = 2;
    const int x0 = (LCD_W - FRAME_WIDTH * scale) / 2;
    const int y0 = (LCD_H - FRAME_HEIGHT * scale) / 2;

    for(int j = 0; j < FRAME_HEIGHT; j++){
        for(int i = 0; i < FRAME_WIDTH; i++){
            uint16_t v = ir_frame[j][i];

            uint16_t c = LutToRGB565((v) & 0xFF);

            int x = x0 + i * scale;
            int y = y0 + j * scale;
            lcd_draw_point(x, y, c);
            // lcd_draw_point(x + 1, y, c);
            // lcd_draw_point(x, y + 1, c);
            // lcd_draw_point(x + 1, y + 1, c);
        }
    }
}

/**
 * @brief 将原始值转换为摄氏度温度
 * @param value 原始值
 * @return 摄氏度温度
 */
float IrValueToCelsius(uint16_t value)
{
    return ((float)value) * 0.1f;
}

// ! ========================= 私 有 函 数 实 现 ========================= ! //

/**
 * @brief 解析接收到的热成像图像数据帧
 * @note 该函数将原始字节数据转换为二维像素数组
 */
static void ParseIRFrame(void)
{
    // 跳过帧头
    const uint8_t* data_ptr = ir_raw_buf + FRAME_HEADER_SIZE;

    if(ir_received_bytes < FRAME_SIZE + FRAME_HEADER_SIZE) return;

    for(uint32_t i = 0; i < FRAME_SIZE / 2; i++){
        uint32_t y = i / FRAME_WIDTH;
        uint32_t x = i % FRAME_WIDTH;

        uint16_t pixel = (data_ptr[i * 2]);
        ir_frame[y][x] = pixel;
    }

    ir_draw_ready = 1;
}

/**
 * @brief 使用伪彩色映射将灰度值转换为RGB565颜色值
 * @param g 灰度值 (0-255)
 * @return RGB565颜色值
 */
static uint16_t LutToRGB565(uint8_t gray)
{
    uint8_t R = lut_r[gray];
    uint8_t G = lut_g[gray];
    uint8_t B = lut_b[gray];
    return (uint16_t)(((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3));
}

/* ===== __weak替换 ===== */
void USBH_CDC_InterfaceInit(USBH_HandleTypeDef* phost)
{
    ir_acquire_state = 0;
}

/* ===== __weak替换 ===== */
void USBH_CDC_TransmitCallback(USBH_HandleTypeDef* phost)
{
    if(ir_acquire_state == 1){
        ir_received_bytes = 0;

        // 启动接收（请求整个缓冲区）
        USBH_CDC_Receive(phost, ir_raw_buf, 64);

        ir_acquire_state = 2;
        ir_draw_ready = 0;
    }
}

/* ===== __weak替换 ===== */
void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef* phost)
{
    uint32_t len = USBH_CDC_GetLastReceivedDataSize(phost);

    if(ir_acquire_state == 2 && len > 0){
        ir_received_bytes += len;

        if(ir_received_bytes >= FRAME_SIZE + FRAME_HEADER_SIZE){
            ParseIRFrame();
            ir_acquire_state = 0;
        }
        else{
            // 继续接收剩余数据
            uint32_t request_len = 64;
            if(ir_received_bytes + request_len > sizeof(ir_raw_buf))
                request_len = sizeof(ir_raw_buf) - ir_received_bytes;
            USBH_CDC_Receive(phost, ir_raw_buf + ir_received_bytes, request_len);
        }
    }
}
