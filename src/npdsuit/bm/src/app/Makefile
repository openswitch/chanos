CC = $(COMPILER_PREFIX)gcc
AR = $(COMPILER_PREFIX)ar
RANLIB = $(COMPILER_PREFIX)ranlib
KMOD_DIR = ../kmod
INCLUDES = -I. -I ${KMOD_DIR} -I${ACCAPI_DIR} -I$(KERNEL_ROOT)/include
CFLAGS = -Wall -D__LINUX__ $(INCLUDES)


#Set LANG to POSIX to display traditional simple english instead of other languages, avoid decode error in non UTF8 application.
LANG=POSIX

all: clean bm_cpld_util
#	mv bmutil ../../exec
	mv bm_cpld_util $(KMOD_EXPORT_DIR)
clean:
	rm -rf bmutil *.o *~ Test_cpld
#	rm -f ../../exec/bmutil

bmutil:	bmutil.o
	$(CC)  -o $@ $? 

bm_cpld_util: bm_cpld_util.o bm_nbm_api.o sysinfo_token.o
	$(CC)  -o $@ $?
