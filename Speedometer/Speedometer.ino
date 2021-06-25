#include "U8glib.h"
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI

#define u8g_logo_sat_width 20
#define u8g_logo_sat_height 20

//satellite logo
const unsigned char u8g_logo_sat[] = {
  0x00, 0x01, 0x00, 0x80, 0x07, 0x00, 0xc0, 0x06, 0x00, 0x60, 0x30, 0x00,
  0x60, 0x78, 0x00, 0xc0, 0xfc, 0x00, 0x00, 0xfe, 0x01, 0x00, 0xff, 0x01,
  0x80, 0xff, 0x00, 0xc0, 0x7f, 0x06, 0xc0, 0x3f, 0x06, 0x80, 0x1f, 0x0c,
  0x80, 0x4f, 0x06, 0x19, 0xc6, 0x03, 0x1b, 0x80, 0x01, 0x73, 0x00, 0x00,
  0x66, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x70, 0x00, 0x00
};

// The serial connection to the GPS device
#include <SoftwareSerial.h>
static const int RXPin = 15, TXPin = 10;
static const uint32_t GPSBaud = 9600;
SoftwareSerial ss(RXPin, TXPin);

//GPS Library
#include "TinyGPS++.h"
TinyGPSPlus gps;

//Program variables
double Lat;
double Long;
int day, month, year;
int hour, minute, second;

int num_sat, gps_speed;
String direction;

const int ledCount = 10;    // the number of LEDs in the bar graph

int ledPins[] = {
  2, 3, 4, 5, 6, 14, A5, A4, A3, A2
};   // an array of pin numbers to which LEDs are attached

void setup() {
  Serial.begin(9600);
  
  ss.begin(GPSBaud);

  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }

  //Define ledpins as OUTPUT's
  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    pinMode(ledPins[thisLed], OUTPUT);
  }
}

void loop() 
{
  Get_GPS(); //Get GPS data;
  //Led_Bar(); //Adjust the LED bar

  //Display info in the OLED
  u8g.firstPage();
  do {
    print_speed();
  } while ( u8g.nextPage() );

  Serial.println();
}

/*
  u8g_font_10x20,   //крупные буквы
   u8g_font_4x6,     //очень мелкие буквы
   u8g_font_5x8,     //мелкие буквы
   u8g_font_6x10,    //минимально разборчивый шрифт
   u8g_font_6x12,    //разборчивый шрифт
   u8g_font_8x13B,   //толстые буквы, шрифт крупнее
   u8g_font_8x13,    //шрифт крупнее
   u8g_font_8x13O,   //шрифт крупнее, наклонный
   u8g_font_9x15B,   //крупный шрифт, толстый
   u8g_font_9x15,    //крупный шрифт
   u8g_font_9x18B,   //самый крупный шрифт, толстый
   u8g_font_9x18,    //самый крупный шрифт

   u8g_font_unifont // 7x15
   u8g_font_helvR24r // 17x_
*/

int get_position(int start_i, int max_el_size, int max_elements, int curent_value)
{
  int elems_in_val = 0;
  if(curent_value != 0)
  {
    while (curent_value > 0)
    {
      elems_in_val += 1;
      curent_value /= 10;
    }
  }
  else
  {
    elems_in_val = 1;
  }
  
  return start_i + int(max_el_size * (max_elements - elems_in_val) / 2);
}

void print_speed() 
{  
  u8g.setFont(u8g_font_6x10);
  
  u8g.setPrintPos(2, 10);
  u8g.print("Lat  ");
  u8g.setPrintPos(33, 10);
  u8g.print( Lat, 6);

  u8g.setPrintPos(2, 20);
  u8g.print("Long ");
  u8g.setPrintPos(33, 20);
  u8g.print( Long, 6);

  u8g.setPrintPos(70, 30);
  u8g.print(direction);

  u8g.setFont(u8g_font_helvR24r);
  u8g.setPrintPos(get_position(2,17,3,gps_speed), 60);
  u8g.print(gps_speed , DEC);

  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(60, 60);
  u8g.print("km/h");

  u8g.drawXBM(101, 0, u8g_logo_sat_width, u8g_logo_sat_height, u8g_logo_sat);
  
  u8g.setFont(u8g_font_unifont);
  //u8g.setPrintPos(100, 40);
  u8g.setPrintPos(get_position(100,7,3,num_sat), 40);  
  u8g.print(num_sat, 5);
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    Serial.print(ss.available());
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void Get_GPS()
{
  num_sat = gps.satellites.value();

  if (gps.location.isValid() == 1) {

    Lat = gps.location.lat();
    Long = gps.location.lng();
    gps_speed = gps.speed.kmph();
    direction = gps.cardinal(gps.course.value());
  }
  else
  {
    Serial.println("Gps.location is not valid.");
  }  
  
  if (gps.date.isValid())
  {
    day = gps.date.day();
    month = gps.date.month();
    year = gps.date.year();
    if (gps.time.isValid())
    {
      hour = gps.time.hour();
      minute = gps.time.minute();
      second = gps.time.second();

      Serial.print(day);
      Serial.print("-");
      Serial.print(month);
      Serial.print("-");
      Serial.print(year);
      Serial.print(" ");
      Serial.print(hour);
      Serial.print(":");
      Serial.print(minute);
      Serial.print(":");
      Serial.print(second);
      Serial.println();
    }
  }
  else
  {
    Serial.println("Gps.date is not valid.");
  }  

  smartDelay(1000);

  if (/*millis() > 5000 && */gps.charsProcessed() < 10)
  {
    Serial.println("No GPS detected: check wiring.");
  }
  else
  {
    Serial.print("__");
    Serial.print(gps.charsProcessed());
    Serial.print("__");
    
    Serial.println("GPS OK.");
  }
}

void Led_Bar()
{
  int ledLevel = map(gps_speed, 0, 190, 0, ledCount);

  for (int thisLed = 0; thisLed < ledCount; thisLed++) {
    // if the array element's index is less than ledLevel,
    // turn the pin for this element on:
    if (thisLed < ledLevel) {
      digitalWrite(ledPins[thisLed], HIGH);
    } else { // turn off all pins higher than the ledLevel:
      digitalWrite(ledPins[thisLed], LOW);
    }
  }
}
