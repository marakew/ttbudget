
/*
 */

#include <sys/time.h>

#include "timeout.h"

void SetTimeOut(struct timeval *tv, unsigned int ms)
{
	getmicrotime(tv);
	tv->tv_usec += ms * 1000;

	tv->tv_sec += (tv->tv_usec / 1000000);
	tv->tv_usec %= 1000000;
	if (tv->tv_usec < 0)
	{
		tv->tv_sec --;
		tv->tv_usec += 1000000;
	}

}

int GetTimeOut(struct timeval *tv)
{
	struct timeval now;
	int res;
	getmicrotime(&now);

	res = timevalcmp(&now, tv, <);

	return res;
}

void Sleep(int seconds)
{
	struct timeval now;

	SetTimeOut(&now, seconds * 1000);

	while(GetTimeOut(&now));

}

