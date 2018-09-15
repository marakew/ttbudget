
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
#include <sys/malloc.h>

#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/resource.h>
#include <sys/rman.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

#include <sys/bus.h>

#include <sys/selinfo.h>

//#include <sys/_lock.h>
//#include <sys/_mutex.h>

#include "dvbapi.h"

#include "devio.h"
#include "dvbif.h"
#include "util.h"
#include "ttutil.h"
}

#include "pci.h"
#include "i2c.h"
#include "debi.h"
#include "net.h"
#include "tsdemux.h"
#include "fifodemux.h"
#include "tuner.h"
#include "mutex.h"
#include "fifo.h"

#include "tda10086.h"
#include "pn1010.h"
#include "stb0899.h"
#include "su1278.h"

#include "adapter.h"
#include "ttadapter.h"

#include "ttbudget.h"

CDvbAdapter::CDvbAdapter(device_t dev):device(dev)
{
	idVendor = 0;
	idDevice = 0;
	idSubVendor = 0;
	idSubDevice= 0;

	pci = 0;
	i2c = 0;
	debi = 0;

	net = 0;
	tsdemux = 0;

	tuner = 0;
	fifodemux = 0;
	tsfifo = 0;

	DMAStatus = 0;

	CiISR = 0;
	IrISR = 0;
	DataISR = 0;

	HWIRQ = 0;
	MyIRQ = 0;
	DPC = 0;
	DPCReq = 0;
	IRQCb = 0;
	EvenFields = 0;
	OddFields = 0;
	
	TSABCb = 0;

	MyHW = 0;

	FifoFull = 0;

	NoSecCSum = 0;
	BadSecCSum = 0;
	BadSecCRC = 0;
	NotMPE = 0;
	Scrmbl = 0;
	NotIP = 0;
	SecBadL = 0;
	FltBadL = 0;

}

CDvbAdapter::~CDvbAdapter()
{
	HWDisableDataDMA();
	HWDeInitialize(11);
}

void CDvbAdapter::Dump(void)
{
	printf("HWAdapter:\n");
	printf("HWIRQ %lu\n", HWIRQ);
	printf("MyIRQ %lu\n", MyIRQ);
	printf("DPC %lu\n", DPC);
	printf("DPCReq %lu\n", DPCReq);
	printf("IRQCb %lu\n", IRQCb);
	printf("EvenFields %lu\n", EvenFields);
	printf("OddFields %lu\n", OddFields);
	printf("TSABCb %lu\n", TSABCb);
	printf("FifoFull %lu\n", FifoFull);
	printf("\n");
	printf("NoSecCSum %lu\n", NoSecCSum);
	printf("BadSecCSum %lu\n", BadSecCSum);
	printf("BadSecCRC %lu\n", BadSecCRC);
	printf("NotMPE %lu\n", NotMPE);
	printf("Scrmbl %lu\n", Scrmbl);
	printf("NotIP %lu\n", NotIP);
	printf("SecBadL %lu\n", SecBadL);
	printf("FltBadL %lu\n", FltBadL);

	fifodemux->Dump();
}

void CDvbAdapter::SetGPIO(int gpioPin, int gpioMode)
{
	pci->SetGPIO(gpioPin, gpioMode);
}

int CDvbAdapter::GetGPIO(int gpioPin)
{
	return pci->GetGPIO(gpioPin);
}

int CDvbAdapter::I2CReadSeq(int slave, unsigned char *readBuf, int readLen)
{
	int *readbuf;
	int res;
	int i;

	i2c->SetSlave(slave >> 1);

	readbuf = new int [readLen * 4];
	if (readbuf == NULL)
	{
		//"Warning: I2CReadSeq(slave=0x%02x) failed. Resource could not be allocated!\n"
		return 1;
	}

	res = i2c->ReadSeq(&readbuf[0], readLen);
	if (res != 0)
	{
		 printf("Warning: I2CReadSeq(slave=0x%02x) failed (%d)!\n", slave, res);
	}

	for (i = 0; i < readLen; i++)
	{
		readBuf[i] = readbuf[i];
	}

	delete []readbuf;

	return res;
}

int CDvbAdapter::I2CWriteSeq(int slave, unsigned char *writeBuf, int writeLen)
{
	int *writebuf;
	int res;
	int i;

	i2c->SetSlave(slave >> 1);

	writebuf = new int [writeLen * 4];
	if (writebuf == NULL)
	{
		//"Warning: I2CWriteSeq(slave=0x%02x) failed. Resource could not be allocated!"
		return 1;
	}

	for (i = 0; i < writeLen; i++)
	{
		writebuf[i] = writeBuf[i];
	}

	res = i2c->WriteSeq(&writebuf[0], writeLen);
	if (res != 0)
	{
		 printf("Warning: I2CWriteSeq(slave=0x%02x) failed (%d)!\n", slave, res);
	}

	delete []writebuf;

	return res;
}

int CDvbAdapter::I2CCombinedSeq(int slave,
	unsigned char *writeBuf, int writeLen, unsigned char *readBuf, int readLen)
{
	int *writebuf;
	int *readbuf;
	int res;
	int i;

	i2c->SetSlave(slave >> 1);

	if (writeLen != 0)
	{
		writebuf = new int [writeLen * 4];
		if (writebuf == NULL)
		{
			//"Warning: I2CCombinedSeq(slave=0x%02x) failed. Resource could not be allocated!\n",
			return 1;
		}
	}

	if (readLen != 0)
	{
		readbuf = new int [readLen * 4];
		if (readbuf == NULL)
		{
			//"Warning: I2CCombinedSeq(slave=0x%02x) failed. Resource could not be allocated!\n",
			if (writebuf != NULL)
				delete []writebuf;
			return 1;
		}
	}

	for (i = 0; i < writeLen; i++)
	{
		writebuf[i] = writeBuf[i];
	}

	res = i2c->CombinedSeq(&writebuf[0], writeLen, &readbuf[0], readLen);

	if (res != 0)
	{
		printf("Warning: I2CCombinedSeq(slave=0x%02x) failed (%d)!\n", slave, res);
	}

	for (i = 0; i < readLen; i++)
	{
		readBuf[i] = readbuf[i];
	}

	if (writebuf != NULL)
		delete []writebuf;

	if (readbuf != NULL)
		delete []readbuf;

	return res;
}

int CDvbAdapter::CreateTuner(int subdevice)
{
	int res;

	res = 0;

	switch (subdevice)
	{
	case 0x100F:	/* SU1278 */
			tuner = new CTunerSU1278(this);
			break;

	case 0x100C:
	case 0x1016:	/* PN1010 aka S5H1420 Frontend ?, PLL TDA8260 */
			tuner = new CTunerPN1010(this);
			break;

	case 0x1018:	/* Frontend TDA10086, PLL TDA8263 */
			tuner = new CTunerTDA10086(this);
			break;

	case 0x1019:	/* Frontend STB0899, PLL STB6100 */
			tuner = new CTunerSTB0899(this);
			break;
	default:
			printf("Warning: unknown tuner!\n");
	}

	if (tuner != NULL && tuner->IsThisTunerInstalled() == 0)
	{
		delete tuner;
		tuner = 0;//NULL;
		res = 0;
	}

	if (tuner == NULL)
	{
		tuner = new CTuner(this);
		res = 0;
		printf("Error: Invalid Tuner!\n");
	}

	res = tuner->Initialize();

	return res;
}

void CDvbAdapter::TSCallback(unsigned char filterType, unsigned char filterNo, unsigned char *data, unsigned int len, unsigned char flag)
{
	unsigned short seclen;
	unsigned short datalen;

	if (filterType == MPE_SECTION_FILTER ||
	    filterType == MULTI_MPE_FILTER)
	{
		seclen = sec_length(data);

		if (data[5] & 2)
		{
			if (data[18] == 8 && data[19] == 0)
				datalen = (data[22] << 8) + data[23] + 8;
			else
				datalen = seclen - 16;
		} else
		{
			datalen = (data[14] << 8) + data[15];
		}

		if (data[0] != 0x3E)
		{
			++ NotMPE;
		}

		if ( !(data[5] & 0x3C) )
		{
			if ( (data[1] & 0x80) && !SectionCrcCheck(data, seclen))
			{
				++ BadSecCRC;
				goto endcheck;
			}

			if ( !(data[1] & 0x80) )
			{
				if ( *(unsigned int *)&data[seclen - 4] == 0)
				{
					++ NoSecCSum;
					goto endcheck;
				}

				if ( !SectionCSumCheck(data, seclen))
				{
					++ BadSecCSum;
					goto endcheck;
				}
			} else
			{
				datalen += 16;
				if (seclen < datalen)
				{
					++ SecBadL;
					goto endcheck;
				}

				if (seclen > len || datalen > len)
				{
					++ FltBadL;
					goto endcheck;
				}
			}

		} else
		{
			++ Scrmbl;

			if (data[5] & 2 && (data[18] != 8 || data[19] != 0))
			{
				++ NotIP;
			}
		}
	}

endcheck:
#if 0
	if (filterType != MPE_SECTION_FILTER && filterType != MULTI_MPE_FILTER)
	{
		len -= 28;
	} else
	{
		len -= 4;	//?checksum
	}
#endif
	/*
	TODO
	*/

	net->NetCallback(&data[0], len);
}

void CDvbAdapter::HWIsrCallback(unsigned int irqtype, unsigned int isrnum)
{
	int res;
	unsigned char *buffer;

	buffer = dma.vbuffer;

	if (CiISR == isrnum && irqtype == 8)	//GPIN0 CiEvent
	{

	}

	if (IrISR == isrnum && irqtype == 0x40)	//GPIN3 IrEvent
	{

	}

	if (DataISR == isrnum && irqtype == 0x80)	//
	{
		if ( !(pci->GetReg(0x110) & 0x80) )
		{
			OddFields ++;

//			tsdemux->DemuxTS(&buffer[0], 512);
		} else
		{
			EvenFields ++;

			buffer += (512 * 188);

//			tsdemux->DemuxTS(&buffer[0], 512);
		}
#if 0
		tsdemux->DemuxTS(&buffer[0], 512);
#else
		tsfifo->Lock();
		res = tsfifo->Put(&buffer[0], 188*512);
		tsfifo->Unlock();

		if (res == 0 || res != 188*512)
		{
			tsfifo->Reset();
			FifoFull ++;
		}

		fifodemux->Schedule();
#endif
		TSABCb ++;
	}

	IRQCb ++;
}

int CDvbAdapter::HWEnableDataDMA(void)
{
	DMAStatus = 1;

	HWIRQ = 0;
	MyIRQ = 0;
	DPC = 0;
	DPCReq = 0;
	IRQCb = 0;
	EvenFields = 0;
	OddFields = 0;
	
	TSABCb = 0;

	FifoFull = 0;
//
	NoSecCSum = 0;
	BadSecCSum = 0;
	BadSecCRC = 0;
	NotMPE = 0;
	Scrmbl = 0;
	NotIP = 0;
	SecBadL = 0;
	FltBadL = 0;
//
	tsfifo->Reset();
	tsdemux->ResetStat();

	tsdemux->EnablePIDCount(1);

	DataISR = pci->RegisterRing0Isr(0x80, this, 2);

	pci->SetReg(MC1, 0x4100410, 1);

	printf("start DMA\n");
	return 0;
}

int CDvbAdapter::HWDisableDataDMA(void)
{
	pci->SetReg(MC1, 0x100000, 1);

	pci->UnregisterIsr(DataISR);
	DataISR = 0;
//
	tsfifo->Reset();

	tsdemux->EnablePIDCount(0);

	DMAStatus = 0;

	printf("stop DMA\n");
	return 0;
}

int CDvbAdapter::HWReadMAC(unsigned int *macOID, unsigned int *macL3B)
{
	int res;
	unsigned char subaddr;
	unsigned char readMAC[20];

	*macOID = 0;
	*macL3B = 0;

	subaddr = 0xCC;
	res = I2CCombinedSeq(0xA0, &subaddr, 1, &readMAC[0], 20);
	if (res != 0)
	{
		printf("Error: Failed to read MAC from EEPROM!\n");
		return 0;
	}

	res = TTMAC_DECRYPT(macOID, macL3B, (unsigned short *)&readMAC[0]);
	if (res != 0)
	{
		printf("Error: Failed to decrypt MAC!\n");
		return 0;
	}

	//printf(" -> MAC=0x%06X%06X\n", *macOID, *macL3B);
	return 1;
}

int CDvbAdapter::HWInstallInterrupt( void (*ISRHandler)(void *) )
{
	int rid;
	int err;

	err = 0;
	rid = 0;

	rirq = bus_alloc_resource(device, SYS_RES_IRQ, &rid, 0, ~0, 1, RF_SHAREABLE | RF_ACTIVE);
	if (rirq == NULL)
	{
		return 1;
	}

	err = bus_setup_intr(device, rirq, INTR_TYPE_AV | INTR_MPSAFE, 
#if (__FreeBSD_version >= 700000)
					0,
#endif
					ISRHandler, this, &rih);
	if (err)
	{
		bus_release_resource(device, SYS_RES_IRQ, 0, rirq);
		return 1;
	}

	return 0;
}

void CDvbAdapter::HWRemoveInterrupt(void)
{

	if (rih)
	{
		bus_teardown_intr(device, rirq, rih);
	}

	if (rirq)
	{
		bus_release_resource(device, SYS_RES_IRQ, 0, rirq);
	}
}

void CDvbAdapter::HWInterruptService(CDvbAdapter  *dvb)
{
	CSpciIoIrq	*pci;

	pci = dvb->pci;

	dvb->HWIRQ++;

	if (pci != 0)
	{
		dvb->MyHW = pci->OnHardwareInt();
	} else
	{
		dvb->MyHW = 0;
	}

	if (dvb->MyHW != 0)
	{
		dvb->MyIRQ++;
		//.
		//dvb->DPCReq++;
	}
}

int CDvbAdapter::HWSetup7146IoSpace(void)
{
	int rid;

	rid = PCIR_BAR(0);

	rmem = bus_alloc_resource(device, SYS_RES_MEMORY, &rid, 0, ~0, 1, RF_ACTIVE);
	if (rmem == NULL)
	{
		return 1;
	}
	return 0;
}

void CDvbAdapter::HWRemove7146IoSpace(void)
{
	int rid;

	rid = PCIR_BAR(0);

	if (rmem != NULL)
	{
		bus_release_resource(device, SYS_RES_MEMORY, rid, rmem);
	}
}

int CDvbAdapter::HWSetupSharedMemory(void)
{
	dma.size = (188 * 512) * 2;
	dma.pbuffer = 0;
	dma.vbuffer = (unsigned char *)contigmalloc(dma.size,
				M_DEVBUF, M_NOWAIT, 0, ~0, PAGE_SIZE, 0);

	if (dma.vbuffer == NULL)
	{
		dma.size = 0;
		return 1;
	}

	dma.pbuffer = vtophys(dma.vbuffer);

	return 0;
}

void CDvbAdapter::HWRemoveSharedMemory(void)
{
	if (dma.vbuffer != NULL)
	{
		contigfree(dma.vbuffer, dma.size, M_DEVBUF);
		dma.vbuffer = (unsigned char *)NULL;
		dma.pbuffer = 0;
		dma.size = 0;

	}
}

void CDvbAdapter::HWInit7146Register(int ci)
{
	unsigned int pbuffer;

	pbuffer = dma.pbuffer;

	pci->SetReg(MC1, 0x100000, 1);

//	pci->SetGPIO(2, 0x40);
//	DELAY(100);
//	pci->SetGPIO(2, 0x50);

	pci->SetReg(BASE_ODD3, pbuffer, 1);
	pci->SetReg(BASE_EVEN3, pbuffer+(188*512), 1);
	pci->SetReg(PROT_ADDR3, pbuffer+(188*512)*2, 1);
	pci->SetReg(PITCH3, 188, 1);
	pci->SetReg(BASE_PAGE3, 0, 1);
	pci->SetReg(NUM_LINE_BYTE3, (512 << 16) | 188, 1);

	pci->SetReg(DD1_INIT, 0x6C006C0, 1);
	pci->SetReg(DD1_STREAM_A, 0, 1);
	pci->SetReg(DD1_STREAM_B, 0, 1);

	pci->SetReg(BRS_CTRL, 0x60000000, 1);

	pci->SetReg(MC2, 0x7100710, 1);
}

int CDvbAdapter::HWDeInitialize(int level)
{
	switch(level)
	{
	case 11: {
			if (tuner != NULL)
			{
				delete tuner;
				tuner = 0;//NULL;
			}
		}

	case 10: {
			if (fifodemux != NULL)
			{
				delete fifodemux;
				fifodemux = 0;//NULL
			}
		}

	case 9: {
			if (tsfifo != NULL)
			{
				delete tsfifo;
				tsfifo = 0;//NULL;
			}
		}

	case 8:	{
			if (tsdemux != NULL)
			{
				delete tsdemux;
				tsdemux = 0;//NULL;
			}
		}

	case 7:	{
			if (net != NULL)
			{
				delete net;
				net = 0;//NULL;
			}
		}

	case 6:	{
			HWRemoveSharedMemory();
		}

	case 5:	{
			HWRemoveInterrupt();
		}

	case 4:	{
			if (debi != NULL)
			{
				delete debi;
				debi = 0;//NULL;
			}
		}

	case 3:	{
			if (i2c != NULL)
			{
				delete i2c;
				i2c = 0;//NULL;
			}
		}

	case 2:	{
			if (pci != NULL)
			{
				delete pci;
				pci = 0;//NULL;
			}
		}

	case 1:	{
			HWRemove7146IoSpace();
		}

	default:;
	}

	return ENXIO;
}

int CDvbAdapter::HWInitialize(int subdevice)
{
	int res;

	idSubDevice = subdevice;

	res = HWSetup7146IoSpace();
	if (res != 0)
	{
		return 1;
	}

	pci = new CSpciIoIrq(rmem);
	if (pci == NULL)
	{
		HWDeInitialize(1);
		return 2;
	}

	pci->DeviceReset();
	pci->SetResetLine(0);

	i2c = new CSpciI2C(pci);
	if (i2c == NULL)
	{
		HWDeInitialize(2);
		return 3;
	}

	debi = new CSpciDebi(pci, 0);
	if (debi == NULL)
	{
		HWDeInitialize(3);
		return 4;
	}

	res = HWInstallInterrupt(SETISRFN(&CDvbAdapter::HWInterruptService));
	if (res != 0)
	{
		HWDeInitialize(4);
		return 5;
	}

	res = HWSetupSharedMemory();
	if (res != 0)
	{
		HWDeInitialize(5);
		return 6;

	}

	HWInit7146Register(0);


	if ( HWReadMAC(&MACAddressOID, &MACAddressL3B) )
	{
		MACAddress[2] = MACAddressOID & 0xff;
		MACAddress[0] = (MACAddressOID >> 16) & 0xff;
		MACAddress[1] = (MACAddressOID >> 8) & 0xff;

		MACAddress[5] = MACAddressL3B & 0xff;
		MACAddress[3] = (MACAddressL3B >> 16) & 0xff;
		MACAddress[4] = (MACAddressL3B >> 8) & 0xff;

		printf("MAC address : %6D\n", MACAddress, ":");
	}


	net = new CNetDev(this, MACAddress);
	if (net == NULL)
	{
		HWDeInitialize(6);
		return 7;
	}

	tsdemux = new CTSDemux(this);
	if (tsdemux == NULL)
	{
		HWDeInitialize(7);
		return 8;
	}

	tsfifo = new CFifo(5000000);
	if (tsfifo == NULL || tsfifo->IsOK() == 0)
	{
		HWDeInitialize(8);
		return 9;
	}

	fifodemux = new CFifoDemux(tsfifo, tsdemux);
	if (fifodemux == NULL || fifodemux->IsOK() == 0)
	{
		HWDeInitialize(9);
		return 10;
	}

	pci->SetResetLine(0);
	DELAY(20*5);
	pci->SetResetLine(1);
	DELAY(20*5);

	res = CreateTuner(idSubDevice);
	if (res != 0)
	{
		HWDeInitialize(10);
		return 11;
	}

	return 0;
}

int CDvbAdapter::HWDvbControl(CDvbAdapter *dvb, struct dvbapi *parm)
{
	CTuner	*tuner = dvb->tuner;
	CTSDemux *tsdemux = dvb->tsdemux;
	CFifo *tsfifo = dvb->tsfifo;
	int res;

	res = 0;

	switch(parm->code)
	{
	case	IOCARD: {

			bcopy(&dvb->MACAddress[0], &parm->u.card.MACAddress[0], 6);

			} break;

	case	IODMA: {

			parm->u.dma.status = dvb->DMAStatus;

			tsfifo->Lock();
			parm->u.dma.fullness = (tsfifo->Size() % 5000000) / 100000;
			tsfifo->Unlock();

			dvb->Dump();

			} break;

	case	IOTUNES: {

			tuner->SetTunes(&parm->u.param);

			} break;

	case	IOTUNER: {

			tuner->GetTuner(&parm->u.state);

			} break;

	case	IODEMUX: {

			parm->u.demux.TSPkts = tsdemux->GetTSPkts();
			parm->u.demux.BadTSPkts = tsdemux->GetBadTSPkts();
			parm->u.demux.DisCont = tsdemux->GetDisCont();

			} break;

	case	IOGETFILTER: {

			unsigned char	filterNo;

			filterNo = parm->u.filter.filterNo;

			parm->u.filter.pid = tsdemux->GetPid(filterNo);
			parm->u.filter.bytes = tsdemux->GetBytes(filterNo);
			parm->u.filter.errors = tsdemux->GetErrors(filterNo);

			} break;

	case	IOADDFILTER: {

			unsigned char	filterNo;

			tsdemux->AddFilter(filterNo,
					MPE_SECTION_FILTER,
					parm->u.filter.pid,
					&parm->u.filter.mac[0],
					&parm->u.filter.mask[0],
					188*3);

			//TODO
			} break;

	case	IODELFILTER: {

			unsigned char	filterNo;

			filterNo = parm->u.filterNo;

			tsdemux->DeleteFilter(filterNo);

			//TODO
			} break;
	default:
		res = 1;
	}

	return res;
}
