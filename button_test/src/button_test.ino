void setup() {
    Serial.begin(9600);
    pinMode(A0, INPUT_PULLUP);
}

void loop() {
    int sensor_value = analogRead(A0);
    if (sensor_value < 30) {
        Serial.println("Button #1");
    } else if (sensor_value < 50) {
        Serial.println("Button #2");
    } else if (sensor_value < 80) {
        Serial.println("Button #3");
    } else if (sensor_value < 102) {
        Serial.println("Button #4");
    } else if (sensor_value < 126) {
        Serial.println("Button #5");
    } else if (sensor_value < 146) {
        Serial.println("Button #6");
    } else if (sensor_value < 168) {
        Serial.println("Button #7");
    } else if (sensor_value < 190) {
        Serial.println("Button #8");
    } else {
        Serial.println("No button pressed.");
    }
}
