
/*
 */

#ifndef _sched_h_
#define _sched_h_

typedef void 	(*SchedFN)(void *);
#define SETSCHEDFN(x)	reinterpret_cast<SchedFN>(x)

class CSoftScheduler
{
protected:
	void	*softIh;

	static struct intr_event *softIntr;

public:

	CSoftScheduler(void *obj, void (*SchedProc)(void *) );
	~CSoftScheduler();

	void	Schedule(void);
};

#endif

