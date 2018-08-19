
/*
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>

#include <sys/socket.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <sys/sockio.h>
#include <net/if_types.h>

#include "dvbif.h"

static int if_num = 0;

void
net_start(struct ifnet *ifp)
{
	struct ifdvb	*dvbp;
	dvbp = (struct ifdvb *)ifp->if_softc;

	ifp->if_flags &= ~(IFF_DRV_RUNNING | IFF_DRV_OACTIVE);
	
	if (dvbp->if_start != NULL && dvbp->if_softc != NULL)
		dvbp->if_start(dvbp->if_softc);


	ifp->if_mtu = 4096;

	ifp->if_flags |= IFF_DRV_RUNNING;
	ifp->if_flags &= ~IFF_DRV_OACTIVE;
}

void
net_stop(struct ifnet *ifp)
{
	struct ifdvb	*dvbp;
	dvbp = (struct ifdvb *)ifp->if_softc;

	ifp->if_flags &= ~(IFF_DRV_RUNNING | IFF_DRV_OACTIVE);

	if (dvbp->if_stop != NULL && dvbp->if_softc != NULL)
		dvbp->if_stop(dvbp->if_softc);

}

int
if_ioctl(struct ifnet *ifp, unsigned int cmd, caddr_t arg)
{
	int s, err;

	switch(cmd)
	{

	case SIOCSIFMTU:
		ifp->if_mtu = 4096;
		err = 0;
		break;

	case SIOCSIFADDR:
	case SIOCGIFADDR:
		err = ether_ioctl(ifp, cmd, arg);
		break;

	case SIOCSIFFLAGS:

		if (ifp->if_flags & IFF_UP)
		{
			if ((ifp->if_flags & IFF_DRV_RUNNING) == 0)
				net_start(ifp);
		} else {
			if (ifp->if_flags & IFF_DRV_RUNNING)
				net_stop(ifp);
		}
		err = 0;
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:

	case SIOCGIFMEDIA:
	case SIOCSIFMEDIA:
	default:
		err = EINVAL;
	}
	return err;
}

void
if_watchdog(struct ifnet *ifp)
{

}

void
if_start(struct ifnet *ifp)
{

}

int
if_output(struct ifnet *ifp,
		struct mbuf *m,
		struct sockaddr *sa,
		struct rtentry *rt)
{
	m_freem(m);
	return EIO;
}

void
if_init(void *arg)
{
	struct ifdvb *dvbp;
	struct ifnet *ifp;

	dvbp = (struct ifdvb *)arg;
	ifp = dvbp->ifp;

	if ((ifp->if_flags & IFF_DRV_RUNNING) == 0)
		net_start(ifp);
}

void
dvb_ifattach(struct ifdvb *dvbp, unsigned char *eaddr)
{
	struct ifnet *ifp;

	ifp = if_alloc(IFT_ETHER);
	if (ifp == NULL)
	{
		dvbp->ifp = (struct ifnet *)NULL;
		return;
	}

	dvbp->ifp = ifp;

	ifp->if_softc = dvbp;	//

	if_initname(ifp, "dvb", if_num ++);

	ifp->if_mtu = 4096;
	ifp->if_flags = IFF_SIMPLEX;
	ifp->if_ioctl = if_ioctl;
	ifp->if_output = if_output;
	ifp->if_start = if_start;
	ifp->if_watchdog = if_watchdog;
	ifp->if_init = if_init;
	ifp->if_baudrate = 40000000;

	ether_ifattach(ifp, eaddr);
}

void
dvb_ifdetach(struct ifdvb *dvbp)
{
	struct ifnet *ifp;

	ifp = dvbp->ifp;

	if (ifp != NULL)
	{	
		ether_ifdetach(ifp);
		if_free(ifp);
	}
}

void
dvb_input(struct ifdvb *dvbp, unsigned char *ipmpe, int len)
{
	struct ifnet *ifp;
	struct ether_header eh;
	u_int8_t *eth;
	u_int8_t *ethf;
	struct mbuf *m;	
	int snap = 0;

	ifp = dvbp->ifp;
	if (ifp == NULL)
	{
		return;
	}

	eth = (u_int8_t *)&eh;

	/* note: pkt_len includes a 32bit checksum */
	if (len < 16)
	{
		printf("IP/MPE packet length = %d too small\n", len);
		ifp->if_ierrors++;
		return;
	}

	/* it seems some ISPs manage to screw up here, so we have to
	   * relax the error checks... */
#if 0
	if ((ipmpe[5] & 0xfd) != 0xc1)
	{
		/* drop scrambled or broken packets */
#else
	if ((ipmpe[5] & 0x3c) != 0x00)
	{
		/* drop scrambled */
#endif
		ifp->if_ierrors++;
		return;
	}

	if (ipmpe[5] & 0x02)
	{
		/* FIXME: handle LLC/SNAP, see rfc-1042 */
		if (len < 24 || memcmp(&ipmpe[12], "\xaa\xaa\x03\0\0\0", 6))
		{
			ifp->if_iqdrops++;
			return;
		}

		snap += 8;
	}

	if (ipmpe[7])
	{
		/* FIXME: assemble datagram from multiple sections */
		ifp->if_ierrors++;
		return;
	}

	/* we have
	 * +14 byte ethernet header (ip header follows);
	 * -12 byte MPE header;
	 *  -4 byte checksum; +2 byte alignment
	 *  -8 byte LLC/SNA
	 */

	/* ether_dhost[6] */
	eth[0]  = ipmpe[11];
	eth[1]  = ipmpe[10];
	eth[2]  = ipmpe[9];
	eth[3]  = ipmpe[8];
	eth[4]  = ipmpe[4];
	eth[5]  = ipmpe[3];

	/* ether_shost[6] */
	eth[6]  = 0;
	eth[7]  = 0;
	eth[8]  = 0;
	eth[9]  = 0;
	eth[10] = 0;
	eth[11] = 0;

	if (snap)
	{
		eth[12] = ipmpe[18];
		eth[13] = ipmpe[19];
	} else
	{
		/* protocol numbers are from rfc-1700 or
		 * http://www.iana.org/assignments/ethernet-numbers
		 */
		if (ipmpe[12] >> 4 == 6) /* version field from IP header */
		{
			eth[12] = 0x86; /* IPv6 */
			eth[13] = 0xdd;
		} else
		{
			eth[12] = 0x08; /* IPv4 */
			eth[13] = 0x00;
		}
	}

	if (ifp->if_flags & IFF_DEBUG)
	{
		printf("MAC: %6D -> %6D : type %u of %u bytes\n",
			eh.ether_shost, ":", eh.ether_dhost, ":",
			eh.ether_type, len -12 -4);

		if (eh.ether_type == ntohs(ETHERTYPE_IP))
		{
			printf("IP: %u.%u.%u.%u -> %u.%u.%u.%u\n",
			ipmpe[12+12], ipmpe[12+13], ipmpe[12+14], ipmpe[12+15],
			ipmpe[12+16], ipmpe[12+17], ipmpe[12+18], ipmpe[12+19]);
		}
	}

	/* skip filled Ethernet Header eth +14
	 * skip MPE header pkt +12
	 * total_len = -12(MPE) -4(crc) -(snap)
	 */

	MGETHDR(m, M_DONTWAIT, MT_DATA);
	if (m == NULL)
	{
		return;
	}

	m->m_pkthdr.rcvif = ifp;
	m->m_pkthdr.len = m->m_len = (len - 12) +14;

	if (((len-12) + 14 + 2) > MHLEN)
	{
		MCLGET(m, M_DONTWAIT);
		if ((m->m_flags & M_EXT) == 0)
		{
			m_freem(m);
			return;
		}
	}

	m->m_data += 2;
	ethf = (u_int8_t *)mtod(m, struct ether_header *);
	bcopy(eth, ethf, 14);
	bcopy(ipmpe +12 + snap, ethf +14, len-12-snap);

	if (m == NULL)
	{
		ifp->if_ierrors++;
		return;
	}

	ifp->if_ipackets++;

	ether_input(ifp, m);

	return;
}

