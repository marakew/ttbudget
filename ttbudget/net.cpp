
/*
 */

extern "C"
{
#include <sys/types.h>
#include <sys/param.h>
#include <sys/bus.h>

#include "dvbif.h"
}

#include "adapter.h"
#include "ttadapter.h"
#include "net.h"

CNetDev::CNetDev(CDvbAdapter *dvb, unsigned char *MACAddress):dvb(dvb)
{
	dvbif.if_softc = this;
	dvbif.if_start = NETFUNC(&CNetDev::netStart);
	dvbif.if_stop = NETFUNC(&CNetDev::netStop);

	dvb_ifattach(&dvbif, MACAddress);
}

CNetDev::~CNetDev()
{
	dvb_ifdetach(&dvbif);
}

void
CNetDev::netStart(CNetDev *net)
{
	(net->dvb)->HWEnableDataDMA();
}

void
CNetDev::netStop(CNetDev *net)
{
	(net->dvb)->HWDisableDataDMA();
}

void
CNetDev::NetCallback(unsigned char *ipmpe, int len)
{
	dvb_input(&dvbif, ipmpe, len);
}

