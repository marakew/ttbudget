
/*
 */

extern "C"
{
#include <sys/types.h>
#include <sys/param.h>
#if __FreeBSD_version >= 700000
#include <sys/systm.h>
#else
#include "systm.h"
#endif
#include <sys/bus.h>

#include "dvbapi.h"
}

#include "adapter.h"
#include "ttadapter.h"
#include "tuner.h"


CTuner::CTuner(CDvbAdapter *dvb):dvb(dvb)
{
	bzero(&state, sizeof(state));

	state.status = FE_TIMEOUT;
	state.ber = 0;
	state.signal = 0;
}

CTuner::~CTuner()
{

}

void CTuner::Sleep(unsigned int Milliseconds)
{
	DELAY(Milliseconds * 10);
}

int CTuner::SetTunes(struct tunes *params)
{
	int res;
	int	freq;
	unsigned int	sr;
	int updatelnb;

	bcopy(params, &state.param, sizeof(*params));

	printf("LNB: %d Polar %d Tone %d\n",
		state.param.lnb.no,
		state.param.lnb.polar,
		state.param.lnb.tone);

	res = SetLNB(state.param.lnb.no,
		     state.param.lnb.polar,
		     state.param.lnb.tone);


	freq = (state.param.tuner.freq - state.param.lnb.freq);
	if (freq < 0)
		freq = -freq;

	if (freq < 950 || freq > 2175)
	{
		printf("Error: invalid freq %d range!\n", freq);
		freq = 1300;
	}

	sr = state.param.tuner.sr;

	if (sr < 1000 || sr > 45000)
	{
		printf("Error: invalid sr %lu range!\n", sr);
		sr = 22000;
	}

	printf("Tuner: IFreq %d, SR %d, FEC %d\n",
		freq,
		sr,
		state.param.tuner.inner);

	res = SetTuner(freq,
			sr,
			state.param.tuner.inner);

	return 0;
}

int CTuner::GetTuner(struct tuner *params)
{
	int res;

	res = GetStatus(&state.status, &state.ber, &state.signal);

	bcopy(&state, params, sizeof(state));
}

void CTuner::UpdateStatus(CTuner *tuner)
{

}

int CTuner::AcquireSignal(void)
{
	return 0;
}

int CTuner::Initialize(void)
{
	return 0;
}

int CTuner::IsThisTunerInstalled(void)
{
	return 1;
}

int CTuner::GetStatus(fe_status *status, unsigned int *ber, unsigned char *signal)
{
	*status = FE_TIMEOUT;
	*ber = 1;
	*signal = 1;

	return 0;
}

int CTuner::SetLNB(int no, fe_polarity_t polar, fe_tone_t tone)
{
	return 0;
}

int CTuner::SetTuner(unsigned int freq, unsigned int sr, fe_inner_t inner)
{
	return 0;
}

