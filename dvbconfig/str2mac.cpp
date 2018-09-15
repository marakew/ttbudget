
/*
 */

#include <string.h>
#include <stdlib.h>

int str2mac(unsigned char *pMac, char *szMac)
{
	if (strlen(szMac) < 12 || strlen(szMac) > 17)
	{
		return 0;
	}

	//[0 1]  [2 3] [4 5] [6 7] [8 9] [10 11]

	if (strlen(szMac) == 12)
	{
		pMac[5] = strtoul(szMac+10, NULL, 16);
		szMac[10] = 0;

		pMac[4] = strtoul(szMac+8, NULL, 16);
		szMac[8] = 0;
	
		pMac[3] = strtoul(szMac+6, NULL, 16);
		szMac[6] = 0;

		pMac[2] = strtoul(szMac+4, NULL, 16);
		szMac[4] = 0;

		pMac[1] = strtoul(szMac+2, NULL, 16);
		szMac[2] = 0;

		pMac[0] = strtoul(szMac, NULL, 16);
		szMac[0] = 0;
	} else
	//[0 1] 2 [3 4] 5 [6 7] 8 [9 10] 11 [12 13] 14 [15 16]
	if (strlen(szMac) == 17)
	{
		pMac[5] = strtoul(szMac+15, NULL, 16);
		szMac[14] = 0;

		pMac[4] = strtoul(szMac+12, NULL, 16);
		szMac[11] = 0;
	
		pMac[3] = strtoul(szMac+9, NULL, 16);
		szMac[8] = 0;

		pMac[2] = strtoul(szMac+6, NULL, 16);
		szMac[5] = 0;

		pMac[1] = strtoul(szMac+3, NULL, 16);
		szMac[2] = 0;

		pMac[0] = strtoul(szMac, NULL, 16);
		szMac[0] = 0;
	}

	return 1;
}

