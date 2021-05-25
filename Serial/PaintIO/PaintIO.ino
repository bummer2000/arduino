/**
 * Takes in three parameters off of serial as comma separated text-encoded
 * data: BrushType, shapeSize, drawMode and draws a shape accordingly.
 * 
 * BrushType is either 0, 1, 2 corresponding to CIRCLE, SQUARE, TRIANGLE
 * shapeSize is a float between [0, 1] inclusive that corresponds to shape size
 * drawMode is either 0, 1 corresponding to FILL, OUTLINE
 * 
 * Designed to work with the p5.js app:
 *  - Live page: http://makeabilitylab.github.io/p5js/WebSerial/p5js/DisplayShapeBidirectional
 *  - Code: https://github.com/makeabilitylab/p5js/tree/master/WebSerial/p5js/DisplayShapeBidirectional
 * 
 * By Jon E. Froehlich
 * @jonfroehlich
 * http://makeabilitylab.io
 * 
 */

// #include <SPI.h> // Comment out when using i2c
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

enum BrushType {
  CIRCLE,
  SQUARE,
  TRIANGLE,
  NUM_BRUSH_TYPES
};

BrushType _curBrushType = CIRCLE;

enum BrushFillMode{
  FILL,
  OUTLINE,
  NUM_FILL_MODES
};

BrushFillMode _curBrushFillMode = FILL;

const int BRUSH_SELECTION_BUTTON_PIN = 4;
const int BRUSH_FILLMODE_BUTTON_PIN = 5;
const int CLEAR_DRAWING_BUTTON_PIN = 6;

const int BRUSH_X_ANALOG_INPUT = A0;
const int BRUSH_Y_ANALOG_INPUT = A1;
const int BRUSH_SIZE_ANALOG_INPUT = A2;
const int MAX_ANALOG_VAL = 1023; // change to 4095 if using 12-bit ADC

int _lastShapeSelectionButtonVal = HIGH;
int _lastDrawModeButtonVal = HIGH;
int _lastClearDrawingButtonVal = HIGH;

const long BAUD_RATE = 115200;
void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(BRUSH_SELECTION_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BRUSH_FILLMODE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CLEAR_DRAWING_BUTTON_PIN, INPUT_PULLUP);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  _display.clearDisplay();
  _display.setTextSize(1);      // Normal 1:1 pixel scale
  _display.setTextColor(SSD1306_WHITE); // Draw white text
  _display.setCursor(0, 0);     // Start at top-left corner
  _display.print("Waiting to receive\ndata from serial...");
  _display.println("\n");
  _display.print("Baud rate:");
  _display.print(BAUD_RATE);
  _display.print(" bps");
  _display.display();
}

void loop() {
  
  checkAndParseSerial();
  readBrushButtons();

  if(checkClearDrawingButton()){
   
    // draw clear to screen
    _display.setTextSize(3);
    int16_t x1, y1;
    uint16_t wText, hText;
    const String strCleared = "CLEARED";
    const String strDrawing = "DRAWING";

    int xCenter = _display.width() / 2;
    int yCenter = _display.height() / 2; 

    
    _display.clearDisplay();
    _display.getTextBounds(strCleared, 0, 0, &x1, &y1, &wText, &hText);
    int yText = yCenter - hText;
    _display.setCursor(xCenter - wText / 2, yText);
    _display.print(strCleared);

    yText += hText + 3;
    _display.getTextBounds(strDrawing, 0, 0, &x1, &y1, &wText, &hText);
    _display.setCursor(xCenter - wText / 2, yText);
    _display.print(strDrawing);
    _display.display();

    delay(1000);
    Serial.println("cls");
  }

  int xVal = analogRead(BRUSH_X_ANALOG_INPUT); 
  int yVal = analogRead(BRUSH_Y_ANALOG_INPUT); 
  int sizeVal = analogRead(BRUSH_SIZE_ANALOG_INPUT); 

  float xFrac = xVal / (float)(MAX_ANALOG_VAL);
  float yFrac = yVal / (float)(MAX_ANALOG_VAL);
  float sizeFrac = sizeVal / (float)(MAX_ANALOG_VAL);

  Serial.print(xFrac, 4);
  Serial.print(",");
  Serial.print(yFrac, 4);
  Serial.print(",");
  Serial.print(sizeFrac, 4);
  Serial.print(",");
  Serial.print(_curBrushType);
  Serial.print(", ");
  Serial.println(_curBrushFillMode);

  drawBrushInfo(xFrac, yFrac, sizeFrac);
  delay(10);
}

void drawBrushInfo(float xFrac, float yFrac, float sizeFrac){
  const int CHAR_HEIGHT = 8;
  const int MAX_MARGIN = (2 * CHAR_HEIGHT) + 2;
  const int MAX_BRUSH_SIZE = min(_display.width() - MAX_MARGIN, _display.height() - MAX_MARGIN);
  
  _display.clearDisplay();

  int brushSize = sizeFrac * MAX_BRUSH_SIZE;
  int halfBrushSize = brushSize / 2;
  int xCenter = _display.width() / 2;
  int yCenter = _display.height() / 2; 
  int xLeft =  xCenter - halfBrushSize;
  int yTop =  yCenter - halfBrushSize;

  String strBrushInfo;
  if(_curBrushType == CIRCLE){
    strBrushInfo = "Circle: ";
    if(_curBrushFillMode == FILL){
      _display.fillRoundRect(xLeft, yTop, brushSize, brushSize, halfBrushSize, SSD1306_WHITE);
    }else{
      _display.drawRoundRect(xLeft, yTop, brushSize, brushSize, halfBrushSize, SSD1306_WHITE);
    }
  }else if(_curBrushType == SQUARE){
    strBrushInfo = "Square: ";
    if(_curBrushFillMode == FILL){
      _display.fillRect(xLeft, yTop, brushSize, brushSize, SSD1306_WHITE);
    }else{
      _display.drawRect(xLeft, yTop, brushSize, brushSize, SSD1306_WHITE);
    }
  }else if(_curBrushType == TRIANGLE){
    strBrushInfo = "Triangle: ";
    int x1 = xCenter - halfBrushSize;
    int y1 = yCenter + halfBrushSize;

    int x2 = xCenter;
    int y2 = yCenter - halfBrushSize;

    int x3 = xCenter + halfBrushSize;
    int y3 = y1;

    if(_curBrushFillMode == FILL){
      _display.fillTriangle(x1, y1, x2, y2, x3, y3, SSD1306_WHITE);
    }else{
      _display.drawTriangle(x1, y1, x2, y2, x3, y3, SSD1306_WHITE);
    }
  }

  if(_curBrushFillMode == FILL){
    strBrushInfo += "Fill";
  }else{
    strBrushInfo += "Outline";
  }

  // Print out brush info
  int16_t x1, y1;
  uint16_t wText, hText;
  _display.setTextSize(1);
  _display.getTextBounds(strBrushInfo, 0, 0, &x1, &y1, &wText, &hText);
  _display.setCursor(xCenter - wText / 2, 0);
  _display.print(strBrushInfo);

  // Print out brush size beneath shape
  _display.setTextSize(1);
  _display.getTextBounds("X.XXX", 0, 0, &x1, &y1, &wText, &hText);
  _display.setCursor(xCenter - wText / 2, yCenter + halfBrushSize + 2);
  _display.print(sizeFrac, 3);

  _display.display();
}

/**
 * Checks the serial port for new data. Expects comma separated text lines with
 * <shape type>, <shape size fraction>, and <draw mode>
 */
void checkAndParseSerial(){
  // Check to see if there is any incoming serial data
  if(Serial.available() > 0){
    // If we're here, then serial data has been received
    // Read data off the serial port until we get to the endline delimeter ('\n')
    // Store all of this data into a string
    String rcvdSerialData = Serial.readStringUntil('\n'); 

    // Parse out the comma separated string
    int startIndex = 0;
    int endIndex = rcvdSerialData.indexOf(',');
    if(endIndex != -1){
      // Parse out the shape type, which should be 0 (circle), 1 (square), 2 (triangle)
      String strBrushType = rcvdSerialData.substring(startIndex, endIndex);
      int brushType = strBrushType.toInt();
      _curBrushType = (BrushType)brushType;

      // Parse out draw mode 0 (fill), 1 (outline)
      startIndex = endIndex + 1;
      endIndex = rcvdSerialData.length();
      String strBrushFillMode = rcvdSerialData.substring(startIndex, endIndex);
      int brushFillMode = strBrushFillMode.toInt();
      _curBrushFillMode = (BrushFillMode)brushFillMode;
    } 
    
    // Echo the data back on serial (for debugging purposes)
    // Prefix debug output with '#' as a convention
    Serial.print("# Arduino Received: '");
    Serial.print(rcvdSerialData);
    Serial.println("'");
  }
}

bool checkClearDrawingButton(){
  // Read the clear button val and write out to serial if active
  int clearDrawingButtonVal = digitalRead(CLEAR_DRAWING_BUTTON_PIN);
  bool buttonPress = false;
  if(clearDrawingButtonVal == LOW && clearDrawingButtonVal != _lastClearDrawingButtonVal){
    buttonPress = true;
  }

  _lastClearDrawingButtonVal = clearDrawingButtonVal;
  return buttonPress;
}

/**
 * Check the shape selection and draw mode buttons and update the global 
 * variables _curBrushType and _curDrawMode accordingly
 */
void readBrushButtons(){
  // Read the shape selection button (active LOW)
  int shapeSelectionButtonVal = digitalRead(BRUSH_SELECTION_BUTTON_PIN);
  if(shapeSelectionButtonVal == LOW && shapeSelectionButtonVal != _lastShapeSelectionButtonVal){
    // Increment the shape type
    _curBrushType = (BrushType)((int)_curBrushType + 1);

    // Reset back to CIRCLE if we've made it to NUM_SHAPE_TYPES
    if(_curBrushType >= NUM_BRUSH_TYPES){
      _curBrushType = CIRCLE;
    }
  }

  // Read the shape draw mode button val (active LOW)
  int shapeDrawModeButtonVal = digitalRead(BRUSH_FILLMODE_BUTTON_PIN);
  if(shapeDrawModeButtonVal == LOW && shapeDrawModeButtonVal != _lastDrawModeButtonVal){
    // Increment the draw mode
    _curBrushFillMode = (BrushFillMode)((int)_curBrushFillMode + 1);

    // Reset back to FILL if we've made it to NUM_DRAW_MODES
    if(_curBrushFillMode >= NUM_FILL_MODES){
      _curBrushFillMode = FILL;
    }
  }

  // Set last button values (so nothing happens if user just holds down a button)
  _lastShapeSelectionButtonVal = shapeSelectionButtonVal;
  _lastDrawModeButtonVal = shapeDrawModeButtonVal;
}