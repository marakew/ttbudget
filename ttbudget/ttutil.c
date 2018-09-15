
/*
 */

#include "ttutil.h"

short TTMAC_CRC(unsigned char *mac, short len)
{
	short tmp;
	short crc;
	int i;

	tmp = 0xffff;

	for (i = 0; i < len-2; i++)
	{
		tmp = ( (tmp <<8) &0xff00 | (tmp >>8) &0xff) ^ mac[i];
		tmp ^= (tmp & 0xff) >> 4;

		tmp ^= ( (tmp <<8) &0xff00 | ((tmp & 0xff) >>8)) << 4;
		tmp ^= (tmp & 0xff) << 5;
	}


	crc = ~(mac[len-2]<<8 | mac[len-1]);

	mac[len-1] = ~tmp & 0xff;
	mac[len-2] = ~tmp >> 8;

	if ( (tmp^crc) == 0)
		return 0;

	return -1;
}

short TTMAC_DECRYPT(unsigned int *macOID, unsigned int *macL3B, unsigned short *encMAC)
{
	unsigned short xored[10] = { 0x2372, 0x1968, 0xA85C, 0x2C71, 0xD354,
				   0xF17B, 0x239E, 0xF616, 0x361D, 0x7864 };

	unsigned char decMAC[10+2];
	short tmp[10];
	int i;
	int crcOK;

	*macOID = 0;
	*macL3B = 0;

	for (i = 0; i < 10; i++)
	{
		tmp[i] = encMAC[i] ^ xored[i];
		decMAC[i] = (tmp[i] >> ((tmp[i] >> 14)&3)) & 0xff;
	}

	crcOK = TTMAC_CRC(&decMAC[0], 10);

	if (crcOK != -1)
	{
		*macOID = *(unsigned int *)&decMAC[0];
		*macL3B = *(unsigned int *)&decMAC[4];
	}

	for (i = 0; i < 10; i++)
	{
		tmp[i] = 0;
		decMAC[i] = 0;
		xored[i] = 0;
	}

	return crcOK;
}
