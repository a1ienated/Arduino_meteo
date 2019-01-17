/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystal
 */

// include the library code:
#include <LiquidCrystal.h>
#include "DHT_NEW.h"
#include <OneWire.h>
DHT dht;
OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary)
float humidity;
float temperature;
unsigned long _dht1LRT = 0UL;
unsigned long _dht1Tti = 0UL;

bool _isTimer(unsigned long startTime, unsigned long period );
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  dht.setup(7);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("T: ");
  lcd.setCursor(9, 0);
  lcd.print("H: ");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
}

void loop() {
  if(_isTimer(_dht1Tti, 1000)) {
   if(_isTimer(_dht1LRT,( dht.getMinimumSamplingPeriod()))) {
      dht.readSensor();
      _dht1LRT = millis();
      _dht1Tti = millis();
   }
  }
  //delay(dht.getMinimumSamplingPeriod());
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  //Serial.print(dht.getStatusString());
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):

  //lcd.print(dht.getStatusString());
  lcd.setCursor(2, 0);
  if(temperature > 0)
    lcd.print(temperature);
  lcd.setCursor(11, 0);
  if(humidity > 0)
    lcd.print(humidity);

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  //lcd.setCursor(0, 1);
  if ( !ds.search(addr)) {
    // lcd.print("No more addresses.");
    ds.reset_search();
    delay(250);
    return;
  }

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      lcd.print("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      lcd.print("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      lcd.print("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      lcd.print("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  //fahrenheit = celsius * 1.8 + 32.0;
  lcd.setCursor(7, 1);
  lcd.print(celsius);
}

bool _isTimer(unsigned long startTime, unsigned long period ) {
  unsigned long currentTime;
  currentTime = millis();
  if (currentTime>= startTime) {return (currentTime>=(startTime + period));}
    else {return (currentTime >= (4294967295-startTime+period));}
}

