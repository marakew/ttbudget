
/*
 */

extern "C"
{
	#include <sys/param.h>

	#include <sys/time.h>

	#include "timeout.h"
}

#include "pci.h"
#include "i2c.h"

CSpciI2C::CSpciI2C(CSpciIoIrq *pci):pci(pci)
{
	Init();
}

CSpciI2C::~CSpciI2C()
{
}

void CSpciI2C::Sleep(unsigned int ms)
{
	/*
	TODO
	*/
}

void CSpciI2C::Init(void)
{
	m_nErrNo = 0;
	m_wError = 0;

	m_dwIICTRF = 0;
	m_nIdx = 0;

	m_peState = 0;

	m_pRdBuf[0] = (int *)NULL;
	m_pRdBuf[1] = (int *)NULL;
	m_pRdBuf[2] = (int *)NULL;

	m_dwStatus = 0x600;	//speed 320bs

	m_wPciClock = 33000;

	pci->SetReg(0xFC, 0x01000100, 1);

	pci->SetReg(0x8C, 0, 0);
	pci->SetReg(0x90, m_dwStatus, 1);

	m_dwSlave = 0;
}

int CSpciI2C::GetSlave(void)
{
	return m_dwSlave;
}

void CSpciI2C::SetSlave(int slave)
{
	m_dwSlave = slave;
}

int CSpciI2C::GetErrNo(void)
{
	return m_nErrNo;
}

void CSpciI2C::SetIICTRF(int data, unsigned char cmd, unsigned char index)
{
	unsigned char dataIndex;
	unsigned char cmdIndex;

	dataIndex = 24 - (index * 8);
	cmdIndex = 6 - (index * 2);

	m_dwIICTRF &= ~((3 << cmdIndex) | ( 0xff << dataIndex));

	m_dwIICTRF |= (data & 0xff) << dataIndex;
	m_dwIICTRF |= (cmd & 0xff) << cmdIndex;
}

void CSpciI2C::ClearIICTRF(void)
{
	m_dwIICTRF = 0;
	m_nIdx = 0;
}

void CSpciI2C::SetBitRate(unsigned short pciSpeed, unsigned short i2cSpeed)
{
	int i;
	unsigned int dwRate;

	unsigned int dwBitRate[8] = 
		{ 60, 80, 120, 240, 320, 480, 3200, 6400 };

	unsigned int dwStatus[8] =
		{ 0x300, 0x200, 0, 0x700, 0x600, 0x400, 0x100, 0x500 };

	m_wPciClock = pciSpeed * 1000;

	dwRate = m_wPciClock / i2cSpeed;

	for (i = 0; i < 7; i++)
	{
		if (dwRate <= dwBitRate[i])
		{
			m_dwStatus = dwStatus[i];
			break;
		}
	}

	pci->SetReg(0x8C, 0, 0);
	pci->SetReg(0x90, m_dwStatus, 0);
}

unsigned short CSpciI2C::GetBitRate(void)
{
	unsigned int dwRate;

	dwRate = m_dwStatus & 0x700;

	if (dwRate == 0x100)
		return m_wPciClock / 3200;

	if (dwRate == 0x200)
		return m_wPciClock / 80;

	if (dwRate == 0x300)
		return m_wPciClock / 60;

	if (dwRate == 0x400)
		return m_wPciClock / 480;

	if (dwRate == 0x500)
		return m_wPciClock / 6400;

	if (dwRate == 0x600)
		return m_wPciClock / 320;

	if (dwRate == 0x700)
		return m_wPciClock / 240;

	if (dwRate == 0)
		return m_wPciClock / 120;

	return 0;
}

void CSpciI2C::ReadIICTRF(void)
{
	if (m_pRdBuf[0] != NULL)
	{
		*m_pRdBuf[0] = (m_dwIICTRF >> 24 ) & 0xff;
		m_pRdBuf[0] = (int *)NULL;
	}

	if (m_pRdBuf[1] != NULL)
	{
		*m_pRdBuf[1] = (m_dwIICTRF >> 16 ) & 0xff;
		m_pRdBuf[1] = (int *)NULL;
	}

	if (m_pRdBuf[2] != NULL)
	{
		*m_pRdBuf[2] = (m_dwIICTRF >> 8 ) & 0xff;
		m_pRdBuf[2] = (int *)NULL;
	}
}

int CSpciI2C::CheckIICSTATE(void)
{
	int error;

	error = pci->GetReg(0x90);

	if ( !(m_dwIICTRF & 2) && !(error & 0x7E))
	{
		return 1;
	}

	m_wError |= (error & 0x7C);

	pci->SetReg(0x90, m_dwStatus | 0x80, 1);
	pci->SetReg(0x90, m_dwStatus, 1);

	error = pci->GetReg(0x90);

	if (error & 0x7F)
		pci->SetReg(0x90, m_dwStatus, 1);

	m_nErrNo++;

	return 0;
}

void CSpciI2C::FlushIICTRF(void)
{
	int status;
	struct timeval timeout;

	Sleep(0);

	m_wError = 0;

	if (pci->GetReg(0x90) & 3)
	{
		pci->SetReg(0x8C, 0, 0);
		pci->SetReg(0x90, m_dwStatus | 0x80, 1);
		pci->SetReg(0x90, m_dwStatus, 1);

		Sleep(0);

		if (pci->GetReg(0x90) & 3)
			pci->SetReg(0x90, m_dwStatus, 1);
	}

	/* write 3bytes */
	pci->SetReg(0x8C, m_dwIICTRF, 1);

	SetTimeOut(&timeout, 100);	//100
	Sleep(0);

	status = pci->GetReg(0x100);
	if ( !(status & 1) )
	{
		while (GetTimeOut(&timeout))
		{
			Sleep(0);
			status = pci->GetReg(0x100);
			if ( status & 1 )
				break;
		}

		if ( !(status & 1) )
		{
			m_wError |= 1;
			m_dwIICTRF = 0xFFFFFF02;
			return;
		}
	}

	/* read 3bytes */
	SetTimeOut(&timeout, 100);	//100
	Sleep(0);

	m_dwIICTRF = pci->GetReg(0x8C);
	if ( m_dwIICTRF & 1 )
	{
		while (GetTimeOut(&timeout))
		{
			Sleep(0);
			m_dwIICTRF = pci->GetReg(0x8C);
			if ( !(m_dwIICTRF & 1) )
				break;
		}

		if ( m_dwIICTRF & 3 )
		{
			m_wError |= 1;
			m_dwIICTRF = 0xFFFFFF02;
			return;
		}
	}

	Sleep(0);
}

void CSpciI2C::AddIICTRF(int *data, unsigned char cmd, int res)
{
	m_pRdBuf[m_nIdx] = (res == 1) ? data : 0;// NULL;

	SetIICTRF(*data, cmd, m_nIdx++);

	if (m_nIdx == 3)
	{
		FlushIICTRF();
		ReadIICTRF();
		CheckIICSTATE();
		ClearIICTRF();
	}
}

int CSpciI2C::ReadSeq(int *readBuf, int readLen)
{
	int slave;
	int errNo;
	int i;

	ClearIICTRF();

	slave = (GetSlave() << 1) | 1;
	errNo = (GetErrNo());

	AddIICTRF(&slave, 3, 0);	//START WRITE

	for (i = 0; i < readLen-1; i++)
	{
		AddIICTRF(&readBuf[i], 2, 1);	//COUNT READ
		if (errNo < GetErrNo())
			return m_wError;
	}

	AddIICTRF(&readBuf[i], 1, 1);	//STOP READ

	if (m_nIdx != 0)
		FlushIICTRF();

	if (CheckIICSTATE())
		ReadIICTRF();

	return m_wError;
}

int CSpciI2C::WriteSeq(int *writeBuf, int writeLen)
{
	int slave;
	int errNo;
	int i;

	ClearIICTRF();

	slave = (GetSlave() << 1);
	errNo = (GetErrNo());

	AddIICTRF(&slave, 3, 0);	//START WRITE

	for (i = 0; i < writeLen-1; i++)
	{
		AddIICTRF(&writeBuf[i], 2, 0);	//COUNT WRITE
		if (errNo < GetErrNo())
			return m_wError;
	}

	AddIICTRF(&writeBuf[i], 1, 0);	//STOP WRITE

	if (m_nIdx != 0)
		FlushIICTRF();

	CheckIICSTATE();

	return m_wError;
}

int CSpciI2C::CombinedSeq(int *writeBuf, int writeLen, int *readBuf, int readLen)
{
	int slave;
	int errNo;
	int i;

	ClearIICTRF();

	slave = (GetSlave() << 1);	//
	errNo = (GetErrNo());

	AddIICTRF(&slave, 3, 0);	//START WRITE

	for (i = 0; i < writeLen; i++)
	{
		AddIICTRF(&writeBuf[i], 2, 0);	//COUNT WRITE
		if (errNo < GetErrNo())
			return m_wError;
	}

	slave = (GetSlave() << 1) | 1;

	AddIICTRF(&slave, 3, 0);	//I2C_START WRITE
	if (errNo < GetErrNo())
		return m_wError;

	for (i = 0; i < readLen-1; i++)
	{
		AddIICTRF(&readBuf[i], 2, 1);	//I2C_COUNT READ
		if (errNo < GetErrNo())
			return m_wError;
	}

	AddIICTRF(&readBuf[i], 1, 1);	//I2C_STOP READ

	if (m_nIdx != 0)
		FlushIICTRF();

	if(CheckIICSTATE())
		ReadIICTRF();

	return m_wError;
}

void CSpciI2C::Stop(void)
{
}

int CSpciI2C::StartR(void)
{
}

int CSpciI2C::StartW(void)
{
}

int CSpciI2C::WriteByte(int)
{
}

int CSpciI2C::ReadByte(int)
{
}
