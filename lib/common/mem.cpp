
/*
 */

extern "C"
{
	#include <sys/param.h>
	#include <sys/malloc.h>
}

void *operator new (size_t size)
{
        return malloc(size, M_DEVBUF, M_NOWAIT);
}

void *operator new [](size_t size)
{
        return operator new(size);
}

void operator delete (void *buffer)
{
	if (buffer != NULL)
        	free(buffer, M_DEVBUF);
}

void operator delete [](void *buffer)
{
	operator delete(buffer);
}
