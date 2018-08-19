
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
	pci->SetReg(0xFC, 0x8800000, 1);
}

void
CSpciDebi::Init(void)
{
	pci->SetReg(0xFC, 0x8800880, 1);
}

void
CSpciDebi::Setup(struct CSpciDebi::tDebiSetup *pSetup)
{
}

void
CSpciDebi::Out8(unsigned short, unsigned char, unsigned int)
{
}

void
CSpciDebi::Out16(unsigned short, unsigned short, unsigned int)
{
}

unsigned int
CSpciDebi::In8(unsigned short, unsigned int)
{
}

unsigned int
CSpciDebi::In16(unsigned short, unsigned int)
{
}

int
CSpciDebi::WaitDebiCompletion(unsigned short, unsigned int)
{
}
