#ifndef _ADF4351_H_
#define _ADF4351_H_

#include "sys.h"

#define ADF4351_CLK PBout(15)
#define ADF4351_OUTPUT_DATA PBout(14)
#define ADF4351_LE PBout(13)
#define ADF4351_CE PBout(12)

#define ADF4351_INPUT_DATA PBin(14)


void ADF4351Init(void);
void ReadToADF4351(u8 count, u8 *buf);
void WriteToADF4351(u8 count, u8 *buf); 
void ADF4351WriteFreq(u16 freq);		
#endif

