
/*
 */

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dvbapi.h"

#include "feutil.h"

int
readconfig(const char *filename, int chan_list, int chan_no, struct tunes *parm)
{
	FILE *chf;
	char buf[4096];
	char *field, *tmp, *name;
	unsigned int line, chan_line;

	unsigned int lnbfreq, freq, satno, sr;
	fe_tone_t	hiband;
	fe_polarity_t	polar;
	fe_inner_t	inner;

	line = 0;
	chan_line = 0;
	chf = fopen(filename, "r");
	if (chf == NULL)
	{
		fprintf(stderr, "error open file '%s'\n", filename);
		return 0;
	}

	while (!feof(chf))
	{
		if (fgets(buf, sizeof(buf), chf))
		{
			line ++;

			if (chan_no && chan_no != line)
				continue;

		tmp = buf;

			if (buf[0] == '#' || buf[0] == 10)
				continue;

			chan_line ++;
			/* name */
			if (!(name = strsep(&tmp, ":")))
				goto syntax_err;

			/* freq */
			if (!(field = strsep(&tmp, ":")))
				goto syntax_err;
			freq = strtoul(field, NULL, 0);

			/* polarity */
			if (!(field = strsep(&tmp, ":")))
				goto syntax_err;

			polar = encodePolar(field);

			/* fec */
			if (!(field = strsep(&tmp, ":")))
				goto syntax_err;

			inner = encodeFEC(field);

			/* sat no */
			if (!(field = strsep(&tmp, ":")))
				goto syntax_err;
			satno = strtoul(field, NULL, 0);

			/* sr */
			if (!(field = strsep(&tmp, ":")))
				goto syntax_err;
			sr = strtoul(field, NULL, 0);

			if (chan_list)
			{
				if (chan_line == 1)
				printf("%-s %+20s | %+3s | %+9s | %+8s | %+5s | %+8s |\n",
				"Num", "Name", "LNB", "Freq MHz", "Polar", "FEC", "SR kS/s");
				printf("%03u ", line);
				printf("%+20s | ", name);
				printf("%+3u | ", satno);
				printf("%+9u | ", freq);
				printf("%+8s | ", decodePolar(polar));
				printf("%+5s | ", decodeFEC(inner));
				printf("%+8u |", sr);
				printf("\n");
				continue;
			}

			fclose(chf);


			if (freq >= 11700)	/* Ku hiBand */
			{
				hiband = SEC_TONE_ON;
				lnbfreq = 10600;
			} else
			if (freq < 5150)	/* C Band */
			{
				hiband = SEC_TONE_OFF;
				lnbfreq = 5150;
			} else
			{			/* Ku lowBand */
				hiband = SEC_TONE_OFF;
				lnbfreq = 9750;
			}

			parm->lnb.no = satno;
			parm->lnb.polar = polar;
			parm->lnb.freq = lnbfreq;
			parm->lnb.tone = hiband;

			parm->tuner.freq = freq;
			parm->tuner.sr = sr;
			parm->tuner.inner = inner;

			return 1;
syntax_err:
			fprintf(stderr,
				"syntax error in line %u: '%s'\n", line, buf);

		} else
		if (ferror(chf))
		{
			fprintf(stderr,
				"error reading channel list '%s': %d %m\n",
							filename, errno);
			fclose(chf);
			return 0;
		} else
		{
			fclose(chf);

			if (chan_list)
				return 0;
			return 1;
		}
	}

	fclose(chf);
	return 1;
}

