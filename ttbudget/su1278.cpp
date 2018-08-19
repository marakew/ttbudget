
/*
 */

extern "C"
{
#include <sys/types.h>
#include <sys/param.h>
#if __FreeBSD_version >= 700000
#include <sys/systm.h>
#else
#include "systm.h"
#endif
#include <sys/bus.h>

#include "dvbapi.h"
}

#include "adapter.h"
#include "ttadapter.h"
#include "tuner.h"
#include "su1278.h"


CTunerSU1278::CTunerSU1278(CDvbAdapter *dvb):CTuner(dvb)
{
}

CTunerSU1278::~CTunerSU1278()
{
}

