int trig = A4;
int echo = A5;

float maxTank2Volume = 4000.00; //ml
float maxTank2Height = 20.00; //cm
float sensorOffset = 10.00; //cm

float volumePerLevel = maxTank2Volume / maxTank2Height; // ml/cm


void setup()
{
  Serial.begin(9600);
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT); 
}

void loop() {
  getFuelLevel();
  delay(100);
}

void printMessage(String message, float value) {
  Serial.println(message);
  Serial.println(value);
}

float getFuelLevel(){
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
  printMessage("Echo time: ", echoTime);
  delay(500);

  // Calculate distance from the sensor to the fuel
  fuelDistance = (echoTime / 58) - sensorOffset;
  printMessage("Fuel Distance: ", fuelDistance);
  delay(500);

  // Calculate fuel volumene for fuelDistance
  float fuelVolume = fuelDistance * volumePerLevel; // cm * ml/cm = ml
  printMessage("Fuel volume: ", fuelVolume);
  delay(500);

  // returning value
  return fuelVolume;
}
