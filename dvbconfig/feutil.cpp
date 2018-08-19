
/*
 */

#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "dvbapi.h"
#include "feutil.h"

char *
decodeTone(fe_tone_t tone)
{
	switch(tone)
	{
	case SEC_TONE_OFF:
		return "off";
	case SEC_TONE_ON:
		return "on";
	default:
		return "error";
	}
}

fe_polarity_t
encodePolar(char *polar)
{
	polar[0] = toupper(polar[0]);

	if (strcmp(polar, "H") == 0 || strcmp(polar, "Hor") == 0)
		return SEC_POLARITY_H;
	else
	if (strcmp(polar, "V") == 0 || strcmp(polar, "Ver") == 0)
		return SEC_POLARITY_V;
}

char *
decodePolar(fe_polarity_t polar)
{
	switch(polar)
	{
	case SEC_POLARITY_H:
		return "Hor";
	case SEC_POLARITY_V:
		return "Ver";
	default:
		return "error";
	}
}

fe_inner_t
encodeFEC(char *inner)
{
	if (strcmp(inner, "1/2") == 0)
		return FEC_1_2;
	else
	if (strcmp(inner, "2/3") == 0)
		return FEC_2_3;
	else
	if (strcmp(inner, "3/4") == 0)
		return FEC_3_4;
	else
	if (strcmp(inner, "5/6") == 0)
		return FEC_5_6;
	else
	if (strcmp(inner, "6/7") == 0)
		return FEC_6_7;
	else
	if (strcmp(inner, "7/8") == 0)
		return FEC_7_8;
	else
	if (strcmp(inner, "auto") == 0)
		return FEC_AUTO;
}

char *
decodeFEC(fe_inner_t inner)
{
	switch (inner)
	{
	case FEC_1_2:
		return "1/2";
	case FEC_2_3:
		return "2/3";
	case FEC_3_4:
		return "3/4";
	case FEC_5_6:
		return "5/6";
	case FEC_6_7:
		return "6/7";
	case FEC_7_8:
		return "7/8";
	case FEC_AUTO:
		return "auto";
	default:
		return "error";
	}
}

