
/*
 */

#ifndef _fifodemux_h_
#define _fifodemux_h_

class CFifo;
class CSoftScheduler;
class CTSDemux;

class CFifoDemux
{
protected:
	CFifo		*tsfifo;
	CTSDemux	*tsdemux;

	unsigned char	*tsbuffer;
	CSoftScheduler	*sched;

	unsigned int	lFifoSmallSize;
	unsigned int	lFifoInvalidSize;
	unsigned int	lFifoDemuxSize;

public:
	CFifoDemux(CFifo *tsfifo, CTSDemux *tsdemux);
	~CFifoDemux();

	void	Dump(void);

	int	IsOK(void);

	static	void	DemuxFifo(CFifoDemux *fifodemux);

	int	DequeueFifo(void);
	void	Schedule(void);
};

#endif

