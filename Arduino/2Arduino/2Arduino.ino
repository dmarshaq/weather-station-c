//Rain gauge (Hall effect sensor)

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
}

const float mmPerTilt = 0.173;
float mmTotal = 0;
bool state = 0;
bool previousState = 0;
void loop() {
  state = analogRead(A0) < 470;
  if(state != previousState){
    mmTotal += mmPerTilt;
  }
  delay(500);
  previousState = state;
  Serial.print("Total rainfall: ");
  Serial.print(mmTotal);
  Serial.print("\n");
  Serial.println(analogRead(A0));
}
