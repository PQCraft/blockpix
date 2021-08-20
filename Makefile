LIBA := libblockpix.a
CC = gcc
INSTDIR = /usr/lib/
HINSTDIR = /usr/include/
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
	@rm -f $(EGBINS)

clean_lib:
	@rm -f $(LIBOBJS) $(LIBA)

clean: clean_examples clean_lib

install: $(LIBA)
	@cp $(LIBA) $(INSTDIR)
	@cp blockpix.h $(HINSTDIR)

remove:
	@rm $(INSTDIR)/$(LIBA)
	@rm $(HINSTDIR)/blockpix.h

reinstall: remove clean $(LIBA) install

