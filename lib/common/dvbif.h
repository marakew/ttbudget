
/*
 */

#ifndef _dvbif_h_
#define _dvbif_h_

struct ifdvb
{
	struct	ifnet	*ifp;

	void	*if_softc;
	void	(*if_stop)(void *if_softc);
	void	(*if_start)(void *if_softc);
};

typedef	void (*netFunction)(void *);
#define	NETFUNC(x)	reinterpret_cast<netFunction>(x)

void
dvb_ifattach(struct ifdvb *dvbp, unsigned char *eaddr);

void
dvb_ifdetach(struct ifdvb *dvbp);

void
dvb_input(struct ifdvb *dvbp, unsigned char *ipmpe, int len);

#endif

