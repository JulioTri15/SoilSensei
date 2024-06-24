//
//
// Date May. 10 2024 5:57PM

#include <SPI.h>
#include <Wire.h>
#include <WiFi.h> 
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#define OLED_SDA 21
#define OLED_SCL 22
Adafruit_SH1106 display(21, 22);
#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

WiFiServer server(80);
IPAddress    apIP(192,168,20,24);    // Defining a static IP address: local & gateway
                                    // Default IP in AP mode is 192.168.4.1
const char *Ssid = "SoilSensei_2024";
const char *Password = "123456789";

int dataA = 0;
int dataB = 0;

String Srxdata;

String senderNumber;

/// SENSORS OUTPUT
float   ph_val    = 0;
int     nitro_val = 0;
int     phos_val  = 0;
int     pota_val  = 0;
int     moist_val = 0;

/// SENSE COUNTER
int     Cph       = 0;
int     Cnitro    = 0;
int     Cphos     = 0;
int     Cpota     = 0;
int     Cmoist    = 0;

/// SENSE FLAG TRIGGER
int     Sph       = 0;
int     Snitro    = 0;
int     Sphos     = 0;
int     Spota     = 0;
int     Smoist    = 0;


/// SENSE TIME DELAY
int     nitro_dly    = 0;
int     phos_dly     = 0;
int     pota_dly     = 0;
int     moist_dly    = 0;

/// SENSE TIMER

int     nitro_tmr    = 0;
int     phos_tmr     = 0;
int     pota_tmr     = 0;
int     moist_tmr    = 0;

int     hour_pulse     = 0;
int     hour_reff      = 3600; /// 3600

int     nitro_hour     = 0;
int     phos_hour      = 0;
int     pota_hour      = 0;
int     moist_hour     = 0;

/// MOTOR PUMP PIN
const int nitro_rly   = 12;
const int phos_rly    = 26;
const int pota_rly    = 27;
const int moist_rly   = 14;

int     sen_counter = 0;

const byte ph[]    = {0x02, 0x05, 0x00, 0x07, 0x00, 0x03, 0x61, 0x3B}; // 
const byte nitro[] = {0x02,0x05, 0x00, 0x1f, 0x00, 0x03, 0xe43, 0x4c};//
const byte phos[]  = {0x02,0x05, 0x00, 0x1e, 0x00, 0x03, 0xb52, 0x6c};// 
const byte pota[]  = {0x02,0x05, 0x00, 0x21, 0x00, 0x03, 0x81, 0xc7}; // 
byte values[11];

unsigned long time_now = 0; /// time delay millis
void setup(){                
  Serial.begin(115200);             // Serial port to computer  
  Serial.println("ESP32_SoilSensei"); 
  Serial2.begin(9600);  // 
  delay(1000);
  EEPROM.begin(512);  //
  pinMode(nitro_rly,OUTPUT); 
  pinMode(phos_rly,OUTPUT); 
  pinMode(pota_rly,OUTPUT); 
  pinMode(moist_rly,OUTPUT);  
  delay(1000); 
  Initialize_LCD();
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(Ssid, Password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();   
  delay(1000);
  }

void loop(){
    if(millis() > time_now + 1000){ ////
    time_now = millis(); 
    pulse_sec();
    pulse_hr();
    rlypin();
    sensors();
    disp_all();
    } 
    get_data();
    }

void pulse_sec(){
     if (nitro_dly != 0){
     nitro_dly --;  
     }                                  
 if (phos_dly != 0){
     phos_dly --;  
     }                                 
 if (pota_dly != 0){
     pota_dly --;  
     }                                 
 if (moist_dly != 0){
     moist_dly --;  
     }                                 
     }
     
void pulse_hr(){
     nitro_hour ++;
     if (nitro_hour >= hour_reff){  //// set to hour 60
     nitro_hour = 0;
     if (nitro_tmr != 0){
     nitro_tmr --;  
     }             
     }
       
     phos_hour ++;
     if (phos_hour >= hour_reff){  //// set to hour 60
     phos_hour = 0;
     if (phos_tmr != 0){
     phos_tmr --;  
     }    
     }
 
     pota_hour ++;
     if (pota_hour >= hour_reff){  //// set to hour 60
     pota_hour = 0;
     if (pota_tmr != 0){
     pota_tmr --;  
     }        
     }

     moist_hour ++;
     if (moist_hour >= hour_reff){  //// set to hour 60
     moist_hour = 0;
     if (moist_tmr != 0){
     moist_tmr --;  
     }             
     }
     }
 
void rlypin(){
     if (nitro_dly != 0){
     digitalWrite(nitro_rly,HIGH);  
     }else{ 
     digitalWrite(nitro_rly,LOW);                                 
     }
 if (phos_dly != 0){
     digitalWrite(phos_rly,HIGH);  
     }else{ 
     digitalWrite(phos_rly,LOW);                                 
     }
 if (pota_dly != 0){
     digitalWrite(pota_rly,HIGH);  
     }else{ 
     digitalWrite(pota_rly,LOW);                                 
     }
 if (moist_dly != 0){
     digitalWrite(moist_rly,HIGH);  
     }else{ 
     digitalWrite(moist_rly,LOW);                                 
     }
     }

void test_nitro(){
     if ((nitro_val < 15 )&&(nitro_tmr == 0)) {
     Cnitro ++;
     if (Cnitro > 4 ){
     Cnitro = 0; 
     reset_nitro();
     ///
     }
     }else{
     Cnitro = 0;  
     }
     }

void test_phos(){
     if ((phos_val < 25 )&&(phos_tmr == 0)) {
     Cphos ++;
     if (Cphos > 4 ){
     Cphos = 0; 
     reset_phos();
     ///
     }
     }else{
     Cphos = 0;  
     }
     }

void test_pota(){
     if ((pota_val < 60 )&&(pota_tmr == 0)) {
     Cpota ++;
     if (Cpota > 4 ){
     Cpota = 0; 
     reset_pota();
     ///
     }
     }else{
     Cpota = 0;  
     }
     }

void test_moist(){
     if ((moist_val < 50 )&&(moist_tmr == 0)) {
     Cmoist ++;
     if (Cmoist > 4 ){
     Cmoist = 0; 
     reset_moist();
     ///
     }
     }else{
     Cmoist = 0;  
     }
     }

void reset_nitro(){
     load_nitro_dly(); /// load pump delay
     load_nitro_tmr(); /// load pump tmr
     nitro_hour = 0;
     }

void  reset_phos(){
     load_phos_dly(); /// load pump delay
     load_phos_tmr(); /// load pump tmr
     phos_hour = 0;
     }

void  reset_pota(){
     load_pota_dly(); /// load pump delay
     load_pota_tmr(); /// load pump tmr
     pota_hour = 0;
     }
     
void  reset_moist(){
     load_moist_dly(); /// load pump delay
     load_moist_tmr(); /// load pump tmr
     moist_hour = 0;
     }

////
 void get_data(){ 
    WiFiClient client = server.available();
  while (!client.available()) {
  delay(1);
  }
  String req = client.readStringUntil('\r');  
  req = req.substring(req.indexOf("/") + 1, req.indexOf("HTTP") - 1);     
  client.print ("HTTP/1.1 200 OK\n\n"); 
  client.flush();

  if (req != 0){   
  Srxdata = (req); // do command    
  Serial.println(Srxdata);////////////////////
  sel_string();
  }        
  delay(5);
  }

void sel_string(){
       int  stdex;
       int  stdexe;      
       String Srx1;
       String Srx2;            
       Srx1  =  Srxdata.substring(stdex,stdexe);    
       stdex++;     
       Srx2   =  Srxdata.substring(stdex,stdexe);   
       Srxdata = ""; 
       Serial.println(Srx1);
       Serial.println(Srx2); 
       //
       if (Srx1 == "NITRO_ON"){
       load_nitro_dly(); 
       }else if (Srx1 == "PHOS_ON"){ 
       load_phos_dly();
       }else if (Srx1 == "PHOTA_ON"){ 
       load_pota_dly(); 
       }else if (Srx1 == "MOIST_ON"){ 
       load_moist_dly(); 
       ///
       }else if (Srx1 == "NITRO_DLY"){
       senderNumber = Srx2;
       save_nitro_dly();
       }else if (Srx1 == "PHOS_DLY"){ 
       senderNumber = Srx2;
       save_phos_dly();
       }else if (Srx1 == "PHOTA_DLY"){ 
       senderNumber = Srx2;
       save_pota_dly();
       }else if (Srx1 == "MOIST_DLY"){ 
       senderNumber = Srx2;
       save_moist_dly();
       ///
       }else if (Srx1 == "NITRO_TMR"){
       senderNumber = Srx2;
       save_nitro_tmr();
       }else if (Srx1 == "PHOS_TMR"){ 
       senderNumber = Srx2;
       save_phos_tmr();
       }else if (Srx1 == "PHOTA_TMR"){ 
       senderNumber = Srx2;
       save_pota_tmr();
       }else if (Srx1 == "MOIST_TMR"){ 
       senderNumber = Srx2;
       save_moist_tmr();
       }          
       }


//////

void Initialize_LCD(){
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SH1106_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
  display.clearDisplay();
  // text display tests
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(4,10);
  display.print("SOILSENSI");
  display.setTextSize(1);
  display.setCursor(20,30);
  display.print("Soil Nutrient");
  display.setCursor(40,40);
  display.print("Monitor");
  display.display();
  delay(5000);
  }

void disp_all(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,0);
  display.print("PH LEVEL:");
  display.setCursor(70,0);
  display.print(ph_val,1);
  display.print(" pH");
  display.setCursor(0,10);
  display.print("NITROGEN:");
  display.setCursor(70,10);
  display.print(nitro_val);
  display.print(" mg/kg");
  display.setCursor(0,20);
  display.print("PHOSPHORUS:");
  display.setCursor(70,20);
  display.print(phos_val);
  display.print(" mg/kg");
  
  display.setCursor(0,30);
  display.print("POTASSIUM:");
  display.setCursor(70,30);
  display.print(pota_val);
  display.print(" mg/kg");

  display.setCursor(0,40);
  display.print("MOISTURE:");
  display.setCursor(70,40);
  display.print(moist_val);
  display.print(" %"); 
   
  display.setCursor(0,50);
  display.print("COMMAND:");
  display.setCursor(70,50);
  display.print(senderNumber);
  
  display.display();
  delay(10);
  }


void sensors(){
     if (sen_counter == 0){
     soil_ph(); 
     sen_counter ++;   
     }else if(sen_counter == 1){
     soil_nitrogen();
     sen_counter ++;  
     test_nitro(); //////////////////////////////    
     }else if(sen_counter == 2){
     soil_phosphorous(); 
     sen_counter ++; 
     test_phos();   ///////////////////////    
     }else if(sen_counter == 3){
     soil_potassium(); 
     sen_counter ++;
     test_pota();  ////////////////////////
     }else if(sen_counter >= 4){
     get_soilmoist(); 
     sen_counter = 0;  
     test_moist(); ///////       
     }
     }
     
void soil_ph(){
  if (Serial2.write(ph, sizeof(ph)) == 8){
  delay(100);
  for (byte i = 0; i < 7; i++){ /// 7
  values[i] = Serial2.read(); 
  }
  byte two = (values[2]);
  unsigned long combined = two;
  ph_val = combined / 100.0;
 // delay (1000);
  Serial.print("PH: ");
  Serial.println(ph_val); 
  }
  }

void soil_nitrogen(){
  if (Serial2.write(nitro, sizeof(nitro)) == 8){
  delay(100);
  for (byte i = 0; i < 7; i++){
  values[i] = Serial2.read(); 
  }
  nitro_val = (values[6]);
 // delay (1000);
  Serial.print("NITROGEN: ");
  Serial.println(nitro_val); 
  }
  }

void soil_phosphorous(){
  if (Serial2.write(phos, sizeof(phos)) == 8){
  delay(100);
  for (byte i = 0; i < 7; i++){
  values[i] = Serial2.read(); 
  }
  phos_val = (values[6]);
 // delay(1000);
  Serial.print("PHOSPHOROUS: ");
  Serial.println(phos_val); 
  }
  }

void soil_potassium(){
  if (Serial2.write(pota, sizeof(pota)) == 8){
  delay(100);
  for (byte i = 0; i < 7; i++){
  values[i] = Serial2.read(); 
  }
  pota_val = (values[6]);
 // delay(1000);
  Serial.print("POTASSIUM: ");
  Serial.println(pota_val);
  }
  }

void get_soilmoist(){
  int data_in1  = 0;
  uint32_t data_in1_ = 0; /// int
  float avg_volt = 0; 
  for (int i=0; i < 100; i++) {       
  data_in1 = analogRead(35); // pin 34
  data_in1_ = data_in1_ + data_in1;                          
  }                                   
  data_in1 = data_in1_ / 100;
//  CALIB
//  Serial.print("DATA_IN1: "); 
//  Serial.println(data_in1);
  moist_val = map(data_in1, 4095,2350, 0, 100); // 4095,2371
  if( moist_val >= 100){
  moist_val = 100;
  }else if ( moist_val <= 0){
  moist_val = 0;
  }
  }



/// EEPROM SAVE
void save_nitro_dly(){
      writeStringToEEPROM(10,senderNumber);      
      String nitro_dlystr = readStringFromEEPROM(10);   
      nitro_dly = nitro_dlystr.toInt();
      }
void save_phos_dly(){
      writeStringToEEPROM(14,senderNumber);      
      String phos_dlystr  = readStringFromEEPROM(14);   
      phos_dly  = phos_dlystr.toInt();
      }      
void save_pota_dly(){
      writeStringToEEPROM(18,senderNumber);      
      String pota_dlystr = readStringFromEEPROM(18);   
      pota_dly = pota_dlystr.toInt();
      }
void save_moist_dly(){
      writeStringToEEPROM(22,senderNumber);      
      String moist_dlystr = readStringFromEEPROM(22);   
      moist_dly = moist_dlystr.toInt();
      }

/// TIMER
void save_nitro_tmr(){
      writeStringToEEPROM(26,senderNumber);      
      String nitro_tmrstr = readStringFromEEPROM(26);   
      nitro_tmr = nitro_tmrstr.toInt();
      }
void save_phos_tmr(){
      writeStringToEEPROM(30,senderNumber);      
      String phos_tmrstr  = readStringFromEEPROM(30);   
      phos_tmr  = phos_tmrstr.toInt();
      }      
void save_pota_tmr(){
      writeStringToEEPROM(34,senderNumber);      
      String pota_tmrstr = readStringFromEEPROM(34);   
      pota_tmr = pota_tmrstr.toInt();
      }
void save_moist_tmr(){
      writeStringToEEPROM(38,senderNumber);      
      String moist_tmrstr = readStringFromEEPROM(38);   
      moist_tmr = moist_tmrstr.toInt();
      }
///
void load_nitro_dly(){      
      String nitro_dlystr = readStringFromEEPROM(10);   
      nitro_dly = nitro_dlystr.toInt();
      }
      
void load_phos_dly(){         
      String phos_dlystr  = readStringFromEEPROM(14);   
      phos_dly  = phos_dlystr.toInt();
      }    

void load_pota_dly(){        
      String pota_dlystr = readStringFromEEPROM(18);   
      pota_dly = pota_dlystr.toInt();
      }

void load_moist_dly(){       
      String moist_dlystr = readStringFromEEPROM(22);   
      moist_dly = moist_dlystr.toInt();
      }
////
void load_nitro_tmr(){         
      String nitro_tmrstr = readStringFromEEPROM(26);   
      nitro_tmr = nitro_tmrstr.toInt();
      }

void load_phos_tmr(){        
      String phos_tmrstr  = readStringFromEEPROM(30);   
      phos_tmr  = phos_tmrstr.toInt();
      }
      
void load_pota_tmr(){                 
      String pota_tmrstr = readStringFromEEPROM(34);   
      pota_tmr = pota_tmrstr.toInt(); 
      }

void load_moist_tmr(){       
      String moist_tmrstr = readStringFromEEPROM(38);   
      moist_tmr = moist_tmrstr.toInt();
      }

////
void writeStringToEEPROM(int addrOffset, const String &strToWrite)
  {
  EEPROM.commit();
  }

  String readStringFromEEPROM(int addrOffset)
  {
  EEPROM.commit();
  }


   
  
