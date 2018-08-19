
/*
 */

#ifndef _fifo_h_
#define _fifo_h_

class CMutex;

class CFifo
{
protected:
	unsigned char	*pBuffer;
	unsigned int	lSize;	
	unsigned int	ReadIndex;
	unsigned int	WriteIndex;

	CMutex		mutex;
public:
	CFifo(unsigned int size);
	~CFifo();

	int	Init(unsigned int size);
	int	IsOK(void);

	void	Reset(void);

	unsigned int	Size(void);
	unsigned int	Put(unsigned char *buffer, unsigned int size);
	unsigned int	Get(unsigned char *buffer, unsigned int size);

	void	Lock(void);
	void	Unlock(void);
};

#endif

