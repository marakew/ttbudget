
/*
 */

#ifndef _mutex_h_
#define _mutex_h_


class CMutex
{
protected:

public:
	CMutex();
	~CMutex();

	void	Lock(void);
	void	Unlock(void);
};

#endif
