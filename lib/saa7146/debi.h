
/*
 */

#ifndef _debi_h_
#define _debi_h_

class CSpciDebi
{
protected:
	CSpciIoIrq	*pci;

public:
	enum tDebiErrors
	{
	};

	struct tDebiSetup
	{

	};

	CSpciDebi(CSpciIoIrq *pci, struct CSpciDebi::tDebiSetup *pSetup);
	~CSpciDebi();

	void	Init(void);

	void	Setup(struct CSpciDebi::tDebiSetup *pSetup);

	void	Out8(unsigned short, unsigned char, unsigned int);
	void	Out16(unsigned short, unsigned short, unsigned int);

	unsigned int	In8(unsigned short, unsigned int);
	unsigned int	In16(unsigned short, unsigned int);

	int	WaitDebiCompletion(unsigned short, unsigned int);
};

#endif

