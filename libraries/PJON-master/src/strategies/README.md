
#### What is a Strategy?
PJON codebase uses strategies to physically communicate through the medium used, abstracting the data link layer from its procedure. 9 strategies are proposed to communicate data through various media, take a look at the [strategies video introduction](https://www.youtube.com/watch?v=yPu45xoAHGg) for a brief showcase of their features. A Strategy is a class containing a set of methods used to physically send and receive data along with the required getters to handle retransmission and collision:

```cpp
bool begin(uint8_t additional_randomness = 0)
```
Returns `true` if the strategy is correctly initialized (receives a optional uint8_t used for randomness)

```cpp
uint32_t back_off(uint8_t attempts)
```
Returns the suggested delay related to the attempts passed as parameter

```cpp
bool can_start()
```
Returns `true` if the medium is free for use and `false` if the medium is in use by some other device

```cpp
void handle_collision()
```
Handles a collision

```cpp
uint8_t get_max_attempts()
```
Returns the maximum number of attempts in case of failed transmission

```cpp
void send_string(uint8_t *string, uint16_t length)
```
Sends a string of a certain length through the medium

```cpp
uint16_t receive_string(uint8_t *string, uint16_t max_length) { ... };
```
Receives a pointer where to store received information and an unsigned integer signaling the maximum string length. It should return the number of bytes received or `PJON_FAIL`.

```cpp
void send_response(uint8_t response) { ... };
```
Send a response to the packet's transmitter

```cpp
uint16_t receive_response() { ... };
```
Receives a response from the packet's receiver

You can define your own set of methods to use PJON with your own strategy on the medium you prefer. If you need other custom configuration or functions, those can be defined in your Strategy class. Other communication protocols could be used inside those methods to transmit and receive data:

```cpp
// Simple Serial data link layer implementation example
void send_response(uint8_t response) {
  Serial.print(response);
};
```

#### How to define a new strategy
To define the strategy you have only to create a new folder named for example `YourStrategyName` in `strategies`
directory and write the necessary file `YourStrategyName.h`:

```cpp
class YourStrategyName {
  public:
    uint32_t back_off(uint8_t attempts) { ... };
    bool     begin(uint8_t additional_randomness) { ... };
    bool     can_start() { ... };
    uint8_t  get_max_attempts() { ... };
    uint16_t receive_string(uint8_t *string, uint16_t max_length) { ... };
    uint16_t receive_response() { ... };
    void     send_response(uint8_t response) { ... };
    void     send_string(uint8_t *string, uint16_t length) { ... };
};
```

Simply add your code in the functions declaration shown above and instantiate PJON using the strategy type you
have created: `PJON<YourStrategyName> bus();`.
