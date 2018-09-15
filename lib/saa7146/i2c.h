
/*
 */

#ifndef _i2c_h_
#define _i2c_h_


class CSpciI2C
{
protected:

    // I2C errors (return values for sequence interface)
    enum {
      I2CERR_NO_ERROR = 0x00,   // No error
      I2CERR_TIMEOUT  = 0x01,   // Timeout during transmission
      I2CERR_AL       = 0x04,   // Arbitration lost
      I2CERR_DRERR    = 0x08,   // NACK - Error when receiving data
      I2CERR_DTERR    = 0x10,   // NACK - Error in data transmission
      I2CERR_APERR    = 0x20,   // NACK - Error in address phase
      I2CERR_SPERR    = 0x40    // Bus error due to invalid start/stop 
                                // condidion
    } I2C_ERRORS;

	int	m_dwSlave;	// slave address of device
	int	m_wError;	// Store error status
	int	m_nErrNo;	// Number of I2C Seq-errors
	int	m_nIdx;		// index to IICTRF data byte

	int	m_dwIICTRF;	// shadow of IICTRF register
	int	m_dwStatus;	// shadow of I2C_STATUS register

	int	*m_pRdBuf[3];	// Array of read back pointers

	int	m_wPciClock;	// Pci clock in kHz (for bitrate)

	int	m_peState;	// State of protocol element 
				// interf.

	CSpciIoIrq *pci;

	void	SetIICTRF(int cData, unsigned char cAddtr, unsigned char cPosition);
	void	ClearIICTRF(void);
	void	AddIICTRF(int *pData, unsigned char cAttr, int read_back);
	void	FlushIICTRF(void);
	void	ReadIICTRF(void);
	int	CheckIICSTATE(void);

public:
	CSpciI2C(CSpciIoIrq *pci);
	~CSpciI2C();

	void	Sleep(unsigned int ms);

	void	Init(void);

	int	GetSlave();
	void	SetSlave(int slave);

	int	GetErrNo(void);		// return number of errors

	void	SetBitRate(unsigned short nPciClock,	// PCI clock in MHz
			   unsigned short nBitRate);	// max. Bit Rate in kHz
        unsigned short	GetBitRate(void);	// Get current Bit Rate in kHz

	//--- Sequence interface -------------------
	int	ReadSeq(int seq[], int len);	// read  some bytes from I2C
	int	WriteSeq(int seq[], int len);	// write some bytes to   I2C
	int	CombinedSeq(int seqWr[], int lenWr,	 // read after write
			    int seqRd[], int lenRd);

	//--- Protocol element interface -----------
	//    It is recommended to use the sequence interface...
	void	Stop(void);		// I2C stop  condition
	int	StartR(void);		// I2C start rd + slave addr
	int	StartW(void);		// I2C start wr + slave addr
	int	WriteByte(int byte);	// write 1 byte to   I2C bus
	int	ReadByte(int ack);	// read  1 byte from I2C bus

    enum
    {   // I2c byte attributes...
        START_ATTR= 3,
        CONT_ATTR=  2,
        STOP_ATTR=  1,
        NOP_ATTR=   0
    } I2cAttribs;

    enum Bitrate
    {
        PCI_6400 =  0x500,
        PCI_3200 =  0x100,
        PCI_480  =  0x400,
        PCI_320  =  0x600,
        PCI_240  =  0x700,
        PCI_120  =  0x000,
        PCI_80   =  0x200,
        PCI_60   =  0x300
    };

    enum I2C_STATUS
    {
        I2C_BSY   = 0x0001,
        I2C_ERR   = 0x0002,
        I2C_ABORT = 0x0080
    };

    enum PE_Status                           // States of protocol element 
                                             // interface
    {
        IDLE,                                // nothing happened yet
        WR_S,                                // write started
        RD_S,                                // read started
        CONT,                                // write continued
        END_READ,                            // read neg acknowledged
        PERR                                 // protocol error
    };

    enum
    {
        none    =-1
    };

};

const int I2C_LOW   =0;                      // low
const int I2C_HIGH3 =~0;                     // high, tristate

#endif

