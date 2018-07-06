all:
#	g++ -DLINUX -I. -I ../../../../../../libraries/PJON-11.0/src -std=gnu++11 DeviceGeneric.cpp -o owpshell -lwiringPi -lpthread -lcrypt -lm -lrt
	# https://stackoverflow.com/questions/39978762/linux-executable-cant-find-shared-library-in-same-folder
	g++ -DLINUX -I. -I ../../../../../../libraries/PJON-11.0/src -std=gnu++11 DeviceGeneric.cpp -o owpshell -Wl,-rpath,'$$ORIGIN' -lwiringPi -lpthread -lcrypt -lm -lrt
	readelf -d ./owpshell
