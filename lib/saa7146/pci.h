
/*
 */

#ifndef _pci_h_
#define _pci_h_

struct ISRTable
{
	unsigned int	isrtype;
	unsigned int	isrnum;
	unsigned int	irqtype;
	void		*data;
};

class CSpciIoIrq
{
protected:

	struct resource	*memory;

	unsigned int	isrnum;	
	unsigned int	spin;
	unsigned int	irq;

	unsigned short		isrTableSize;
	struct	ISRTable	isrTableEntry[32];

public:
	CSpciIoIrq(struct resource *memory);
	~CSpciIoIrq();

	void	Init(void);

	void	DeviceReset(void);

	unsigned int	GetUploadMask(int reg);
	void	UploadRegs(unsigned int mask);

	unsigned int	GetReg(int reg);
	void	SetReg(int reg, unsigned int value, int upload);

	void	SetOut22KHzLine(int state);
	int	GetOut22HKzLine(void);

	void	SetResetLine(int state);
	int	GetResetLine(void);

	void	SetGPIO(int gpioPin, int gpioMode);
	int	GetGPIO(int gpioPin);

	unsigned int	EnableIrq(unsigned int irqtype);
	unsigned int	DisableIrq(unsigned int irqtype);

	void	PhysicalEnable(void);
	void	PhysicalDisable(void);


	int	RegisterIsr(unsigned int irqtype, void *data, unsigned int isrtype, unsigned short *isrSize, struct ISRTable *isrTable);
	int	RegisterRing0Isr(unsigned int irqtype, void *data, unsigned int isrtype);


	int	ScanIsrTable(unsigned int isrnum, unsigned short *isrSize, struct ISRTable *isrTable);
	int	UnregisterIsr(unsigned int isrnum);

	unsigned int	CallRing0Isrs(unsigned int irqtype);

	int	OnHardwareInt(void);
};

#endif

