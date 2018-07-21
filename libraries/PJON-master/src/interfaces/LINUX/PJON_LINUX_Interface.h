
/* PJON Linux Interface
   ___________________________________________________________________________

    Copyright 2018 Fred Larsen

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License. */

#pragma once

#ifdef LINUX
  #include <stdint.h>
  #include <inttypes.h>
  #include <stdlib.h>
  #include <string.h>

  #include <chrono>
  #include <thread>
  #include <sstream>

  #define OUTPUT 1
  #define INPUT 0
  #define HIGH 1
  #define LOW 0
  #define INPUT_PULLUP 0x2
  #define LSBFIRST 1
  #define MSBFIRST 2

  auto start_ts = std::chrono::high_resolution_clock::now();
  auto start_ts_ms = std::chrono::high_resolution_clock::now();

  uint32_t micros() {
    auto elapsed_usec =
      std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start_ts
      ).count();

    if(elapsed_usec >= UINT32_MAX) {
      start_ts = std::chrono::high_resolution_clock::now();
      return 0;
    } else return elapsed_usec;
  };

  uint32_t millis() {
    return (uint32_t)
      std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start_ts_ms
      ).count();
  };

  void delayMicroseconds(uint32_t delay_value) {
    auto begin_ts = std::chrono::high_resolution_clock::now();
    while(true) {
      auto elapsed_usec =
        std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now() - begin_ts
        ).count();
      if(elapsed_usec >= delay_value) break;
      std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
  };

  void delay(uint32_t delay_value_ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_value_ms));
  }

  /* Generic constants ---------------------------------------------------- */

  #ifndef A0
    #define A0 0
  #endif

  #ifndef LED_BUILTIN
    #define LED_BUILTIN -1
  #endif

  /* LINUX IO system calls ------------------------------------------------ */

  #if !defined(PJON_ANALOG_READ)
    #define PJON_ANALOG_READ(P) 0
  #endif

  #if !defined(PJON_IO_WRITE)
    #define PJON_IO_WRITE(P, V)
  #endif

  #if !defined(PJON_IO_READ)
    #define PJON_IO_READ(P) 0
  #endif

  #if !defined(PJON_IO_MODE)
    #define PJON_IO_MODE(P, V)
  #endif

  #if !defined(PJON_IO_PULL_DOWN)
    #define PJON_IO_PULL_DOWN(P)
  #endif

  /* Random --------------------------------------------------------------- */

  #ifndef PJON_RANDOM
    #define PJON_RANDOM(randMax) (int)((1.0 + randMax) * rand() / ( RAND_MAX + 1.0 ) )
    /* Scale rand()'s return value against RAND_MAX using doubles instead of
       a pure modulus to have a more distributed result */
  #endif

  #ifndef PJON_RANDOM_SEED
    #define PJON_RANDOM_SEED srand
  #endif

  /* Timing --------------------------------------------------------------- */

  #ifndef PJON_DELAY
    #define PJON_DELAY delay
  #endif

  #ifndef PJON_DELAY_MICROSECONDS
    #define PJON_DELAY_MICROSECONDS delayMicroseconds
  #endif

  #ifndef PJON_MICROS
    #define PJON_MICROS micros
  #endif

  #ifndef PJON_MILLIS
    #define PJON_MILLIS millis
  #endif
#endif
