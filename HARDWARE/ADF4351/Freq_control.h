#ifndef _FREQ_CONTROL_H_
#define _FREQ_CONTROL_H_

#include "sys.h"

void Freq_control_init(void);			//PLL输出频率控制系统初始化
void mode_select(void);						//控制模式选择
void auto_scan_mode(void);				//模式一：自动扫描模式
void self_scan_mode(void);				//模式二：手动扫描模式
void single_freq_mode(void);		//模式三：设置点频


#endif