/*******Interrupt-based Rotary Encoder Sketch*******

adapted from a sketch by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence

*/

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
volatile byte headPos = 0;

int state = 0;
int random_hide = random(500, 10000);
int random_show = random(500, 10000);

static int motorPinLeft = 5;
static int motorPinRight = 6;

void setup() {
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(motorPinLeft, OUTPUT);
  pinMode(motorPinRight, OUTPUT);
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
  Serial.begin(9600); // start the serial monitor link
}

void PinA(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    headPos--;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}


void PinB(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    headPos++;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}


void loop(){
  if(oldEncPos != encoderPos) {
    Serial.println(encoderPos);
    oldEncPos = encoderPos;
  }

  if(state == 0){
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 0);
    //if(millis() - previousTime){
    delay(random_hide);
    random_hide = random(2000, 10000);
    state++;
  }else if(state == 1){
    if(headPos < 7){
      analogWrite(motorPinLeft, 255);
      analogWrite(motorPinRight, 0);
    }else{
      analogWrite(motorPinLeft, 0);
      analogWrite(motorPinRight, 0);
      state++;
    }

  }else if(state == 2){
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 0);
    delay(random_show);
    random_show = random(200, 4000);
    state++;
  }else if(state == 3){
    if(headPos > 1 && headPos < 9){
      analogWrite(motorPinLeft, 0);
      analogWrite(motorPinRight, 255);
    }else{
      analogWrite(motorPinLeft, 0);
      analogWrite(motorPinRight, 0);
      headPos = 0;
      state = 0;
    }
  }
}
