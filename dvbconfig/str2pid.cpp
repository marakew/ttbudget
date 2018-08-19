
/*
 */

#include <string.h>
#include <stdlib.h>

void
str2pid(const char *szPid, int *piPid)
{
	if (strlen(szPid) > 2 &&
		(strncmp(szPid, "0x", 2) == 0 ||
		strncmp(szPid, "0X", 2) == 0))
	{
		*piPid = strtoul(szPid+2, NULL, 16);
	} else
	{
		*piPid = strtoul(szPid, NULL, 0);
	}
}

