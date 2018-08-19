
/*
 */

#ifndef _tda10086_h_
#define _tda10086_h_

class CDvbAdapter;
class CTunerTDA10086 : public CTuner
{
protected:
	unsigned char	TunerAddr;	// TDA8263
	unsigned char	DemodAddr;	// TDA10086

	unsigned int	MCLK;

	unsigned char	Gate;
	unsigned char	Inversion;
	unsigned char	ViterbiRate;
	unsigned char	Status;	

	unsigned char	Tone;
	unsigned char	AGC;
	unsigned char	Signal;
	unsigned char	Wait;

	unsigned char	TunerData[11];

	unsigned char	Lock;
public:
	CTunerTDA10086(CDvbAdapter *dvb);
	~CTunerTDA10086();

	virtual int	SetTunes(struct tunes *params);

	int	ReadSeq(int slave, unsigned char *readBuf, int readLen);
	int	WriteSeq(int slave, unsigned char *writeBuf, int writeLen);
	int	CombinedSeq(int slave,
		unsigned char *writeBuf, int writeLen, unsigned char *readBuf, int readLen);

	virtual int     IsThisTunerInstalled(void);

	virtual int	Initialize(void);

	int	InitTuner(void);
	int	InitDemod(void);

	int	GateControl(int enable);

	int	SetPolarity(fe_polarity_t polar);
	int	SetFreq(unsigned int tunerFreq);

	int	SetViterbiRate(fe_inner_t inner);

	int	SetInversion(void);

	int	SetSymbolRate(unsigned int sr);

	int	SetTone(fe_tone_t tone);

	virtual int	SetLNB(int no, fe_polarity_t polar, fe_tone_t tone);
	virtual int	SetTuner(unsigned int freq, unsigned int sr, fe_inner_t inner);

	virtual	int	GetStatus(fe_status_t *status, unsigned int *ber, unsigned char *signal);

	int	UpdateGain(void);
	int	DiSEqCMsg(int a, int len, unsigned int cmd);
};

#endif

