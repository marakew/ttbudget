
/*
 */

#ifndef _stb0899_h_
#define _stb0899_h_

class CDvbAdapter;
class CTunerSTB0899 : public CTuner
{
protected:


public:
	CTunerSTB0899(CDvbAdapter *dvb);
	~CTunerSTB0899();

	int	STI2C_Write(int slave ,unsigned char *buf, int len);
	int	STI2C_Read(int slave, unsigned char *buf, int len);
	int	STI2C_Combined(int dir, int slave, unsigned char *buf, int len);

	int	CheckLNBP21();
};

#endif

