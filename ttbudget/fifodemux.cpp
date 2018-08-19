
/*
 */

extern "C"
{
	#include <sys/param.h>
#if __FreeBSD_version >= 700000
	#include <sys/systm.h>
#else
	#include "systm.h"
#endif
	//#include <sys/_lock.h>
	//#include <sys/_mutex.h>

}

#include "tsdemux.h"
#include "mutex.h"
#include "fifo.h"
#include "sched.h"
#include "fifodemux.h"

CFifoDemux::CFifoDemux(CFifo *tsfifo, CTSDemux *tsdemux):
		tsfifo(tsfifo), tsdemux(tsdemux)
{
	lFifoSmallSize = 0;
	lFifoInvalidSize = 0;
	lFifoDemuxSize = 0;

	tsbuffer = new unsigned char[188*512+10];
	if (tsbuffer == NULL)
	{
		/* Error !! */
	}

	sched = new CSoftScheduler(this, SETSCHEDFN(&CFifoDemux::DemuxFifo));
	if (sched == NULL)
	{
		/* Error !! */
	}
}

CFifoDemux::~CFifoDemux()
{
	if (sched != NULL)
	{
		delete sched;
		sched = 0;
	}

	if (tsbuffer != NULL)
	{
		delete []tsbuffer;

		tsbuffer = 0;
	}
}

void
CFifoDemux::Dump(void)
{
	printf("FifoDemux\n");
	printf("lFifoSmallSize: %lu\n", lFifoSmallSize);
	printf("lFifoInvalidSize: %lu\n", lFifoInvalidSize);
	printf("lFifoDemuxSize: %lu\n", lFifoDemuxSize);

	lFifoSmallSize = 0;
	lFifoInvalidSize = 0;
	lFifoDemuxSize = 0;
}

int
CFifoDemux::IsOK(void)
{
	if (tsfifo == NULL || tsdemux == NULL)
		return 0;

	if (sched == NULL || tsbuffer == NULL)
		return 0;

	return 1;
}

void
CFifoDemux::DemuxFifo(CFifoDemux *fifodemux)
{
	int res;

	res = fifodemux->DequeueFifo();

	if (res == 1)
	{

	} else
	if (res == 2)
	{
		fifodemux->lFifoSmallSize ++;
	} else
	if (res == 3)
	{
		fifodemux->lFifoInvalidSize ++;
	} else
	if (res == 0)
	{
		fifodemux->lFifoDemuxSize += 188*512;
	}
}

int
CFifoDemux::DequeueFifo(void)
{
	unsigned int size;
	unsigned int getsize;

	tsfifo->Lock();
	size = tsfifo->Size();
	tsfifo->Unlock();

	if (size < 188*512)
	{
		return 2;
	}

	size = 188*512;

	tsfifo->Lock();
	getsize = tsfifo->Get(&tsbuffer[0], size);
	tsfifo->Unlock();

	if (size != getsize)
	{
		return 3;
	}

	size = 512;

	tsdemux->DemuxTS(&tsbuffer[0], 512);

	return 0;
}

void
CFifoDemux::Schedule(void)
{
	if (sched != NULL)
	{
		sched->Schedule();
	}
}

