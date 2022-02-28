#include <LiquidCrystal.h>
#include <OneWire.h> // Temp sensor library


/// Ultrasonic Sensor Variables and Pin Definition
int trig = A4;
int echo = A5;

// Tank 1 Parameters (Station's Tank)
float fuelLevel = 5000.00;

/// Tank 2 parameters
float maxTank2Volume = 4000.00; //ml
float maxTank2Height = 20.00; //cm
float sensorOffset = 10.00; //cm
float volumePerLevel = maxTank2Volume / maxTank2Height; // ml/cm

/// LCD definition and Inicialization
const int rs = 12, en = 11, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/// Temperture Sensor Variables and Pin Definition
OneWire  ds(10); // Temp sensor data pin 10

// variable to let the getTempC function know that it's its firts iteration
bool firstTime = true;

// variable to hold the value of the temperture sensor
float temp;

// count limit the number of times the temperture is measure
int count = 0;
int secondCounter = 0;

// Temperture normal range (0 > temp < 30) (°C)
float highTemp = 30.00;
float lowTemp = 0.00;

/// Potentiomer For Fuel Level Input
int fuelInput = A0;
int potValue = 0; // variable to hold potentiometer value

// Buttons to initialize fuel supply
const int fuelButton = 8;

// Button to cancel any operation in execution
const int cancelButton = 9;

// Variables to hold buttons state
int fuelButtonState = 0;
int cancelButtonState = 0;


void setup() {
  // LCD configuration
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  
  // Settings for the ultrasonic sensor
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT); 

  pinMode(fuelButton, INPUT);
  pinMode(cancelButton, INPUT);

  welcomeMessage(); // print on LCD a start message
  checkStatus();    // print temp and tank level
  firstTime = false;
}

void loop() {
  fuelSupply();
  checkStatus();
}

// Helper functions definition
void oneByOneLetterWithAutoscroll(String message, int cursorPos) {
  lcd.setCursor(0, cursorPos);

  for (int i = 0; i < message.length(); i++) {
    lcd.print(message[i]);
    delay(20);
  }

  if (message.length() > 16) {
    for (int i = 0; i < message.length() - 16; i++) {
      lcd.scrollDisplayLeft();
      delay(20);
    }
  }
}

void oneByOneLetter(String row_0 = "", String row_1 = "") {
  lcd.clear();

  if (row_0 != "") {
    oneByOneLetterWithAutoscroll(row_0, 0);
  }

  if (row_1 != "") {
    oneByOneLetterWithAutoscroll(row_1, 1);
  }
}

float getTempC() {
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
      fuelSupply();
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
  celsius = (float)raw / 16.0;
  return celsius;
}

String formatValueForLCD(float value, int ch) {
  int valueLength = String(value).length();
  String formattedValue = String(value);

  if (valueLength < ch);
    for (int i = 0; i < ch - valueLength; i++) {
      formattedValue  = "0" + formattedValue;
    }
    return formattedValue;
 }

void clearAtCursor(int column, int row) {
  lcd.setCursor(column, row);
  lcd.print("                ");
  lcd.setCursor(column, row);
}

void welcomeMessage() {
  oneByOneLetter("Bienvenido!");
  delay(350);
  lcd.clear();
  oneByOneLetter("Iniciando el", "sistema...");
  delay(350);
  lcd.clear();
}

void checkStatus() {
  temp = getTempC();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nivel: ");
  lcd.print(int(fuelLevel));
  lcd.print(" ml");
  
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print(" C");
}

void fuelSupply() {
  fuelButtonState = digitalRead(fuelButton);
  cancelButtonState = digitalRead(cancelButton);

  if (fuelButtonState == HIGH) {
    if (temp < lowTemp or temp > highTemp) {
      lcd.clear();
      lcd.print("Pelirgo!");
      lcd.setCursor(0, 1);

      if (temp > highTemp) {
        lcd.print("Temp > ");
        lcd.print(highTemp);
      }
      else if (temp < lowTemp) {
        lcd.print("Temp < ");
        lcd.print(lowTemp);
      }
      lcd.print(" C");
      delay(1000);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      return;
    }
    else {
      lcd.clear();
      lcd.print("Distribuir:");
      lcd.setCursor(0, 1);
      delay(50);
      lcd.print(".");
      delay(50);
      lcd.print(".");
      delay(50);
      lcd.print(".");
      delay(50);
      lcd.setCursor(0, 1);
      delay(50);
    
      float quantityPerBit = fuelLevel / 1023;
      float quantityToSupply = 0.0;
    
      // Read the pot value and determine the quantity of gas to e supplied
      while(true) {
        fuelButtonState = digitalRead(fuelButton);
        cancelButtonState = digitalRead(cancelButton);
    
        if (cancelButtonState == HIGH) {
          lcd.clear();
          lcd.print("Distribución");
          lcd.setCursor(0, 1);
          lcd.print("cancelada");
          delay(250);
          break;
        }
        if (fuelButtonState == HIGH) {
          lcd.clear();
          fuelLevel = fuelLevel - quantityToSupply;

          // function to measure the supplied fuel
              //  while suppliedFuel < quantityToSupply
                      // continue
                  //  print "fuel supplied"
                  
          lcd.print("Fuel supplied");
          delay(250);
          lcd.clear();
          break;
        }
    
        // Determine the quantity to be supplied
        potValue = analogRead(fuelInput);
        quantityToSupply = potValue * quantityPerBit;
        float volumeAvailableOnSecondTank = getVolumeAvailableOnSecondTank();

        if (quantityToSupply > volumeAvailableOnSecondTank) {
          lcd.clear();
          lcd.print("Alerta!");
          lcd.setCursor(0, 1);
          String alertMessage = "La cantidad selecciona supera la capacidad disponible en el tanque a ser suministrado";
          
          oneByOneLetterWithAutoscroll(alertMessage, 1);
          delay(200);
          
          lcd.clear();
          oneByOneLetter("Capacidad", "disponible: ");
          lcd.print(volumeAvailableOnSecondTank);
          delay(500);
          
          oneByOneLetter("La cantidad a suministrar fue modificada");
          delay(200);
          lcd.clear();

          quantityToSupply = volumeAvailableOnSecondTank;
          lcd.print("Distribuir:");
        }
        
        lcd.setCursor(0, 1);
        lcd.print(formatValueForLCD(quantityToSupply, String(fuelLevel).length()));
        lcd.print(" ml");
      } // while loop
    } // else statement
  } // if fuelButtonState
} // void

void printMessage(String message, float value) {
  Serial.println(message);
  Serial.println(value);
}

float getFuelVolumeOnSecondTank(){
  long echoTime = 0; // holds time transcurred from the trig singnal to the echo signal
  long fuelDistance = 0; // holds distance from sensor to the fuel
  
  // Transmitting pulse
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  // Waiting for pulse
  echoTime = pulseIn(echo, HIGH);
  //printMessage("Echo time: ", echoTime); // Debug
  //delay(500);

  // Calculate distance from the sensor to the fuel
  fuelDistance = (echoTime / 58) - sensorOffset;
  //printMessage("Fuel Distance: ", fuelDistance); // Debug
  //delay(500);

  // Calculate fuel volumene for fuelDistance
  float fuelVolume = fuelDistance * volumePerLevel; // cm * ml/cm = ml
  //printMessage("Fuel volume: ", fuelVolume); // Debug
  //delay(500);

  // returning value
  return fuelVolume;
}

float getVolumeAvailableOnSecondTank() {
  float currentTankVolume = getFuelVolumeOnSecondTank();
  float availableVolume = maxTank2Volume - currentTankVolume;
  
  return availableVolume;
}
