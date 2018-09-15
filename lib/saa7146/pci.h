
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

  //- SAA7146 Memory registers -------------------------------------------------
  //
  //  Note: PCI Configuration registers have 0x1000 added their offset to 
  //        allow the software to distinguish between the memory registers and
  //        the PCI configuration registers.
  //
  enum SpciReg {
//---------- Start of register structure
    BASE_ODD1        =   0x00,  // Video DMA 1 registers
    BASE_EVEN1       =   0x04,
    PROT_ADDR1       =   0x08,
    PITCH1           =   0x0C,
    BASE_PAGE1       =   0x10,  // Video DMA 1 base page
    NUM_LINE_BYTE1   =   0x14,

    BASE_ODD2        =   0x18,  // Video DMA 2 registers
    BASE_EVEN2       =   0x1C,
    PROT_ADDR2       =   0x20,
    PITCH2           =   0x24,
    BASE_PAGE2       =   0x28,  // Video DMA 2 base page
    NUM_LINE_BYTE2   =   0x2C,

    BASE_ODD3        =   0x30,  // Video DMA 3 registers
    BASE_EVEN3       =   0x34,
    PROT_ADDR3       =   0x38,
    PITCH3           =   0x3C,
    BASE_PAGE3       =   0x40,  // Video DMA 3 base page
    NUM_LINE_BYTE3   =   0x44,
//----------                    // DMA Busts & FIFO Thresholds
    PCI_BT_V1        =   0x48,  // Video/FIFO 1
    PCI_BT_V2        =   0x49,  // Video/FIFO 2
    PCI_BT_V3        =   0x4A,  // Video/FIFO 3
    PCI_BT_DEBI      =   0x4B,  // DEBI
    PCI_BT_A         =   0x4C,  // Audio
//----------
    DD1_INIT         =   0x50,  // Init setting of DD1 interface
//----------
    DD1_STREAM_B     =   0x54,  // DD1 B video data stream handling
    DD1_STREAM_A     =   0x56,  // DD1 A video data stream handling
//----------
    BRS_CTRL         =   0x58,  // BRS control register
    HPS_CTRL         =   0x5C,  // HPS control register
    HPS_V_SCALE      =   0x60,  // HPS vertical scale
    HPS_V_GAIN       =   0x64,  // HPS vertical ACL and gain
    HPS_H_PRESCALE   =   0x68,  // HPS horizontal prescale
    HPS_H_SCALE      =   0x6C,  // HPS horizontal scale
    BCS_CTRL         =   0x70,  // BCS control
    CHROMA_KEY_RANGE =   0x74,
    CLIP_FORMAT_CTRL =   0x78,  // HPS outputs, formats & clipping
//----------
    DEBI_CONFIG      =   0x7C,
    DEBI_COMMAND     =   0x80,
    DEBI_PAGE        =   0x84,
    DEBI_AD          =   0x88,
//----------
    I2C_TRANSFER     =   0x8C,
    I2C_STATUS       =   0x90,
//----------
    BASE_A1_IN       =   0x94,  // Audio 1 input DMA
    PROT_A1_IN       =   0x98,
    PAGE_A1_IN       =   0x9C,

    BASE_A1_OUT      =   0xA0,  // Audio 1 output DMA
    PROT_A1_OUT      =   0xA4,
    PAGE_A1_OUT      =   0xA8,

    BASE_A2_IN       =   0xAC,  // Audio 2 input DMA
    PROT_A2_IN       =   0xB0,
    PAGE_A2_IN       =   0xB4,

    BASE_A2_OUT      =   0xB8,  // Audio 2 output DMA
    PROT_A2_OUT      =   0xBC,
    PAGE_A2_OUT      =   0xC0,
//----------
    RPS_PAGE0        =   0xC4,  // RPS task 0 page register
    RPS_PAGE1        =   0xC8,  // RPS task 1 page register
//----------
    RPS_THRESH0      =   0xCC,  // HBI threshold for task 0
    RPS_THRESH1      =   0xD0,  // HBI threshold for task 1

    RPS_TOV0         =   0xD4,  // RPS timeout for task 0
    RPS_TOV1         =   0xD8,  // RPS timeout for task 1
//----------
    IER              =   0xDC,  // Interrupt enable register
//----------
    GPIO_CTRL        =   0xE0,  // GPIO 0-3 register
//----------
    EC1SSR           =   0xE4,  // Event cnt set 1 source select
    EC2SSR           =   0xE8,  // Event cnt set 2 source select
    ECT1R            =   0xEC,  // Event cnt set 1 thresholds
    ECT2R            =   0xF0,  // Event cnt set 2 thresholds
//----------
    ACON1            =   0xF4,
    ACON2            =   0xF8,
//----------
    MC1              =   0xFC,  // Main control register 1
    MC2              =  0x100,  // Main control register 2
//----------
    RPS_ADDR0        =  0x104,  // RPS task 0 address register
    RPS_ADDR1        =  0x108,  // RPS task 1 address register
//----------
    ISR              =  0x10C,  // Interrupt status register
    PSR              =  0x110,  // Primary status register
    SSR              =  0x114,  // Secondary status register
//----------
    EC1R             =  0x118,  // Event counter set 1 register
    EC2R             =  0x11C,  // Event counter set 2 register
//----------
    PCI_VDP1         =  0x120,  // Video DMA pointer of FIFO 1
    PCI_VDP2         =  0x124,  // Video DMA pointer of FIFO 2
    PCI_VDP3         =  0x128,  // Video DMA pointer of FIFO 3
    PCI_ADP1         =  0x12C,  // Audio DMA pointer of audio out 1
    PCI_ADP2         =  0x130,  // Audio DMA pointer of audio in 1
    PCI_ADP3         =  0x134,  // Audio DMA pointer of audio out 2
    PCI_ADP4         =  0x138,  // Audio DMA pointer of audio in 2
    PCI_DMA_DDP      =  0x13C,  // DEBI DMA pointer
//----------
    LEVEL_REP        =  0x140,
    A_TIME_SLOT1     =  0x180,  // from 180 - 1BC
    A_TIME_SLOT2     =  0x1C0,  // from 1C0 - 1FC
//----------
    END_OF_MEM_REG   =  0x200   // invalid register offset
  }; // end of SpciReg


	CSpciIoIrq(struct resource *memory);
	~CSpciIoIrq();

	void	Init(void);

	void	DeviceReset(void);

	unsigned int	GetUploadMask(int nReg);
	void	UploadRegs(unsigned int dwMask);

	unsigned int	GetReg(int nReg);
	void	SetReg(int nReg, unsigned int dwValue, int bUpload = 1);

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

