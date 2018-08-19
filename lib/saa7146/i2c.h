
/*
 */

#ifndef _i2c_h_
#define _i2c_h_


class CSpciI2C
{
protected:

	int	m_dwSlave;
	int	m_wError;
	int	m_nErrNo;
	int	m_nIdx;

	int	m_dwIICTRF;	
	int	m_dwStatus;

	int	*m_pRdBuf[3];

	int	m_wPciClock;

	int	m_peState;

	CSpciIoIrq	*pci;

	void	SetIICTRF(int data, unsigned char cmd, unsigned char idx);
	void	ClearIICTRF(void);
	void	AddIICTRF(int *data, unsigned char cmd, int res);
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

	int	GetErrNo(void);

	void	SetBitRate(unsigned short pciSpeed, unsigned short i2cSpeed);
        unsigned short	GetBitRate(void);

	int	ReadSeq(int *readBuf, int readLen);
	int	WriteSeq(int *writeBuf, int writeLen);
	int	CombinedSeq(int *writeBuf, int writeLen, int *readBuf, int readLen);

	void	Stop(void);
	int	StartR(void);
	int	StartW(void);
	int	WriteByte(int);
	int	ReadByte(int);
};

#endif

