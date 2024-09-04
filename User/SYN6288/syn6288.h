#ifndef __SYN6288_H
#define __SYN6288_H

#include "sys.h"


void SYN_FrameInfo(uint8_t Music, uint8_t *HZdata);
void YS_SYN_Set(uint8_t *Info_data);
void open_say(void);
void error_say(void);
void alarm_say(void);
#endif

