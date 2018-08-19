
/*
 */

#ifndef _memio_h_
#define _memio_h_

unsigned char
read_reg_uchar(struct resource *memory, unsigned int reg);

unsigned short
read_reg_ushort(struct resource *memory, unsigned int reg);

unsigned int
read_reg_ulong(struct resource *memory, unsigned int reg);

void
write_reg_uchar(struct resource *memory, unsigned int reg, unsigned char value);

void
write_reg_ushort(struct resource *memory, unsigned int reg, unsigned short value);

void
write_reg_ulong(struct resource *memory, unsigned int reg, unsigned int value);

#endif
