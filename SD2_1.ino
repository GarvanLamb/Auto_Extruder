 
#include <AccelStepper.h> //Lib for stepper acceleration and movement
#include <Adafruit_MAX31865.h>

// Define two steppers and the pins they will use
#define dirPin 2
#define stepPin 3
AccelStepper stepper(1, stepPin, dirPin); //setting up the motor outputs

int rc;
//Amplifier setup
Adafruit_MAX31865 thermo = Adafruit_MAX31865(10, 11, 12, 13);
// The value of the Rref resistor. Use 430.0 for PT100 and 4300.0 for PT1000
#define RREF      430.0
// The 'nominal' 0-degrees-C resistance of the sensor
// 100.0 for PT100
#define RNOMINAL  100.0
//Temporary Temp Pin
float tempHigh;
float temp;
float tempLow;
float tempSense;
int tempPin = 2;
int iterator = 0;
int remainder;

//init motor settings
int maxspeed;
int Speed ;
int motorAccel;
//      stepper.setAcceleration(-motorAccel);

int x = 0;


int Limit_1 = 0; //interrupter 1
int Limit_2 = 1 ; //interrupter 2
int val_1 = 0; // variable to store the value coming from the sensor 1
int val_2 = 0; // variable to store the value coming from the sensor 1
bool flip1;// read that the limit 1 is flipped
bool flip2;//read the limit 2 is flipped
bool endcycle = false; // bool to end motor
int s = 0; // used to figure out end of one cycle

//Heat pad set up relay pin 7 start off
int relaypin = 7;
bool Heat = false;

//Used to get information from python GUI
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

// variables to hold the parsed data from GUI
char messageFromPC[numChars] = {0};
int Choice;
float Temp = 0;
int cycles = 0;
float Velocity = 0.0;

//checks for new data
boolean newData = false;

void setup() {
  Serial.begin(9600);// initialize serial communication:
  //declare thermo
  thermo.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary
  // Declare pins as output:
  pinMode(relaypin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(6, OUTPUT);  // initialize the LED pin as an output:

  //Heat pad starts off
  digitalWrite(relaypin,LOW);
}
void loop() {

  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    // this temporary copy is necessary to protect the original data
    //   because strtok() used in parseData() replaces the commas with \0
    parseData();
    //showParsedData();
    //Serial.Print(letter);
    newData = false;
  }

  // Takes Choice which is preprogrammed and picks one
  if (Choice == 1) { //Connection test
    Serial.print("test");
  }
  if (Choice == 3) { // Reset Motor position
    StepperReset();
  }
  if (Choice == 2) { //Run Process
    Serial.print(cycles);
    StepperRun();
  }
}

//These are all used to get the data from the GUI
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();

    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }

    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}
void parseData() {      // split the data into its parts

  char * strtokIndx; // this is used by strtok() as an index

  strtokIndx = strtok(tempChars, ",");     // get the first part - the string
  strcpy(messageFromPC, strtokIndx); // copy it to messageFromPC


  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  cycles = atoi(strtokIndx);     // convert this part to an integer

  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  Choice = atoi(strtokIndx);
  
  strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
  Temp = atof(strtokIndx);


  strtokIndx = strtok(NULL, ",");
  Velocity = atof(strtokIndx);     // convert this part to a float

}
void showParsedData() {
  Serial.print("Message ");
  Serial.println(messageFromPC);

  Serial.print("Integer ");
  Serial.println(cycles);
  Serial.print("Integer ");
  Serial.println(Choice);
  Serial.print("Integer ");
  Serial.println(Temp);
  Serial.print("Float ");
  Serial.println(Velocity);
}

//Functions to use stepper motor
void StepperRun() {
  getTemp();
  //delay(1000);
  while(tempSense < 0){
    getTemp();
    //delay(1000);
  }
  
  if (Temp >15 and Temp != 0){
    Heat = true;
    tempHigh = Temp*1.1;
    tempLow  = Temp*.9;
    digitalWrite(relaypin, HIGH);
    getTemp();
    while (tempSense < Temp){
        getTemp();
         digitalWrite(6,HIGH);
        delay(500);
        digitalWrite(6,LOW);
        delay(500);
    }
  }
  // turn off heater 
  digitalWrite(relaypin, LOW);
    
  // Now handled in python
  //Velocity =  12800*(Velocity/2.54);
  int Speed = Velocity;//Speed of the motor in steps
  int motorAccel = Speed *.5; //steps/second/second to accelerate
  int maxspeed = Speed *1.25;//max speed of the motor
  if (Speed <1500){
      remainder = 70000;
  }else if(Speed <2500 and Speed>1499){
      remainder = 60000;
  }else if(Speed <4500 and Speed>2499){
      remainder = 50000;
  }else{
      remainder = 40000;
  }
  

  stepper.enableOutputs();
  delay(500);
  s=0;
  while (cycles > 0) {        //cycles to run 

        // Read Limit sensors
    val_1 = analogRead(Limit_1); // read the value from the sensor 1
    val_2 = analogRead(Limit_2); // read the value from the sensor 2

    // checks if limit sensors are hit  or x ==12391  or 26059
    if (val_1 > 4 ) {
      flip1 = true;
      //endcycle= false;
    }else if (val_2 > 4  ) {
      flip2 = true;
      if (s == 1) {
        endcycle = true;
      }
    }
    
    //Sets counter clockwise motion when limit one is hit
    if (flip1 == true and endcycle == false) {
      stepper.setAcceleration(-motorAccel);
      stepper.setMaxSpeed(-maxspeed);
      stepper.setSpeed(-Speed);
      //digitalWrite(13, HIGH);
      flip1 = false;
      s = 1;
    }
     //Sets clockwise motion if limit 2 is hit
    else if ( flip2 == true and endcycle == false) {
      stepper.setAcceleration(motorAccel);
      stepper.setMaxSpeed(maxspeed);
      stepper.setSpeed(Speed);
      //digitalWrite(13, LOW);
      flip2 = false;
    } 
    //if one cycle is hit subtracts cycle 
    else if (endcycle == true ) {
      cycles = cycles - 1;
      s=0;
      x=0;
      endcycle = false;
      //If cycles are done end stepper motor
      if (cycles == 0) {
        stepper.stop();
       // stepper.disableOutputs();
        delay(2000);
        Choice = 8;
      }
    }
    stepper.runSpeed();
    x=x+1;
    if( x%remainder==0 and Heat ==true){
      
      tempSense = thermo.temperature(RNOMINAL, RREF);
      //stepper.runSpeed();
      if (tempSense < Temp and tempSense > 0){
        digitalWrite(relaypin,HIGH);
        digitalWrite(6,HIGH);
        stepper.runSpeed();
     }else if (tempSense>Temp){
        digitalWrite(relaypin,LOW);
        digitalWrite(6,LOW);
        stepper.runSpeed();
        }
    }
    // Pause Resume button
    if (Serial.available()>0){
        rc = Serial.read();
        //if command = H stop
        if(rc == 'H'){
          while (rc =='H'){
            if (Serial.available()>0){
              rc = Serial.read();
              }
            }
          }
        }

     }  
  //turn Heat off after cycle
  digitalWrite(relaypin,LOW);
  digitalWrite(6,LOW);
  
  Heat = false;
}

void StepperReset() {

  int Speed = Velocity;//Speed of the motor in steps
  int motorAccel = Velocity - 400; //steps/second/second to accelerate
  int maxspeed = Velocity + 800;//max speed of the motor
  stepper.enableOutputs();
  //delay(2000);
  flip1 = true;
  val_2 = analogRead(Limit_2); // read the value from the sensor 2
  if(val_2 > 4){
    cycles = 0;
  }
  while (cycles > 0) {    
    //val_1 = analogRead(Limit_1); // read the value from the sensor 1
    val_2 = analogRead(Limit_2); // read the value from the sensor 2
     if (val_2 > 50 ) {
     flip2 = true;
     endcycle = true; 
      }

    if (flip1 == true and endcycle == false) {
      stepper.setAcceleration(-motorAccel);
      stepper.setMaxSpeed(-maxspeed);
      stepper.setSpeed(-Speed);
      digitalWrite(13, HIGH);
      flip1 = false;
      s = 1;
    }
    else if (endcycle == true ) {
      stepper.stop();
      //stepper.disableOutputs();
      cycles = 0;
      endcycle = false;
      Choice = 8;
    }

    stepper.runSpeed();
  }
}


void getTemp(){
  uint16_t rtd = thermo.readRTD();
  float ratio = rtd;
  ratio /= 32768;
  tempSense = thermo.temperature(RNOMINAL, RREF);

}
