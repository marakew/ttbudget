
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>

#include <sys/selinfo.h>

#include "devio.h"
}

#include "adapter.h"
#include "ttadapter.h"

#include "ttbudget.h"

static int ttprobe(device_t dev);
static int ttattach(device_t dev);
static int ttdetach(device_t dev);
static int ttshutdown(device_t dev);

static device_method_t ttmethods[] = {
	DEVMETHOD(device_probe, ttprobe),
	DEVMETHOD(device_attach, ttattach),
	DEVMETHOD(device_detach, ttdetach),
	DEVMETHOD(device_shutdown, ttshutdown),
	{0, 0}
};

static driver_t ttdriver = {
	"ttbudget",
	ttmethods,
	sizeof(struct devcallback)
};

devclass_t ttdevclass;
DRIVER_MODULE(ttbudget, pci, ttdriver, ttdevclass, 0, 0);


static int ttprobe(device_t dev)
{
	int find;
	static char subdevice[80];

	find = 0;

	/* Philips Semiconductors, SAA7146 */
	if ((pci_get_vendor(dev) == 0x1131) && (pci_get_device(dev) == 0x7146))
	{
		/* TechnoTrend */
		if (pci_get_subvendor(dev) == 0x13C2)
		{
			switch(pci_get_subdevice(dev))
			{
			case 0x1003:
			case 0x1009:
			case 0x100C:
			case 0x1013:	{
					sprintf(&subdevice[60], "S-unknown");
					find = 1;
				} break;
			case 0x100F:	{
					sprintf(&subdevice[60], "S-1100");
					find = 1;
				} break;
			case 0x1016:	{
					sprintf(&subdevice[60], "S-1400");
					find = 1;
				} break;
			case 0x1017:	{
					sprintf(&subdevice[60], "S-1500");
					find = 1;
				} break;
			case 0x1018:	{
					sprintf(&subdevice[60], "S-1401");
					find = 1;
				} break;
			case 0x1019:	{
					sprintf(&subdevice[60], "S2-3200");
					find = 1;
				} break;
			case 0x101B:	{
					sprintf(&subdevice[60], "S-1501");
					find = 1;
				} break;
			default:;
			}
		}
	}

	if (find == 1)
	{
		sprintf(&subdevice[0], "TechnoTrend budget DVB %s PCI adapter", &subdevice[60]);
		device_set_desc(dev, subdevice);
		return 0;
	}
	return EIO;
}

static int ttattach(device_t dev)
{
	CDvbAdapter	*dvb;
	struct devcallback *dvbdev;
	u_int32_t val;
	int res;

	dvbdev = (struct devcallback *)device_get_softc(dev);

	bzero(dvbdev, sizeof(*dvbdev));

	val = pci_read_config(dev, PCIR_COMMAND, 4);
	val |= (PCIM_CMD_MEMEN  | PCIM_CMD_BUSMASTEREN);
	pci_write_config(dev, PCIR_COMMAND, val /* & 0x000000ff*/, 4);
	
	val = pci_read_config(dev, PCIR_COMMAND, 4);

	if (!(val & PCIM_CMD_PORTEN) && !(val & PCIM_CMD_MEMEN))
	{
		device_printf(dev, "Error: Failed to enable I/O ports and memory mapping!\n");
		return ENXIO;
	}

	dvb = new CDvbAdapter(dev);

	res = dvb->HWInitialize(pci_get_subdevice(dev));
	if (res != 0)
	{
		switch(res)
		{
		case 1:
		device_printf(dev, "Error: Failed to setup i/o space!\n");
			break;

		case 2:
		device_printf(dev, "Error: Failed to create pci!\n");
			break;

		case 3:
		device_printf(dev, "Error: Failed to create i2c!\n");
			break;

		case 4:
		device_printf(dev, "Error: Failed to create debi!\n");
			break;

		case 5:
		device_printf(dev, "Error: Failed to install interrupts!\n");
			break;

		case 6:
		device_printf(dev, "Error: Failed to setup shared memory!\n");
			break;

		case 7:
		device_printf(dev, "Error: Failed to create net!\n");
			break;

		case 8:
		device_printf(dev, "Error: Failed to create tsdemux!\n");
			break;

		case 9:
		device_printf(dev, "Error: Failed to create fifo!\n");
			break;

		case 10:
		device_printf(dev, "Error: Failed to create fifodemux!\n");

		case 11:
		device_printf(dev, "Error: Failed to create tuner!\n");
			break;

		default:
		device_printf(dev, "Error: Failed unknown error!\n");
		}

		delete dvb;
		return ENXIO;
	}

	dvbdev->data = (void *)dvb;
	dvbdev->devControl = DEVFUNC(&CDvbAdapter::HWDvbControl);
	dvb_init(dvbdev, dev);

	return 0;
}

static int ttdetach(device_t dev)
{
	CDvbAdapter *dvb;
	struct devcallback *dvbdev;

	dvbdev = (struct devcallback *)device_get_softc(dev);
	dvb = (CDvbAdapter *)dvbdev->data;

	dvb_free(dvbdev);

	delete dvb;

	return 0;
}

static int ttshutdown(device_t dev)
{
	CDvbAdapter *dvb;
	struct devcallback *dvbdev;

	dvbdev = (struct devcallback *)device_get_softc(dev);
	dvb = (CDvbAdapter *)dvbdev->data;

	dvb_free(dvbdev);

	delete dvb;

	return 0;
}
