
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
#include "pn1010.h"


CTunerPN1010::CTunerPN1010(CDvbAdapter *dvb):CTuner(dvb)
{
	m_dwGPIO = dvb->GetGPIO(8);

	m_dwAddr = 0xA6;	//DemodAddr
}

CTunerPN1010::~CTunerPN1010()
{
}

int CTunerPN1010::SetSlave(int slave)
{
	dvb->SetGPIO(3, m_dwGPIO == 0 ? 0x30 : 0);

	return slave;
}

int CTunerPN1010::WriteSeq(int slave, unsigned char *buf, int len)
{
	int i;
	int res;

	SetSlave(slave);

	for (i = 0; i < 3; i++)
	{
		res = dvb->I2CWriteSeq(slave, buf, len);
		if (res == 0)
			break;

		Sleep(1);
	}

	return res;
}

int CTunerPN1010::ReadSeq(int slave, unsigned char *buf, int len)
{
	int i;
	int res;

	SetSlave(slave);

	for (i = 0; i < 3; i++)
	{
		res = dvb->I2CReadSeq(slave, buf, len);
		if (res == 0)
			break;

		Sleep(1);
	}

	return res;
}

int CTunerPN1010::ReadI2CReg1010(int slave, unsigned char reg, unsigned char *val)
{
	int res;
	unsigned char tmp;

	*val = 0;

	tmp = reg;

	res = WriteSeq(slave, &tmp, 1);
	if (res != 0)
		return res;

	res = ReadSeq(slave, &tmp, 1);
	if (res != 0)
		return res;

	*val = tmp;
	return 0;
}

int CTunerPN1010::WriteI2CReg1010(int slave, unsigned char reg, unsigned char val)
{
	int res;

	unsigned char tmp[2];

	tmp[0] = reg;
	tmp[1] = val;

	res = WriteSeq(slave, &tmp[0], 2);

	return res;
}

int CTunerPN1010::ReadI2CReg(unsigned char reg)
{
	unsigned char val;

	val = reg;

	WriteSeq(m_dwAddr, &val, 1);
	ReadSeq(m_dwAddr, &val, 1);

	return val;
}

void CTunerPN1010::WriteI2CReg(unsigned char reg, unsigned char val)
{
	unsigned char tmp[2];

	tmp[0] = reg;
	tmp[1] = val;

	WriteSeq(m_dwAddr, &tmp[0], 2);
}

