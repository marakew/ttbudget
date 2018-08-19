
/*
 */

extern "C"
{
	#include <sys/types.h>
	#include <sys/param.h>
#if __FreeBSD_version >= 700000
	#include <sys/systm.h>
#else
	#include "systm.h"
#endif
	//#include <sys/_lock.h>
	//#include <sys/_mutex.h>

}

#include "mutex.h"
#include "fifo.h"

CFifo::CFifo(unsigned int size)
{
	pBuffer = 0;//NULL;
	lSize = 0;

	ReadIndex = 0;
	WriteIndex = 0;

	Init(size);
}

CFifo::~CFifo()
{
	if (pBuffer != NULL)
	{
		delete []pBuffer;
		pBuffer = 0;//NULL;
	}
}

int
CFifo::Init(unsigned int size)
{

	unsigned char *buffer;

	buffer = new unsigned char[size];
	if (buffer == NULL)
	{
		return 0;
	}

	pBuffer = buffer;

	ReadIndex = 0;
	WriteIndex = 0;

	lSize = size;

	return 1;
}

int
CFifo::IsOK(void)
{
	if (pBuffer == NULL)
		return 0;

	return 1;
}

void
CFifo::Reset(void)
{
	ReadIndex = 0;
	WriteIndex = 0;
}

unsigned int
CFifo::Size(void)
{
	unsigned int resSize;

	resSize = 0;

	if (pBuffer == NULL)
	{
		return 0;
	}

	if (ReadIndex == WriteIndex)
	{
		return 0;
	}

	if (ReadIndex < WriteIndex)
	{
		resSize = WriteIndex - ReadIndex;
	} else
	{
		resSize = (lSize - ReadIndex) + WriteIndex;
	}

	return resSize;
}

unsigned int
CFifo::Put(unsigned char *buffer, unsigned int size)
{
	unsigned int resSize;

	unsigned int TotalSize;
	unsigned int FreeSize;
	unsigned int AvailSize;

	if (pBuffer == NULL)
	{
		return 0;
	}

	if (ReadIndex > WriteIndex)
	{
		AvailSize = (ReadIndex - WriteIndex) - 1;
		FreeSize = 0;
	} else
	{
		AvailSize = lSize - WriteIndex;
		if (ReadIndex == 0)
		{
			AvailSize --;
			FreeSize = 0;
		} else
		{
			FreeSize = ReadIndex - 1;
		}
	}

	TotalSize = AvailSize + FreeSize;

		resSize = size;
	if (TotalSize < size)
		resSize = TotalSize;

//
	unsigned int AvailWrite;

		AvailWrite = resSize;
	if (AvailSize < AvailWrite)
		AvailWrite = AvailSize;

	if (AvailWrite > 0)
	{
		memcpy(pBuffer + WriteIndex, buffer, AvailWrite);
		WriteIndex += AvailWrite;
		buffer += AvailWrite;
	}
//
	unsigned int FreeWrite;

		FreeWrite = resSize - AvailWrite;
	if (FreeSize < FreeWrite)
		FreeWrite = FreeSize;

	if (FreeWrite > 0)
	{
		memcpy(pBuffer, buffer, FreeWrite);
		WriteIndex = FreeWrite;
	}
//
	if (WriteIndex == lSize)
	{
		WriteIndex = 0;
	}

	return resSize;
}

unsigned int
CFifo::Get(unsigned char *buffer, unsigned int size)
{
	unsigned int resSize;

	unsigned int TotalSize;
	unsigned int FreeSize;
	unsigned int AvailSize;

	if (pBuffer == NULL)
	{
		return 0;
	}

	if (ReadIndex == WriteIndex)
	{
		return 0;
	}

	if (ReadIndex < WriteIndex)
	{
		AvailSize = WriteIndex - ReadIndex;
		FreeSize = 0;
	} else
	{
		AvailSize = lSize - ReadIndex;
		FreeSize = WriteIndex;
	}

	TotalSize = AvailSize + FreeSize;

		resSize = size;
	if (TotalSize < size)
		resSize = TotalSize;

//
	unsigned int AvailRead;

		AvailRead = resSize;
	if (AvailSize < AvailRead)
		AvailRead = AvailSize;

	if (AvailRead > 0)
	{
		memcpy(buffer, pBuffer + ReadIndex, AvailRead);
		ReadIndex += AvailRead;
		buffer += AvailRead;
	}
//
	unsigned int FreeRead;

		FreeRead = resSize - AvailRead;
	if (FreeSize < FreeRead)
		FreeRead = FreeSize;

	if (FreeRead > 0)
	{
		memcpy(buffer, pBuffer, FreeRead);
		ReadIndex = FreeRead;
	}
//
	if (ReadIndex == lSize)
	{
		ReadIndex = 0;
	}

	return resSize;
}

void
CFifo::Lock(void)
{
	//mutex.Lock();
}

void
CFifo::Unlock(void)
{
	//mutex.Unlock();
}

