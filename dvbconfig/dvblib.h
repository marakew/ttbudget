
/*
 */

#ifndef _dvblib_h_
#define _dvblib_h_

#define	MASK_UNICAST	1
#define MASK_PROMISC	2

class CDVBDevice
{
protected:
	static	int	handle;

public:
	CDVBDevice();
	~CDVBDevice();

	int	Open(int ifnum);
	int	CallAPI(struct dvbapi *parm);

	int	AddFilter(unsigned short pid, unsigned char *mac, int mfilter);
	int	DelFilter(unsigned char filterNo);
	int	GetFilter(unsigned char filterNo, unsigned short *pid, unsigned char *mac, u_int64_t *bytes, unsigned int *errors);

	int	GetDMA(unsigned int *status, unsigned int *fullness);
	int	GetCARD(unsigned int *idSubDevice, unsigned char *mac);
	int	GetDemux(unsigned int *TSPkts, unsigned int *BadTSPkts, unsigned int *DisCont);

	int	ReadTuner(struct tuner *state);
	int	WriteTuner();

};

#endif

