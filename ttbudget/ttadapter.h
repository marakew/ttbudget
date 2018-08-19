
/*
 */

#ifndef _ttadapter_h_
#define _ttadapter_h_

class CSpciIoIrq;
class CSpciI2C;
class CSpciDebi;

class CNetDev;
class CTSDemux;

class CTuner;
class CFifo;
class CFifoDemux;

struct dmaspace
{
        unsigned char   *vbuffer;
        unsigned int   pbuffer;
        unsigned int   size;
};

typedef void    (*ISRFN)(void *);
#define SETISRFN(x)     reinterpret_cast<ISRFN>(x)

class CDvbAdapter : public CAdapter
{
protected:
	device_t        device;

	unsigned int   idVendor;       //0x1131        Philips
	unsigned int   idDevice;       //0x7146        SAA7146

	unsigned int   idSubVendor;	//+176	//0x13C2        TechoTrend
	unsigned int   idSubDevice;    //......

//bus io
	struct resource *rmem;
//bus irq
	struct resource *rirq;
	void		*rih;

	struct dmaspace	dma;

	CFifoDemux	*fifodemux;

	CSpciIoIrq	*pci;
	CSpciI2C	*i2c;
	CSpciDebi	*debi;

	CNetDev		*net;
	CTSDemux	*tsdemux;

	CTuner		*tuner;

	CFifo		*tsfifo;

	unsigned int	DMAStatus;

	unsigned int   CiISR;
	unsigned int   IrISR;
	unsigned int   DataISR;

	unsigned int	NoSecCSum;
	unsigned int	BadSecCSum;
	unsigned int	BadSecCRC;
	unsigned int	NotMPE;	
	unsigned int	Scrmbl;
	unsigned int	NotIP;
	unsigned int	SecBadL;
	unsigned int	FltBadL;
	

	unsigned int	HWIRQ;
	unsigned int	MyIRQ;
	unsigned int	DPC;
	unsigned int	DPCReq;
	unsigned int   IRQCb;
	unsigned int   EvenFields;
	unsigned int   OddFields;

	unsigned int	TSABCb;	

	unsigned int	MyHW;

	unsigned int	FifoFull;

	unsigned int   MACAddressOID;
	unsigned int   MACAddressL3B;

	unsigned char   MACAddress[6];

public:
	CDvbAdapter(device_t dev);
	~CDvbAdapter();

	void	Dump(void);

	void	SetGPIO(int gpioPin, int gpioMode);
	int	GetGPIO(int gpioPin);


	int	I2CReadSeq(int slave, unsigned char *readBuf, int readLen);
	int	I2CWriteSeq(int slave, unsigned char *writeBuf, int writeLen);
	int	I2CCombinedSeq(int slave,
        			unsigned char *writeBuf, int writeLen, unsigned char *readBuf, int readLen);

	int	CreateTuner(int subdevice);

	void	TSCallback(unsigned char filterType, unsigned char filterNo, unsigned char *data, unsigned int len, unsigned char flag);

	void	HWIsrCallback(unsigned int irqtype, unsigned int isrnum);
	int	HWEnableDataDMA(void);
	int	HWDisableDataDMA(void);
	int	HWReadMAC(unsigned int *macOID, unsigned int *macL3B);

	int	HWInstallInterrupt( void (*ISRHandler)(void *) );
	void	HWRemoveInterrupt(void);

	static void	HWInterruptService(CDvbAdapter *dvb);

	int	HWSetup7146IoSpace(void);
	void	HWRemove7146IoSpace(void);

	int	HWSetupSharedMemory(void);
	void	HWRemoveSharedMemory(void);

	void	HWInit7146Register(int ci);

	int	HWDeInitialize(int level);
	int	HWInitialize(int subdevice);

	static	int	HWDvbControl(CDvbAdapter *dvb, struct dvbapi *parm);
};

#endif
