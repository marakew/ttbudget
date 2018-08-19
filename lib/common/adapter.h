
/*
 */

#ifndef _adapter_h_
#define _adapter_h_

class CAdapter
{
protected:

public:
	CAdapter();
	~CAdapter();

	virtual void TSCallback(unsigned char filterType, unsigned char filterNo, unsigned char *data, unsigned int len, unsigned char flag);

	virtual void HWIsrCallback(unsigned int irqtype, unsigned int isrnum);
};

#endif

