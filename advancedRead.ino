#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <SoftwareSerial.h>

#define SS_PIN 10
#define RST_PIN 9
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
SoftwareSerial BTserial(3, 2); 

byte ouruid[4] = {0x82, 0xA2, 0x42, 0x03};
byte nuidPICC[4];
char pincode[6]={' ',' ',' ',' ',' ',' '};
char ourcode[6]={'A', '2', '5','#', 'D', '9',};
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
const char keys[ROWS][COLS] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};
byte rowPins[ROWS] = {4, 5, 6, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A2, A3, A4, A5}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup() { 
  Serial.begin(9600);
  BTserial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  for(int i=0;i<4;i++){
    ouruid[i] = EEPROM.read(i);
  }
  for(int i=0;i<6;i++){
    //EEPROM.write(i+4, ourcode[i]);
    ourcode[i] = EEPROM.read(i+4);
  }
}


bool unlocked = false;
bool locked = true;
int debounce = 0;
long prev = 0;
bool reset = false;
long prevPhone = millis();
long lastOK = millis();
char here[2];
bool phoneWasHere = false;
bool setNewPass = false;
 
void loop() {

  if(millis()-prevPhone>500){
    prevPhone=millis();
    BTserial.write('A');
    BTserial.write('T');
  }

  if(millis()-lastOK>2000){
    if(!phoneWasHere)unlock();
    phoneWasHere=true;
  }
  
  keyCheck();
  
  if(Serial.available()){
    int a = Serial.parseInt();
    if(a==1) reset = true;
    if(a==2) setNewPin();
    if(a==3) Serial.println("end");
  }

  if (BTserial.available())
    {  
        here[0]=here[1];
        here[1]=BTserial.read();
        //Serial.write(here[1]);
        if(here[0]=='O' && here[1]=='K'){
          if(phoneWasHere){
            Serial.println("Phone is lost!");
            Serial.println("lock");
            phoneWasHere=false;
          }
          lastOK = millis();
        }
    }
  
  if(debounce<0){
    if(!locked)Serial.println("lock");
    locked = true;
    debounce = 0;
  }
  rfid.PICC_IsNewCardPresent();
  if(rfid.PICC_ReadCardSerial()){
    //printHex(rfid.uid.uidByte, rfid.uid.size);Serial.println();
    //
    if(reset){
      for(int i=0; i<4; i++){
        ouruid[i] = rfid.uid.uidByte[i];
      }
      for(int i=0;i<4;i++){
        EEPROM.write(i, ouruid[i]);
      }
      reset = false;
    }
    debounce=3;
    if(locked && (rfid.uid.uidByte[0] == ouruid[0] || 
        rfid.uid.uidByte[1] == ouruid[1] || 
        rfid.uid.uidByte[2] == ouruid[2] || 
        rfid.uid.uidByte[3] == ouruid[3] )){
      unlock();
    }
    locked = false;
  }
  if(millis()-prev>1000){
    debounce--;
    prev = millis();
    //Serial.println(debounce);
  }

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }


  
}


void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] <   0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

void keyCheck(){
  if(setNewPass && pincode[0]!=' '){
    for(int j=0;j<6;j++){
      ourcode[j]=pincode[j];
      EEPROM.write(j+4, ourcode[j]);
    }
    setNewPass = false;
    Serial.println("newPinIsSet");
  }
  for(int j=0;j<6;j++) {
    if (ourcode[j] != pincode[j]){
      break;
    }
    if (j==5) {
      unlock();
      //Serial.println("Success!");
      for(int j=0;j<6;j++){
        pincode[j]=' ';
      }
    }
  }
  char key=keypad.getKey();
  if (key != NO_KEY) {
    //pincode[kCounter] = key;
    for(int j=0;j<5;j++){
      pincode[j]=pincode[j+1];
    }
    pincode[5]=key;
    //Serial.print(pincode[kCounter]);
  }
}

void setBluePin(){
  String command = "AT+PIN";
  for(int i=0; i<4; i++){
    command+=Serial.parseInt();
  }
  Serial.println(command);
}

void setNewPin(){
  Serial.println("setNewPin");
  setNewPass = true;
  for(int j=0;j<6;j++){
    pincode[j]=' ';
  }
}

void unlock(){
  Serial.println("unlock");
  delay(60);
  int rnd=-1;
  rnd=Serial.parseInt();
  int alt = rnd*37+317;
  Serial.println(alt);
}
