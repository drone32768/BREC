
#------------------------------------------------------------------------------
#HOSTARCH := $(shell arch)
HOSTARCH := $(shell uname -m)
export HOSTARCH

# if empty, not equal, find string of i686 in hostarch
ifneq (,$(findstring i686,$(HOSTARCH)))
    TGT=x86
    CPP_TGT_ARGS=-DTGT_X86
else
    TGT=arm
    CPP_TGT_ARGS=-DTGT_ARM
endif


#------------------------------------------------------------------------------
CPPFLAGS=-O3 -Wall -g $(CPP_TGT_ARGS)
#CPPFLAGS=-Wall -g $(CPP_TGT_ARGS)

#------------------------------------------------------------------------------
$(TGT)	:
	mkdir -p $(TGT)
