
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

	m_dwStatus = PCI_320;

	m_wPciClock = 33000;

	pci->SetReg(MC1, 0x01000100, 1);

	pci->SetReg(I2C_TRANSFER, 0, 0);
	pci->SetReg(I2C_STATUS, m_dwStatus, 1);

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
	int ret = m_nErrNo;
	m_nErrNo = 0;
	return ret;
}

void CSpciI2C::SetIICTRF(int cData, unsigned char cAttr, unsigned char cPosition)
{
	unsigned char dataIndex;
	unsigned char cmdIndex;

	dataIndex = 24 - (cPosition * 8);
	cmdIndex = 6 - (cPosition * 2);

	m_dwIICTRF &= ~((3 << cmdIndex) | ( 0xff << dataIndex));

	m_dwIICTRF |= (cData & 0xff) << dataIndex;
	m_dwIICTRF |= (cAttr & 0xff) << cmdIndex;
}

void CSpciI2C::ClearIICTRF(void)
{
	m_dwIICTRF = 0;
	m_nIdx = 0;
}

void CSpciI2C::SetBitRate(unsigned short nPciClock, unsigned short nBitRate)
{
	int i;
	unsigned int dwRate;

	unsigned int dwBitRate[8] = { 60, 80, 120, 240, 320, 480, 3200, 6400 };
	unsigned int dwStatus[8] = { PCI_60, PCI_80, PCI_120, PCI_240, PCI_320, PCI_480, PCI_3200, PCI_6400 };

	m_wPciClock = nPciClock * 1000;

	dwRate = m_wPciClock / nBitRate;

	for (i = 0; i < 7; i++)
	{
		if (dwRate <= dwBitRate[i])
		{
			m_dwStatus = dwStatus[i];
			break;
		}
	}

	pci->SetReg(I2C_TRANSFER, 0, 0);
	pci->SetReg(I2C_STATUS, m_dwStatus, 0);
}

unsigned short CSpciI2C::GetBitRate(void)
{
	unsigned int dwRate;

	dwRate = m_dwStatus & 0x700;

	if (dwRate == PCI_3200)	return m_wPciClock / 3200;
	if (dwRate == PCI_80)	return m_wPciClock / 80;
	if (dwRate == PCI_60)	return m_wPciClock / 60;
	if (dwRate == PCI_480)	return m_wPciClock / 480;
	if (dwRate == PCI_6400)	return m_wPciClock / 6400;
	if (dwRate == PCI_320)	return m_wPciClock / 320;
	if (dwRate == PCI_240)	return m_wPciClock / 240;
	if (dwRate == PCI_120)	return m_wPciClock / 120;

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

	error = pci->GetReg(I2C_STATUS);

	if ( !(m_dwIICTRF & 2) && !(error & 0x7E))
	{
		return 1;
	}

	m_wError |= (error & 0x7C);

	pci->SetReg(I2C_STATUS, m_dwStatus | 0x80, 1);
	pci->SetReg(I2C_STATUS, m_dwStatus, 1);

	error = pci->GetReg(I2C_STATUS);

	if (error & 0x7F)
		pci->SetReg(I2C_STATUS, m_dwStatus, 1);

	m_nErrNo++;

	return 0;
}

void CSpciI2C::FlushIICTRF(void)
{
	int status;
	struct timeval timeout;

	Sleep(0);

	m_wError = 0;

	if (pci->GetReg(I2C_STATUS) & 3)
	{
		pci->SetReg(I2C_TRANSFER, 0, 0);
		pci->SetReg(I2C_STATUS, m_dwStatus | 0x80, 1);
		pci->SetReg(I2C_STATUS, m_dwStatus, 1);

		Sleep(0);

		if (pci->GetReg(I2C_STATUS) & 3)
			pci->SetReg(I2C_STATUS, m_dwStatus, 1);
	}

	/* write 3bytes */
	pci->SetReg(I2C_TRANSFER, m_dwIICTRF, 1);

	SetTimeOut(&timeout, 100);	//100
	Sleep(0);

	status = pci->GetReg(MC2);
	if ( !(status & 1) )
	{
		while (GetTimeOut(&timeout))
		{
			Sleep(0);
			status = pci->GetReg(MC2);
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

	m_dwIICTRF = pci->GetReg(I2C_TRANSFER);
	if ( m_dwIICTRF & 1 )
	{
		while (GetTimeOut(&timeout))
		{
			Sleep(0);
			m_dwIICTRF = pci->GetReg(I2C_TRANSFER);
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

void CSpciI2C::AddIICTRF(int *pData, unsigned char cAttr, int read_back)
{
	m_pRdBuf[m_nIdx] = (read_back == 1) ? pData : 0;// NULL;

	SetIICTRF(*pData, cAttr, m_nIdx++);

	if (m_nIdx == 3)
	{
		FlushIICTRF();
		ReadIICTRF();
		CheckIICSTATE();
		ClearIICTRF();
	}
}

int CSpciI2C::ReadSeq(int seq[], int len)
{
	int slave;
	int errNo;
	int i;

	ClearIICTRF();

	slave = (GetSlave() << 1) | 1;
	errNo = (GetErrNo());

	AddIICTRF(&slave, START_ATTR, 0);

	for (i = 0; i < len-1; i++)
	{
		AddIICTRF(&seq[i], CONT_ATTR, 1);
		if (errNo < GetErrNo())
			return m_wError;
	}

	AddIICTRF(&seq[i], STOP_ATTR, 1);

	if (m_nIdx != 0)
		FlushIICTRF();

	if (CheckIICSTATE())
		ReadIICTRF();

	return m_wError;
}

int CSpciI2C::WriteSeq(int seq[], int len)
{
	int slave;
	int errNo;
	int i;

	ClearIICTRF();

	slave = (GetSlave() << 1);
	errNo = (GetErrNo());

	AddIICTRF(&slave, START_ATTR, 0);

	for (i = 0; i < len-1; i++)
	{
		AddIICTRF(&seq[i], CONT_ATTR, 0);
		if (errNo < GetErrNo())
			return m_wError;
	}

	AddIICTRF(&seq[i], STOP_ATTR, 0);

	if (m_nIdx != 0)
		FlushIICTRF();

	CheckIICSTATE();

	return m_wError;
}

int CSpciI2C::CombinedSeq(int seqWr[], int lenWr, int seqRd[], int lenRd)
{
	int slave;
	int errNo;
	int i;

	ClearIICTRF();

	slave = (GetSlave() << 1);	//
	errNo = (GetErrNo());

	AddIICTRF(&slave, START_ATTR, 0);

	for (i = 0; i < lenWr; i++)
	{
		AddIICTRF(&seqWr[i], CONT_ATTR, 0);
		if (errNo < GetErrNo())
			return m_wError;
	}

	slave = (GetSlave() << 1) | 1;

	AddIICTRF(&slave, START_ATTR, 0);
	if (errNo < GetErrNo())
		return m_wError;

	for (i = 0; i < lenRd-1; i++)
	{
		AddIICTRF(&seqRd[i], CONT_ATTR, 1);
		if (errNo < GetErrNo())
			return m_wError;
	}

	AddIICTRF(&seqRd[i], STOP_ATTR, 1);

	if (m_nIdx != 0)
		FlushIICTRF();

	if(CheckIICSTATE())
		ReadIICTRF();

	return m_wError;
}

void CSpciI2C::Stop(void)
{
#if 0
	switch(m_peState)
	{
	case 0:	//IDLE
		break;
	case 3: //COUNT
		unsigned char idx = 8 - (m_nIdx*2);
		m_dwIICTRF &= ~(3 << val & 0xFF);
		m_dwIICTRF |= (1 << val & 0xFF);
		FlushIICTRF();
		CheckIICSTATE();
	case 4: //END_READ
		m_peState = 0; //IDLE
		break;
	default:
		m_nErrNo++;
		m_peState = 0; //IDLE
	}
#endif
}

int CSpciI2C::StartR(void)
{
#if 0
	int res;
	switch(m_peState)
	{
	case 0: //IDLE
		ClearIICTRF();
	case 3: //COUNT
		int slave = (m_dwSlave << 1)|1;
		AddIICTRF(&slave, START_ATTR, 0);
		FlushIICTRF();
		if (CheckIICSTATE())
		{
			m_peState = 2; //RD_S
			res = 0;
		} else
		{
			m_peState = 0; //IDLE
			res = -1;
		}
	default:
		m_nErrNo++;
		m_peState = 0; //IDLE
		res = -1;
	}
	return res;
#endif
}

int CSpciI2C::StartW(void)
{
#if 0
	int res;
	switch(m_peState)
	{
	case 0: //IDLE
		int slave = (m_dwSlave << 1);
		ClearIICTRF();
		AddIICTRF(&slave, START_ATTR, 0);
		m_peState = 1; //WR_S
		res = 0;
		break;
	default:
		m_peState = 0; //IDLE
		m_nErrNo++;
		res = -1;
	}

	return res;
#endif
}

int CSpciI2C::WriteByte(int byte)
{
#if 0
	int res;
	switch(m_peState)
	{
	case 1: //WR_S
		m_peState = 3; //CONT
	case 3:
		if (m_nIdx == 3) //??? 0
		{
			FlushIICTRF();

			if (!CheckIICSTATE())
			{
				m_peState = 0; //IDLE
				res = -1;
				break;
			}

			ClearIICTRF();
		} 

		SetIICTRF(byte, CONT_ATTR, m_nIdx++);
		res = 0;
		break;
	default:
		m_peState = 0; //IDLE
		m_nErrNo++;
		res = -1;
	}
	return res;
#endif
}

int CSpciI2C::ReadByte(int ack)
{
#if 0
	int res;
	switch(m_peState)
	{
	case 2: //RD_S
		ClearIICTRF();
		if (ack == -1)
		{
			SetIICTRF(0, STOP_ATTR, 0);
			m_peState = 4; //END_READ
		} else
		{
			SetIICTRF(0, CONT_ATTR, 0);
		}

		FlushIICTRF();

		if (CheckIICSTATE())
		{
			res = (m_dwIICTRF >> 24);
		} else
		{
			m_peState = 0; //IDLE
			res = -1;
		}
		break;
	deafult:
		m_nErrNo++;
		m_peState = 0; //IDLE
		res = -1;
	}
	return res;
#endif
}
