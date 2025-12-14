#ifndef _D_THERMAL_IMAGE_H_
#define _D_THERMAL_IMAGE_H_

#include <stdint.h>

// ! ========================= 接 口 变 量 / Typedef 声 明 ========================= ! //

#define FRAME_WIDTH   160
#define FRAME_HEIGHT  120
// 160 * 120 * 2 字节
#define FRAME_SIZE    (FRAME_WIDTH * FRAME_HEIGHT * 2)

extern uint16_t ir_frame[FRAME_HEIGHT][FRAME_WIDTH];
extern uint8_t ir_raw_buf[FRAME_SIZE + 256];
extern volatile uint8_t ir_acquire_state;
extern volatile uint32_t ir_received_bytes;

// ! ========================= 接 口 函 数 声 明 ========================= ! //

void ParseIRFrame(void);
void IrTriggerOneFrame(void);

#endif
