
/*
 */

#include <sys/param.h>
#include <sys/systm.h>

#include <sys/bus.h>
#include <sys/rman.h>

#include <machine/bus.h>

#include "memio.h"

unsigned char
read_reg_uchar(struct resource *memory, unsigned int reg)
{
	return bus_space_read_1(rman_get_bustag(memory), rman_get_bushandle(memory), reg);
}

unsigned short
read_reg_ushort(struct resource *memory, unsigned int reg)
{
	return bus_space_read_2(rman_get_bustag(memory), rman_get_bushandle(memory), reg);
}

unsigned int
read_reg_ulong(struct resource *memory, unsigned int reg)
{
	return bus_space_read_4(rman_get_bustag(memory), rman_get_bushandle(memory), reg);
}

void
write_reg_uchar(struct resource *memory, unsigned int reg, unsigned char value)
{
	bus_space_write_1(rman_get_bustag(memory), rman_get_bushandle(memory), reg, value);
}

void
write_reg_ushort(struct resource *memory, unsigned int reg, unsigned short value)
{
	bus_space_write_2(rman_get_bustag(memory), rman_get_bushandle(memory), reg, value);
}

void
write_reg_ulong(struct resource *memory, unsigned int reg, unsigned int value)
{
	bus_space_write_4(rman_get_bustag(memory), rman_get_bushandle(memory), reg, value);
}

