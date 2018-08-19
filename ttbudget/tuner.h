
/*
 */

#ifndef _tuner_h_
#define _tuner_h_

class CDvbAdapter;
class CTuner
{
protected:
	CDvbAdapter	*dvb;

	struct	tuner	state;

public:
	CTuner(CDvbAdapter *dvb);
	~CTuner();

	void		Sleep(unsigned int Milliseconds);

	virtual int	SetTunes(struct tunes *params);
	int		GetTuner(struct tuner *params);

	static void	UpdateStatus(CTuner *tuner);

	int		AcquireSignal(void);

	virtual	int	Initialize(void);

	virtual int	IsThisTunerInstalled(void);

	virtual int	GetStatus(fe_status_t *status, unsigned int *ber, unsigned char *signal);

	virtual int     SetLNB(int no, fe_polarity_t polar, fe_tone_t tone);
	virtual int	SetTuner(unsigned int freq, unsigned int sr, fe_inner_t inner);

};

#endif
