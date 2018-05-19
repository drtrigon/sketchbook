
( osboxes@osboxes:~/sketchbook/cli/owslave$ sudo apt-get install gcc-avr_4.5.3 )


--- using ATtiny84 (attiny84) ---

---- programmer ----

Use Arduino Uno and my "Programmer Shield".

Open Arduino IDE (e.g. 1.6.5) and open File > Examples > ArdinoISP

Upload the sketch to an Arduino Uno board.

Adopt "Programmer Shield" to be wired for a tiny84, see sketch and:
http://ww1.microchip.com/downloads/en/DeviceDoc/8006S.pdf
http://www.3bm.de/2013/09/17/attiny84-mit-arduino-uno-programmieren/

  * Arduino 5V an ATtiny84 Pin 1 (VCC +)
  * Arduino GND an ATTiny84 Pin 14 (GND -)
  * Arduino Pin 10 an ATtiny84 Pin 4 (Reset)
  * Arduino Pin 11 an ATTiny84 Pin 7 (Pin 6, PWM, Analog Input 7)
  * Arduino Pin 12 an ATTiny84 Pin 8 (Pin 5, Analog Input 5, PWM, MISO)
  * Arduino Pin 13 an ATTiny84 Pin 9 (Pin 4, Analog Input 4, SCK)

---- chip ----

create or add you project.cfg (project.cfg.tiny84) to the folder

osboxes@osboxes:~/sketchbook/cli/owslave$ nano Makefile

change the line
	$(AVRDUDE) -c $(PROG) -p $(MCU_PROG) -C +$$TF\
into
	$(AVRDUDE) -c $(PROG) -p t84 -b 19200 -P /dev/ttyACM0 -C +$$TF\
save and quit

osboxes@osboxes:~/sketchbook/cli/owslave$ chmod +x cfg Cfg.py cfg_write gen_eeprom ordered_yaml.py 

osboxes@osboxes:~/sketchbook/cli/owslave$ make CFG=project.cfg try1
make[1]: Entering directory `/home/osboxes/sketchbook/cli/owslave'
mkdir -p device/try1
MCU:atmega88
MCU_PROG:m88
PROG:avrisp
AVRDUDE:sudo avrdude
CFILES:moat_backend.c main.c jmp.S dev_data.c config.o timer.c ds2423.c onewire.c crc.c
TYPE:alert -1 count 0 config -1 port 0 humid 0 pid 0 temp 0 console 0 smoke 0 pwm 0 adc 0
mkdir -p device/try1
./cfg project.cfg .hdr try1
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/moat_backend.o moat_backend.c
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/main.o main.c
main.c:120:17: warning: ‘bootseq’ defined but not used [-Wunused-variable]
 static uint16_t bootseq __attribute__((section(".noinit")));
                 ^
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/jmp.o jmp.S
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/dev_data.o dev_data.c
set -e; \
        ./gen_eeprom device/try1/eprom.bin type $(./cfg project.cfg .type try1); \
        if ./gen_eeprom device/try1/eprom.bin name >/dev/null 2>&1 ; then : ; else \
                ./gen_eeprom device/try1/eprom.bin name try1 ; fi ; \
        if [ ds2423 != 0 ] ; then \
                if ./gen_eeprom device/try1/eprom.bin owid serial >/dev/null 2>&1 ; then \
                        SER=$(./gen_eeprom device/try1/eprom.bin owid serial); \
                        if ./cfg project.cfg devices.try1.onewire_id >/dev/null 2>&1 ; then \
                                test "$(./cfg project.cfg devices.try1.onewire_id)" = "$SER" ; \
                        else \
                                ./cfg_write project.cfg devices.try1.onewire_id x$SER; \
                        fi; \
                elif ./cfg project.cfg .nofollow devices.try1.onewire_id >/dev/null 2>&1 ; then \
                        SER=$(./cfg project.cfg devices.try1.onewire_id); \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial $SER; \
                else \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial random; \
                        ./cfg_write project.cfg devices.try1.onewire_id x$(./gen_eeprom device/try1/eprom.bin owid serial); \
                fi; \
        fi
avr-objcopy -I binary -O elf32-avr --prefix-sections=.eeprom \
                --redefine-sym "_binary_device_try1_eprom_bin_start=_econfig_start" \
                --redefine-sym "_binary_device_try1_eprom_bin_size=_econfig_size" \
                --redefine-sym "_binary_device_try1_eprom_bin_end=_econfig_end" \
                device/try1/eprom.bin device/try1/config.o
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/timer.o timer.c
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/ds2423.o ds2423.c
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/onewire.o onewire.c
onewire.c:553:2: warning: #warning "Ignore the 'appears to be a misspelled signal handler' warning" [-Wcpp]
 #warning "Ignore the 'appears to be a misspelled signal handler' warning"
  ^
onewire.c: In function ‘real_PIN_INT’:
onewire.c:554:6: warning: ‘real_PIN_INT’ appears to be a misspelled signal handler [enabled by default]
 void real_PIN_INT(void) {
      ^
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/crc.o crc.c
avr-gcc  -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -o device/try1/image.elf -Wl,-Map,device/try1/image.map,--cref device/try1/moat_backend.o device/try1/main.o device/try1/jmp.o device/try1/dev_data.o device/try1/config.o device/try1/timer.o device/try1/ds2423.o device/try1/onewire.o device/try1/crc.o
avr-objcopy -R .eeprom -O ihex device/try1/image.elf device/try1/image.hex
avr-objcopy -j .eeprom --change-section-address .eeprom=0 -O ihex device/try1/image.elf device/try1/eprom.hex
avr-objdump -h -S device/try1/image.elf > device/try1/image.lss
make[1]: Leaving directory `/home/osboxes/sketchbook/cli/owslave'

Press RESET button on "Programmer Shield".

Insert tiny84 into "Programmer Shield" socket (connect ISP).

Press RESET button on "Programmer Shield". LED (Pin 8) should be off.

osboxes@osboxes:~/sketchbook/cli/owslave$ make CFG=project.cfg burn_try1
BURN try1
make[1]: Entering directory `/home/osboxes/sketchbook/cli/owslave'
LFUSE:E2
HFUSE:DC
EFUSE:00
EEPROM:1
MCU:atmega88
MCU_PROG:m88
PROG:avrisp
AVRDUDE:sudo avrdude
CFILES:moat_backend.c main.c jmp.S dev_data.c config.o timer.c ds2423.c onewire.c crc.c
TYPE:alert -1 humid 0 temp 0 pid 0 smoke 0 adc 0 port 0 config -1 pwm 0 console 0 count 0
set -e; \
        ./gen_eeprom device/try1/eprom.bin type $(./cfg project.cfg .type try1); \
        if ./gen_eeprom device/try1/eprom.bin name >/dev/null 2>&1 ; then : ; else \
                ./gen_eeprom device/try1/eprom.bin name try1 ; fi ; \
        if [ ds2423 != 0 ] ; then \
                if ./gen_eeprom device/try1/eprom.bin owid serial >/dev/null 2>&1 ; then \
                        SER=$(./gen_eeprom device/try1/eprom.bin owid serial); \
                        if ./cfg project.cfg devices.try1.onewire_id >/dev/null 2>&1 ; then \
                                test "$(./cfg project.cfg devices.try1.onewire_id)" = "$SER" ; \
                        else \
                                ./cfg_write project.cfg devices.try1.onewire_id x$SER; \
                        fi; \
                elif ./cfg project.cfg .nofollow devices.try1.onewire_id >/dev/null 2>&1 ; then \
                        SER=$(./cfg project.cfg devices.try1.onewire_id); \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial $SER; \
                else \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial random; \
                        ./cfg_write project.cfg devices.try1.onewire_id x$(./gen_eeprom device/try1/eprom.bin owid serial); \
                fi; \
        fi
avr-objcopy -I binary -O elf32-avr --prefix-sections=.eeprom \
                --redefine-sym "_binary_device_try1_eprom_bin_start=_econfig_start" \
                --redefine-sym "_binary_device_try1_eprom_bin_size=_econfig_size" \
                --redefine-sym "_binary_device_try1_eprom_bin_end=_econfig_end" \
                device/try1/eprom.bin device/try1/config.o
avr-gcc  -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -o device/try1/image.elf -Wl,-Map,device/try1/image.map,--cref device/try1/moat_backend.o device/try1/main.o device/try1/jmp.o device/try1/dev_data.o device/try1/config.o device/try1/timer.o device/try1/ds2423.o device/try1/onewire.o device/try1/crc.o
avr-objcopy -R .eeprom -O ihex device/try1/image.elf device/try1/image.hex
avr-objcopy -j .eeprom --change-section-address .eeprom=0 -O ihex device/try1/image.elf device/try1/eprom.hex
avr-objdump -h -S device/try1/image.elf > device/try1/image.lss
TF=$(mktemp avrdude-cfg.XXXXX); echo "default_safemode = no;" >$TF; \
        EFUSE=00; \
        [ "" != "" ] && SET_EFUSE="-U efuse:w:0x00:m"; \
        sudo avrdude -c avrisp -p t84 -b 19200 -P /dev/ttyACM0 -C +$TF\
                -U flash:w:device/try1/image.hex:i -U eeprom:w:device/try1/eprom.hex:i \
                -U lfuse:w:0xE2:m \
                -U hfuse:w:0xDC:m \
                 \
        ; X=$?; rm $TF; exit $X

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.05s

avrdude: Device signature = 0x1e930c
avrdude: NOTE: "flash" memory has been specified, an erase cycle will be performed
         To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: reading input file "device/try1/image.hex"
avrdude: writing flash (3338 bytes):

Writing | ################################################## | 100% 5.65s

avrdude: 3338 bytes of flash written
avrdude: verifying flash memory against device/try1/image.hex:
avrdude: load data flash data from input file device/try1/image.hex:
avrdude: input file device/try1/image.hex contains 3338 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 3.90s

avrdude: verifying ...
avrdude: 3338 bytes of flash verified
avrdude: reading input file "device/try1/eprom.hex"
avrdude: writing eeprom (26 bytes):

Writing | ################################################## | 100% 1.55s

avrdude: 26 bytes of eeprom written
avrdude: verifying eeprom memory against device/try1/eprom.hex:
avrdude: load data eeprom data from input file device/try1/eprom.hex:
avrdude: input file device/try1/eprom.hex contains 26 bytes
avrdude: reading on-chip eeprom data:

Reading | ################################################## | 100% 0.29s

avrdude: verifying ...
avrdude: 26 bytes of eeprom verified
avrdude: reading input file "0xE2"
avrdude: writing lfuse (1 bytes):

Writing | ################################################## | 100% 0.02s

avrdude: 1 bytes of lfuse written
avrdude: verifying lfuse memory against 0xE2:
avrdude: load data lfuse data from input file 0xE2:
avrdude: input file 0xE2 contains 1 bytes
avrdude: reading on-chip lfuse data:

Reading | ################################################## | 100% 0.02s

avrdude: verifying ...
avrdude: 1 bytes of lfuse verified
avrdude: reading input file "0xDC"
avrdude: writing hfuse (1 bytes):

Writing | ################################################## | 100% 0.02s

avrdude: 1 bytes of hfuse written
avrdude: verifying hfuse memory against 0xDC:
avrdude: load data hfuse data from input file 0xDC:
avrdude: input file 0xDC contains 1 bytes
avrdude: reading on-chip hfuse data:

Reading | ################################################## | 100% 0.02s

avrdude: verifying ...
avrdude: 1 bytes of hfuse verified

avrdude done.  Thank you.

make[1]: Leaving directory `/home/osboxes/sketchbook/cli/owslave'
osboxes@osboxes:~/sketchbook/cli/owslave$

BINGO! It works! :))

test now...
(https://github.com/M-o-a-T/owslave/blob/master/HOWTO.md)
  onewire_io
    Choose which pin to connect your 1wire bus to. For now, only pins with dedicated interrupts (INTx) can be used. On an ATmega168, these are pins D2 and D3.
    The default is INT0.
http://ww1.microchip.com/downloads/en/DeviceDoc/8006S.pdf
    INT0 -> Pin 5
  PCINT0 -> Pin 13
     VCC -> Pin 1
     GND -> Pin 14

DOES NOT WORK ON THE BUS - MAY BE DUE TO THE FACT THAT 't84' IS NOT RECOGNIZED AND IT COMPILES AS 'm88'.

See also:
https://electronics.stackexchange.com/questions/205055/using-avrdude-to-program-attiny-via-arduino-as-isp
https://forum.arduino.cc/index.php?topic=349912.0



--- Issues ---

In project.cfg "defaults.target.t85" is recognized as attiny85 but does NOT compile.

In project.cfg "defaults.target.t84" is NOT recognized as attiny84 but does compile as m88 (atmega88).



--- using ATmega88 (mega88) ---

https://www.avrprogrammers.com/articles/atmega8-vs-atmega328

---- programmer ----

Use Arduino Uno and my "Programmer Shield".

Open Arduino IDE (e.g. 1.6.5) and open File > Examples > ArdinoISP

Upload the sketch to an Arduino Uno board.

Adopt "Programmer Shield" to be wired for a mega88, see sketch and:
http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2545-8-bit-AVR-Microcontroller-ATmega48-88-168_Summary.pdf

  * Arduino 5V an ATmega88 Pin 7 (VCC +)
              und ATmega88 Pin 20 AVCC
  * Arduino GND an ATmega88 Pin 8 (GND -)
  * Arduino Pin 10 an ATmega88 Pin 1 PC6 (PCINT14/RESET)
  * Arduino Pin 11 an ATmega88 Pin 17 PB3 (MOSI/OC2A/PCINT3)
  * Arduino Pin 12 an ATmega88 Pin 18 PB4 (MISO/PCINT4)
  * Arduino Pin 13 an ATmega88 Pin 19 PB5 (SCK/PCINT5)

---- chip ----

create or add you project.cfg (project.cfg.mega88) to the folder

osboxes@osboxes:~/sketchbook/cli/owslave$ nano Makefile

change the line
	$(AVRDUDE) -c $(PROG) -p $(MCU_PROG) -C +$$TF\
into
	$(AVRDUDE) -c $(PROG) -p $(MCU_PROG) -b 19200 -P /dev/ttyACM0 -C +$$TF\
save and quit

( osboxes@osboxes:~/sketchbook/cli/owslave$ chmod +x cfg Cfg.py cfg_write gen_eeprom ordered_yaml.py )

osboxes@osboxes:~/sketchbook/cli/owslave$ make CFG=project.cfg try1
make[1]: Entering directory `/home/osboxes/sketchbook/cli/owslave'
mkdir -p device/try1
MCU:atmega88
MCU_PROG:m88
PROG:avrisp
AVRDUDE:sudo avrdude
CFILES:moat_backend.c main.c jmp.S dev_data.c config.o timer.c ds2423.c onewire.c crc.c
TYPE:port 0 temp 0 pwm 0 alert -1 adc 0 console 0 pid 0 config -1 humid 0 count 0 smoke 0
mkdir -p device/try1
./cfg project.cfg .hdr try1
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/moat_backend.o moat_backend.c
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/main.o main.c
main.c:120:17: warning: ‘bootseq’ defined but not used [-Wunused-variable]
 static uint16_t bootseq __attribute__((section(".noinit")));
                 ^
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/jmp.o jmp.S
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/dev_data.o dev_data.c
set -e; \
        ./gen_eeprom device/try1/eprom.bin type $(./cfg project.cfg .type try1); \
        if ./gen_eeprom device/try1/eprom.bin name >/dev/null 2>&1 ; then : ; else \
                ./gen_eeprom device/try1/eprom.bin name try1 ; fi ; \
        if [ ds2423 != 0 ] ; then \
                if ./gen_eeprom device/try1/eprom.bin owid serial >/dev/null 2>&1 ; then \
                        SER=$(./gen_eeprom device/try1/eprom.bin owid serial); \
                        if ./cfg project.cfg devices.try1.onewire_id >/dev/null 2>&1 ; then \
                                test "$(./cfg project.cfg devices.try1.onewire_id)" = "$SER" ; \
                        else \
                                ./cfg_write project.cfg devices.try1.onewire_id x$SER; \
                        fi; \
                elif ./cfg project.cfg .nofollow devices.try1.onewire_id >/dev/null 2>&1 ; then \
                        SER=$(./cfg project.cfg devices.try1.onewire_id); \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial $SER; \
                else \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial random; \
                        ./cfg_write project.cfg devices.try1.onewire_id x$(./gen_eeprom device/try1/eprom.bin owid serial); \
                fi; \
        fi
avr-objcopy -I binary -O elf32-avr --prefix-sections=.eeprom \
                --redefine-sym "_binary_device_try1_eprom_bin_start=_econfig_start" \
                --redefine-sym "_binary_device_try1_eprom_bin_size=_econfig_size" \
                --redefine-sym "_binary_device_try1_eprom_bin_end=_econfig_end" \
                device/try1/eprom.bin device/try1/config.o
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/timer.o timer.c
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/ds2423.o ds2423.c
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/onewire.o onewire.c
onewire.c:553:2: warning: #warning "Ignore the 'appears to be a misspelled signal handler' warning" [-Wcpp]
 #warning "Ignore the 'appears to be a misspelled signal handler' warning"
  ^
onewire.c: In function ‘real_PIN_INT’:
onewire.c:554:6: warning: ‘real_PIN_INT’ appears to be a misspelled signal handler [enabled by default]
 void real_PIN_INT(void) {
      ^
avr-gcc -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -c -o device/try1/crc.o crc.c
avr-gcc  -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -o device/try1/image.elf -Wl,-Map,device/try1/image.map,--cref device/try1/moat_backend.o device/try1/main.o device/try1/jmp.o device/try1/dev_data.o device/try1/config.o device/try1/timer.o device/try1/ds2423.o device/try1/onewire.o device/try1/crc.o
avr-objcopy -R .eeprom -O ihex device/try1/image.elf device/try1/image.hex
avr-objcopy -j .eeprom --change-section-address .eeprom=0 -O ihex device/try1/image.elf device/try1/eprom.hex
avr-objdump -h -S device/try1/image.elf > device/try1/image.lss
make[1]: Leaving directory `/home/osboxes/sketchbook/cli/owslave'

Press RESET button on "Programmer Shield" if LED (Pin 8) is on (should be off).

osboxes@osboxes:~/sketchbook/cli/owslave$ make CFG=project.cfg burn_try1
BURN try1
make[1]: Entering directory `/home/osboxes/sketchbook/cli/owslave'
LFUSE:E2
HFUSE:DC
EFUSE:00
EEPROM:1
MCU:atmega88
MCU_PROG:m88
PROG:avrisp
AVRDUDE:sudo avrdude
CFILES:moat_backend.c main.c jmp.S dev_data.c config.o timer.c ds2423.c onewire.c crc.c
TYPE:smoke 0 humid 0 temp 0 pwm 0 adc 0 port 0 config -1 alert -1 console 0 count 0 pid 0
set -e; \
        ./gen_eeprom device/try1/eprom.bin type $(./cfg project.cfg .type try1); \
        if ./gen_eeprom device/try1/eprom.bin name >/dev/null 2>&1 ; then : ; else \
                ./gen_eeprom device/try1/eprom.bin name try1 ; fi ; \
        if [ ds2423 != 0 ] ; then \
                if ./gen_eeprom device/try1/eprom.bin owid serial >/dev/null 2>&1 ; then \
                        SER=$(./gen_eeprom device/try1/eprom.bin owid serial); \
                        if ./cfg project.cfg devices.try1.onewire_id >/dev/null 2>&1 ; then \
                                test "$(./cfg project.cfg devices.try1.onewire_id)" = "$SER" ; \
                        else \
                                ./cfg_write project.cfg devices.try1.onewire_id x$SER; \
                        fi; \
                elif ./cfg project.cfg .nofollow devices.try1.onewire_id >/dev/null 2>&1 ; then \
                        SER=$(./cfg project.cfg devices.try1.onewire_id); \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial $SER; \
                else \
                        ./gen_eeprom device/try1/eprom.bin owid type 0x$(./cfg project.cfg codes.onewire.ds2423) serial random; \
                        ./cfg_write project.cfg devices.try1.onewire_id x$(./gen_eeprom device/try1/eprom.bin owid serial); \
                fi; \
        fi
avr-objcopy -I binary -O elf32-avr --prefix-sections=.eeprom \
                --redefine-sym "_binary_device_try1_eprom_bin_start=_econfig_start" \
                --redefine-sym "_binary_device_try1_eprom_bin_size=_econfig_size" \
                --redefine-sym "_binary_device_try1_eprom_bin_end=_econfig_end" \
                device/try1/eprom.bin device/try1/config.o
avr-gcc  -g -mmcu=atmega88 -Wall -Wstrict-prototypes -Os -mcall-prologues -fshort-enums -Idevice/try1 -o device/try1/image.elf -Wl,-Map,device/try1/image.map,--cref device/try1/moat_backend.o device/try1/main.o device/try1/jmp.o device/try1/dev_data.o device/try1/config.o device/try1/timer.o device/try1/ds2423.o device/try1/onewire.o device/try1/crc.o
avr-objcopy -R .eeprom -O ihex device/try1/image.elf device/try1/image.hex
avr-objcopy -j .eeprom --change-section-address .eeprom=0 -O ihex device/try1/image.elf device/try1/eprom.hex
avr-objdump -h -S device/try1/image.elf > device/try1/image.lss
TF=$(mktemp avrdude-cfg.XXXXX); echo "default_safemode = no;" >$TF; \
        EFUSE=00; \
        [ "" != "" ] && SET_EFUSE="-U efuse:w:0x00:m"; \
        sudo avrdude -c avrisp -p m88 -b 19200 -P /dev/ttyACM0 -C +$TF\
                -U flash:w:device/try1/image.hex:i -U eeprom:w:device/try1/eprom.hex:i \
                -U lfuse:w:0xE2:m \
                -U hfuse:w:0xDC:m \
                 \
        ; X=$?; rm $TF; exit $X
[sudo] password for osboxes: 

avrdude: AVR device initialized and ready to accept instructions

Reading | ################################################## | 100% 0.02s

avrdude: Device signature = 0x1e930a
avrdude: NOTE: "flash" memory has been specified, an erase cycle will be performed
         To disable this feature, specify the -D option.
avrdude: erasing chip
avrdude: reading input file "device/try1/image.hex"
avrdude: writing flash (3338 bytes):

Writing | ################################################## | 100% 4.99s

avrdude: 3338 bytes of flash written
avrdude: verifying flash memory against device/try1/image.hex:
avrdude: load data flash data from input file device/try1/image.hex:
avrdude: input file device/try1/image.hex contains 3338 bytes
avrdude: reading on-chip flash data:

Reading | ################################################## | 100% 2.60s

avrdude: verifying ...
avrdude: 3338 bytes of flash verified
avrdude: reading input file "device/try1/eprom.hex"
avrdude: writing eeprom (26 bytes):

Writing | ################################################## | 100% 1.37s

avrdude: 26 bytes of eeprom written
avrdude: verifying eeprom memory against device/try1/eprom.hex:
avrdude: load data eeprom data from input file device/try1/eprom.hex:
avrdude: input file device/try1/eprom.hex contains 26 bytes
avrdude: reading on-chip eeprom data:

Reading | ################################################## | 100% 0.11s

avrdude: verifying ...
avrdude: 26 bytes of eeprom verified
avrdude: reading input file "0xE2"
avrdude: writing lfuse (1 bytes):

Writing | ################################################## | 100% 0.02s

avrdude: 1 bytes of lfuse written
avrdude: verifying lfuse memory against 0xE2:
avrdude: load data lfuse data from input file 0xE2:
avrdude: input file 0xE2 contains 1 bytes
avrdude: reading on-chip lfuse data:

Reading | ################################################## | 100% 0.01s

avrdude: verifying ...
avrdude: 1 bytes of lfuse verified
avrdude: reading input file "0xDC"
avrdude: writing hfuse (1 bytes):

Writing | ################################################## | 100% 0.02s

avrdude: 1 bytes of hfuse written
avrdude: verifying hfuse memory against 0xDC:
avrdude: load data hfuse data from input file 0xDC:
avrdude: input file 0xDC contains 1 bytes
avrdude: reading on-chip hfuse data:

Reading | ################################################## | 100% 0.01s

avrdude: verifying ...
avrdude: 1 bytes of hfuse verified

avrdude done.  Thank you.

make[1]: Leaving directory `/home/osboxes/sketchbook/cli/owslave'
osboxes@osboxes:~/sketchbook/cli/owslave$ 

Test the device by connecting it to a 1-wire bus and check for the presence of a new DS2423.

(https://github.com/M-o-a-T/owslave/blob/master/HOWTO.md)
  onewire_io
    Choose which pin to connect your 1wire bus to. For now, only pins with dedicated interrupts (INTx) can be used. On an ATmega168, these are pins D2 and D3.
    The default is INT0.

  * DATA to ATmega88 Pin 4 PD2 (PCINT18/INT0)
  *   5V to ATmega88 Pin 7 (VCC +)
  *  GND to ATmega88 Pin 8 (GND -)
  *   5V to ATmega88 Pin 20 AVCC
  * IN_A to ATmega88 Pin 23 PC0 (ADC0/PCINT8)
  * IN_B to ATmega88 Pin 24 PC1 (ADC1/PCINT9)

BINGO! It works! :))



--- using Tiny (not working yet) ---

Issues with ATtiny85: https://github.com/M-o-a-T/owslave/issues/16
Support for ATtiny84: https://github.com/M-o-a-T/owslave/issues/17



--- using Arduino Uno or Nano Board (not recommened) ---

You need to bypass the bootloader and program the raw/whole chip by connecting
another Arduino Board to the ISP header of the first (the one to be programmed).
See https://www.arduino.cc/en/Tutorial/ArduinoISP

During this process the bootloader get bypassed and overwritten, so to use the
Board as usual again you have to load the bootloader by ISP header again.

You should also check first whether the pins wirde on the Board are the ones
you need and that they are wired properly.

The Makefile should then may be contain something like:
	$(AVRDUDE) -c arduino -p atmega328p -b 57600 -P /dev/ttyUSB0 -C +$$TF\