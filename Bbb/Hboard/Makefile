include ../common.mk

ifeq ($(TGT),arm)
    PRU_INC=/home/root/am335x_pru_package-master/pru_sw/app_loader/include 
    PRU_LIB=/home/root/am335x_pru_package-master/pru_sw/app_loader/lib
    PASM=/home/root/am335x_pru_package-master/pru_sw/utils/pasm
else
    PRU_INC=../PruEmul/
endif

all	:
	echo target is $(TGT)
	make $(TGT)/Hboard.o
ifeq ($(TGT),arm)
	make pru_images.h
	cd Dt; make all
endif

$(TGT)/Hboard.o : Hboard.cpp Hboard.h pru_images.h | $(TGT)
	g++  $(CPPFLAGS) -c -I$(PRU_INC) Hboard.cpp -o $(TGT)/Hboard.o

$(TGT)/pru00_sim.bin : pru00_sim.p Hpru.h pru.hp | $(TGT)
	${PASM} -V3 -b pru00_sim.p
	mv pru00_sim.bin $(TGT)/pru00_sim.bin

$(TGT)/pru01_sim.bin : pru01_sim.p Hpru.h pru.hp | $(TGT)
	${PASM} -V3 -b pru01_sim.p
	mv pru01_sim.bin $(TGT)/pru01_sim.bin

$(TGT)/pru00_downsample.bin : pru00_downsample.p Hpru.h pru.hp | $(TGT)
	${PASM} -V3 -b pru00_downsample.p
	mv pru00_downsample.bin $(TGT)/pru00_downsample.bin

$(TGT)/pru01_downsample.bin : pru01_downsample.p Hpru.h pru.hp | $(TGT)
	${PASM} -V3 -b pru01_downsample.p
	mv pru01_downsample.bin $(TGT)/pru01_downsample.bin

$(TGT)/pru00_cic_2_2.bin : pru00_cic_2_2.p Hpru.h pru.hp | $(TGT)
	${PASM} -V3 -b pru00_cic_2_2.p
	mv pru00_cic_2_2.bin $(TGT)/pru00_cic_2_2.bin

$(TGT)/pru01_cic_2_2.bin : pru01_cic_2_2.p Hpru.h pru.hp | $(TGT)
	${PASM} -V3 -b pru01_cic_2_2.p
	mv pru01_cic_2_2.bin $(TGT)/pru01_cic_2_2.bin

ifeq ($(TGT),arm)
pru_images.h	: ../Util/$(TGT)/ccat \
	$(TGT)/pru00_sim.bin        $(TGT)/pru01_sim.bin        \
	$(TGT)/pru00_downsample.bin $(TGT)/pru01_downsample.bin \
	$(TGT)/pru00_cic_2_2.bin $(TGT)/pru01_cic_2_2.bin \
	| $(TGT)

	echo "/* Generated File */"  >  pru_images.h
	../Util/$(TGT)/ccat -f $(TGT)/pru00_sim.bin \
		-n pru00_sim >> pru_images.h
	../Util/$(TGT)/ccat -f $(TGT)/pru01_sim.bin \
	        -n pru01_sim >> pru_images.h

	../Util/$(TGT)/ccat -f $(TGT)/pru00_downsample.bin \
		-n pru00_downsample >> pru_images.h
	../Util/$(TGT)/ccat -f $(TGT)/pru01_downsample.bin \
		-n pru01_downsample >> pru_images.h

	../Util/$(TGT)/ccat -f $(TGT)/pru00_cic_2_2.bin \
		-n pru00_cic_2_2 >> pru_images.h
	../Util/$(TGT)/ccat -f $(TGT)/pru01_cic_2_2.bin \
		-n pru01_cic_2_2 >> pru_images.h
endif


clean	:
	rm -f $(TGT)/*
	rm -rf $(TGT)
	rm -f *~
	rm -f tmp
