DATE_STR=$(shell date  +'%F-%H-%M-%S')

all	:
	cd Util;       make all
	cd Interfaces; make all
	cd Adf4351;    make all
	cd Aboard;     make all
	cd Bboard;     make all
	cd Rboard ;    make all
	cd Hboard ;    make all
	cd Iboard ;    make all
	cd Mboard ;    make all
	cd Lboard ;    make all
	cd PruEmul;    make all
	cd SdrSvr ;    make all
	cd WbSvr  ;    make all
	cd SynHttpd;   make all

clean	:
	cd Util;       make clean
	cd Interfaces; make clean
	cd Adf4351;    make clean
	cd Aboard;     make clean
	cd Bboard;     make clean
	cd Rboard ;    make clean
	cd Hboard ;    make clean
	cd Iboard ;    make clean
	cd Mboard ;    make clean
	cd Lboard ;    make clean
	cd PruEmul;    make clean
	cd SdrSvr ;    make clean
	cd WbSvr  ;    make clean
	cd SynHttpd;   make clean

clean_all :
	find . -name "arm" -exec rm -rf {} ";"
	find . -name "x86" -exec rm -rf {} ";"

dist	:
	echo "Distribution date is:" $(DATE_STR)
	# make clean_all
	cd ../; tar -zcvf dist-Bbb-$(DATE_STR).tgz Bbb
	echo "Distribution date is:" $(DATE_STR)
