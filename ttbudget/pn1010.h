
/*
 */

#ifndef _pn1010_h_
#define _pn1010_h_

class CDvbAdapter;
class CTunerPN1010 : public CTuner
{
protected:
	int	m_dwAddr;
	int	m_dwGPIO;

public:
	CTunerPN1010(CDvbAdapter *dvb);
	~CTunerPN1010();

	int	SetSlave(int slave);
	int	WriteSeq(int slave, unsigned char *buf, int len);
	int	ReadSeq(int slave, unsigned char *buf, int len);

	int	ReadI2CReg1010(int slave, unsigned char reg, unsigned char *val);
	int	WriteI2CReg1010(int slave, unsigned char reg, unsigned char val);

	int	ReadI2CReg(unsigned char reg);
	void	WriteI2CReg(unsigned char reg, unsigned char val);
};
#endif

