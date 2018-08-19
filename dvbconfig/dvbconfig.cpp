
/*
 */

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "getopt.h"

#include "dvbapi.h"

#include "dvblib.h"

#include "readconfig.h"
#include "str2mac.h"
#include "str2pid.h"
#include "long2str.h"
#include "feutil.h"

void
usage(void)
{
	fprintf(stderr,
		"usage:\n"
		"\t dvbconfig -l -c[file]\n"
		"\t dvbconfig dvb[num]\n"
		"\t dvbconfig dvb[num] -c[file] -n[num]\n"
		"\t dvbconfig dvb[num] -p[pid] -m[mac]\n"
		"\t dvbconfig dvb[num] -d[num]\n");

	exit(1);
}

static struct option longopt[] = {
	{ "?",		no_argument, NULL, 'h'},
	{ "help",	no_argument, NULL, 'h'},

	{ "config",	required_argument, NULL, 'c'},
	{ "list",	no_argument, NULL, 'l'},
	{ "num",	required_argument, NULL, 'n'},

	{ "p",		required_argument, NULL, 'p'},
	{ "v",		required_argument, NULL, 'v'},
	{ "a",		required_argument, NULL, 'a'},

	{ "delete",	required_argument, NULL, 'd'},

	{ "mu",		required_argument, NULL, 'm'},
	{ "mp",		required_argument, NULL, 'm'},

	{ NULL, NULL, NULL, NULL},
};

int
main(int argc, char **argv)
{
	int nfopt = -1;

	char cfgfile[256];
	int chan_list = 0, cfile = 0;
	int chan_no = 0;

	int ifnum = -1;
	int pid = -1;
	int filterNo = -1;

	int bMac = -1;
	unsigned char	mac[6];
	int mfilter = -1;

	struct dvbapi parm;

	int res = 0;
	int i;
	
	CDVBDevice	dvbdev;	//

	memset(&mac[0], 0, sizeof(mac));

	if (argc > 1 && strncmp(argv[1], "dvb", 3) == 0)
	{
		ifnum = strtoul(argv[1]+3, NULL, 0);

		argc--;
		argv++;
	}
	
	while ((nfopt = getopt_long_only(argc, argv, "", longopt, NULL)) != -1)
	{
		switch (nfopt)
		{
		case 'l':
			chan_list = 1;
			break;

		case 'c':
			cfile = 1;
			strncpy(cfgfile, optarg, sizeof(cfgfile));
			break;

		case 'n':
			chan_no = strtoul(optarg, NULL, 0);
			break;

		case 'a':
			/* audio */
			str2pid(optarg, &pid);
			break;

		case 'p':
			/* mpe */
			str2pid(optarg, &pid);
			break;

		case 'v':
			/* video */
			str2pid(optarg, &pid);
			break;

		case 'd':
			/* filter */
			filterNo = strtoul(optarg, NULL, 0);
			break;

		case 'm':
			/* mac addr */
			if (str2mac(&mac[0], optarg) == 1)
			{
				bMac = 1;
			}

			if (strcmp(argv[optind-2], "-mu") == 0)
			{
				//UNICAST
				mfilter = MASK_UNICAST;
			} else
			if (strcmp(argv[optind-2], "-mp") == 0)
			{
				//PROMISC
				mfilter = MASK_PROMISC;
			}
			break;

		case '?':
		case 'h':
		default:
			usage();
			;
		}

	}

	argc -= optind;
	argv += optind;

	if (ifnum == -1 && chan_list != 1)
		usage();

	if (ifnum != -1)
	{
		res = dvbdev.Open(ifnum);
		if (res != 1)
			return 1;
	}


	if (pid != -1 && filterNo == -1)
	{
		res = dvbdev.AddFilter(pid, &mac[0], mfilter);
		if (res != 0)
			printf("AddFilter: error %d\n", res);	
	}

	if (pid == -1 && filterNo != -1)
	{
		res = dvbdev.DelFilter(filterNo);
		if (res != 0)
			printf("DelFilter: error %d\n", res);	
	}

	if (cfile == 0 && pid == -1 && filterNo == -1)
	{
		printf("dvb%d: ", ifnum);

		unsigned int status;
		unsigned int fullness;
		//.
		res = dvbdev.GetDMA(&status, &fullness);

		printf("flags=<");
			if (res != -1)
			{
				printf("DMA:");
					if (status)
					{
						printf("START");
					} else
					{
						printf("STOP");
					}
			} else
			{
				printf("error");
			}
		printf(">");
			printf(" fullness %lu%%", fullness);
		printf("\n");

		unsigned int idSubDevice;
		unsigned char MACAddress[6];
		//.
		res = dvbdev.GetCARD(&idSubDevice, &MACAddress[0]);

		printf("        ");
			printf("MAC: ");
			if (res != -1)
			{
				printf("%02x:%02x:%02x:%02x:%02x:%02x",
					MACAddress[0],
					MACAddress[1],
					MACAddress[2],
					MACAddress[3],
					MACAddress[4],
					MACAddress[5]);

			} else
			{
				printf("error");
			}
		printf("\n");

		struct tuner state;
		//.
		memset(&state, 0, sizeof(state));
		res = dvbdev.ReadTuner(&state);

		printf("        ");
			printf("LNB: ");
			if (res != -1)
			{
				printf("%d <Polar:%s, LOF:%d MHz, 22KHz:%s>",
					state.param.lnb.no,
					decodePolar(state.param.lnb.polar),
					state.param.lnb.freq,
					decodeTone(state.param.lnb.tone));
			} else
			{
				printf("error");
			}
		printf("\n");

		printf("        ");
			printf("Tuner: ");
			if (res != -1)
			{
				printf("<Freq:%d MHz, SR:%d kS/s, FEC:%s>",
					state.param.tuner.freq,
					state.param.tuner.sr,
					decodeFEC(state.param.tuner.inner));
			} else
			{
				printf("error");
			}
		printf("\n");

		printf("        ");
			printf("Signal: ");
			if (res != -1)
			{
				printf("%d%%", state.signal);
					printf(" [");
					for (i = 0; i < (100/2); i++)
						if (i < (state.signal/2))
							printf("#");
						else	printf(".");
					printf("]");
			} else
			{
				printf("error");
			}
		printf("\n");

		printf("        ");
			printf("BER: ");
			if (res != -1)
			{
				printf("%e", (float)state.ber);
			} else
			{
				printf("error");
			}
		printf("\n");

		printf("        ");
			printf("Status: ");
			if (res != -1)
			{
				if (state.status & FE_LOCK)
					printf("lock ");
			//	else
			//	{
					if (state.status & FE_SIGNAL)
						printf("signal ");

					if (state.status & FE_CARRIER)
						printf("carrier ");

					if (state.status & FE_VITERBI)
						printf("viterbi ");

					if (state.status & FE_SYNC)
						printf("sync ");

			//	}
			} else
			{
				printf("error");
			}
		printf("\n");

		printf("\n");

		unsigned int TSPkts, BadTSPkts, DisCont;

		//.
		res = dvbdev.GetDemux(&TSPkts, &BadTSPkts, &DisCont);

		printf("        ");
			printf("Demux: ");
			if (res != -1)
			{
				printf("TSPkts %s, ", long2str(TSPkts));
				printf("BadTSPkts %s, ", long2str(BadTSPkts));
				printf("DisCont %s", long2str(DisCont));
			} else
			{
				printf("error");
			}
		printf("\n");


		for (i = 1; i < 256; i++)
		{
			unsigned short pid;
			unsigned char mac[6];
			u_int64_t bytes;
			unsigned int errors;
			unsigned int mant;

			//.
			res = dvbdev.GetFilter(i, &pid, &mac[0], &bytes, &errors);

			if (res != -1)
			{
				if (i == 1)
				{
					printf("        ");
					printf("------------------------------------------------------------------------");
					printf("\n");

					printf("        ");
					printf("|%-3s|%+7s|%5s|%18s|%20s|%12s|",
						"N", "Pid:", "Type:", "MAC:", "Bytes:", "Errors:");
					printf("\n");

					printf("        ");
					printf("------------------------------------------------------------------------");
					printf("\n");
				}

				//.
				printf("        ");
				printf("|%-3d", i);
				printf("| 0x%04x", pid);
				printf("|%5s", "UN");
				printf("|%18s", "xx:xx:xx:xx:xx:xx");

				printf("|%20s", long2str(bytes));

				printf("|%12s|", long2str(errors));
				printf("\n");
			}

		}

	} else
	if (cfile != 0)
	{
		memset(&parm, 0, sizeof(parm));
		parm.code = IOTUNES;

		res = readconfig(cfgfile, chan_list, chan_no, &parm.u.param);
		if (res == 0)
		{
			return 1;
		}

		res = dvbdev.CallAPI(&parm);
		//
		printf("tuning...");
		if (res == -1)
			printf("error");
		printf("\n");
	}

	return res;
}
