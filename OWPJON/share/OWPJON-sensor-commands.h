// more can be added as needed ...
// do not use 0x00 as this is the string terminator

#define READ_INFO  0x01  // return generic sensor info
#define READ_VCC   0x11  // return supply voltage
#define READ_TEMP  0x12  // return chip temperature
#define READ       0x21  // return memory data
#define WRITE      0x22  // store memory data
//#define READ_CAL  0x31
#define WRITE_CAL  0x32  // store calibration value in memory
//#include READ_RTC  ....

#define READ_DS2438_TEMP   0x41
#define READ_DS2438_CHA    0x42
#define READ_DS2438_CHB    0x43
#define READ_DS18x20_TEMP  0x44
#define READ_DS2438_ROM    0x4A
#define READ_DS18x20_ROM   0x4B
//#define READ_DS2438_RTC    ....

#define READ_SENS_TEMP  0x51
#define READ_SENS_HUM   0x52
#define READ_SENS_PRES  0x53
#define READ_LIGHT_BB   0x54
#define READ_LIGHT_UV   0x55
#define READ_LIGHT_VIS  0x56
#define READ_LIGHT_IR   0x57
#define READ_AUDIO_CHA  0x58
#define READ_AUDIO_CHB  0x59
#define READ_OPTION     0x5F  // READ_AS7265X
