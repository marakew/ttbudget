
/*
 */

#ifndef _devio_h_
#define _devio_h_

struct devcallback
{
	struct	cdev	*dev;

	struct	selinfo	kq_sel;

	void	*data;
	int	(*devControl)(void *, struct dvbapi *);
};

typedef int (*devFunction)(void *, struct dvbapi *);
#define DEVFUNC(x)      reinterpret_cast<devFunction>(x)

void
dvb_init(struct devcallback *dvbdev, device_t dev);

void
dvb_free(struct devcallback *dvbdev);

#endif

