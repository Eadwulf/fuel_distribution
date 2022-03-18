#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h> // Temp sensor library

LiquidCrystal_I2C lcd(0x3F,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display


/// Temperture Sensor Variables and Pin Definition
OneWire  ds(10); // Temp sensor data pin 10

// variable to let the getTempC function know that it's its firts iteration
bool firstTime = true;

// variable to hold the value of the temperture sensor
float temp;

// count limit the number of times the temperture is measure
int count = 0;
int secondCounter = 0;

float highTemp = 30.00;
float lowTemp = 0.00;

void setup() {
  // LCD configuration
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  
  lcd.setCursor(0, 0);

  firstTime = false;
}

void loop() {
  checkStatus();
}

void getTempC() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius;

  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }

  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      //Serial.println("Device is not a DS18x20 family device.");
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  // 1000 ms delay
  if (!firstTime) {
    for (int i = 0; i < 20; i++) {
      // fuelSupply();
      delay(50);
    }
  } else {
    delay(1000);
  }
  
  // we might do a ds.depower() here, but the reset will take care of it.
  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

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
  lcd.print((float)raw / 16.0);
  //return celsius;
}

void checkStatus() {
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  getTempC();
  lcd.print(" C");
}
