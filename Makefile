LIBA := libblockpix.a
CC = gcc
CFLAGS = -Wall -Wextra -O3
LIBCFLAGS = $(CFLAGS)

ifndef OS

INSTDIR = /usr/lib/
HINSTDIR = /usr/include/
LIBSRCS := $(shell ls *.c)
LIBOBJS := $(addsuffix .o, $(basename $(LIBSRCS)))
EGSRCS := $(shell find ./examples -name '*.c')
EGBINS := $(addsuffix .bin, $(basename $(EGSRCS)))

all: $(LIBA)
examples: $(EGBINS)

%.o: %.c
	@echo "Compiling object $<"
	@$(CC) $(LIBCFLAGS) -c $< -o $@

%.bin: %.c $(LIBA)
	@echo "Compiling example $<"
	@$(CC) -o $@ $< $(CFLAGS) -L. -I. -lblockpix

%.exe: %.c $(LIBA)
	@echo "Cross-compiling example $<"
	@$(CC) -o $@ $< $(CFLAGS) -L. -I. -lblockpix

$(LIBA): $(LIBOBJS)
	@echo "Generating $@"
	@$(AR) rcs $@ $(LIBOBJS)

clean_examples:
	@rm -f $(EGBINS)

clean_lib:
	@rm -f $(LIBOBJS) $(LIBA)

.PHONY: clean
clean: clean_examples clean_lib

install: $(LIBA)
	@cp $(LIBA) $(INSTDIR)
	@cp blockpix.h $(HINSTDIR)

remove:
	@rm $(INSTDIR)/$(LIBA)
	@rm $(HINSTDIR)/blockpix.h

reinstall: remove clean $(LIBA) install

.PHONY: cross
cross:
	@true #prevents the "Nothing to be done" message
	$(eval CC = x86_64-w64-mingw32-gcc)
	$(eval EGBINS := $(addsuffix .exe, $(basename $(EGSRCS))))

else

LIBSRCS := $(shell cmd /c "dir /b /a-d *.c")
LIBOBJS := $(addsuffix .o, $(basename $(LIBSRCS)))
EGSRCS := $(shell cmd /c "dir /b /s /a-d examples\\*.c")
EGBINS := $(addsuffix .exe, $(basename $(EGSRCS)))

all: $(LIBA)
examples: $(EGBINS)

%.o: %.c
	@echo Compiling object $<
	@$(CC) $(LIBCFLAGS) -c $< -o $@

%.exe: $(LIBA)
	@echo Cross-compiling example $<
	$(CC) -o $@ $(addsuffix .c, $(basename $@)) $(CFLAGS) -L. -I. -lblockpix

$(LIBA): $(LIBOBJS)
	@echo Generating $@
	@$(AR) rcs $@ $(LIBOBJS)

clean_examples:
	@cd examples
	@del /f /q $(EGBINS) 2> nul

clean_lib:
	@del /f /q $(LIBOBJS) $(LIBA) 2> nul

.PHONY: clean
clean: clean_examples clean_lib

endif

