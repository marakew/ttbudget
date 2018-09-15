
/*
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "dvbapi.h"
#include "dvblib.h"

int CDVBDevice::handle = -1;

CDVBDevice::CDVBDevice()
{
}

CDVBDevice::~CDVBDevice()
{
	if (handle != -1)
		close(handle);
}

int CDVBDevice::Open(int ifnum)
{
	char name[256];

	sprintf(name, "/dev/dvb%d", ifnum);

	handle = open(name, O_RDWR);
	if (handle < 0)
	{
		printf("error open %s\n", name);
		return 0;
	}

	return 1;
}

int CDVBDevice::CallAPI(struct dvbapi *parm)
{
	return ioctl(handle, DVBAPI, parm);
}

int CDVBDevice::AddFilter(unsigned short pid, unsigned char *mac, int mfilter)
{
	int res;
	struct dvbapi	parm;

	unsigned char	mask_unicast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	unsigned char	mask_promisc[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	memset(&parm, 0, sizeof(parm));

	parm.code = IOADDFILTER;
	parm.u.filter.pid = pid;

	memcpy(&parm.u.filter.mac[0], &mac[0], sizeof(parm.u.filter.mac));

	if (mfilter == MASK_UNICAST)
	{
		memcpy(&parm.u.filter.mask[0], &mask_unicast[0],
					sizeof(mask_unicast));
	} else
	if (mfilter == MASK_PROMISC)
	{
		memcpy(&parm.u.filter.mask[0], &mask_promisc[0],
					sizeof(mask_promisc));
	} else
		return -2;

	res = CallAPI(&parm);

	return res;
}

int CDVBDevice::DelFilter(unsigned char filterNo)
{
	int res;
	struct dvbapi	parm;

	memset(&parm, 0, sizeof(parm));

	parm.code = IODELFILTER;
	parm.u.filterNo = filterNo;

	res = CallAPI(&parm);

	return res;
}

int CDVBDevice::GetFilter(unsigned char filterNo, unsigned short *pid, unsigned char *mac, u_int64_t *bytes, unsigned int *errors)
{
	int res;
	struct dvbapi	parm;

	memset(&parm, 0, sizeof(parm));

	parm.code = IOGETFILTER;
	parm.u.filter.filterNo = filterNo;

	res = CallAPI(&parm);
	if (res != -1)
	{
		if (parm.u.filter.pid >= 0 && parm.u.filter.pid <= 0x1fff)
		{
			memcpy(mac, &parm.u.filter.mac[0],
					sizeof(parm.u.filter.mac));
			*pid = parm.u.filter.pid;
			*bytes = parm.u.filter.bytes;
			*errors = parm.u.filter.errors;
			return 0;
		}
		return -1;
	}
	return res;
}

int CDVBDevice::GetDMA(unsigned int *status, unsigned int *fullness)
{
	int res;
	struct dvbapi	parm;

	memset(&parm, 0, sizeof(parm));

	parm.code = IODMA;
	res = CallAPI(&parm);

	*status = parm.u.dma.status;
	*fullness = parm.u.dma.fullness;

	return res;
}

int CDVBDevice::GetCARD(unsigned int *idSubDevice, unsigned char *mac)
{
	int res;
	struct dvbapi	parm;

	memset(&parm, 0, sizeof(parm));

	parm.code = IOCARD;
	res = CallAPI(&parm);

	memcpy(mac, &parm.u.card.MACAddress[0],
			sizeof(parm.u.card.MACAddress));
	*idSubDevice = parm.u.card.idSubDevice;

	return res;
}

int CDVBDevice::GetDemux(unsigned int *TSPkts, unsigned int *BadTSPkts, unsigned int *DisCont)
{
	int res;
	struct dvbapi	parm;

	memset(&parm, 0, sizeof(parm));

	parm.code = IODEMUX;
	res = CallAPI(&parm);

	*TSPkts = parm.u.demux.TSPkts;
	*BadTSPkts = parm.u.demux.BadTSPkts;
	*DisCont = parm.u.demux.DisCont;

	return res;
}

int CDVBDevice::ReadTuner(struct tuner *state)
{
	int res;
	struct dvbapi	parm;

	memset(&parm, 0, sizeof(parm));

	parm.code = IOTUNER;
	res = CallAPI(&parm);

	memcpy(state, &parm.u.state, sizeof(parm.u.state));

	return res;
}

int CDVBDevice::WriteTuner()
{
	int res;
	struct dvbapi	parm;

	memset(&parm, 0, sizeof(parm));

	res = CallAPI(&parm);

	return res;
}
