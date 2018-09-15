
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
}

#include "adapter.h"
#include "tsdemux.h"

CTSDemux::CTSDemux(CAdapter *sc):sc(sc)
{
	CHANNEL	*channel;
	FILTER	*filter;
	int i;

	bzero(&Pid2Chan[0], sizeof(Pid2Chan));

	for (i = 0; i < CHANNELS; i++)
	{
		channel = &Channels[i];

		channel->ftype = NO_FILTER_TYPE;
		channel->pid = 0xffff;
		channel->filter.no = 0;
		channel->state = 0;
		channel->cc = 0xff;
		channel->flag = 0;
		channel->seclen = 0;
		channel->len = 0;
		channel->basebuf = 0;
		channel->baselen = 0;
		channel->buf = 0;
		channel->unkn24 = 0;

		channel->unkn48 = 0;
		channel->unkn49 = 0;

		channel->unkn4A = 0xffff;
	}

	for (i = 0; i < FILTERS; i++)
	{
		filter = &Filters[i];

		filter->chanNo = 0;
		filter->scrambl = 0;
		filter->pid = 0xffff;
		filter->macfilter = 0;
		filter->bytes = 0;
	}


	TSPkts = 0;
	BadTSPkts = 0;

	TSErrors = 0;

	DisCont = 0;

	Pids = 0;
}

CTSDemux::~CTSDemux()
{
	EnablePIDCount(0);
	DelAllFilters();
}

void CTSDemux::ResetStat(void)
{
	TSPkts = 0;
	BadTSPkts = 0;
	TSErrors = 0;
	DisCont = 0;

}

unsigned int CTSDemux::GetTSPkts(void)
{
	return TSPkts;
}

unsigned int CTSDemux::GetBadTSPkts(void)
{
	return BadTSPkts;
}

unsigned int CTSDemux::GetTSErrors(void)
{
	return TSErrors;
}

unsigned int CTSDemux::GetDisCont(void)
{
	return DisCont;
}

unsigned char CTSDemux::GetFreeFilter(void)
{
	unsigned int	filterNo;

	for (filterNo = 1; filterNo < FILTERS; filterNo++)
	{
		if (Filters[filterNo].chanNo == 0)
			break;
	}
	return filterNo;
}

unsigned char CTSDemux::GetFreeChannel(void)
{
	unsigned int	chanNo;

	for (chanNo = 1; chanNo < CHANNELS; chanNo++)
	{
		if (Channels[chanNo].ftype == NO_FILTER_TYPE)
			break;
	}
	return chanNo;
}

unsigned int CTSDemux::GetScrambleFlags(unsigned char filterNo)
{
	CHANNEL	*channel;
	unsigned char chanNo;
	unsigned int scrambl;

	if (filterNo == 0 || filterNo >= FILTERS)
		return 0;

	chanNo = Filters[filterNo].chanNo;

	if (chanNo == 0 || chanNo >= CHANNELS)
		return 0;

	channel = &Channels[chanNo];

	scrambl = (channel->unkn48 << 8) | channel->unkn49;

	return (scrambl << 8) | Filters[filterNo].scrambl;
}

unsigned short CTSDemux::GetPid(unsigned char filterNo)
{
	if (filterNo == 0 || filterNo >= FILTERS)
		return 0;

	return Filters[filterNo].pid;
}

u_int64_t CTSDemux::GetBytes(unsigned char filterNo)
{
	if (filterNo == 0 || filterNo >= FILTERS)
		return 0;

	return Filters[filterNo].bytes;
}

unsigned int CTSDemux::GetErrors(unsigned char filterNo)
{
	unsigned short pid;

	if (filterNo == 0 || filterNo >= FILTERS)
		return 0;

	pid = Filters[filterNo].pid;

	if (pid > 8192)
		return 0;

	if (Pids == NULL)
		return 0;

	return Pids->Errors[pid];
}

unsigned int CTSDemux::GetCounts(unsigned char filterNo)
{
	unsigned short pid;

	if (filterNo == 0 || filterNo >= FILTERS)
		return 0;

	pid = Filters[filterNo].pid;

	if (pid > 8192)
		return 0;

	if (Pids == NULL)
		return 0;

	return Pids->Counts[pid];
}

enum TSErrReturn CTSDemux::EnablePIDCount(int enable)
{
	PIDCOUNT	*tmp;
	int i;

	if (enable == 1)
	{
		if (Pids != NULL)
		{
			return ERROR6;
		}

		tmp = new PIDCOUNT;
		if (tmp == NULL)
		{
			return TSNOMEM;
		}

		for (i = 0; i < 8192; i++)
		{
			tmp->CC[i] = 0xff;
			tmp->Errors[i] = 0;
			tmp->Counts[i] = 0;
		}

		Pids = tmp;

		return TSOK;
	}

	if (Pids == NULL)
	{
		return ERROR6;
	}

	delete Pids;
	Pids = 0;

	return TSOK;
}

enum TSErrReturn CTSDemux::AddFilter(unsigned char &fNo, enum FILTERTYPE ftype, unsigned short pid, unsigned char *mac, unsigned char *mask, unsigned int size)
{
	CHANNEL		*channel;
	FILTER		*filter;
	MACFILTER	*macfilter;

	unsigned char	*buffer;

	unsigned char	filterNo;
	unsigned char	chanNo;

	unsigned char	*pChanNo;

	if (pid > 8192)
	{
		return TSEINVAL;	//ERROR1;
	}

	if (size < 184 || size > 1024*1024)
	{
		return TSEINVAL;	//ERROR1;
	}

	filterNo = GetFreeFilter();
	if (filterNo == 0)
	{
		return ERROR2;
	}

	pChanNo = &Pid2Chan[pid];

	if (*pChanNo != 0 && ftype != TS_FILTER)
	{
		chanNo = *pChanNo;
	} else
	{
		chanNo = GetFreeChannel();
	}

	if (chanNo == 0)
	{
		return ERROR2;
	}

	switch(ftype)
	{
	case STREAMING_FILTER:
	case PIPING_FILTER:
	case PES_FILTER:
	case ES_FILTER:
	case PID_FILTER: {
#if 0
			if (*pChanNo != 0)
			{
				return ERROR4;
			}

			buffer = new unsigned char[size];
			if (buffer == NULL)
			{
				return TSNOMEM;
			}

			channel = &Channels[ chanNo ];

			channel->ftype = ftype;
			channel->pid = pid;
			channel->filter.no = filterNo;	//?

			switch(ftype)
			{
			case STREAMING_FILTER:
			case ES_FILTER:
				channel->state = 6;
				break;

			case PES_FILTER:
				channel->state = 1;
				break;

			default:
				channel->state = 0;
			}

			channel->flag = 0;
			channel->seclen = 0;

			channel->len = 0;
			channel->basebuf = buffer;
			channel->baselen = size;
			channel->buf = buffer;
			channel->unkn24 = buffer;

			channel->unkn48 = 0;
			channel->unkn49 = 0;
				
			filter = &Filters[ filterNo ];

			filter->macfilter = 0;

			filter->chanNo = chanNo;
			filter->scrambl = 0;
			filter->pid = pid;
			filter->bytes = 0;

			*pChanNo = chanNo;
#endif

		} break;

	case SECTION_FILTER:
	case MPE_SECTION_FILTER:
	case MULTI_MPE_FILTER: {
			if (mac == NULL || mask == NULL)
			{
				return TSEINVAL;	//ERROR1;
			}

			macfilter = new MACFILTER;
			if (macfilter == NULL)
			{
				return TSNOMEM;
			}

			if (*pChanNo == 0)
			{
				if (ftype == MULTI_MPE_FILTER)
				{
					return ERROR6;
				}

				buffer = new unsigned char[4284];
				if (buffer == NULL)
				{
					delete macfilter;
					return TSNOMEM;
				}

				channel = &Channels[ chanNo ];

				channel->ftype = ftype;
				channel->pid = pid;
				channel->filter.mac = macfilter;
				channel->state = 0xB;

				channel->flag = 0;
				channel->seclen = 0;

				channel->len = 0;
				channel->basebuf = buffer;
				channel->baselen = 4284;
				channel->buf = buffer;
				channel->unkn24 = buffer;

				channel->unkn48 = 0;
				channel->unkn49 = 0;
				channel->unkn4A = 0xffff;

				filter = &Filters[ filterNo ];

				filter->macfilter = macfilter;

				filter->chanNo = chanNo;
				filter->scrambl = 0;
				filter->pid = pid;
				filter->bytes = 0;

				macfilter->filterNo = filterNo;

				bzero(&macfilter->mac[0], 16);
				bzero(&macfilter->mask[0], 16);

				macfilter->mac[0] = 0x3e;
				macfilter->mac[3] = mac[5];
				macfilter->mac[4] = mac[4];
				macfilter->mac[8] = mac[3];
				macfilter->mac[9] = mac[2];
				macfilter->mac[10] = mac[1];
				macfilter->mac[11] = mac[0];

				macfilter->mask[0] = 0xff;
				macfilter->mask[3] = mask[5];
				macfilter->mask[4] = mask[4];
				macfilter->mask[8] = mask[3];
				macfilter->mask[9] = mask[2];
				macfilter->mask[10] = mask[1];
				macfilter->mask[11] = mask[0];

				macfilter->prev = 0;//NULL;
				macfilter->next = 0;//NULL;
				
				*pChanNo = chanNo;

				printf("filter mac = %6D\n", mac, ":");
				printf("filter mask = %6D\n", mask, ":");

			} else
			{


			}
		} break;

	case TS_FILTER: {
			if (Pid2Chan[8192] != 0)
			{
				return ERROR4;
			}

			buffer = new unsigned char[size];
			if (buffer == NULL)
			{
				return TSNOMEM;
			}

			channel = &Channels[ chanNo ];

			channel->ftype = ftype;
			channel->pid = 0xfffe;
			channel->filter.no = filterNo;	//?
			channel->state = 0;

			channel->flag = 0;
			channel->seclen = 0;

			channel->len = 0;
			channel->basebuf = buffer;
			channel->baselen = size;
			channel->buf = buffer;
			channel->unkn24 = buffer;

			channel->unkn48 = 0;
			channel->unkn49 = 0;
			channel->unkn4A = 0xffff;

			filter = &Filters[ filterNo ];

			filter->macfilter = 0;

			filter->chanNo = chanNo;
			filter->scrambl = 0;
			filter->pid = 0xfffe;
			filter->bytes = 0;


			Pid2Chan[8192] = chanNo;
		} break;

	default:
		return TSEINVAL;	//ERROR1;
	}

	fNo = filterNo;

	return TSOK;
}

enum TSErrReturn CTSDemux::DeleteFilter(unsigned char filterNo)
{
	unsigned char	chanNo;
	unsigned short	pid;
	int i;

	if (filterNo == 0 || filterNo >= FILTERS)
		return ERROR3;

	chanNo = Filters[ filterNo ].chanNo;
	if (chanNo == 0)
	{
		return ERROR3;
	}
	
	Filters[ filterNo ].pid = 0xffff;
	pid = Channels[ chanNo ].pid;

	for(i = 1; i < FILTERS; i++)
	{
		if (Filters[i].pid == pid)
			break;
	}

	if (i == FILTERS && Channels[ chanNo ].ftype != MULTI_MPE_FILTER)
	{
		if (Channels[ chanNo ].ftype == MULTI_PID_FILTER)
		{
			for (i = 0; i < 8192; i++)
			{
				if (Pid2Chan[i] == pid)
					Pid2Chan[i] = 0;
			}
		} else
		if (Channels[ chanNo ].ftype == TS_FILTER)
		{
			Pid2Chan[8192] = 0;
		} else
		{
			Pid2Chan[ pid ] = 0;
		}

		DeleteChannel(chanNo);
	}

	DeleteFilterStruct(filterNo);

	return TSOK;
}

void CTSDemux::DelAllFilters(void)
{
	FILTER	*filter;
	CHANNEL	*channel;
	int i;

	for(i = 0; i < FILTERS; i++)
	{
		filter = &Filters[i];

		if (filter->chanNo != 0)
		{
			DeleteFilter(i);
		}
	}

	for(i = 0; i < CHANNELS; i++)
	{
		channel = &Channels[i];

		if (channel->ftype == MULTI_MPE_FILTER)
		{
			if (channel->pid == channel->unkn4A && channel->filter.mac)
				delete channel->filter.mac;

			channel->filter.mac = 0;//NULL;

			Pid2Chan[ channel->pid ] = 0;
			
			DeleteChannel(i);
		}
	}
}

void CTSDemux::DeleteChannel(unsigned char chanNo)
{
	CHANNEL *channel;

	channel = &Channels[ chanNo ];

	delete []channel->basebuf;

	channel->pid = 0xffff;
	channel->unkn4A = 0xffff;

	channel->ftype = NO_FILTER_TYPE;
	channel->state = 0;
	channel->filter.mac = 0;

	channel->basebuf = 0;//NULL;
	channel->baselen = 0;
	channel->unkn24 = 0;//NULL;
	channel->buf = 0;//NULL;

}

void CTSDemux::DeleteFilterStruct(unsigned char filterNo)
{
	FILTER	*filter;
	MACFILTER	*macfilter;
	unsigned char	chanNo;

	filter = &Filters[ filterNo ];

	macfilter = filter->macfilter;

	chanNo = filter->chanNo;

	filter->chanNo = 0;
	filter->pid = 0xffff;
	filter->bytes = 0;

	if (macfilter == NULL)
		return;

	if (macfilter->prev != NULL)
	{
		macfilter->prev->next = macfilter->next;
	} else
	{
		Channels[ chanNo ].filter.mac = macfilter;
	}

	if (macfilter->next != NULL)
	{
		macfilter->next->prev = macfilter->prev;
	}

	delete macfilter;

	filter->macfilter = 0;
}

unsigned char CTSDemux::GetNoOfFilters(void)
{
	unsigned int	filterNo;
	unsigned char	count;

	count = 1;
	for (filterNo = 1; filterNo < FILTERS; filterNo++)
	{
		if (Filters[filterNo].chanNo != 0)
			count++;
	}
	return count;
}

void CTSDemux::DemuxTS(unsigned char *data, unsigned short len)
{
	int i;
	unsigned char *buffer;

	buffer = data;

	if (buffer == NULL || len <= 0)
		return;

	TSPkts += len;

	for (i = 0; i < len; i++, buffer += 188)
	{
			/* Sync && !TEI */
		if (buffer[0] == 0x47 && !(buffer[1] & 0x80))
		{
			unsigned short pid;
			unsigned char chanNo;

			pid = ts_pid(buffer);

			if (Pids != 0)
			{
				unsigned char	cc;
				unsigned char	afc;

				Pids->Counts[pid]++;

				afc = (buffer[3] >> 4) & 3;
				cc = (buffer[3] & 0xf);


				if (afc != 0 && afc != 2 &&
				    (afc != 3 || buffer[4] == 0 ||
				    !(buffer[5] & 0x80) ))
				{
					unsigned char	prevCC;
					unsigned char	nextCC;

					prevCC = Pids->CC[pid];
					nextCC = (prevCC+1) & 0xf;

					if (cc != nextCC &&
					    pid != 8191 &&
					    prevCC != 0xff)
					{
						Pids->Errors[pid]++;
						DisCont++;
					}
				}

				Pids->CC[pid] = cc;
			}

			chanNo = Pid2Chan[pid];
			if (chanNo != 0)
			{
				ParsTSPack(&buffer[0], chanNo);
			}

			continue;
		}
		BadTSPkts ++;
	}
}

void CTSDemux::ParsTSPack(unsigned char *data, unsigned char chanNo)
{
	CHANNEL	*channel;

	unsigned char	nextCC;

	unsigned char	*tmpbuf;
	unsigned short	tmplen;

	channel = &Channels[ chanNo ];

	channel->unkn48 = (data[3] >> 6);

	if (channel->ftype == PID_FILTER ||
	    channel->ftype == MULTI_PID_FILTER ||
	    channel->ftype == TS_FILTER)
	{
		ParsPID(chanNo, &data[0], 188);
		return;
	}

	PUSI = (data[1] >> 6) & 1;
	TP = (data[1] >> 5) & 1;

	SC = (data[3] >> 6) & 3;
	AFC = (data[3] >> 4) & 3;

	CC = (data[3] & 0xf);	

	nextCC = (channel->cc + 1) & 0xf;
	if (CC == nextCC || channel->cc == 0xff)
	{
		Error = 0;
		channel->cc = CC;
	} else
	{
		Error = 1;
		channel->cc = CC;

		TSErrors ++;
	}

	Payload = 0;

	if (AFC & 2)
	{
		Payload = data[4] + 1;
	}

	if (! (AFC & 1) )
	{
		return;
	}

	tmplen = 184 - Payload;
	tmpbuf = &data[ Payload + 4 ];

	switch(channel->ftype)
	{
	case STREAMING_FILTER:
	case ES_FILTER:
		ParsES(chanNo, &tmpbuf[0], tmplen);
		break;

	case PIPING_FILTER:
		ParsPipe(chanNo, &tmpbuf[0], tmplen);
		break;

	case PES_FILTER:
		ParsPES(chanNo, &tmpbuf[0], tmplen);
		break;

	case SECTION_FILTER:
	case MPE_SECTION_FILTER:
	case MULTI_MPE_FILTER:
		ParsSec(chanNo, &tmpbuf[0], tmplen);
		break;

	default:
		;
	}
}

void CTSDemux::ParsPES(unsigned char chanNo, unsigned char *data, unsigned short len)
{
#if 0
	CHANNEL	*channel;

	unsigned char	*tmpbuf;
	unsigned short	tmplen;

	channel = &Channels[ chanNo ];

	if (Error != 0)	//
	{
		FlushPESPack(chanNo);
	}

	if (PUSI != 0)	//
	{
		if (channel->state == 1)
		{
			channel->len = 0;
			channel->buf = channel->basebuf;
		} else
		if (channel->state == 5)
		{
			NotifyPack(chanNo, &channel->basebuf[0], channel->len, channel->flag);

			channel->flag = 0;
			channel->state = 1;
			channel->buf = channel->basebuf;
			channel->len = 0;
		} else
		{
			FlushPESPack(chanNo);
		}
	} else
	if (channel->state == 1)
		return;

	if (channel->buf + len > channel->basebuf + channel->baselen)
	{
		NotifyPack(chanNo, &channel->basebuf[0], channel->len, channel->flag);

		channel->seclen -= channel->len;
		channel->len = 0;
		channel->buf = channel->basebuf;
		channel->flag = 0;
	}

	memcpy(&channel->buf[0], &data[0], len);
	channel->buf += len;
	channel->len += len;

	tmpbuf = channel->basebuf;
	tmplen = channel->len;

	switch(channel->state)
	{
	case 1:
		if (tmplen < 3)
			break;

		if (tmpbuf[0] != 0 ||
		    tmpbuf[1] != 0 ||
		    tmpbuf[2] != 1)
		{
			FlushPESPack(chanNo);
			break;
		}

		channel->flag = 1;
		channel->state = 2;

	case 2:
		if (tmplen < 6)
			break;

		channel->state = 3;
		channel->seclen = pes_length(tmpbuf);	//((tmpbuf[4] << 8) | tmpbuf[5]) + 6;

	case 3:
		if (tmplen < 9)
			break;

		channel->unkn49 = (tmpbuf[6] >> 4) & 3;	//scramble

		if (channel->seclen == 6)
		{
			channel->state = 5;
			break;
		} else
		{
			channel->state = 4;
		}
	case 4:
		if (tmplen < channel->seclen)
			break;

		if (tmplen == channel->seclen)
			NotifyPack(chanNo, &tmpbuf[0], tmplen, channel->flag);
		else
			FlushPESPack(chanNo);

		channel->flag = 0;
		channel->state = 1;
		channel->buf = channel->basebuf;

		break;

	default:
		break;
	}
#endif
}

void CTSDemux::FlushPESPack(unsigned char chanNo)
{
	CHANNEL	*channel;

	channel = &Channels[ chanNo ];

	channel->state = 1;
	channel->buf = channel->basebuf;
	channel->len = 0;
	channel->seclen = 0;
	channel->flag = 0;
}

void CTSDemux::ParsES(unsigned char chanNo, unsigned char *data, unsigned short len)
{
#if 0
	CHANNEL	*channel;

	unsigned char	*tmpbuf;
	unsigned short	tmplen, tmp1;

	channel = &Channels[ chanNo ];

	if (Error != 0)	//
	{
		FlushESPack(chanNo);
	}

	if (PUSI != 0)	//
	{
		if (channel->state == 6)
		{
			channel->len = 0;
			channel->buf = channel->basebuf;
		} else
		if (channel->state == 10)
		{
			NotifyPack(chanNo, &channel->basebuf[0], channel->len, channel->flag);

			channel->flag = 0;
			channel->state = 6;
			channel->buf = channel->basebuf;
			channel->len = 0;
		} else
		{
			FlushESPack(chanNo);
		}
	} else
	if (channel->state == 6)
		return;

	if (channel->buf + len > channel->basebuf + channel->baselen)
	{
		NotifyPack(chanNo, &channel->basebuf[0], channel->len, channel->flag);

		channel->seclen -= channel->len;
		channel->len = 0;
		channel->buf = channel->basebuf;
		channel->flag = 0;
	}

	memcpy(&channel->buf[0], &data[0], len);
	channel->buf += len;
	channel->len += len;

	tmpbuf = channel->basebuf;
	tmplen = channel->len;

	switch(channel->state)
	{
	case 6:
		if (tmplen < 3)
			break;

		if (tmpbuf[0] != 0 ||
		    tmpbuf[1] != 0 ||
		    tmpbuf[2] != 1)
		{
			FlushESPack(chanNo);
			break;
		}

		channel->flag = 1;
		channel->state = 7;

	case 7:
		if (tmplen < 6)
			break;

		channel->state = 8;
		channel->seclen = pes_length(tmpbuf);	//((tmpbuf[4] << 8) | tmpbuf[5]) + 6;

	case 8:
		if (tmplen < 9)
			break;

		channel->unkn49 = (tmpbuf[6] >> 4) & 3;	//scramble

		tmp1 = (tmpbuf[8] + 9);

		if (tmplen < tmp1)
			break;

		if (channel->seclen == 6)
		{
			channel->seclen = 0;
			channel->len = tmplen - tmp1;

			memmove(&tmpbuf[0], &tmpbuf[ tmp1 ], channel->len);

			channel->buf -= tmp1;
			channel->state = 10;
			break;
		} else
		{
			channel->seclen = tmp1;
			channel->len = tmplen - tmp1;

			memmove(&tmpbuf[0], &tmpbuf[ tmp1 ], channel->len);

			channel->buf -= tmp1;
			channel->state = 9;
		}
	case 9:
		if (channel->len < channel->seclen)
			break;

		if (channel->len == channel->seclen)
			NotifyPack(chanNo, &tmpbuf[0], channel->len, channel->flag);
		else
			FlushESPack(chanNo);

		channel->flag = 0;
		channel->state = 6;
		channel->buf = channel->basebuf;

		break;

	default:
		break;
	}
#endif
}

void CTSDemux::FlushESPack(unsigned char chanNo)
{
	CHANNEL	*channel;

	channel = &Channels[ chanNo ];

	channel->state = 6;
	channel->buf = channel->basebuf;
	channel->len = 0;
	channel->seclen = 0;
	channel->flag = 0;
}

void CTSDemux::ParsPID(unsigned char chanNo, unsigned char *data, unsigned short len)
{
	CHANNEL	*channel;

	channel = &Channels[ chanNo ];

	if (channel->buf + len > channel->basebuf + channel->baselen)
	{
		NotifyPack(chanNo, &channel->basebuf[0], channel->len, 0);
		channel->len = 0;
		channel->buf = channel->basebuf;
	}

	memcpy(&channel->buf[0], &data[0], len);

	channel->buf += len;
	channel->len += len;
}

void CTSDemux::ParsSec(unsigned char chanNo, unsigned char *data, unsigned short len)
{
	CHANNEL	*channel;
	unsigned char	pusilen;

	unsigned char	*tmpbuf;
	unsigned short	tmplen, tmpmax;

	channel = &Channels[ chanNo ];

	if (Error != 0)	//
	{
		FlushSecPack(chanNo);
	}

	if (PUSI != 0)	//
	{
		pusilen = data[0];

		data ++;
		len --;

		if (channel->state == 0xB)
		{
				data += pusilen;
				len -= pusilen;

				channel->len = 0;
				channel->buf = channel->basebuf;
		} else
		if (channel->state != 0xC)
		{
			if (channel->seclen - channel->len != pusilen)
			{
				FlushSecPack(chanNo);

				data += pusilen;
				len -= pusilen;
			}
		}
	} else
	if (channel->state == 0xB)
	{
		return;
	}

	tmpbuf = channel->basebuf;

	if (channel->buf + len > channel->basebuf + channel->baselen)
	{
		FlushSecPack(chanNo);
		return;
	}

	while (len > 0)
	{
		switch(channel->state)
		{
		case 0xB:

			channel->flag = 1;
			channel->state = 0xC;

		case 0xC:

			tmplen = 3 - channel->len;
			if (tmplen >= len)
			{
				tmplen = len;
			}
			if (tmplen != 0)
			{
				if (channel->buf + tmplen > channel->basebuf + channel->baselen)
				{
					FlushSecPack(chanNo);
					return;
				}

				memcpy(&channel->buf[0], &data[0], tmplen);

				data += tmplen;
				len -= tmplen;

				channel->buf += tmplen;
				channel->len += tmplen;
			}

			if (channel->len != 3)
				break;

			channel->state = 0xD;

			channel->seclen = sec_length(tmpbuf);

			if (len == 0)
			{
				return;
			}

		case 0xD:

			tmplen = channel->seclen - channel->len;
			tmpmax = 16 - channel->len;

			if (tmplen >= tmpmax)
			{
				tmplen = tmpmax;
			}

			if (tmplen >= len)
			{
				tmplen = len;
			}
			if (tmplen != 0)
			{
				if (channel->buf + tmplen > channel->basebuf + channel->baselen)
				{
					FlushSecPack(chanNo);
					return;
				}

				memcpy(&channel->buf[0], &data[0], tmplen);

				data += tmplen;
				len -= tmplen;

				channel->buf += tmplen;
				channel->len += tmplen;
			}

			if (channel->len > 16)
			{
				FlushSecPack(chanNo);
			}

			if (channel->seclen <= 16 && channel->len == channel->seclen)
			{
				if (CompareSecFilter(chanNo, &tmpbuf[0]) != 0)
				{
					NotifyPack(chanNo, &tmpbuf[0], channel->seclen, channel->flag);

					FlushSecPack(chanNo);

					if (len == 0 || data[0] == 0xff)
					{
						return;
					}

					break;
				}
			}
			
			if (channel->len == 16)
			{
				channel->state = 0xf - (CompareSecFilter(chanNo, &tmpbuf[0]) != 0);
			}

			if (len == 0)
			{
				return;
			}

		case 0xE:

			if (channel->state != 0xf)
			{
				if (channel->len > channel->seclen)
				{
					FlushSecPack(chanNo);
					return;
				}

				tmplen = channel->seclen - channel->len;

				if (tmplen >= len)
				{
					tmplen = len;
				}
				if (tmplen == 0)
				{
					FlushSecPack(chanNo);
					return;
				}
				if (channel->buf + tmplen > channel->basebuf + channel->baselen)
				{
					FlushSecPack(chanNo);
					return;
				}

				memcpy(&channel->buf[0], &data[0], tmplen);

				data += tmplen;
				len -= tmplen;

				channel->buf += tmplen;
				channel->len += tmplen;

				if (channel->len != channel->seclen)
					break;

				NotifyPack(chanNo, &tmpbuf[0], channel->seclen, channel->flag);
			} else

		case 0xF:

			{
				tmplen = channel->seclen - channel->len;
				if (tmplen >= len)
				{
					tmplen = len;
				}

				if (tmplen == 0)
				{
					FlushSecPack(chanNo);
					return;
				}

				data += tmplen;
				len -= tmplen;

				channel->buf += tmplen;
				channel->len += tmplen;

				if (channel->len != channel->seclen)
					break;
			} 

			FlushSecPack(chanNo);

			if (len == 0 || data[0] == 0xff)
			{
				return;
			}

			break;
		default:

			FlushSecPack(chanNo);
			break;
		}

	}
	//
}

unsigned char CTSDemux::CompareSecFilter(unsigned char chanNo, unsigned char *data)
{
	CHANNEL	*channel;
	MACFILTER	*macfilter;
	unsigned char	res;
	int pos;

	unsigned int	*bitfilter;

	unsigned int	*tsdata;
	unsigned int	*mac;
	unsigned int	*mask;

	res = 0;
	channel = &Channels[ chanNo ];

	bzero(&channel->bitfilter[0], sizeof(channel->bitfilter));
//
	macfilter = channel->filter.mac;

	do {
		tsdata = (unsigned int *)data;
		mac = (unsigned int *)&macfilter->mac[0];
		mask = (unsigned int *)&macfilter->mask[0];

		if ( ( ( (tsdata[0] ^ mac[0]) & mask[0]) == 0) &&
		     ( ( (tsdata[1] ^ mac[1]) & mask[1]) == 0) &&
		     ( ( (tsdata[2] ^ mac[2]) & mask[2]) == 0) &&
		     ( ( (tsdata[3] ^ mac[3]) & mask[3]) == 0) )
		{
			pos = macfilter->filterNo - 1;

			bitfilter = &channel->bitfilter[0];
			
			bitfilter[ pos >> 5 ] |= (1 << (pos & 0x1f));
			res ++;
		}

	} while (macfilter = macfilter->next);

	return res;
}

void CTSDemux::FlushSecPack(unsigned char chanNo)
{
	CHANNEL	*channel;

	channel = &Channels[ chanNo ];

	channel->state = 0xB;
	channel->buf = channel->basebuf;
	channel->len = 0;
	channel->seclen = 0;
	channel->flag = 0;
}

void CTSDemux::ParsPipe(unsigned char chanNo, unsigned char *data, unsigned short len)
{
	CHANNEL	*channel;

	channel = &Channels[ chanNo ];

	if (PUSI != 0)
	{
		NotifyPack(chanNo, &channel->basebuf[0], channel->len, channel->flag);
		channel->len = 0;
		channel->flag = 1;
		channel->buf = channel->basebuf;
	}

	if (channel->buf + len > channel->basebuf + channel->baselen)
	{
		NotifyPack(chanNo, &channel->basebuf[0], channel->len, channel->flag);
		channel->len = 0;
		channel->flag = 0;
		channel->buf = channel->basebuf;
	}

	if (Error != 0)
	{
		channel->flag |= 0x80;
	}

	memcpy(&channel->buf[0], &data[0], len);

	channel->buf += len;
	channel->len += len;
}

int CTSDemux::NotifyPack(unsigned char chanNo, unsigned char *data, unsigned int len, unsigned char flag)
{
	unsigned char ftype;
	unsigned char filterNo;

	ftype = Channels[chanNo].ftype;

	if (ftype == NO_FILTER_TYPE)
	{
		return 0;
	}

	if (ftype == SECTION_FILTER ||
	    ftype == MPE_SECTION_FILTER ||
	    ftype == MULTI_MPE_FILTER)
	{
		CHANNEL *channel;
		MACFILTER *macfilter;
		int pos;

		channel = &Channels[chanNo];
		macfilter = channel->filter.mac;

		if (macfilter == 0)
		{
			return 0;
		}

		do {
			filterNo = macfilter->filterNo;
			pos = filterNo - 1;

			if (channel->bitfilter[pos >> 5] & (1 << (pos & 0x1f)) )
			{
				sc->TSCallback(ftype, filterNo, &data[0], len, flag);
				Filters[filterNo].bytes += len;

				if (len >= 6 && data[0] == 0x3e)
				{
					Filters[filterNo].scrambl = (data[5] >> 2) & 0xf;
				}
			}

		} while (macfilter = macfilter->next);

	} else
	{
		filterNo = Channels[chanNo].filter.no;
		if (filterNo == 0)
		{
			return 0;
		}

		sc->TSCallback(ftype, filterNo, &data[0], len, flag);
		Filters[filterNo].bytes += len;
	}
	return 0;
}
