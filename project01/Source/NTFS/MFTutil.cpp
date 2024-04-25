#include "MFTutil.h"

void getFilename(BYTE* MFTRec, uint8_t* name)
{
	if (strncmp(reinterpret_cast<char*>(MFTRec), "FILE",4) == 0)
	{
		int length = bytesToDecimal(readInfo(0x00F0, 1, MFTRec));
		for (int i = 0; i < length; i++) {
			name[i] = MFTRec[0x00F2 + i * 2];
		}
		name[length] = '\0';
	}
}
