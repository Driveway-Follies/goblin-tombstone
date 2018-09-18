/*******Interrupt-based Rotary Encoder Sketch*******

adapted from a sketch by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence

*/

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changejjjjjjjjd (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent
volatile byte headPos = 8;

int hide_max = 10000;
int hide_min = 1000;

int show_max = 5000;
int show_min = 500;

int state = 0;
int random_hide = random(hide_min, hide_max);
int random_show = random(show_min, show_max);
int choice = random(0, 2);

static int motorPinLeft = 5;
static int motorPinRight = 6;

static int zeroPin = 9; // Our second hardware interrupt pin is digital pin 3
int ledPin = 13;


void setup() {
  pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(zeroPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
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
    encoderPos--; //decrement the encoder's position count
    headPos=headPos-1;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  sei(); //restart interrupts
}


void PinB(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos++; //increment the encoder's position count
    headPos=headPos+1;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

long previousTime = millis();

void loop(){

  Serial.print("Encoder at: ");
  Serial.print(headPos); 
  Serial.print(", State: "); 
  Serial.print(state);
  int zeroed = digitalRead(zeroPin);
  /*
  if( digitalRead(zeroPin) == LOW ){
    digitalWrite(ledPin, HIGH); 
    state = 0;
  }else{
    digitalWrite(ledPin, LOW); 
  }
  */
  
  Serial.print(", Zero: "); 
  Serial.println(zeroed);
  
  // State 0 - Hide behind Tombstone
  if(state == 0){
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 0);
    if(millis() - previousTime > random_hide){
      //choice = 1;
      Serial.println(choice);
      //delay(random_hide);
      random_hide = random(hide_min, hide_max);
      if(choice == 0){
        state = 1;
      }else{
        state = 4;
      }
      choice = random(0, 2);
    }
  } 
  
  // State 1 - Pop out
  else if(state == 1){
    analogWrite(motorPinLeft, 255);
    analogWrite(motorPinRight, 0);
    // Head fully exposed, exit state
    if(headPos == 13){
      analogWrite(motorPinLeft, 0);
      analogWrite(motorPinRight, 0);
      previousTime = millis();
      state++;
    }
  }
  // State 2 - Show Goblin Head for X seconds
  else if(state == 2){
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 0);
    if(millis() - previousTime > random_show){
      //delay(random_show);
      // Exit state afer random delay
      random_show = random(show_min, show_max);
      state++;
    }
  }
  // State 3 - Pop back in
  else if(state == 3){
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 255);
    // Head hidden again, return to state 0
    if( zeroed == LOW ){
        digitalWrite(ledPin, HIGH); 
        analogWrite(motorPinLeft, 0);
        analogWrite(motorPinRight, 0);
        headPos = 8;
        previousTime = millis();
        state=0;
    }else{
      digitalWrite(ledPin, LOW); 
    }
  }
  
  // State 1 - Pop out
  if(state == 4){
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 255);
    // Head fully exposed, exit state
    if(headPos == 3){
      analogWrite(motorPinLeft, 0);
      analogWrite(motorPinRight, 0);
      //headPos = 2;
      previousTime = millis();
      state++;
    }
  }
  // State 2 - Show Goblin Head for X seconds
  else if(state == 5){
    analogWrite(motorPinLeft, 0);
    analogWrite(motorPinRight, 0);
    if(millis() - previousTime > random_show){
      //delay(random_show);
      // Exit state afer random delay
      random_show = random(show_min, show_max);
      state++;
    }
  }
  // State 3 - Pop back in
  else if(state == 6){
    analogWrite(motorPinLeft, 255);
    analogWrite(motorPinRight, 0);
    // Head hidden again, return to state 0
    if( zeroed == LOW ){
        digitalWrite(ledPin, HIGH); 
        analogWrite(motorPinLeft, 0);
        analogWrite(motorPinRight, 0);
        headPos = 8;
        previousTime = millis();
        state=0;
    }else{
      digitalWrite(ledPin, LOW); 
    }
  }
}
