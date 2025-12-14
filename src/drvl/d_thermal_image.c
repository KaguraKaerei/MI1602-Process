#include "d_thermal_image.h"

#include "usbh_cdc.h"
#include <stdio.h>

#include "d_led.h"

// ! ========================= 变 量 声 明 ========================= ! //

extern USBH_HandleTypeDef hUsbHostFS;

// 接收缓冲区，多留空间防溢出
uint8_t ir_raw_buf[FRAME_SIZE + 256];
// 解析后的热成像图像数据
uint16_t ir_frame[FRAME_HEIGHT][FRAME_WIDTH];

volatile uint32_t ir_received_bytes = 0;
// 0: 空闲  1: 等待发送完成  2: 正在接收一帧数据
volatile uint8_t ir_acquire_state = 0;

// 获取单帧数据触发命令
const uint8_t ir_trigger_cmd[] = "#000CWREGB10102DC";

// ! ========================= 私 有 函 数 声 明 ========================= ! //



// ! ========================= 接 口 函 数 实 现 ========================= ! //

/**
 * @brief 解析接收到的红外图像数据帧
 * @note 该函数将原始字节数据转换为二维像素数组
 */
void ParseIRFrame(void)
{
    // 跳过帧头
    const uint32_t header_offset = 32;
    const uint8_t* frame_data = ir_raw_buf + header_offset;

    if(ir_received_bytes < header_offset + FRAME_SIZE) return;
    
    for(uint32_t i = 0; i < FRAME_SIZE / 2; i++){
        uint32_t y = i / FRAME_WIDTH;
        uint32_t x = i % FRAME_WIDTH;

        uint16_t pixel = frame_data[i * 2] | (frame_data[i * 2 + 1] << 8);
        ir_frame[y][x] = pixel;
    }

    // 帧就绪后，可设置标志通知主任务（避免在此处 printf）
}

/**
 * @brief 触发获取一帧红外图像数据
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

// ! ========================= 私 有 函 数 实 现 ========================= ! //

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
        USBH_CDC_Receive(phost, ir_raw_buf, sizeof(ir_raw_buf));

        ir_acquire_state = 2;
    }
}

/* ===== __weak替换 ===== */
void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef* phost)
{
    uint32_t len = USBH_CDC_GetLastReceivedDataSize(phost);

    if(ir_acquire_state == 2 && len > 0){
        ir_received_bytes += len;

        if(ir_received_bytes >= FRAME_SIZE + 32){
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
