

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "long2str.h"


char *long2str(u_int64_t number)
{
	u_int64_t mants;
	u_int64_t bytes;
	int len;
	static char str[30];

	memset(&str[0], 0, sizeof(str));
	len = 0;

	bytes = 1000ULL*1000ULL*1000ULL*1000ULL*1000ULL;
	mants = ( (number/(bytes)) * (bytes) - (number/(bytes*1000)) * (bytes*1000))/ (bytes);
	if (mants > 0)
	{
		if (len > 0)
			len += sprintf(str+len, "%03llu'", mants);
		else
			len += sprintf(str+len, "%3llu'", mants);
	}

	bytes = 1000ULL*1000ULL*1000ULL*1000ULL;
	mants = ( (number/(bytes)) * (bytes) - (number/(bytes*1000)) * (bytes*1000))/ (bytes);
	if (mants > 0)
	{
		if (len > 0)
			len += sprintf(str+len, "%03llu'", mants);
		else
			len += sprintf(str+len, "%3llu'", mants);
	}

	bytes = 1000ULL*1000ULL*1000ULL;
	mants = ( (number/(bytes)) * (bytes) - (number/(bytes*1000)) * (bytes*1000))/ (bytes);
	if (mants > 0)
	{
		if (len > 0)
			len += sprintf(str+len, "%03llu'", mants);
		else
			len += sprintf(str+len, "%3llu'", mants);
	}
		
	bytes = 1000*1000;
	mants = ( (number/(bytes)) * (bytes) - (number/(bytes*1000)) * (bytes*1000))/ (bytes);
	if (mants > 0)
	{
		if (len > 0)
			len += sprintf(str+len, "%03llu'", mants);
		else
			len += sprintf(str+len, "%3llu'", mants);
	}

	bytes = 1000;
	mants = ( (number/(bytes)) * (bytes) - (number/(bytes*1000)) * (bytes*1000))/ (bytes);
	if (mants > 0)
	{
		if (len > 0)
			len += sprintf(str+len, "%03llu'", mants);
		else
			len += sprintf(str+len, "%3llu'", mants);
	}

	bytes = 1;
	mants = ( (number/(bytes)) * (bytes) - (number/(bytes*1000)) * (bytes*1000))/ (bytes);
	if (mants >= 0)
	{
		if (len > 0)
			len += sprintf(str+len, "%03llu", mants);
		else
			len += sprintf(str+len, "%3llu", mants);
	}

	return str;
}
