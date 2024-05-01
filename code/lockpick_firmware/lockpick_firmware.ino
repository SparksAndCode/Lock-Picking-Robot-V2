#include <string.h>
#include <AccelStepper.h>

#define STEP_MAX 1500
#define STEP_INTERVAL 150
#define STEPS STEP_MAX / STEP_INTERVAL

// PIN CONFIG *******************************************
#define pin1HomePin 14
#define pin2HomePin 17
#define pin3HomePin 19
#define pin4HomePin 16
#define pin5HomePin 18

#define PIN_1_STEP  5  // x-step on shield 1 - d2
#define PIN_1_DIR   4  // x-dir on shield 1 - d5
#define PIN_2_STEP  3  // y-step on shield 1 - d3
#define PIN_2_DIR   2  // y-dir on shield 1 - d6
#define PIN_3_STEP  12 // z-step on shield 1 - d4
#define PIN_3_DIR   13 // z-dir on shield 1 - d7
#define PIN_4_STEP  11 // x-step on shield 2 - d2
#define PIN_4_DIR   10 // x-dir on shield 2 - d5
#define PIN_5_STEP  9  // z-step on shield 2 - d4
#define PIN_5_DIR   8  // z-dir on shield 2 - d7
#define TWIST_STEP  6  // twsit step
#define TWIST_DIR   7  // twist dir

#define ENCODER_A 21 // Pin for Encoder A
#define ENCODER_B 22 // Pin for Encoder B
// *******************************************************

// MOTOR PARAMS ******************************************
#define NUM_OF_BIND_MEASUREMENTS 5
#define TEST_ARRAY_SIZE 100
#define TEST_STEP_PULSE_OFFSET 1
#define TEST_DIFF_THRESH 5



volatile int encoder_value = 0; // Global variable for storing the encoder position

AccelStepper pin1Stepper;
AccelStepper pin2Stepper;
AccelStepper pin3Stepper;
AccelStepper pin4Stepper;
AccelStepper pin5Stepper;
AccelStepper twistStepper;
long target_positions [6];

bool isHomed = false;
// END MOTOR PARAMS **************************************

// Motor FUNCTIONS ******************************
bool debouncePin(int pin){
  int lastState = digitalRead(pin);
  int consectReads = 5;
  int readCounter = 1;
  while (readCounter < consectReads) {
    delay(1);
    int currentState = digitalRead(pin);
    if(currentState == lastState){
      readCounter ++;
    } else {
      lastState = currentState;
      readCounter = 0;
    }
  }
  return lastState;
}

bool isHomeSwitchTriggered (int pin) {
  return debouncePin(pin) == LOW;
}

void homeStepper(AccelStepper stepper, int homePin) {
  float originalSpeed = stepper.speed();
  
  if (isHomeSwitchTriggered(homePin)) {
    while(isHomeSwitchTriggered(homePin)) {
      stepper.runSpeed();
    }
    stepper.setCurrentPosition(0);
    if (originalSpeed < 0) {
      stepper.runToNewPosition(-800);
    } else {
      stepper.runToNewPosition(800);
    }
    
  } 
  
  stepper.setSpeed(-originalSpeed); // run in reverse
  while(isHomeSwitchTriggered(homePin) == false) {
    stepper.runSpeed();
  }
  stepper.setSpeed(originalSpeed);  // switch it back
  stepper.setCurrentPosition(0);
}
// End Motor FUNCTIONS ******************************

void encoder_isr() {
  // If the state of A changed, it means the encoder has been rotated
  if ((digitalRead(ENCODER_A) == HIGH) != (digitalRead(ENCODER_B) == LOW)) {
    encoder_value--;
  } else {
    encoder_value++;
  }
}

void homeAll() {
  homeStepper(pin1Stepper, pin1HomePin);
  homeStepper(pin2Stepper, pin2HomePin);
  homeStepper(pin3Stepper, pin3HomePin);
  homeStepper(pin4Stepper, pin4HomePin);
  homeStepper(pin5Stepper, pin5HomePin);
  isHomed = true;
}

void bruteForce(){
  Serial.println("Starting Brute Force");
  long totalTries = 0;
  for (long a=0; a < STEP_MAX; a += STEP_INTERVAL) {
    for (long b=0; b < STEP_MAX; b += STEP_INTERVAL) {
      for (long c=0; c < STEP_MAX; c += STEP_INTERVAL) {
        Serial.println("Trying " + String(a) + "," + String(b) + "," + String(c) + ",0,0");
        pin1Stepper.runToNewPosition(0);
        pin2Stepper.runToNewPosition(a * -1);
        pin3Stepper.runToNewPosition(b * -1);
        pin4Stepper.runToNewPosition(c * -1);
        pin5Stepper.runToNewPosition(0);

        testKey();
        // delay(200);
        totalTries ++;
        Serial.println("Total Tries " + String(totalTries));
      }
    }
  }
  pin1Stepper.runToNewPosition(0);
  pin2Stepper.runToNewPosition(0);
  pin3Stepper.runToNewPosition(0);
  pin4Stepper.runToNewPosition(0);
  pin5Stepper.runToNewPosition(0);
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  pin1Stepper = AccelStepper(1, PIN_1_STEP, PIN_1_DIR);
  pin2Stepper = AccelStepper(1, PIN_2_STEP, PIN_2_DIR);
  pin3Stepper = AccelStepper(1, PIN_3_STEP, PIN_3_DIR);
  pin4Stepper = AccelStepper(1, PIN_4_STEP, PIN_4_DIR);
  pin5Stepper = AccelStepper(1, PIN_5_STEP, PIN_5_DIR);
  twistStepper = AccelStepper(1, TWIST_STEP, TWIST_DIR);

  pin1Stepper.setMaxSpeed(2000);
  pin2Stepper.setMaxSpeed(2000);
  pin3Stepper.setMaxSpeed(2000);
  pin4Stepper.setMaxSpeed(2000);
  pin5Stepper.setMaxSpeed(2000);
  twistStepper.setMaxSpeed(2000);

  pin1Stepper.setAcceleration(3000);
  pin2Stepper.setAcceleration(3000);
  pin3Stepper.setAcceleration(3000);
  pin4Stepper.setAcceleration(3000);
  pin5Stepper.setAcceleration(3000);
  twistStepper.setAcceleration(2000);

  pin1Stepper.setSpeed(-3000);
  pin2Stepper.setSpeed(-3000);
  pin3Stepper.setSpeed(-3000);
  pin4Stepper.setSpeed(-3000);
  pin5Stepper.setSpeed(-3000);
  twistStepper.setSpeed(2000);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);
  // Attaching the ISR to encoder A
  attachInterrupt(digitalPinToInterrupt(ENCODER_A), encoder_isr, CHANGE);

  // SETUP MOTOR / Home Pin
  pinMode(pin1HomePin, INPUT_PULLUP);
  pinMode(pin2HomePin, INPUT_PULLUP);
  pinMode(pin3HomePin, INPUT_PULLUP);
  pinMode(pin4HomePin, INPUT_PULLUP);
  pinMode(pin5HomePin, INPUT_PULLUP);
  
  // bruteForce();
}

String generateStatus() {
  String status = "~:";
  status += String(millis());
  status += String(':');
  status += String(encoder_value);
  status += String(':');
  status += String(isHomeSwitchTriggered(pin1HomePin));
  status += String(':');
  status += String(isHomeSwitchTriggered(pin2HomePin));
  status += String(':');
  status += String(isHomeSwitchTriggered(pin3HomePin));
  status += String(':');
  status += String(isHomeSwitchTriggered(pin4HomePin));
  status += String(':');
  status += String(isHomeSwitchTriggered(pin5HomePin));
  status += String(':');
  status += String(pin1Stepper.currentPosition());
  status += String(':');
  status += String(pin2Stepper.currentPosition());
  status += String(':');
  status += String(pin3Stepper.currentPosition());
  status += String(':');
  status += String(pin4Stepper.currentPosition());
  status += String(':');
  status += String(pin5Stepper.currentPosition());
  status += String(':');
  status += String(twistStepper.currentPosition());
  status += String(':');
  status += String(isHomed);
  return status;
}

void sendStatus(){
  Serial.println(generateStatus());
}

void sendUpdate(String msg){
  Serial.print("#");
  Serial.println(msg);
}

void testKey(){
  // DO NOT REMOVE THE DELAYS IN HERE
  // REQUIRED FOR SOME BS REASON

  twistStepper.move(-150);
  bool inBind = false;
  int bindDiff = 12;
  
  int steps = 0;
  int lastEncoderValue = encoder_value;

  // move N steps to break binding free
  for (int x=0; x < 16 * 3; x++) {
    twistStepper.runSpeed();
    delay(30);
  }

  while (inBind == false && twistStepper.distanceToGo() != 0) {
    twistStepper.runSpeed();
    delay(60);
    bool encoder_moved = encoder_value != lastEncoderValue;
    if (encoder_moved) {
      // Encoder has moved
      delay(30);
      steps = 0;
      lastEncoderValue = encoder_value;
    } else {
      delay(30);
      if (steps == bindDiff) {
        delay(30);
        inBind = true;
      }
      steps ++;
    }
  }

  if (twistStepper.distanceToGo() == 0) {
    while (true) {
      sendUpdate("LOCK HAS BEEN PICKED");
      delay(1000);
    }
  } else {
    sendUpdate("Encoder_Value: " + String(encoder_value) + " Twist_Value: " + String(twistStepper.currentPosition()));
    twistStepper.runToNewPosition(0);
  }
}

void executeCommand(String command) {
  int commandLen = command.length();
  if (commandLen > 0) {
    if (command[0] == 'H') {
      homeAll();
    }

    if(command[0] == 'B') {
      bruteForce();
    }

    if (command[0] == 'X' || command[0] == 'M' || command[0] == 'T') {
      if (command[0] == 'M' || command[0] == 'X') {
        if (commandLen > 1) {
          command = command.substring(2, commandLen);
          processCommand(command);
          pin1Stepper.runToNewPosition(target_positions[0]);
          pin2Stepper.runToNewPosition(target_positions[1]);
          pin3Stepper.runToNewPosition(target_positions[2]);
          pin4Stepper.runToNewPosition(target_positions[3]);
          pin5Stepper.runToNewPosition(target_positions[4]);
          twistStepper.runToNewPosition(target_positions[5]);
        }
      }

      if (command[0] == 'T' || command[0] == 'X') {
        testKey();
      }
    }

    if (command[0] == 'R') {
      encoder_value = 0;
      twistStepper.setCurrentPosition(0);
    }
    sendStatus();
  }
}

void processCommand(String msg) {
  uint8_t i = 0, j = 0;
  String chunk = "";
  while (j < msg.length()){
    if (msg.charAt(j) == ':'){
      target_positions[i] = chunk.toInt() * -1;
      i++;
      chunk = "";
    } else {
      chunk += msg.charAt(j);
    }
    j++;
  }
  target_positions[i] = chunk.toInt() * -1;
}


void loop() {
  if (Serial.available()) {
    String command = Serial.readString();
    executeCommand(command);
  }
}