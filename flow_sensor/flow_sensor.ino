int flowSensorPin = 2;
double flowRate;
volatile int count;


void setup() {
  pinMode(flowSensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), flow, RISING);

  Serial.begin(9600);
}

void loop() {
  count = 0;
  interrupts();
  delay(1000);
  noInterrupts();

  // start the math
  flowRate = (count * 2.25); // ml/s

  //flowRate = flowRate * 60;
  //flowRate = flowRate / 1000;

  Serial.println(flowRate);
}

void flow() {
  count++;
}
