
/*
 */

extern "C"
{
#include <sys/types.h>
#include <sys/param.h>
#if __FreeBSD_version >= 700000
#include <sys/systm.h>
#else
#include "systm.h"
#endif
#include <sys/bus.h>

#include "dvbapi.h"
}

#include "adapter.h"
#include "ttadapter.h"
#include "tuner.h"
#include "stb0899.h"

CTunerSTB0899::CTunerSTB0899(CDvbAdapter *dvb):CTuner(dvb)
{
	CheckLNBP21();
}

CTunerSTB0899::~CTunerSTB0899()
{
}

int
CTunerSTB0899::STI2C_Write(int slave ,unsigned char *buf, int len)
{
	unsigned char tmp[64];
	int res;
	int i;

	if (len > 0)
		memcpy(&tmp[0],  buf, len);

	for (i = 0; i <= 3; i++)
	{
		res = dvb->I2CWriteSeq(slave, &tmp[0], len);
		if (res == 0)
			break;

		Sleep(10);
	}

	if (i > 3)
		return 4;

	return 0;
}

int
CTunerSTB0899::STI2C_Read(int slave, unsigned char *buf, int len)
{
	unsigned char tmp[64];
	int res;
	int i;

	buf[0] = 0;

	for (i = 0; i <= 3; i++)
	{
		res = dvb->I2CReadSeq(slave, &tmp[0], len);
		if (res == 0)
			break;

		Sleep(10);
	}

	if (i > 3)
		return 4;

	if (len > 0)
		memcpy(buf, &tmp[0], len);

	return 0;
}

int
CTunerSTB0899::STI2C_Combined(int dir, int slave, unsigned char *buf, int len)
{
	int res;

	if (len > 0x39)
		return 3;

	if (dir == 0)
		res = STI2C_Read(slave, buf, len);

	if (dir == 1)
		res = STI2C_Write(slave, buf, len);

	return res;
}

int
CTunerSTB0899::CheckLNBP21()
{
	int error;
	unsigned char val;
	int res;

	val = 0x58;
	error = STI2C_Write(0x10, &val, 1);

	val = 0;
	error = STI2C_Read(0x10, &val, 1);

	if (error == 0 && (val & 0xFC == 0x58))
		res = 1;
	else
		res = 0;

	printf("SYS: HAL: LNBP21 = %s\n", res ? "TRUE" : "FALSE");

	return res;
}

