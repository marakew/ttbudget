
.PATH:	${.CURDIR}

PROG=	dvbconfig
SRCS=	dvbconfig.cpp
SRCS+=	dvblib.cpp
SRCS+=	str2mac.cpp
SRCS+=	str2pid.cpp
SRCS+=	long2str.cpp
SRCS+=	feutil.cpp
SRCS+=	readconfig.cpp
SRCS+=	getopt.c

NOMAN=

BINDIR=	/usr/local/bin
BINMODE=4550
BINOWN= root
BINGRP= wheel

CFLAGS+= -I../include
#CFLAGS+= -g
CFLAGS+= -s

# no C++
CFLAGS+= -fno-exceptions
CFLAGS+= -fno-rtti
CFLAGS+= -fno-builtin

# enable optimization
CFLAGS+= -mpush-args
CFLAGS+= -mno-accumulate-outgoing-args
CFLAGS+= -O

#CXXFLAGS+=	${CFLAGS}

.include <bsd.prog.mk>

.if defined(FROMPORT)
CXXFLAGS+=      ${CFLAGS}
.endif

