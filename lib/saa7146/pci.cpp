
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
	#include <machine/resource.h>
	#include <sys/bus.h>

	#include "memio.h"
}

#include "adapter.h"
#include "pci.h"

CSpciIoIrq::CSpciIoIrq(struct resource *memory):memory(memory)
{
	Init();
}

CSpciIoIrq::~CSpciIoIrq()
{
	SetReg(IER, 0, 0);
}

void CSpciIoIrq::Init(void)
{
	isrnum = 0;
	spin = 0;
	irq = 0;

	isrTableSize = 0;
	bzero(&isrTableEntry[0], sizeof(isrTableEntry));

	SetReg(IER, 0, 0);
}

void CSpciIoIrq::DeviceReset(void)
{
	SetReg(MC1, 0x80000000, 1);
}

unsigned int CSpciIoIrq::GetUploadMask(int nReg)
{
	static unsigned int dwUpMaskTable[40] =
		{
		/* DMA 1 */
		0x40004,	/* 0 */
		0x40004,	/* 4 */
		0x40004,	/* 8 */
		0x40004,	/* C */
		0x40004,	/* 10 */
		0x40004,	/* 14 */

		/* DMA 2 */
		0x80008,	/* 18 */
		0x80008,	/* 1C */
		0x80008,	/* 20 */
		0x80008,	/* 24 */
		0x80008,	/* 28 */
		0x80008,	/* 2C */

		/* DMA 3 */
		0x100010,	/* 30 */
		0x100010,	/* 34 */
		0x100010,	/* 38 */
		0x100010,	/* 3C */
		0x100010,	/* 40 */
		0x100010,	/* 44 */

		0,		/* 48 */
		0,		/* 4C */

		/* DDI */
		0x6000600,	/* 50 */

		/* DDI Video Data ushort 2 port */
		0,		/* 54 UPLD_D1_A=200 UPLD_D1_B=400 */

		/* BSR */
		0x1000100,	/* 58 BSR Control Register */

		0x200020,	/* 5C */
		0x200020,	/* 60 */
		0x200020,	/* 64 */

		0x400040,	/* 68 */
		0x400040,	/* 6C */
		0x400040,	/* 70 */

		0x200020,	/* 74 */
		0x200020,	/* 78 */

		/* DEBI */
		0x20002,	/* 7C */
		0x20002,	/* 80 */
		0x20002,	/* 84 */
		0x20002,	/* 88 */

		/* I2C */
		0x10001,	/* 8C */
		0x10001,	/* 90 */
	
		};

	if (nReg <= 0x90)
	{
		switch (nReg)
		{
		/* DDI */
		case 0x3A:	return 0x2000200;	/* UPLD_D1_A 200 10bit */
		case 0x3C:	return 0x4000400;	/* UPLD_D1_B 400 9bit */

		/* FIFO */
		case 0x45:	return 0x20002;		/* DEBI 28-26 */
		case 0x46:	return 0x100010;	/* DMA 20-16 */
		case 0x47:	return 0x80008;		/* DMA 12-8 */
		case 0x48:	return 0x40004;		/* DMA 4-0 */

		default:	return dwUpMaskTable[nReg>>2];
		}
	}
	return 0;
}

void CSpciIoIrq::UploadRegs(unsigned int dwMask)
{
	SetReg(MC2, dwMask, 0);
}

unsigned int CSpciIoIrq::GetReg(int nReg)
{
	unsigned int value;

	value = 0;
	if (nReg >= 0x200)
	{
		/*
		TODO
		*/
		return 0;
	}

	if (nReg >= 0x48 && nReg <= 0x4B)
	{
		value = read_reg_uchar(memory, nReg) & 0xff;
	} else
	if (nReg == 0x54 || nReg == 0x56)
	{
		value = read_reg_ushort(memory, nReg) & 0xffff;
	} else
	{
		value = read_reg_ulong(memory, nReg);
	}

	return value;
}

void CSpciIoIrq::SetReg(int nReg, unsigned int dwValue, int bUpload)
{
	if (nReg >= 0x200)
	{
		/*
		TODO
		*/
		return;
	}

	if (nReg >= 0x48 && nReg <= 0x4B)
	{
		write_reg_uchar(memory, nReg, dwValue & 0xff);
	} else
	if (nReg == 0x54 || nReg == 0x56)
	{
		write_reg_ushort(memory, nReg, dwValue & 0xffff);
	} else
	{
		write_reg_ulong(memory, nReg, dwValue);
	}

	if (bUpload)
	{
		unsigned int dwMask;

		dwMask = GetUploadMask(nReg);
		if (dwMask)
			UploadRegs(dwMask);
	}
}

/* 22KHzLine */

void CSpciIoIrq::SetOut22KHzLine(int state)
{
	if (state == 0)
		SetGPIO(3, 0x40);
	else
		SetGPIO(3, 0x50);
}

int CSpciIoIrq::GetOut22HKzLine(void)
{
	return GetGPIO(3);
}

/*  ResetLine */

void CSpciIoIrq::SetResetLine(int state)
{
	if (state == 0)
		SetGPIO(2, 0x40);
	else
		SetGPIO(2, 0x50);
}

int CSpciIoIrq::GetResetLine(void)
{
	return GetGPIO(2);
}

/* GPIO */

void CSpciIoIrq::SetGPIO(int gpioPin, int gpioMode)
{
	unsigned int value;

	if (gpioPin > 3)
		return;

	value = GetReg(GPIO_CTRL);
	value &= ~(0xFF << (8 * gpioPin));
	value |= (gpioMode << (8 * gpioPin));
	SetReg(GPIO_CTRL, value, 1);
}

int CSpciIoIrq::GetGPIO(int gpioPin)
{
	if (gpioPin > 3)
		return 0;

	return (GetReg(PSR) >> (gpioPin+3) ) & 1;
}

/* Irq */

unsigned int CSpciIoIrq::EnableIrq(unsigned int irqtype)
{
	irq |= irqtype;
	SetReg(ISR, ~GetReg(IER) & irqtype, 1);
	SetReg(IER, GetReg(IER) | irq, 1);
	return irq;
}

unsigned int CSpciIoIrq::DisableIrq(unsigned int irqtype)
{
	unsigned int tmpirq;

	tmpirq = irq;
	irq = ~irqtype & irq;
	SetReg(ISR, tmpirq & irqtype, 1);
	SetReg(IER, (GetReg(IER) & ~tmpirq) | irq, 1);
	return tmpirq;
}

void CSpciIoIrq::PhysicalEnable(void)
{
	if (spin != 0)
		spin --;
#if 0
	if (spin == 0)
		__asm("sti");	
#endif
}

void CSpciIoIrq::PhysicalDisable(void)
{
#if 0
	if (spin == 0)
		__asm("cli");
#endif
	spin ++;
}

int CSpciIoIrq::RegisterIsr(unsigned int irqtype, void *data, unsigned int isrtype, unsigned short *isrSize, struct ISRTable *isrTable)
{
	struct ISRTable *tmpTable;

	if (*isrSize >= 32)
		return 0;

	tmpTable = &isrTable[*isrSize];

	tmpTable->isrtype = isrtype;
	tmpTable->isrnum = ++isrnum;
	tmpTable->irqtype = irqtype;
	tmpTable->data = data;

	PhysicalDisable();

	*isrSize += 1;

	EnableIrq(irqtype);

	PhysicalEnable();

	return isrnum;
}

int CSpciIoIrq::RegisterRing0Isr(unsigned int irqtype, void *data, unsigned int isrtype)
{
	if (irqtype == 0 || (isrtype != 1 && isrtype != 2))
		return 0;

	return RegisterIsr(irqtype, data, isrtype, &isrTableSize, &isrTableEntry[0]);
}

int CSpciIoIrq::ScanIsrTable(unsigned int isrnum, unsigned short *isrSize, struct ISRTable *isrTable)
{
	struct ISRTable *tmpTable, *lastTable;
	unsigned int irqtype;
	int maxSize;
	int i;
	int found;

	irqtype = 0;
	found = 0;
	maxSize = *isrSize;	
	i= 0;

	lastTable = &isrTable[maxSize];

	for (i = 0; i < maxSize; i++)
	{
		tmpTable = &isrTable[i];

		if (tmpTable->isrnum == isrnum)
		{
			if (found == 0)
				PhysicalDisable();

			found = 1;

			irqtype |= tmpTable->irqtype;

			if (maxSize > 1)
			{
				tmpTable->isrtype = lastTable->isrtype;
				tmpTable->isrnum = lastTable->isrnum;
				tmpTable->irqtype = lastTable->irqtype;
				tmpTable->data = lastTable->data;
			}

			*isrSize -= 1;

			lastTable->isrtype = 0;
			lastTable->isrnum = 0;
			lastTable->irqtype = 0;
			lastTable->data = NULL;

			continue;
		}
		irqtype &= ~tmpTable->irqtype;
	}

	if (found == 1)
	{
		DisableIrq(irqtype);
		PhysicalEnable();
	}
	return found;
}

int CSpciIoIrq::UnregisterIsr(unsigned int isrnum)
{
	return ScanIsrTable(isrnum, &isrTableSize, &isrTableEntry[0]);
}


/* ----- */

unsigned int CSpciIoIrq::CallRing0Isrs(unsigned int irqtype)
{
	struct ISRTable	*isrTable;
	unsigned int irqmask;
	int i;
	
	irqmask = 0;

	for (i = 0; i < isrTableSize; i++)
	{
		isrTable = &isrTableEntry[i];

		if (isrTable->irqtype != irqtype)
			continue;

		if (isrTable->isrtype == 1)
		{
			/*
			TODO
			*/
			irqmask |= isrTable->irqtype;
		}

		if (isrTable->isrtype == 2)
		{
			if (isrTable->data != NULL)
			{
				//CAdapter	*sc;
				//sc = (CAdapter *)isrTable->data;
				//sc->HWIsrCallback(irqtype, isrTable->isrnum);
				static_cast<CAdapter *>
					(isrTable->data)->HWIsrCallback(irqtype, isrTable->isrnum);
			}
			irqmask |= isrTable->irqtype;
		}
	}

	return irqmask;
}

int CSpciIoIrq::OnHardwareInt(void)
{
	unsigned int irqtype;
	unsigned int irqmask;

	irqtype = GetReg(ISR);
	irqmask = CallRing0Isrs(irqtype);

	if (irqmask != 0)
	{
		SetReg(ISR, irqtype & irqmask, 1);
		return 1;
	}
	return 0;
}

