// 
//  Rewrite for M590 GPRS module 
//  http://www.banggood.com/GSM-GPRS-SIM900-1800MHz-Short-Message-Service-m590-SMS-Module-DIY-Kit-For-Arduino-p-1043437.html 
//
// Use an ESP8266 for WiFi to complete the package..........Home Automation made simple......SMS routing to other ESP's
//
//   http://www.banggood.com/Geekcreit-Doit-NodeMcu-Lua-ESP8266-ESP-12E-WIFI-Development-Board-p-985891.html ESP module £5
//
// The SIM808 Development Board with the ESP8266 will allow control from any wireless device......incl GPS/Bluetooth
//
//  Referenced AT Commands from......
//
//   http://www.elecrow.com/wiki/images/2/20/SIM800_Series_AT_Command_Manual_V1.09.pdf 
//
// Using RC-Switch Library from.... run the simple recieve demo from the library to find your remote rf codes.....
//
//   https://github.com/sui77/rc-switch/
//
// Parts List.....use any supplier, this was convenient for me.....
//
//   http://www.banggood.com/UNO-R3-ATmega328P-Development-Board-For-Arduino-No-Cable-p-964163.html Arduino Uno Clone £3.50
//   http://www.banggood.com/433Mhz-RF-Transmitter-With-Receiver-Kit-For-Arduino-ARM-MCU-Wireless-p-74102.html RF 433MHZ £1.50
//   http://www.banggood.com/315MHz-XDFST-XDRF5V-Wireless-Transmitter-Receiver-Module-p-925524.html RF 315MHz £1.50
//   http://www.banggood.com/IIC-I2C-1602-Blue-Backlight-LCD-Display-Module-For-Arduino-p-950726.html I2C LCD £3
// GSM KIT £3.50 SMT SOLDERING NEEDED !!!
//   http://www.banggood.com/GSM-GPRS-SIM900-1800MHz-Short-Message-Service-m590-SMS-Module-DIY-Kit-For-Arduino-p-1043437.html 
// Alternative GSM £9 No soldering needed
//   http://www.banggood.com/SIM800L-Module-Board-Quad-Band-SMS-Data-GSM-GPRS-Globally-Available-p-1062819.html 
// SIM808 Development Board with Added GPS/Audio/Bluetooth £23 No soldering needed
//   http://www.banggood.com/SIM808-Module-GSM-GPRS-GPS-SMS-Development-Board-IPX-SMA-with-Antenna-p-1062818.html 
//
// Example Controlled items....These are the items used in the sketch....
//
//   http://www.maplin.co.uk/p/remote-controlled-mains-sockets-set-3-pack-n79ka 3 Pack + Remote Sockets (UK) £15
//
//   http://www.banggood.com/DC5-24V-LED-RF-Remote-Controller-For-RGB-50503528-SMD-LED-Strip-p-75315.html 12V RGB LED Controller £5
//   
//   http://www.banggood.com/E27-Screw-Wireless-Remote-Control-Light-Lamp-Bulb-Holder-Cap-Socket-p-966688.html  Light Fitting + Remote £5
//
//   USB 3.4A 17W (2.4A + 1A) UK Plug £10
//   http://www.pcworld.co.uk/gbuk/phones-broadband-and-sat-nav/mobile-phones-and-accessories/mobile-phone-chargers-adaptors-and-cables/sandstrom-s34amd16-usb-wall-charger-10142693-pdt.html
//
//   Cables and jumper wires to suit...
//
//   Arduino shield to make connections easier.......£2.50
//   http://www.banggood.com/UNO-R3-Sensor-Shield-V5-Expansion-Board-For-Arduino-p-954753.html

# include <RCSwitch.h>
# include <SoftwareSerial.h>
# include <Wire.h> 
# include <LiquidCrystal_I2C.h>

RCSwitch sSwitch = RCSwitch();   // 433MHz Transmitter
RCSwitch s3Switch = RCSwitch();  // 315MHz Transmitter

LiquidCrystal_I2C lcd(0x3F,16,2);// set the LCD address to 0x3F for MY 16 chars and 2 line display, Check Your LCD Address !!!
int charNo = 0;                  // track for positioning on LCD
SoftwareSerial  M590(3,2);       // rx tx on Arduino use appropriate pins on ESP8266 (maybe 4,5)
                                 // Using Suggested pins leaves pin 16 to allow wakeup from sleep + Pins 12 + A0 on an ESP8266-07/12/12E/F
                                 // You could use pin 12 for sensors but it is better to use I2C and use pin 12 for Infra-Red Remote (IR LED) 
                                 
char inchar;                     // Will hold the incoming character from the M590
String conTrol = "44XXXXXXXXXX"; // Only allow control from this number.... format 440000000000 Inc Country Code
boolean conTrolF = false;        // set true when number verifies ok
String callID = "";              // line of text from gsm
String StrCallID = "";           // command selection 
String StrCallID1 = "";          // number being verified
String LastSMS = "";             // Time and Date SMS Sent (NOT ! Recieved !)
int LiNe = 0;                    // I am only using 2 lines 0 + 1 Verify Number ONLY on line 0
int led1 = 13;                   // Use PIN 13 on Arduino or PIN 15 on ESP8266
int flip = 0;                    // Flash the led while actively recieving





void setup()
{ 
// initialize the lcd, use line below for ESP8266
// Wire.begin(0,2);
lcd.init();
delay(1000);  
// initialize both serial ports:  
Serial.begin(9600);
lcd.backlight();
lcd.print("S");
lcd.print("t");
lcd.print("a");
lcd.print("r");
lcd.print("t");
lcd.print("i");
lcd.print("n");
lcd.print("g");
// wake up the GSM shield
M590.begin(9600);
// give time to log on to network.
delay(5500);
// set up the digital pins to control
  pinMode(led1, OUTPUT);
  digitalWrite(led1, LOW);
// Transmitters connected to Pins D5 + D6 on Arduino use appropriate pins on ESP8266 (maybe 13,14)
  sSwitch.enableTransmit(5);               // 433MHz Transmitter
  s3Switch.enableTransmit(6);              // 315MHz Transmitter
M590.print(F("ATE0\r"));                   // Echo OFF, use AT+CFUN=15 to reset M590 it will return +PBREADY when connected to network
delay(500);                                // AT+CNUM get own number, AT+CCID get SIM ID, useful commands for debug.......
M590.print(F("AT+CMGF=1\r"));              // set SMS mode to text
Serial.println(F("set SMS mode to txt"));  // set SMS mode to text
delay(5500);

M590.print(F("AT+CSCS=\"GSM\""));          // "Western or Standard Character Set" to use
M590.print(F("\r"));
Serial.println(F("Character Mode Set")); 
delay(5500);

Serial.println(F("set SMS mode to txt"));  // set SMS mode to text
M590.print(F("AT+CMGF=1\r"));              
delay(5500);
Serial.flush();
M590.flush();
Serial.println(F("Send new SMS Immediately to the M590's serial out")); 
M590.print(F("AT+CNMI=2,2,0,0,0\r")); 
delay(5500);
M590.println(F("AT+CMGD=1,4"));            // delete all SMS, in bank 1, command 4 delete all
Serial.println(F("delete all SMS"));       
delay(2500);
while(Serial.available() > 0) {
    char t = Serial.read();
  }
while(M590.available() > 0) {
    char t = M590.read();
  }
  Serial.println(F("Ready..."));
  lcd.clear();
lcd.print("S");
lcd.print("t");
lcd.print("a");
lcd.print("r");
lcd.print("t");
lcd.print("e");
lcd.print("d");
}




void smsG()
{
  if (LiNe==0 && charNo==0){lcd.clear();}
  delay(50);
  inchar=M590.read(); 
  delay(50);
  if (flip==0){digitalWrite(led1, HIGH);flip=1;}else{digitalWrite(led1, LOW);flip=0;}
  charNo++;
  callID += inchar;
  if (LiNe==0 && charNo>5 && charNo<22){lcd.setCursor(charNo-7,LiNe);lcd.print(inchar);}  // Data +CMT: "+440000000000",,"21/12/21,12:21:12+04"
  if (LiNe==1 && charNo<5){lcd.setCursor(charNo-1,LiNe); lcd.print(inchar);} // Data #XYY
  if (inchar == '\n'){
    charNo = 0;
     Serial.println(callID);
     StrCallID = callID.substring(0,4);     
   if (StrCallID =="+CMT"){
      StrCallID1 = "";
      StrCallID1 += callID.substring(8,20);
      Serial.print(F("The Caller ID to Verify : "));
      Serial.println(StrCallID1);
      LastSMS = callID.substring(24,41);
      Serial.println("Sent @ " + LastSMS);
     if (StrCallID1 == conTrol){
      Serial.println(F("Number Verified....."));
      if (LiNe == 0){conTrolF = true;}else{conTrolF = false;}
       LiNe++;
      }else{
      Serial.println(F("Verification Failed, Control Not Permitted"));
      conTrolF = false;
      LiNe++;
      }
    callID="";
    }else if (StrCallID =="#a00"){
          sSwitch.send(4534565, 24); // 433MHz 
          lcd.print(" ");
          lcd.print("L");
          lcd.print("a");
          lcd.print("m");
          lcd.print("p");
          lcd.print(" ");
          lcd.print("O");
          lcd.print("f");
          lcd.print("f");
          Serial.println(F("Living Room Lamp off"));
    }else if (StrCallID =="#a01"){
          sSwitch.send(4534573, 24); // 433MHz
          lcd.print(" ");
          lcd.print("L");
          lcd.print("a");
          lcd.print("m");
          lcd.print("p");
          lcd.print(" ");
          lcd.print("O");
          lcd.print("n");
          Serial.println(F("Living Room Lamp On"));
    }else if (StrCallID =="#a10"){
          s3Switch.send(2391209, 24); // 315MHz
          lcd.print(" ");
          lcd.print("L");
          lcd.print("i");
          lcd.print("t");
          lcd.print("e");
          lcd.print("L");  
          lcd.print(" ");
          lcd.print("O");
          lcd.print("f");
          lcd.print("f");;
          Serial.println(F("Living Room Light off"));
    }else if (StrCallID =="#a11"){
          s3Switch.send(2391202, 24); // 315MHz
          lcd.print(" ");
          lcd.print("L");
          lcd.print("i");
          lcd.print("t");
          lcd.print("e");
          lcd.print("L");  
          lcd.print(" ");
          lcd.print("O");
          lcd.print("n");
          Serial.println(F("Living Room Light On"));      
    }else if (StrCallID =="#a20"){
          s3Switch.send(2839721, 24); // 315MHz
          lcd.print(" ");
          lcd.print("H");
          lcd.print("a");
          lcd.print("l");
          lcd.print("l");
          lcd.print(" ");
          lcd.print("O");
          lcd.print("f");
          lcd.print("f");
          Serial.println(F("Hall Light off"));
    }else if (StrCallID =="#a21"){
          s3Switch.send(2839714, 24); // 315MHz
          lcd.print(" ");
          lcd.print("H");
          lcd.print("a");
          lcd.print("l");
          lcd.print("l");
          lcd.print(" ");
          lcd.print("O");
          lcd.print("n");
          Serial.println(F("Hall Light On")); 
    }else if (StrCallID =="#b00"){
          sSwitch.send(4534566, 24); // 433MHz
          lcd.print(" ");
          lcd.print("D");
          lcd.print("A");
          lcd.print("B");
          lcd.print(" ");
          lcd.print("O");
          lcd.print("f");
          lcd.print("f");
          Serial.println(F("DAB Radio off"));
    }else if (StrCallID =="#b01"){
          sSwitch.send(4534574, 24); // 433MHz
          lcd.print(" ");
          lcd.print("D");
          lcd.print("A");
          lcd.print("B");
          lcd.print(" ");
          lcd.print("O");
          lcd.print("n");
          Serial.println(F("DAB Radio on"));
    }else if (StrCallID =="#c00"){
          sSwitch.send(16776961, 24); // 433MHz
          lcd.print(" ");
          lcd.print("H");
          lcd.print("a");
          lcd.print("l");
          lcd.print("l");
          lcd.print(" ");
          lcd.print("/");
          lcd.print("/");         
          Serial.println(F("Hall Light On/Off"));
    }else if (StrCallID =="#c01"){
          sSwitch.send(16776970, 24); // 433MHz
          lcd.print(" ");
          lcd.print("H");
          lcd.print("a");
          lcd.print("l");
          lcd.print("l");
          lcd.print(" ");
          lcd.print("+");
          Serial.println(F("Hall Light Colour Plus"));
    }else if (StrCallID =="#d00"){
          sSwitch.send(16776973, 24); // 433MHz
          lcd.print(" ");
          lcd.print("H");
          lcd.print("a");
          lcd.print("l");
          lcd.print("l");
          lcd.print(" ");
          lcd.print("-");
          Serial.println(F("Hall Light Colour Minus"));
    }else if (StrCallID =="#d01"){
          sSwitch.send(16776968, 24); // 433MHz
          lcd.print(" ");
          lcd.print("H");
          lcd.print("a");
          lcd.print("l");
          lcd.print("l");
          lcd.print(" ");
          lcd.print("D");
          lcd.print("e");
          lcd.print("m");          
          lcd.print("O");
          Serial.println(F("Hall Light Demo"));
    }else if (StrCallID =="#e00"){
      if (conTrolF == true){
              delay(250);
              int analogV = analogRead(A0);
              String analogS = "";
              analogS += analogV;
              delay(250);
              Serial.println("SMS Content : " + analogS);
              M590.print("AT+CMGS=\"" + StrCallID1 + "\"\r\n");  // Number to reply to from incomming message
              delay(2000);
              M590.print("Sensor Data Value : " + analogS);      // message content
              delay(1000);
              M590.write(0x1a);                                  // send out SMS
              delay(2000);
      }
    }else if (StrCallID =="#e01"){
      if (conTrolF == true){
             Serial.println(F("delete all SMS...."));
             M590.println("AT+CMGD=1,4");                        // delete all SMS, in bank 1, command 4 "delete all"
             Serial.println(F("All SMS Deleted"));  
      }
    }
  callID="";
  digitalWrite(led1, LOW);
  }
} 
 
void loop(){ 
  conTrolF = false;
  LiNe = 0;
  charNo = 0;
  while(M590.available()){
    smsG();
  }
    //This is where to do some interesting stuff......
}
    
  
