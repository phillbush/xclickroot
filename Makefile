PROG = xclickroot
OBJS = ${PROG:=.o}
SRCS = ${OBJS:.o=.c}
MANS = ${PROG:=.1}

PREFIX ?= /usr/local
MANPREFIX ?= ${PREFIX}/share/man
LOCALINC ?= /usr/local/include
LOCALLIB ?= /usr/local/lib
X11INC ?= /usr/X11R6/include
X11LIB ?= /usr/X11R6/lib

DEFS = -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE -D_BSD_SOURCE -D_DEFAULT_SOURCE
INCS = -I${LOCALINC} -I${X11INC}
LIBS = -L${LOCALLIB} -L${X11LIB} -lX11
PROG_CPPFLAGS = ${DEFS} ${INCS} ${CPPFLAGS}
PROG_CFLAGS = -std=c99 -pedantic ${CFLAGS} ${PROG_CPPFLAGS}
PROG_LDFLAGS = ${LIBS} ${LDFLAGS} ${LDLIBS}

bindir = ${DESTDIR}${PREFIX}/bin
mandir = ${DESTDIR}${MANPREFIX}/man1

all: ${PROG}

${PROG}: ${OBJS}
	${CC} -o $@ ${OBJS} ${PROG_LDFLAGS}

.c.o:
	${CC} ${PROG_CFLAGS} -o $@ -c $<

tags: ${SRCS}
	ctags ${SRCS}

lint: ${SRCS}
	-mandoc -T lint -W warning ${MANS}
	-cppcheck --enable=portability ${PROG_CPPFLAGS} ${SRCS}

clean:
	rm -f ${OBJS} ${PROG} ${PROG:=.core} tags

install: all
	mkdir -p ${bindir}
	mkdir -p ${mandir}
	install -m 755 ${PROG} ${bindir}/${PROG}
	install -m 644 ${MANS} ${mandir}/${MANS}

uninstall:
	rm ${bindir}/${PROG}
	rm ${mandir}/${MANS}

.PHONY: all clean install uninstall lint tags
