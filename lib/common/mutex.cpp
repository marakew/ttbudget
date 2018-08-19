
/*
 */

extern "C"
{
	#include <sys/param.h>
	#include <sys/types.h>
#if __FreeBSD_version >= 700000
	#include <sys/systm.h>
#else
	#include "systm.h"
#endif
	//#include <sys/lock.h>
	//#include <sys/_lock.h>
	//#include <sys/_mutex.h>

}

#include "mutex.h"


CMutex::CMutex()
{
}

CMutex::~CMutex()
{
}

void
CMutex::Lock(void)
{
	//spinlock_enter();
}

void
CMutex::Unlock(void)
{
	//spinlock_exit();
}

