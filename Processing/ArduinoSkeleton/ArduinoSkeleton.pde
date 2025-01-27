/* 
 * This is skeleton code for continuously reading a single float value [0,1] off serial
 *
 * By Jon Froehlich
 * http://makeabilitylab.io
 * 
 */

import processing.serial.*;

// We communicate with the Arduino via the serial port
Serial _serialPort;

// Our serial port index (this will change depending on your computer)
final int ARDUINO_SERIAL_PORT_INDEX = 7; 

float _currentVal = 0.5; // between [0,1], received from serial

void setup(){
  size(640, 480);
  //fullScreen();
  
  // Open the serial port
  _serialPort = new Serial(this, Serial.list()[ARDUINO_SERIAL_PORT_INDEX], 9600);
}

void draw(){
  background(10); // set background color
  
  // TODO: do stuff with _currentVal
}

/**
* Called automatically when new data is received over the serial port. 
*/
void serialEvent (Serial myPort) {
  try {
    // Grab the data off the serial port. See: 
    // https://processing.org/reference/libraries/serial/index.html
    String inString = trim(_serialPort.readStringUntil('\n'));
    
    if(inString != null){
      _currentVal = float(inString); // convert to a float
      println("Read in: " + inString + " converted val: " + _currentVal);
    }
  }
  catch(Exception e) {
    println(e);
  }
}
