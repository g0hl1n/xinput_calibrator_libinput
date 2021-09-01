DESTDIR ?=
PREFIX  ?= /usr
BINDIR  ?= $(PREFIX)/bin

CC	?= gcc
CFLAGS	?= -O2 -Wall -Wextra -Werror
LDFLAGS	?=

PROG = calc_libinput_matrix

INSTALL		:= install
INSTALL_DIR	:= $(INSTALL) -m 755 -d
INSTALL_PROGRAM	:= $(INSTALL) -m 0755
RM		:= rm -f

all: $(PROG) transform_evdev_to_libinput

$(PROG).o: $(PROG).c
	$(CC) -c $*.c -o $@ $(CFLAGS)

$(PROG): $(PROG).o
	$(CC) -o $@ $^ $(LDFLAGS) -lgsl -lgslcblas -lm

transform_evdev_to_libinput: transform_evdev_to_libinput.c
	$(CC) transform_evdev_to_libinput.c -o $@ $(CFLAGS) $(LDFLAGS)

install:
	$(INSTALL_DIR) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(PROG) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) xinput_calibrator_libinput $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) xinput_calibrator_once.sh $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) transform_evdev_to_libinput $(DESTDIR)$(bindir)

clean:
	$(RM) $(PROG).o $(PROG)
