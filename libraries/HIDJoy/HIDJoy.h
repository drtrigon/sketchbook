#ifndef HIDJoy_h
#define HIDJoy_h

#include "Arduino.h"
#include "Print.h"

#define HIDSERIAL_INBUFFER_SIZE 32


struct gamepad_report_t
{
  uint8_t buttons;
  int8_t left_x;
  int8_t left_y;
  int8_t right_x;
  int8_t right_y;
  int8_t ch5;
  int8_t ch6;
  int8_t ch7;
  int8_t ch8;
};  // size: 9 bytes

class HIDJoy : public Print {
public:
  HIDJoy();
  size_t write(uint8_t);  // write one character
  size_t write(const uint8_t *buffer, size_t size); // write a string

  size_t writeGame(int8_t Lx, int8_t Ly, int8_t Rx, int8_t Ry, int8_t ch5, int8_t ch6, int8_t ch7, int8_t ch8);

  static void poll();
  static unsigned char available();
  static unsigned char read(unsigned char *buffer);
  static void begin();
  struct name_t {
    /* data */
  };


private:
  size_t write8(const uint8_t *buffer, size_t size);  // write up to 8 characters
};

#endif
