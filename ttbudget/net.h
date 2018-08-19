
/*
 */

#ifndef _net_h_
#define _net_h_

class CDvbAdapter;

class CNetDev
{
protected:
	CDvbAdapter		*dvb;
	struct ifdvb		dvbif;

	unsigned char	MACAddress[6];

public:
	CNetDev(CDvbAdapter *dvb, unsigned char *MACAddress);
	~CNetDev();

	static void	netStart(CNetDev *);
	static void	netStop(CNetDev *);

	void	NetCallback(unsigned char *ipmpe, int len);

};

#endif

