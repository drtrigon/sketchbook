all:
#	g++ -DLINUX -I. -I../../../../../../src -std=c++11 Transmitter.cpp -o Transmitter
#	g++ -DLINUX -I. -I ../../../../../../libraries/PJON-11.0/src -std=gnu++11 DeviceGeneric.cpp -o owpshell
	g++ -DLINUX -I. -I ../../../../../../libraries/PJON/src -std=gnu++11 DeviceGeneric.cpp -o owpshell
	./owpshell --id
raspi:
	gcc-4.8 -DLINUX -DID=46 -I. -I ../../../../../../src/ -std=c++11 DeviceGeneric.cpp -o owpshell -lstdc++
	./owpshell --id
