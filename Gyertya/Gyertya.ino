void setup() {
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop() {
  analogWrite(3, random(256));
  analogWrite(5, random(256));
  analogWrite(6, random(256));
  analogWrite(9, random(256));
  analogWrite(10, random(256));
  analogWrite(11, random(256));
  delay(30);
  //Serial.println(random(255));
}
