#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <stdlib.h>

#define rxPin 10
#define txPin 11
const int NUMBER_OF_CTD_FIELDS=4;
const int NUMBER_OF_BME_FIELDS=3;
const int PRECISION = 8;
const int BUFFERSIZE = 128;
  
Adafruit_BME280 bme; // I2C

// Set up a new SoftwareSerial object
SoftwareSerial CTDserial =  SoftwareSerial(rxPin, txPin);
int bmeStatus = 0;

float BMEfields[NUMBER_OF_BME_FIELDS];

void setup()  {
  Serial.begin(9600); 
  while (!Serial);
  bmeStatus = setupBme280();
  setupCTD();
}

void loop() {
  int status = 0;
  
  processInput();
  status = readCTDvalues();
  switch(status) {
  case -1:
      delay(50);
      break;
  case 3:
  case 4:
    readBMEvalues(BMEfields, bmeStatus);
    Serial.print(", ");
    Serial.print(BMEfields[0], PRECISION);Serial.print(", ");
    Serial.print(BMEfields[1], PRECISION);Serial.print(", ");
    Serial.print(BMEfields[2], PRECISION);
    Serial.println();
    delay(100);
    break;
  }
}

void setupCTD() {
  // Define pin modes for TX and RX
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  // Set the baud rate for the SoftwareSerial object
  CTDserial.begin(9600);
  Serial.println("CTD setup.");
}


void processInput(){
  char c;
  while (Serial.available()){
    c = Serial.read();
    CTDserial.write(c);
    Serial.write(c);
  }
}


int readCTDvalues(){
  char c;
  static int returnFound = 0;
  static char s[BUFFERSIZE];
  static int n=0;
  int status;

  // The end of a line is characterised by \r\n
  status = -1; // no character read at this cycle
  while (CTDserial.available() > 0) {
    status = 0;
    c = CTDserial.read();
    if (c == '\r'){}//ignore
    else if (c == '\n'){
      returnFound = 1;
    }
    else {
      s[n++] = c;
      Serial.write(c); // Does not write the \n
    }
  }

  if (returnFound == 1){
    returnFound = 0;
    s[n]='\0';
    status = estimateNumberOfValues(s, n);
    n = 0;
    strcpy(s, "");
    if ((status!=3) && (status!=4)){
      Serial.println();
      //Serial.write('\n'); //write \n always except when we have 3 or 4 values.
    }

  }
  return status;
}

int estimateNumberOfValues(char *s, int n){
  int status = 0;
  for(int i=0; i<n; ++i){
    if (s[i]==','){
      status++;
    }
    if ((s[i]>=65) && (s[i]<=122)){ // a letter occurred in string
      status = 0;
      break;
    }
  }
  if(status>0)
    status++;
  return status;
}
  
int setupBme280() {
    int status;

    status = bme.begin();
    if(status){
        Serial.println("BME280 setup.");
    }
    return status;
}



void readBMEvalues(float *bme_fields, int bmeStatus) {
  if (bmeStatus == 1){
    bme_fields[0] = bme.readTemperature();
    bme_fields[1] = bme.readPressure()/1e5;
    bme_fields[2] = bme.readHumidity();
  }
  else {
    bme_fields[0] = 0.0;
    bme_fields[1] = 0.0;
    bme_fields[2] = 0.0;
  }    
}
