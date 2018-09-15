
/*
 */

extern "C"
{
//	#include <sys/param.h>
//	#include <sys/malloc.h>
}

#include "pci.h"
#include "debi.h"


CSpciDebi::CSpciDebi(CSpciIoIrq *pci, struct CSpciDebi::tDebiSetup *pdebi):pci(pci)
{
	Init();

	if (pdebi == 0)
	{
		struct tDebiSetup debi;

		Setup(&debi);
	} else
	{
		Setup(pdebi);
	}
}

CSpciDebi::~CSpciDebi()
{
	pci->SetReg(MC1, 0x8800000, 1);
}

void CSpciDebi::Init(void)
{
	pci->SetReg(MC1, 0x8800880, 1);
}

void CSpciDebi::Setup(struct CSpciDebi::tDebiSetup *pSetup)
{
}

void CSpciDebi::Out8(unsigned short wSlaveAddress, unsigned char cData, unsigned int dwWaitFlags)
{
}

void CSpciDebi::Out16(unsigned short wSlaveAddress, unsigned short wData, unsigned int dwWaitFlags)
{
}

unsigned int CSpciDebi::In8(unsigned short wSlaveAddress, unsigned int dwWaitFlags)
{
}

unsigned int CSpciDebi::In16(unsigned short wSlaveAddress, unsigned int dwWaitFlags)
{
}

int CSpciDebi::WaitDebiCompletion(unsigned short wLength, unsigned int dwWaitFlags)
{
}
