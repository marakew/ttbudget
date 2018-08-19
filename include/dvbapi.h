
/*
 */

#ifndef _dvbapi_h_
#define _dvbapi_h_

#include <sys/ioccom.h>

struct dvbcard
{
	unsigned int	idSubDevice;
	unsigned char	MACAddress[6];
};

typedef enum dmastatus {
	HW_DMA_STOP = 0,
	HW_DMA_START
} dmastatus_t;

struct dmastate
{
	unsigned int	status;
	unsigned int	fullness;
};

typedef enum fe_polarity {
	SEC_POLARITY_NONE = 0,
	SEC_POLARITY_H,
	SEC_POLARITY_V,
	SEC_POLARITY_OFF
} fe_polarity_t;

typedef enum fe_tone {
	SEC_TONE_NONE = 0,
	SEC_TONE_OFF,
	SEC_TONE_ON
} fe_tone_t;

typedef enum fe_inner {
	FEC_NONE = 0,
	FEC_1_2,
	FEC_2_3,
	FEC_3_4,
	FEC_5_6,
	FEC_6_7,
	FEC_7_8,
	FEC_AUTO
} fe_inner_t;

struct tunes
{
	struct
	{
		unsigned int	no;
		fe_polarity_t	polar;
		unsigned int	freq;
		fe_tone_t	tone;
	} lnb;

	struct
	{
		unsigned int	freq;
		unsigned int	sr;
		fe_inner_t	inner;
	} tuner;
};

typedef enum fe_status {
	FE_SIGNAL	= 0x01,
	FE_CARRIER	= 0x02,
	FE_VITERBI	= 0x04,
	FE_SYNC		= 0x08,
	FE_LOCK		= 0x10,
	FE_TIMEOUT	= 0x20,
	FE_REINIT	= 0x40
} fe_status_t;

struct tuner
{
	fe_status_t	status;
	unsigned int	ber;
	unsigned char	signal;
	struct tunes	param;
};

struct dmxinfo
{
	unsigned int	TSPkts;
	unsigned int	BadTSPkts;
	unsigned int	DisCont;
};

struct dmxfilter
{
	unsigned short	pid;
	unsigned char	mac[6];
	unsigned char	mask[6];

	unsigned char	filterNo;
	u_int64_t	bytes;
	unsigned int	counts;
	unsigned int	errors;
};

typedef enum apicode {
	IOCARD	= 1,
	IODMA,
	IOTUNES,
	IOTUNER,

	IODEMUX,
	IOGETFILTER,
	IOADDFILTER,
	IODELFILTER

} apicode_t;

struct dvbapi
{
	apicode_t	code;

	union
	{
		struct	dvbcard		card;
		struct	dmastate	dma;
		struct	tunes		param;

		struct	tuner		state;

		struct	dmxinfo		demux;

		struct	dmxfilter	filter;
		unsigned char		filterNo;

	} u;
};

#define		DVBAPI		_IOWR('o', 61, struct dvbapi)

#endif

