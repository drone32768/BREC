Unforunately, one of the I2C pins was chosen to be used for the
jtag boundary scan interface.  To use this pin, the BBB serial eprom
for capes must be disabled.  See internet references on enabling canbus

1.0 Pull source code with:
   git clone https://github.com/derekmolloy/boneDeviceTree.git
   copy over boneDeviceTree/DTSource3.8.13/

2.0 Remove i2c2 references in am335x-bone-common.dtsi

3.0 Compile device tree
   dtc -O dtb -o am335x-boneblack.dtb -b 0 -@ am335x-boneblack.dts

4.0 Swap out dev tree
   cp /boot/am335x-bone.dtb /boot/am335x-bone.dtb-original
   cp  am335x-boneblack.dtb /boot


