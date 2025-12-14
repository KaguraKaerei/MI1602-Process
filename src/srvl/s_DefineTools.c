#include "s_DefineTools.h"

bool _DT_Delay_Check_Timeout(uint32_t* start_timer, uint32_t ms)
{
    if(*start_timer == 0) *start_timer = HAL_GetTick();
    if(HAL_GetTick() - *start_timer < ms) return false;
    return true;
}
