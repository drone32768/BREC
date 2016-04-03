include ../MakeCommon.mk

all	:
	make $(TGT)/ui2c.o


$(TGT)/ui2c.o	: ui2c.cpp ui2c.h  | $(TGT)
	g++ -I../ -c $(CPP_TGT_ARGS) $(CPPFLAGS) ui2c.cpp \
                -o $(TGT)/ui2c.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp
