void setup() {
    Serial.begin(9600);
    pinMode(A0, INPUT_PULLUP);
}

void loop() {
    int sensor_value = analogRead(A0);
    Serial.println(sensor_value);
}
