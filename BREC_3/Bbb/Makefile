DATE_STR=$(shell date  +'%F-%H-%M-%S')

all	:
	cd Util;       make all
	cd Interfaces; make all
	cd PruEmul;    make all
	cd JtagTools;  make all
	cd Ui2c;       make all
	cd Adf4351;    make all
	cd Fboard;     make all
	cd Bdc;        make all
	cd Ddc100;     make all
	cd Iboard;     make all
	cd Tboard;     make all
	cd Mboard;     make all
	cd AdcHttpd;   make all

clean	:
	cd Util;       make clean
	cd Interfaces; make clean
	cd PruEmul;    make clean
	cd Ui2c;       make clean
	cd Adf4351;    make clean
	cd JtagTools;  make clean
	cd Fboard;     make clean
	cd Bdc;        make clean
	cd Ddc100;     make clean
	cd Iboard;     make clean
	cd Tboard;     make clean
	cd Mboard;     make clean
	cd AdcHttpd;   make clean

clean_all :
	find . -name "arm" -exec rm -rf {} ";"
	find . -name "x86" -exec rm -rf {} ";"

dist	:
	echo "Distribution date is:" $(DATE_STR)
	# make clean_all
	cd ../; tar -zcvf dist-Bbb-$(DATE_STR).tgz Bbb
	echo "Distribution date is:" $(DATE_STR)
