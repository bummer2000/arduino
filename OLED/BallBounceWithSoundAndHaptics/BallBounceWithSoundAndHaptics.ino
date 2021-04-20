
/**
 *  Draws a bouncing ball on the OLED display
 *
 *  Based on: https://makeabilitylab.github.io/p5js/Animation/BallBounce2D
 *  Source: https://github.com/makeabilitylab/p5js/tree/master/Animation/BallBounce2D
 *
 *  Adafruit Gfx Library:
 *  https://learn.adafruit.com/adafruit-gfx-graphics-library/overview 
 *
 *  Adafruit OLED tutorials:
 *  https://learn.adafruit.com/monochrome-oled-breakouts
 *  
 *  By Jon E. Froehlich
 *  @jonfroehlich
 *  http://makeabilitylab.io
 *
 */

#include <Wire.h>
#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int TONE_OUTPUT_PIN = 5;
const int VIBRO_MOTOR_OUTPUT_PIN = 7;
const int DELAY_LOOP_MS = 5; // change to slow down how often to read and graph value

// Ball variables
const int _ballRadius = 5;
int _xBall = 0;
int _yBall = 0;
int _xSpeed = 0;
int _ySpeed = 0;

// for tracking fps
unsigned long _totalFrameCount = 0;
unsigned long _startTimeStamp = 0;

// status bar
const boolean _drawStatusBar = true; // change to show/hide status bar

void setup() {
  Serial.begin(9600);
  
  pinMode(TONE_OUTPUT_PIN, OUTPUT);
  pinMode(VIBRO_MOTOR_OUTPUT_PIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);
  display.println("Screen initialized!");
  display.display();
  delay(500);
  display.clearDisplay();

  // Initialize ball to center of screen
  _xBall = display.width() / 2;
  _yBall = display.height() / 2;

  // Gets a random long between min and max - 1
  // https://www.arduino.cc/reference/en/language/functions/random-numbers/random/
  _xSpeed = random(1, 4);
  _ySpeed = random(1, 4);

  //Serial.println((String)"_xBall:" + _xBall + " _xBall:" + _xBall + " _xSpeed:" + _xSpeed + " _ySpeed:" + _ySpeed);
}

void loop() {
  if(_startTimeStamp == 0){
    _startTimeStamp = millis();
  }

  display.clearDisplay();
  //digitalWrite(VIBRO_MOTOR_OUTPUT_PIN, LOW); // turn off vibro motor
  
  if(_drawStatusBar && _totalFrameCount > 0){
    int16_t x1, y1;
    uint16_t w, h;
    unsigned long elapsedTime = millis() - _startTimeStamp;
    float fps = _totalFrameCount / (elapsedTime / 1000.0);
    display.getTextBounds("XX.XX fps", 0, 0, &x1, &y1, &w, &h);
    display.setCursor(display.width() - w, 0);
    display.print(fps);
    display.print(" fps");
  }
  
  // Update ball based on speed location
  _xBall += _xSpeed;
  _yBall += _ySpeed;

  // Check for ball bounce
  if(_xBall - _ballRadius <= 0 || _xBall + _ballRadius >= display.width()){
    _xSpeed = _xSpeed * -1; // reverse x direction

    // Play tone when ball hits wall
    // See: https://www.arduino.cc/reference/en/language/functions/advanced-io/tone/
    tone(TONE_OUTPUT_PIN, 100, 200);

    digitalWrite(VIBRO_MOTOR_OUTPUT_PIN, HIGH);
  }
  
  if(_yBall - _ballRadius <= 0 || _yBall + _ballRadius >= display.height()){
    _ySpeed = _ySpeed * -1; // reverse y direction

    // Play slightly higher tone when ball hits floor or ceiling
    tone(TONE_OUTPUT_PIN, 200, 200);
    digitalWrite(VIBRO_MOTOR_OUTPUT_PIN, HIGH);
  }

  // Draw circle
  display.drawCircle(_xBall, _yBall, _ballRadius, SSD1306_WHITE);
  
  // Render buffer to screen
  display.display();
  _totalFrameCount++;

  if(DELAY_LOOP_MS > 0){
    delay(DELAY_LOOP_MS);
  }

  //Serial.println((String)"_xBall:" + _xBall + " _xBall:" + _xBall + " _xSpeed:" + _xSpeed + " _ySpeed:" + _ySpeed);
}