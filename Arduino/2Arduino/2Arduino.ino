void setup() {
  Serial.begin(9600);
  pinMode(9, INPUT);
}

const float mmPerTilt = 0.173;
float mmTotal = 0;
bool state = 0;
bool previousState = 0;
char[8] out;

void loop() {
  state = digitalRead(9) < 470;
  if(state != previousState){
    mmTotal += mmPerTilt;
  }
  previousState = state;

  memset(out, 0x00, sizeof(out));
  out[0] = 0x06;
  memcpy(&out[1], &mmTotal, sizeof(float));
  Serial.write(0xAA);
  Serial.write(out, 7);
  Serial.write(0xBB);

  delay(500);
}
