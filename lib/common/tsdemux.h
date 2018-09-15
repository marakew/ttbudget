
/*
 */

#ifndef _tsdemux_h_
#define _tsdemux_h_

class CAdapter;

enum FILTERTYPE
{
	NO_FILTER_TYPE = 0,
	STREAMING_FILTER = 1,
	PIPING_FILTER,
	PES_FILTER,
	ES_FILTER,
	SECTION_FILTER,
	MPE_SECTION_FILTER,
	PID_FILTER,
	MULTI_PID_FILTER,
	TS_FILTER,
	MULTI_MPE_FILTER
};

typedef struct PIDCOUNT	
{
	unsigned int	Counts[8192];
	unsigned int	Errors[8192];
	unsigned char	CC[8192];
} PIDCOUNT;

typedef struct MACFILTER
{
	unsigned char	filterNo;
	unsigned char	mac[16];
	unsigned char	mask[16];

	MACFILTER	*prev;
	MACFILTER	*next;
} MACFILTER;

typedef struct CHANNEL
{
	unsigned int	ftype;
	unsigned int	pid;

	union
	{
		unsigned int	no;
		MACFILTER	*mac;
	} filter;

	unsigned int	state;

	unsigned char	cc;
	unsigned char	flag;
	unsigned short	seclen;

	unsigned int	len;
	unsigned char	*basebuf;
	unsigned int	baselen;
	unsigned char	*buf;
	unsigned char	*unkn24;

	unsigned int	bitfilter[8];

	unsigned char	unkn48;
	unsigned char	unkn49;

	unsigned short	unkn4A;

} CHANNEL;

typedef struct FILTER
{
	unsigned char	chanNo;
	unsigned char	scrambl;
	unsigned short	pid;
	MACFILTER	*macfilter;
	u_int64_t	bytes;

} FILTER;

enum TSErrReturn
{
	TSOK = 0,	//ERROR0
	TSEINVAL,	//ERROR1,
	ERROR2,
	ERROR3,
	ERROR4,
	TSNOMEM,	//ERROR5,
	ERROR6,
	ERROR7,
	ERROR8,
	ERROR9
};

inline unsigned short
ts_pid(const unsigned char *buf)
{
        return ((buf[1] << 8) + buf[2]) & 0x1fff;
}

inline unsigned short
sec_length(const unsigned char *buf)
{
        return (3 + ((buf[1] & 0xf) << 8) + buf[2]);
}

inline unsigned short
pes_length(const unsigned char *buf)
{
	return (6 + ((buf[4] << 8) | buf[5]));
}

#define FILTERS 256
#define CHANNELS 256

class CTSDemux
{
protected:
	unsigned int		TSPkts;
	unsigned int		BadTSPkts;
	unsigned int		TSErrors;
	unsigned int		DisCont;

	PIDCOUNT		*Pids;

	FILTER			Filters[256];
	CHANNEL			Channels[256];
	unsigned char		Pid2Chan[8192];

	/* payload unit section indicator */
	unsigned int		PUSI;
	/* transport priority */
	unsigned int		TP;
	/* scrambler */
	unsigned char		SC;
	/* adaptation field control */
	unsigned char		AFC;
	/* continue counter */
	unsigned short		CC;
	/* discontinue indicator */
	unsigned int		Error;

	unsigned char		Payload;

	CAdapter		*sc;

public:
	CTSDemux(CAdapter *sc);
	~CTSDemux();


	void		ResetStat(void);

	unsigned int	GetTSPkts(void);
	unsigned int	GetBadTSPkts(void);
	unsigned int	GetTSErrors(void);
	unsigned int	GetDisCont(void);

	unsigned char	GetFreeFilter(void);
	unsigned char	GetFreeChannel(void);

	unsigned int	GetScrambleFlags(unsigned char filterNo);

	unsigned short	GetPid(unsigned char filterNo);
	u_int64_t	GetBytes(unsigned char filterNo);

	unsigned int	GetErrors(unsigned char filterNo);
	unsigned int	GetCounts(unsigned char filterNo);

	enum	TSErrReturn	EnablePIDCount(int enable);
	enum	TSErrReturn	AddFilter(unsigned char &fNo, enum FILTERTYPE filterType, unsigned short pid, unsigned char *mac, unsigned char *mask, unsigned int size);

	enum	TSErrReturn	DeleteFilter(unsigned char filterNo);

	void	DelAllFilters(void);
	void	DeleteChannel(unsigned char chanNo);
	void	DeleteFilterStruct(unsigned char filterNo);

	unsigned char	GetNoOfFilters(void);

	void	DemuxTS(unsigned char *data, unsigned short len);
	void	ParsTSPack(unsigned char *data, unsigned char chanNo);

	/* PID */
	void	ParsPID(unsigned char chanNo, unsigned char *data, unsigned short len);

	/* PES */
	void	ParsPES(unsigned char chanNo, unsigned char *data, unsigned short len);
	void	FlushPESPack(unsigned char chanNo);

	/* ES */
	void	ParsES(unsigned char chanNo, unsigned char *data, unsigned short len);
	void	FlushESPack(unsigned char chanNo);

	/* Sec */
	void	ParsSec(unsigned char chanNo, unsigned char *data, unsigned short len);
	unsigned char	CompareSecFilter(unsigned char chanNo, unsigned char *data);
	void	FlushSecPack(unsigned char chanNo);

	/* Pipe */
	void	ParsPipe(unsigned char chanNo, unsigned char *data, unsigned short len);

	int	NotifyPack(unsigned char chanNo, unsigned char *data, unsigned int len, unsigned char flag);
};

#endif

