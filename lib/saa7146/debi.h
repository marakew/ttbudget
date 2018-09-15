
/*
 */

#ifndef _debi_h_
#define _debi_h_

class CSpciDebi
{
protected:
	CSpciIoIrq	*pci;

public:
	enum tDebiErrors			// Error values returned by 
						// GetLastError()
	{
		DEBI_NO_ERROR,			// Last function call was 
						// sucessful
		SETUP_ERROR,			// Null tDebiSetup pointer
		TIMEOUT_ERROR,			// Last call timed out
		DMA_BUFFER_ERROR,		// Last call supplied a bad 
						// CSpciDmaMem object
		DMA_OFFSET_ERROR,		// Last call's offset value 
						// was to large
		BAD_IRQ_FLAG_ERROR,		// Invalid irq flag
		INTERRUPT_ENABLE_ERROR,		// Error enabling interrupts
		INTERRUPT_NOT_ENABLED_ERROR,	// Error interrupt not enabled
		INTERRUPT_DISABLE_ERROR,	// Error disabling interrupts
		REVISION_ERROR			// Setup is not applicable on 
						// chip revision
	};

	enum tTimeOut {
		TIMER_DISABLED = -1,	// Timeout disabled
		TIMEOUT_0      =  0,	// Timeout enabled with value 0
		TIMEOUT_1      =  1,	// Possible timeout values
		TIMEOUT_2      =  2,	// ...
		TIMEOUT_3      =  3,
		TIMEOUT_4      =  4,
		TIMEOUT_5      =  5,
		TIMEOUT_6      =  6,
		TIMEOUT_7      =  7,
		TIMEOUT_8      =  8,
		TIMEOUT_9      =  9,
		TIMEOUT_10     = 10,
		TIMEOUT_11     = 11,
		TIMEOUT_12     = 12,
		TIMEOUT_13     = 13,
		TIMEOUT_14     = 14,
		TIMEOUT_15     = 15
	};

	enum tSwapMode {
		NO_SWAP = 0,	// straight - no swap
		SWAP_2  = 1,	// 2-byte swap
		SWAP_4  = 2	// 4-byte swap
	};

	enum tSlave {
		MOTOROLA = 0,	// Motorola-style interface
		INTEL    = 1	// Intel-style interface
	};

	struct tDebiSetup
	{
		tTimeOut    timeout;	// timeout value
		tSwapMode   endian;	// Swap mode of interface
		BOOL        increment;	// Enable address increment 
					// for block xfer 
					// (Default = TRUE)
		tSlave      iface;	// Intel or Motorola interface
		BOOL        bSlave16;	// If TRUE slave can be 
					// accessed in 16 bit
// The following three entries are for SAA7146A only and must be set 
// to FALSE otherwise!!!
		BOOL        bFast;	// Enable FAST mode 
					// (short Trwi time)
		BOOL        bXIrq;	// Use GPIO3 as external DEBI 
					// Irq
		BOOL        bXResume;	// Resume block transfer when 
					// XIrq was deasserted.
	};

	CSpciDebi(CSpciIoIrq *pci, struct CSpciDebi::tDebiSetup *pSetup);
	~CSpciDebi();

	void	Init(void);

	void	Setup(struct CSpciDebi::tDebiSetup *pSetup);

	enum tDebiWaitFlags			// Flags used as wait 
						// parameters to I/O calls
	{
		NO_WAIT     = 0x00,
		COMPL_WAIT  = 0x01,		// wait polling DEBI status
		ERR_EF_WAIT = 0x02,		// return on format error
		ERR_TO_WAIT = 0x04		// return on timeout error
	};

	void	Out8(unsigned short wSlaveAddress, unsigned char cData, unsigned int dwWaitFlags = COMPL_WAIT);
	void	Out16(unsigned short wSlaveAddress, unsigned short wData, unsigned int dwWaitFlags = COMPL_WAIT);

	unsigned int	In8(unsigned short wSlaveAddress, unsigned int dwWaitFlags = COMPL_WAIT);
	unsigned int	In16(unsigned short wSlaveAddress, unsigned int dwWaitFlags = COMPL_WAIT);

	int	WaitDebiCompletion(unsigned short wLength, unsigned int dwWaitFlags);
};

#endif

