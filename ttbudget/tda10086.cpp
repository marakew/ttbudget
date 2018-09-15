
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
#include "tda10086.h"


CTunerTDA10086::CTunerTDA10086(CDvbAdapter *dvb):CTuner(dvb)
{
	TunerAddr = 0xC0;
	DemodAddr = 0x1C;

}

CTunerTDA10086::~CTunerTDA10086()
{
}

int CTunerTDA10086::SetTunes(struct tunes *params)
{
	int res;

	res = InitDemod();

	res = CTuner::SetTunes(params);

	return 0;
}

int CTunerTDA10086::ReadSeq(int slave, unsigned char *readBuf, int readLen)
{
	return dvb->I2CReadSeq(slave, readBuf, readLen);
}

int CTunerTDA10086::WriteSeq(int slave, unsigned char *writeBuf, int writeLen)
{
	return dvb->I2CWriteSeq(slave, writeBuf, writeLen);
}

int CTunerTDA10086::CombinedSeq(int slave, unsigned char *writeBuf, int writeLen, unsigned char *readBuf, int readLen)
{
	return dvb->I2CCombinedSeq(slave, writeBuf, writeLen, readBuf, readLen);
}

int CTunerTDA10086::IsThisTunerInstalled(void)
{
	unsigned char value;
	unsigned char DemodID;
	int res;
	
	DemodID = 0;

	value = 0x1E;
	res = CombinedSeq(DemodAddr, &value, 1, &DemodID, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return 0;
	}

	if (DemodID != 0xE1)
		return 0;

	return 1;
}

int CTunerTDA10086::InitTuner(void)
{
	int res;
	unsigned char array[11];
	unsigned int freq;

	freq = 1354;

	// powerdown RSSI + the magic value 1
	TunerData[0] = 0x09;
	// 1Mhz + 0.45 VCO
	TunerData[1] = (1<<5) | 0x0B;

	TunerData[2] = (freq >> 7) & 0xff;
	TunerData[3] = (freq << 1) & 0xff;
	// baseband cut-off 19 MHz 77 ??
	TunerData[4] = 0xff;//0x77;//0xff;
	// baseband gain 9 db + no RF attenuation
	TunerData[5] = 0xfe;
	// charge pumps at high, tests off
	TunerData[6] = 0x83;	//(TunerData[5] + 0x85) & 0xff;	//0x83
	// recommended value 4 for AMPVCO + disable ports.
	TunerData[7] = 0x80;
	// normal caltime + recommended values for SELTH + SELVTL
	TunerData[8] = 0x1A;	//(TunerData[7] + 0x9A) & 0xff;	//+1A
	// recommended value 13 for BBIAS + unknown bit set on
	TunerData[9] = 0xD4;

	// subaddress
	array[0] = 0;
	array[1] = TunerData[0];
	array[2] = TunerData[1];
	array[3] = TunerData[2];
	array[4] = TunerData[3];
	array[5] = TunerData[4];
	array[6] = TunerData[5];
	array[7] = TunerData[6];
	array[8] = TunerData[7];
	array[9] = TunerData[8];
	array[10] = TunerData[9];

	res = GateControl(1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	res = WriteSeq(TunerAddr, &array[0], 11);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	res = GateControl(0);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	return 0;
}

int CTunerTDA10086::InitDemod(void)
{
	unsigned char array[7];
	int res;

	array[0] = 0;
	array[1] = 1;		//0
	array[2] = 0x94;	//1

	//array[3] = 0;		//2	0x35
	array[3] = 0x35;		//2	0x35

	array[4] = 0xE4;	//3
	array[5] = 0x43;	//4
	array[6] = 0xB;		//5

	res = WriteSeq(DemodAddr, &array[0], 7);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x11;
	array[1] = 0x81;	//11
	array[2] = 0x81;	//12

	res = WriteSeq(DemodAddr, &array[0], 3);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x17;
	array[1] = 0x70;	//17

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x19;
	array[1] = 0x40;	//19
	array[2] = 0x61;	//20

	res = WriteSeq(DemodAddr, &array[0], 3);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x3A;
	array[1] = 0xB;		//3A
	array[2] = 1;		//3B

	res = WriteSeq(DemodAddr, &array[0], 3);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x55;
	array[1] = 0xC;		//55

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x20;
	array[1] = 0x81;	//20

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x31;
	array[1] = 0x56;	//31

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x34;
	array[1] = 0x78;	//34
	array[2] = 0;		//35	(& 0x78) + 0x78;

	res = WriteSeq(DemodAddr, &array[0], 3);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x3F;
	array[1] = 0xA;		//3F
	array[2] = 0x64;	//40
	array[3] = 0x4F;	//41

	res = WriteSeq(DemodAddr, &array[0], 4);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x56;
	array[1] = 0xC4;	//56

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return 0;
	}

	array[0] = 0x57;
	array[1] = 0x8;		//57

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x58;
	array[1] = 0x61;	//58

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x58;
	array[1] = 0x60;	//58

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x3D;
	array[1] = 0x80;	//3D
	array[2] = 0;		//3E

	res = WriteSeq(DemodAddr, &array[0], 3);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	array[0] = 0x36;
	array[1] = 0;		//36

	res = WriteSeq(DemodAddr, &array[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	MCLK = 96000000;

	return 0;
}

int CTunerTDA10086::Initialize(void)
{
	unsigned char lnbp21;
	int res;

	lnbp21 = 0x58;
	res = WriteSeq(0x10, &lnbp21, 1);
	if (res != 0)
	{
		printf("Error: Write to LNBP21 failed!\n");
		return res;
	}

	res = InitTuner();
	if (res != 0)
	{
		printf("Error: InitTuner!\n");
		return res;
	}

	res = InitDemod();
	if (res != 0)
	{
		printf("Error: InitDemod!\n");
		return res;
	}

	return 0;
}

int CTunerTDA10086::GateControl(int enable)
{
	unsigned char	value[2];
	int res;

	value[0] = 0;
	res = CombinedSeq(DemodAddr, &value[0], 1, &Gate, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	if (enable == 0)
		Gate &= 0xEF;
	else
		Gate |= 0x10;

	value[0] = 0;
	value[1] = Gate;

	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	return 0;
}

int CTunerTDA10086::SetPolarity(fe_polarity_t polar)
{
	unsigned char	value;
	int res;

	res = ReadSeq(0x10, &value, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	value &= 0x1F;
	value |= 0x40;

	switch(polar)
	{
	case SEC_POLARITY_H:
			value |= 0x1C;
			break;

	case SEC_POLARITY_V:
			value &= 0xF7;	//~8
			value |= 0x14;
			break;

	default:
		value &= ~4;//0xFB;
	}

	//value &= 0xEF;

	res = WriteSeq(0x10, &value, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	return 0;
}

int CTunerTDA10086::SetFreq(unsigned int tunerFreq)
{
	int res;
	unsigned char	array[11];
	unsigned int	freq;

	freq = tunerFreq;

	if (freq <= 0)
	{
		printf("%s: %d freq <= 0\n", __FUNCTION__, __LINE__);
		return 0;
	}

	TunerData[2] = (freq >> 7) & 0xff;
	TunerData[3] = (TunerData[3] & 1) | (freq << 1) & 0xff;

	array[0] = 0;
	array[1] = TunerData[0];
	array[2] = TunerData[1];
	array[3] = TunerData[2];
	array[4] = TunerData[3];
	array[5] = TunerData[4];
	array[6] = TunerData[5];
	array[7] = TunerData[6];
	array[8] = TunerData[7];
	array[9] = TunerData[8];
	array[10] = TunerData[9];

	res = GateControl(1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	res = WriteSeq(TunerAddr, &array[0], 11);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	res = GateControl(0);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	return 0;
}

int CTunerTDA10086::SetViterbiRate(fe_inner_t inner)
{
	unsigned char value[2];
	int res;

	value[0] = 0xD;
	res = CombinedSeq(DemodAddr, &value[0], 1, &ViterbiRate, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	switch(inner)
	{
	case FEC_1_2:
			ViterbiRate &= 0xF0;
			break;
	case FEC_2_3:
			ViterbiRate &= 0xF1;
			ViterbiRate |= 1;
			break;
	case FEC_3_4:
			ViterbiRate &= 0xF2;
			ViterbiRate |= 2;
			break;
	case FEC_5_6:
			ViterbiRate &= 0xF4;
			ViterbiRate |= 4;
			break;
	case FEC_7_8:
			ViterbiRate &= 0xF6;
			ViterbiRate |= 6;
			break;
	case FEC_AUTO:
	default:
			ViterbiRate |= 8;
	}

	value[1] = ViterbiRate;
	value[0] = 0xD;

	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	return 0;
}

int CTunerTDA10086::SetInversion(void)
{
	unsigned char	value[2];
	int res;

	value[0] = 0xC;
	res = CombinedSeq(DemodAddr, &value[0], 1, &Inversion, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	//Inversion Auto
	Inversion &= 0x7F;

	value[1] = Inversion;
	value[0] = 0xC;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	return 0;
}

int CTunerTDA10086::SetSymbolRate(unsigned int sr)
{
	unsigned char	value[5];

	unsigned char	dfn;
	unsigned char	afs;
	unsigned char	byp;

	unsigned char	reg37;
	unsigned char	reg42;

	unsigned int	bdri;
	unsigned int	bdr;

	u_int64_t	big;

	unsigned int	srs;
	unsigned int	sym;

	int res;


	if (sr <= 0)
	{
		printf("%s: %d sr <=0\n", __FUNCTION__, __LINE__);
		return 0;
	}

	srs = sr * 1000;
	sym = (MCLK * 10) / srs;


		byp = 0;
		dfn = 4; afs = 1;	//0x84

		reg37 = 0x43;
		reg42 = 0x43;

	if (sym < 720)
	{
		dfn = 4; afs = 0;	//0x4
	}
	if (sym < 480)
	{
		dfn = 3; afs = 1;	//0x83
	}
	if (sym < 360)
	{
		dfn = 3; afs = 0;	//0x3
	}
	if (sym < 240)
	{
		dfn = 2; afs = 1;	//0x82
	}
	if (sym < 180)
	{
		dfn = 2; afs = 0;	//0x2
	}
	if (sym < 120)
	{
		dfn = 1; afs = 1;	//0x81
	}
	if (sym < 90)
	{
		dfn = 1; afs = 0;	//0x1
	}
	if (sym < 60)
	{
		dfn = 0; afs = 1;	//0x80
	}
	if (sym < 45)
	{
		dfn = 0; afs = 0;	//0x0
	}
	if (sym < 30)
	{
		byp = 1;
		dfn = 0; afs = 0;	//0x0

		reg37 = 0x63;
		reg42 = 0x4F;
	}

	value[1] = reg37;
	value[0] = 0x37;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	value[1] = reg42;
	value[0] = 0x42;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}


	value[1] = (afs << 7) | dfn;
	value[0] = 0x21;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}


	value[1] = 0x81 | (byp << 3);
	value[0] = 0x20;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	big = 1ULL;
	big <<= dfn;
	big *= sr;
	big *= 20971520000ULL;
	big /= MCLK;
	big += 5ULL;
	big /= 10ULL;

	bdr = big;

	bdri = MCLK * 32;
	bdri >>= dfn;
	bdri += (srs / 2);
	bdri /= srs;

	//printf("sym %lu sr(%lu %lu)\n", sym, bdr, bdri);

	value[4] = (bdri) & 0xff;
	value[3] = (bdr >> 16) & 0xff;
	value[2] = (bdr >> 8) & 0xff;
	value[1] = (bdr) & 0xff;
	value[0] = 0x6;
	res = WriteSeq(DemodAddr, &value[0], 5);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	return 0;
}

int CTunerTDA10086::SetTone(fe_tone_t tone)
{
	unsigned char	value[2];
	int res;

	value[0] = 0x50;
	res = CombinedSeq(DemodAddr, &value[0], 1, &Wait, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	if ((Wait & 1) == 0)
	{
		printf("%s: wait!\n", __FUNCTION__);
		//printf("Info: Previous DiSEqC message not finished");
		return 7;
	}

	Tone &= 1;

	switch (tone)
	{
	case SEC_TONE_ON:
			Tone |= 1;
			break;
	case SEC_TONE_OFF:
	default:
			Tone &= ~1;	//fe
	}

	value[1] = Tone;
	value[0] = 0x36;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	return 0;
}

int CTunerTDA10086::SetLNB(int no, fe_polarity_t polar, fe_tone_t tone)
{
	unsigned int	DiSEqC;
	int res;

	res = SetPolarity(polar);

	DiSEqC = 0xE01038F0;

	switch (tone)
	{
	case SEC_TONE_ON:
			DiSEqC |= 1;
			break;
	case SEC_TONE_OFF:
			DiSEqC &= ~1;
			break;
	default:;
	}

	switch (polar)
	{
	case SEC_POLARITY_H:
			DiSEqC |= 2;
			break;
	case SEC_POLARITY_V:
			DiSEqC &= ~2;
			break;
	default:;
	}

	DiSEqC |= (no & 0x4) * 4;

//	res = DiSEqCMsg(0, 4, DiSEqC);

	Sleep(100);

	res = SetTone(tone);

	return 0;
}

int CTunerTDA10086::SetTuner(unsigned int freq, unsigned int sr, fe_inner_t inner)
{
	int res;
	unsigned char	value[2];

	value[1] = 0x35;
	value[0] = 0x2;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: error!\n", __FUNCTION__);
	}

	Lock = 0;

	res = SetInversion();
	res = SetViterbiRate(inner);

	Sleep(10);

	res = SetSymbolRate(sr);

	res = UpdateGain();
	res = SetFreq(freq);
	res = UpdateGain();


	return 0;
}

int CTunerTDA10086::GetStatus(fe_status_t *status, unsigned int *ber, unsigned char *signal)
{
	unsigned char	value[2];
	unsigned char	bervalue[3];
	unsigned char	level;
	unsigned int	strength;
	unsigned int	lber;
	int res;

	value[0] = 0xE0;
	res = CombinedSeq(DemodAddr, &value[0], 1, &Status, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	printf("Status %x\n", Status);

	*(int *)status = 0;

	if (Status & 0x01)
		*(int *)status |= FE_SIGNAL;

	if (Status & 0x02)
		*(int *)status |= FE_CARRIER;

	if (Status & 0x04)
		*(int *)status |= FE_VITERBI;

	if (Status & 0x08)
		*(int *)status |= FE_SYNC;

	if (Status & 0x10)
	{
		*(int *)status |= FE_LOCK;

		if (Lock == 0)
		{
			Lock = 1;
			value[1] = 0;
			value[0] = 0x2;
			res = WriteSeq(DemodAddr, &value[0], 2);
			if (res != 0)
			{
				printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
				//return res;
			}
		}

	}

	value[0] = 0x43;
	res = CombinedSeq(DemodAddr, &value[0], 1, &level, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	strength = 255 - level;
	strength = (strength * 100) / 255;
	strength *= 14;

	if (strength > 1000)
		*signal = 100;
	else
		*signal = strength/10;


	value[0] = 0x15;
	res = CombinedSeq(DemodAddr, &value[0], 1, &bervalue[0], 3);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	lber = (bervalue[2] & 0xf);
	lber <<= 8;
	lber |= bervalue[1];
	lber <<= 8;
	lber |= bervalue[0];

	*ber = (lber);


	return 0;
}

int CTunerTDA10086::UpdateGain(void)
{
	int res;
	unsigned char	value[3];
	unsigned char	step;

	value[0] = 0x3D;
	value[1] = 0x80;
	value[2] = 0;
	res = WriteSeq(DemodAddr, &value[0], 3);	//FreqOff Hz 0
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	value[0] = 1;
	value[1] = 0x94;
	res = WriteSeq(DemodAddr, &value[0], 2);	//??
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	value[0] = 0;
	value[1] = 0;
	res = WriteSeq(DemodAddr, &value[0], 2);	//Reset
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	value[0] = 0xE;
	res = CombinedSeq(DemodAddr, &value[0], 1, &Status, 1);	//Status
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	do {
		value[0] = 0x43;
		res = CombinedSeq(DemodAddr, &value[0], 1, &Signal, 1);	//Signal
		if (res != 0)
		{
			printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
			return res;
		}

		value[0] = 0x3F;
		res = CombinedSeq(DemodAddr, &value[0], 1, &AGC, 1);	//AGC
		if (res != 0)
		{
			printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
			return res;
		}

		printf("Gain: Signal %d, AGC %d\n", Signal, AGC & 0x1F);

		if (Signal > 192 && (AGC & 0x1F) > 2)
			step = AGC - 1;
		else
		if (Signal < 17  && (AGC & 0x1F) < 29)
			step = AGC + 1;
		else
			break;

		value[0] = 0x3F;
		value[1] = AGC ^ (step & 0x1F);
		res = WriteSeq(DemodAddr, &value[0], 2);
		if (res != 0)
		{
			printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
			return res;
		}

		Sleep(20);

		value[0] = 0;
		value[1] = 0;
		res = WriteSeq(DemodAddr, &value[0], 2);	//Reset
		if (res != 0)
		{
			printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
			return res;
		}
		
		Sleep(50);

	} while(1);

	if (! (Status & 0x10) )
	{
		value[0] = 0x3D;
		value[1] = 0x80;
		value[2] = 0x5F;
		res = WriteSeq(DemodAddr, &value[0], 3);	//FreqOff Hz 95
		if (res != 0)
		{
			printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
			return res;
		}

		value[0] = 0;
		value[1] = 0;
		res = WriteSeq(DemodAddr, &value[0], 2);	//Reset
		if (res != 0)
		{
			printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
			return res;
		}

	}

	return 0;
}

int CTunerTDA10086::DiSEqCMsg(int a, int len, unsigned int cmd)
{
	int res;
	unsigned char	value[5];
	unsigned char	ucOldDiSEqC;

	if (len > 4)
	{
		printf("%s: >4\n", __FUNCTION__);
		return 4;
	}
	
	printf("%s: cmd %04X len %d\n", __FUNCTION__, cmd, len);

	value[0] = 0x50;
	res = CombinedSeq(DemodAddr, &value[0], 1, &Wait, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	if ((Wait & 1) == 0)
	{
		printf("%s: wait!\n", __FUNCTION__);
		//printf("Info: Previous DiSEqC message not finished");
		return 7;
	}


	value[0] = 0x36;
	res = CombinedSeq(DemodAddr, &value[0], 1, &Tone, 1);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	ucOldDiSEqC = Tone;

	Tone &= ~1;//0xFE;

	value[1] = Tone;
	value[0] = 0x36;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(20);

	//..1
	value[4] = cmd;
	value[3] = cmd >> 8;
	value[2] = cmd >> 16;
	value[1] = (cmd >> 24);
	value[0] = 0x48;
	res = WriteSeq(DemodAddr, &value[0], 5);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Tone = (Tone & 0xBB) | 0x38;
	value[1] = Tone;
	value[0] = 0x36;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(100);
	//..-1

	switch (a)
	{
	case 1:	//MINI_A
		Tone = (Tone & 0xFD) | 4;
		break;
	case 2:	//MINI_B
		Tone |= 6;
		break;
	default:
		Tone &= 0xFB;
	}

	//..1
	value[4] = cmd;
	value[3] = cmd >> 8;
	value[2] = cmd >> 16;
	value[1] = (cmd >> 24) | 1;
	value[0] = 0x48;
	res = WriteSeq(DemodAddr, &value[0], 5);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Tone = (Tone & 0xBF) | 0x38;
	value[1] = Tone;
	value[0] = 0x36;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(100);
	//..-1

	//.r
	value[1] = ucOldDiSEqC;
	value[0] = 0x36;
	res = WriteSeq(DemodAddr, &value[0], 2);
	if (res != 0)
	{
		printf("%s: %d %d\n", __FUNCTION__, __LINE__, res);
		return res;
	}

	Sleep(50);

	return 0;
}

