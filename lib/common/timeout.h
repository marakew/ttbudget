
/*
 */

#ifndef _timeout_h_
#define _timeout_h_

void
SetTimeOut(struct timeval *tv, unsigned int ms);

int
GetTimeOut(struct timeval *tv);

void
Sleep(int seconds);

#endif
