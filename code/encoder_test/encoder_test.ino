#include <string.h>

#define ENCODER_A 21 // Pin for Encoder A
#define ENCODER_B 22 // Pin for Encoder B
#define ENCODER_Z 23 // Pin for Encoder Z
// *******************************************************

volatile int encoder_value = 0; // Global variable for storing the encoder position


void encoder_isr() {
  // If the state of A changed, it means the encoder has been rotated
  if ((digitalRead(ENCODER_A) == HIGH) != (digitalRead(ENCODER_B) == LOW)) {
    encoder_value--;
  } else {
    encoder_value++;
  }
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  pinMode(ENCODER_Z, INPUT_PULLUP);
  
  // Attaching the ISR to encoder A
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoder_isr, CHANGE);
}

void loop() {
  Serial.println(encoder_value);
  delay(200);
}