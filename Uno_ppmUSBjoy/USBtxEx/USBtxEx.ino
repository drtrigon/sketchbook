// *******************************
// USBtxEx by Rickey Ward
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
int mode = 0;
boolean ledon = false;
long lastmillisbtn = millis();
long lastmillisblink = millis();

//vars to hold used configuration, preintialized to Spektrum DX6

int lxul = 1900;
int lyul = 1900;
int rxul = 1105;
int ryul = 1900;

int lxuh = 1105;
int lyuh = 1105;
int rxuh = 1900;
int ryuh = 1105;

//temp vars used for calibration

int lxtl = 0;
int lytl = 0;
int rxtl = 0;
int rytl = 0;

int lxth = 0;
int lyth = 0;
int rxth = 0;
int ryth = 0;

void setup() {
  //set up calibration button pin and calibration mode led pin 
  pinMode(10, INPUT);
  pinMode(9, OUTPUT);
  //start USB communication
  joy.begin();
}

void loop()
{

    //get time of loop start
    long thistime = millis();
    
    //do usb stuff, important to be called often.
    joy.poll();

    //Standard mode, get the data from the transmitter and send it to USB Host
    if(mode == 0)
    {
      
      if(digitalRead(10) == LOW && (thistime - lastmillisbtn > 500 ))
      {
        mode = 1;
        lastmillisbtn = thistime;
      }
      
      uint8_t lx = tx.getChannel(3, lxul, lxuh, 0, 255);
      uint8_t ly = tx.getChannel(0, lyul, lyuh, 0, 255);
      uint8_t rx = tx.getChannel(2, rxul, rxuh, 0, 255);
      uint8_t ry = tx.getChannel(1, ryul, ryuh, 0, 255);
      
      joy.writeGame(lx,ly,rx,ry);
      
    }
    //LOW clibrate mode, get top left value from both axis of both sticks
    else if(mode == 1)
    {
      
      int lxtl = tx.getChannelRaw(3);
      int lytl = tx.getChannelRaw(0);
      int rxtl = tx.getChannelRaw(2);
      int rytl = tx.getChannelRaw(1);
      
      if(digitalRead(10) == LOW && (thistime - lastmillisbtn > 500 ))
      {
        mode = 2;
        lastmillisbtn = thistime;
        
        //user new values    
        lxul = lxtl;
        lyul = lytl;
        rxul = rxtl;
        ryul = rytl;
      }
      
      if(thistime - lastmillisblink > 500)
      {
        lastmillisblink = thistime;
        ledon = !ledon;
        if(ledon)
          digitalWrite(9,HIGH);
        else
          digitalWrite(9, LOW);
      }
    }
    //HIGH calibrate mode, get bottom left value from both axis of both sticks
    else if(mode == 2)
    {
      int lxth = tx.getChannelRaw(3);
      int lyth = tx.getChannelRaw(0);
      int rxth = tx.getChannelRaw(2);
      int ryth = tx.getChannelRaw(1);
      
      if(thistime - lastmillisblink> 250)
      {
        lastmillisblink = thistime;
        ledon = !ledon;
        if(ledon)
          digitalWrite(9,HIGH);
        else
          digitalWrite(9, LOW);
      }
      
      if(digitalRead(10) == LOW && (thistime - lastmillisbtn > 500 ))
      {
        mode = 0;
        lastmillisbtn = thistime;
        
        lxuh = lxth;
        lyuh = lyth;
        rxuh = rxth;
        ryuh = ryth;
        
        digitalWrite(9, LOW);
      }
      
    }//end mode

}//end loop
