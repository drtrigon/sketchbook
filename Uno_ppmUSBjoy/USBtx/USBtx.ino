// *******************************
// USBtx by Rickey Ward
//
// Gamepad with Left and Right
// analog axis - converterd from 
// spektrum flight transmitter
//
// Ex version with calibration mode
//
// ******************************

//include VUSB Library
#include <HIDJoy.h>
//include RcTrainer library
//important! this include must come AFTER HIDJoy.h
#include <RcTrainer.h>


//Create instance of HIDJoystick
HIDJoy joy;
//Create instance of RcTrainer object
RcTrainer tx(1);

//-----------------Vars--------------------------
//vars to hold used configuration, preintialized to Spektrum DX6

//axis lows (top left)
int lxul = 1900;
int lyul = 1900;
int rxul = 1105;
int ryul = 1900;

//adis highs (bottom right)
int lxuh = 1105;
int lyuh = 1105;
int rxuh = 1900;
int ryuh = 1105;


void setup() {
  //start USB communication
  joy.begin();
}

void loop()
{
    //do usb stuff, important to be called often.
    joy.poll();

    //grab data, send it to usb
     
    uint8_t lx = tx.getChannel(3, lxul, lxuh, 0, 255);
    uint8_t ly = tx.getChannel(0, lyul, lyuh, 0, 255);
    uint8_t rx = tx.getChannel(2, rxul, rxuh, 0, 255);
    uint8_t ry = tx.getChannel(1, ryul, ryuh, 0, 255);
    
    joy.writeGame(lx,ly,rx,ry);
      
  
}//end loop
