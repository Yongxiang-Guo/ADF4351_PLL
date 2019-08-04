#include "Freq_control.h"

int main(void)
{		  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	Freq_control_init();
	while(1)
	{
		mode_select();
	}
}

