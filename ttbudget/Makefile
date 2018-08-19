
.PATH:	${.CURDIR}
.PATH:	${.CURDIR}/../lib/saa7146
.PATH:	${.CURDIR}/../lib/common

KMOD=	ttbudget

SRCS=	ttbudget.cpp bus_if.h device_if.h pci_if.h vnode_if.h

# common
SRCS+=	memio.c
SRCS+=	util.c
SRCS+=	timeout.c
SRCS+=	dvbif.c
SRCS+=	devio.c

SRCS+=	mem.cpp
SRCS+=	mutex.cpp
SRCS+=	timer.cpp
SRCS+=	fifo.cpp
SRCS+=	fifodemux.cpp
SRCS+=	adapter.cpp
SRCS+=	tsdemux.cpp

# saa7146
SRCS+=	pci.cpp i2c.cpp debi.cpp


SRCS+=	sched.cpp

# ttbudget
SRCS+=	tuner.cpp

SRCS+=	tda10086.cpp
SRCS+=	pn1010.cpp
SRCS+=	stb0899.cpp
SRCS+=	su1278.cpp
#
SRCS+=	ttutil.c

SRCS+=	net.cpp
SRCS+=	ttadapter.cpp


KMODDIR= /boot/modules

CFLAGS+= -I../include
CFLAGS+= -I../lib/common
CFLAGS+= -I../lib/saa7146

#CFLAGS+= -I../dvbcore
CFLAGS+= -DDEBUG_MODE

# no C++
CFLAGS+= -fno-exceptions
CFLAGS+= -fno-rtti
CFLAGS+= -fno-builtin

# enable optimization
CFLAGS+= -mpush-args
CFLAGS+= -mno-accumulate-outgoing-args
CFLAGS+= -O

DEBUG_FLAGS+= -s
WERROR=

#CXXFLAGS+=      ${CFLAGS}

.include<bsd.kmod.mk>

.if defined(FROMPORT)
CXXFLAGS+=      ${CFLAGS}
.endif

