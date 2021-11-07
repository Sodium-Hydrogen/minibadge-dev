//----------------------------------------//
// SAINTCON I2C EEPROM Programmer
//----------------------------------------//
// This will load data into the eeprom and
// change the address of a 24CWXXX style device
// Project by @Sodium_Hydrogen and @_bashNinja


#define BASE_ADDR 0x50
#define NEW_ADDR 0x52

#define STATUS_LED 10 // Minibadge debugger clock pin

#include <Wire.h>


char preamble[] = {0x00, 0x02, 0}; // Set last byte later in setup to length of text
char data[] = "Hello World";
int data_length = 0;


char combined_buff[200] = {0};

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  preamble[2] = strlen(data); // Store length of string

  // Copy data to combined_buff
  memcpy(combined_buff, preamble, sizeof(preamble));
  memcpy(&combined_buff[sizeof(preamble)], data, strlen(data));

  // Store length of data
  data_length = sizeof(preamble) + strlen(data);

}


void writeCharArray(){
  for(int i = 0; i < data_length; i++){
    // The i2c eeprom we used has a 32 byte write length.
    //  that wasn't working so we used 16 bytes before a reset
    if(i%16 == 0){ 
      Wire.endTransmission();
      delay(100);
      Wire.beginTransmission(BASE_ADDR);
      Wire.write(0);
      Wire.write(i);
    }
    Wire.write(combined_buff[i]);
  }
  Wire.endTransmission();
  
}

bool checkData() {
  Wire.beginTransmission(NEW_ADDR);
  Wire.write(0); // Start com and set two byte page pointer to 0
  Wire.write(0);
  Wire.endTransmission();
  delay(100); // Wait because things don't work if you don't
  for(int i = 0; i < data_length; i++){
    // This is to prevent the i2c buffer from storing too much data
    if(i%16 == 0){ 
      Wire.requestFrom(NEW_ADDR, 16);    // request 6 bytes from slave device #8
      delay(100); // Wait because things don't work if you don't
    }
    bool avail = Wire.available();
    char read_data = Wire.read();
    if(!avail || (combined_buff[i] != read_data) ){
      return false;
    }
  }
  return true;
}

void loop() {
  digitalWrite(STATUS_LED, LOW);
  while(true){
    Wire.beginTransmission(BASE_ADDR);
    // this returns a zero if the device exists at the address
    int error = Wire.endTransmission();
    if(error == 0){
      break;
    }

    Wire.beginTransmission(NEW_ADDR);
    if(Wire.endTransmission() == 0){
      if(checkData()){
        digitalWrite(STATUS_LED, HIGH);
        
      } else {
        // Swap back to old address
        Wire.beginTransmission(NEW_ADDR);
        Wire.write(0x80);
        Wire.write(0x00);
        Wire.write(0x40);
        Wire.write(0x40);
        Wire.endTransmission();
        delay(500);
      }
    }else{
      digitalWrite(STATUS_LED, LOW);
    }
  }
  digitalWrite(STATUS_LED, HIGH);
  delay(500);
  writeCharArray();
  delay(50);
  Wire.beginTransmission(BASE_ADDR);
  Wire.write(0x80); // Write to configuration pages
  Wire.write(0x00);
  Wire.write(0x40);
  Wire.write( 0x40 | ((NEW_ADDR & 1) << 5) | (NEW_ADDR & 0x7)); // Set new address
  Wire.endTransmission();
  delay(500);
}
