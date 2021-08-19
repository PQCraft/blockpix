LIBA := libblockpix.a
CC = gcc
INSTDIR = /usr/lib
LIBSRCS := $(shell ls *.c)
LIBOBJS := $(addsuffix .o, $(basename $(LIBSRCS)))
EGSRCS := $(shell find ./examples -name '*.c')
EGBINS := $(addsuffix .bin, $(basename $(EGSRCS)))
CFLAGS = -Wall -Wextra -O3
LIBCFLAGS = $(CFLAGS)

.PHONY: clean

all: $(LIBA)
examples: $(EGBINS)

%.o: %.c
	@echo "Compiling object $<"
	@$(CC) $(LIBCFLAGS) -c $< -o $@

%.bin: %.c $(LIBA)
	@echo "Compiling example $<"
	@$(CC) -o $@ $< $(CFLAGS) -L. -I. -lblockpix

$(LIBA): $(LIBOBJS)
	@echo "Generating $@"
	@$(AR) rcs $@ $(LIBOBJS)

clean_examples:
	@$(RM) $(EXBINS)

clean_lib:
	@$(RM) $(LIBOBJS) $(LIBA)

clean: clean_examples clean_lib

install: $(LIBA)
	@cp $(LIBA) $(INSTDIR)
	@cp $(LIBSRCDIR)/blockpix.h /usr/include/

remove:
	@$(RM) $(INSTDIR)/$(LIBA)
	@$(RM) $(INSTDIR)/blockpix.h

reinstall: remove clean $(LIBA) install

