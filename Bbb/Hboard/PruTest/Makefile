include ../../common.mk

PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include 
PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm

all	:
	echo target is $(TGT)
ifeq ($(TGT),arm)
	make $(TGT)/pructl
	make $(TGT)/pru00.bin
	make $(TGT)/pru01.bin
	make $(TGT)/follower
endif

$(TGT)/pructl   : pructl.c pructl.h pru.hp | $(TGT)
	gcc -I$(PRU_INC) -L$(PRU_LIB) pructl.c \
		-o $(TGT)/pructl -lprussdrv -lpthread 

$(TGT)/follower   : follower.c pructl.h  | $(TGT)
	g++ -O3 -I$(PRU_INC) -L$(PRU_LIB) follower.c \
                 ../../Util/$(TGT)/mcf.o \
		-o $(TGT)/follower \
                 -lprussdrv -lpthread 

$(TGT)/pru00.bin : pru00.p pructl.h pru.hp | $(TGT)
	${PASM} -V3 -b pru00.p
	mv pru00.bin $(TGT)/pru00.bin

$(TGT)/pru01.bin : pru01.p pructl.h pru.hp | $(TGT)
	${PASM} -V3 -b pru01.p
	mv pru01.bin $(TGT)/pru01.bin

$(TGT)/get_line.o : get_line.s | $(TGT)
	as get_line.s -o $(TGT)/get_line.o

clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp
