
/*
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/bus.h>

#include <sys/selinfo.h>

#include "dvbapi.h"

#include "devio.h"

static int dev_num = 0;

static d_open_t         dvb_open;
static d_close_t        dvb_close;
static d_read_t         dvb_read;
static d_write_t        dvb_write;
static d_ioctl_t        dvb_ioctl;
static d_mmap_t         dvb_mmap;
static d_poll_t         dvb_poll;
static d_kqfilter_t	dvb_kqfilter;

static struct cdevsw dvb_cdevsw = {
	.d_version =    D_VERSION,
	.d_flags =      D_NEEDGIANT,
	.d_open =       dvb_open,
	.d_close =      dvb_close,
	.d_read =       dvb_read,
	.d_write =      dvb_write,
	.d_ioctl =      dvb_ioctl,
	.d_poll =       dvb_poll,
	.d_mmap =       dvb_mmap,
	.d_kqfilter =	dvb_kqfilter,
	.d_name =       "dvb"
};

static void	filt_dvbdetach(struct knote *kn);
static int	filt_dvbread(struct knote *kn, int hint);
static int	filt_dvbwrite(struct knote *kn, int hint);

static struct filterops	dvb_read_filterops = {
	.f_isfd =	1,
	.f_attach =	NULL,
	.f_detach =	filt_dvbdetach,
	.f_event =	filt_dvbread,
};

static struct filterops	dvb_write_filterops = {
	.f_isfd =	1,
	.f_attach =	NULL,
	.f_detach =	filt_dvbdetach,
	.f_event =	filt_dvbwrite,
};

void dvb_init(struct devcallback *dvbdev, device_t dev)
{
	knlist_init(&dvbdev->kq_sel.si_note, NULL, NULL, NULL, NULL);

	dvbdev->dev = make_dev(&dvb_cdevsw,
				dev_num,
				UID_ROOT,
				GID_WHEEL,
				0600,
				"dvb%d", dev_num);

	dvbdev->dev->si_drv1 = dev;

	dev_num ++;
}

void dvb_free(struct devcallback *dvbdev)
{
	destroy_dev(dvbdev->dev);

	knlist_destroy(&dvbdev->kq_sel.si_note);
}


static int dvb_open(struct cdev *dev, int flags, int fmt, struct thread *td)
{
	struct devcallback *dvbdev;
	int unit;
	int err;

	dvbdev = device_get_softc(dev->si_drv1);
	if (dvbdev == NULL)
	{
		return ENXIO;
	}

	device_busy(dev->si_drv1);

	return 0;
}

static int dvb_close(struct cdev *dev, int flags, int fmt, struct thread *td)
{
	struct devcallback *dvbdev;
	int unit;
	int err;

	dvbdev = device_get_softc(dev->si_drv1);
	if (dvbdev == NULL)
	{
		return ENXIO;
	}

	device_unbusy(dev->si_drv1);

	return 0;
}

static int dvb_read(struct cdev *dev, struct uio *uio, int ioflag)
{
	return ENXIO;
}

static int dvb_write(struct cdev *dev, struct uio *uio, int ioflag)
{
	return ENXIO;
}

static int dvb_ioctl(struct cdev *dev, unsigned long cmd, caddr_t arg, int flag, struct thread *td)
{
	struct devcallback *dvbdev;
	int unit;
	int err;

	dvbdev = device_get_softc(dev->si_drv1);
	if (dvbdev == NULL)
	{
		return ENXIO;
	}

	err = 0;

	switch (cmd)
	{
	case DVBAPI:
		{
			struct dvbapi *parm = (struct dvbapi *)arg;

			if (dvbdev->devControl != NULL && dvbdev->data != NULL)
			{
				if (dvbdev->devControl(dvbdev->data, parm) != 0)
				{
					err = EINVAL;
				} else
				{
					err = 0;
				}
			} else
				err = EINVAL;
		}
			break;

	default:
		printf("%s: unsupported ioctl %08x!\n", __FUNCTION__, (unsigned long)cmd);
		err = EINVAL;
	}

	return err;
}

static int dvb_mmap(struct cdev *dev, vm_offset_t offset, vm_paddr_t *paddr, int nprot)
{
	return ENXIO;
}

static int dvb_poll(struct cdev *dev, int events, struct thread *tp)
{
	return ENXIO;
}

static int dvb_kqfilter(struct cdev *dev, struct knote *kn)
{
	int unit;
	struct devcallback *dvbdev;

	dvbdev = device_get_softc(dev->si_drv1);

	switch (kn->kn_filter)
	{
	case EVFILT_READ:
		kn->kn_fop = &dvb_read_filterops;
		break;
	case EVFILT_WRITE:
		kn->kn_fop = &dvb_write_filterops;
		break;

	default:
		return EINVAL;
	}

	kn->kn_hook = dvbdev;

	knlist_add(&dvbdev->kq_sel.si_note, kn, 0);

	return 0;
}

static void filt_dvbdetach(struct knote *kn)
{
	struct devcallback *dvbdev = kn->kn_hook;

	knlist_remove(&dvbdev->kq_sel.si_note, kn, 0);
}

static int filt_dvbread(struct knote *kn, int hint)
{
	struct devcallback *dvbdev = kn->kn_hook;

	return 0;
}

static int filt_dvbwrite(struct knote *kn, int hint)
{
	struct devcallback *dvbdev = kn->kn_hook;

	return 0;
}
