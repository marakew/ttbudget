
/*
 */

extern "C"
{
	#include <sys/param.h>
#if __FreeBSD_version >= 700000
	#include <sys/systm.h>
#else
	#include "systm.h"
#endif
	#include <sys/bus.h>
	#include <sys/interrupt.h>
}

#include "sched.h"


struct intr_event *CSoftScheduler::softIntr = 0;//NULL;

CSoftScheduler::CSoftScheduler(void *obj, void (*SchedProc)(void *))
{
	softIh = NULL;

	swi_add(&CSoftScheduler::softIntr, "scheduler", SchedProc, obj, 7, (intr_type)0, &softIh);
	if (softIh == NULL)
	{
		printf("%s:\n", __FUNCTION__);
	}
}

CSoftScheduler::~CSoftScheduler()
{

	if (softIh != NULL)
	{
		swi_remove(softIh);
		softIh = NULL;
	}

	if ((&CSoftScheduler::softIntr->ie_handlers)->tqh_first == NULL)
	{
		intr_event_destroy(CSoftScheduler::softIntr);
		CSoftScheduler::softIntr = 0;//NULL;
	}
}

void
CSoftScheduler::Schedule(void)
{
	if (softIh != NULL)
	{
		swi_sched(softIh, 0);
	}
}

