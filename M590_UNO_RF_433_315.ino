/*
This uses SoftwareSerial Plus the RC-Switch library from ....
  https://github.com/sui77/rc-switch
  
The list of SMS Commands are based on my setup customise for your own.
Specifically the RF Codes, these are specific to Status Remote Sockets,
12V RGB RF LED Controller on the 433MHz, Chinese Imported Lightbulb fittings on the 315MHz.

This allows control of the RF without verification, the GPRS Modem requires verification.





*/

# include <RCSwitch.h>
# include <SoftwareSerial.h>

RCSwitch sSwitch = RCSwitch();
RCSwitch s3Switch = RCSwitch();

SoftwareSerial  M590(2,3); // rx tx

char inchar;                     // Will hold the incoming character from the GSM shield
String conTrol = "XXXXXXXXXXXX"; // Only allow control from this number.... format 440000000000 Inc Country Code
boolean conTrolF = false;        // set true when number verifies ok
String callID = "";              // line of text from gsm
String StrCallID = "";           // command selection 
String StrCallID1 = "";          // number being verified
int led1 = 10;
int led2 = 11;
int led3 = 12;
int led4 = 13;
int LiNe = 0;




void setup()
{ // initialize both serial ports:
 Serial.begin(9600);
  // wake up the GSM shield
M590.begin(9600);
 // give time to log on to network.
delay(5500);
 // set up the digital pins to control
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
// Transmitters connected to Pins D5 + D6
  sSwitch.enableTransmit(5);    // 433MHz Transmitter
  s3Switch.enableTransmit(6);   // 315MHz Transmitter
M590.print("ATE0\r");       // set echo off, use AT+CFUN=15 to reset the module
delay(500);                 // on connection to GPRS Network returns +PBREADY
M590.print("AT+CMGF=1\r");  // set SMS mode to text
Serial.println("set SMS mode to txt");  // set SMS mode to text
delay(5500);

M590.print("AT+CSCS=\"GSM\"");
M590.print("\r");
delay(5500);

Serial.println("set SMS mode to txt");  // set SMS mode to text
M590.print("AT+CMGF=1\r");  // set SMS mode to text
delay(5500);
Serial.flush();
M590.flush();
  // blurt out contents of new SMS upon receipt to the GSM shield's serial out
Serial.println("blurt out contents of new SMS upon receipt to the GSM shield's serial out"); 
M590.print("AT+CNMI=2,2,0,0,0\r"); 
delay(5500);
M590.println("AT+CMGD=1,4"); // delete all SMS
Serial.println("delete all SMS"); // delete all SMS
delay(2500);
while(Serial.available() > 0) {
    char t = Serial.read();
  }
while(M590.available() > 0) {
    char t = M590.read();
  }
  Serial.println("Ready...");
}




void smsG()
{
  delay(50);
  inchar=M590.read(); 
  delay(50);
  callID += inchar;
  if (inchar == '\n'){// Data +CMT: "+440000000000"
     Serial.println(callID);
     StrCallID = callID.substring(0,4);
   if (StrCallID =="+CMT"){
      StrCallID1 = "";
      StrCallID1 += callID.substring(8,20);
      Serial.print("The Caller ID to Verify : ");
      Serial.println(StrCallID1);
     if (StrCallID1 == conTrol){
      Serial.println("Number Verified.....");
      if (LiNe == 0){conTrolF = true;}
       LiNe++;
      }else{
      Serial.println("Verification Failed, Control Not Permitted");
      conTrolF = false;
      LiNe++;
      }
    callID="";
    }else if (StrCallID =="#a00"){
          sSwitch.send(4534565, 24); 
          delay(1000); 
          digitalWrite(led1, LOW);
          Serial.println("Living Room Lamp off");
    }else if (StrCallID =="#a01"){
          sSwitch.send(4534573, 24);
          digitalWrite(led1, HIGH);
          Serial.println("Living Room Lamp On");
    }else if (StrCallID =="#a10"){
          s3Switch.send(2391209, 24); 
          delay(1000); 
          digitalWrite(led1, LOW);
          Serial.println("Living Room Light off");
    }else if (StrCallID =="#a11"){
          s3Switch.send(2391202, 24);
          digitalWrite(led1, HIGH);
          Serial.println("Living Room Light On");      
    }else if (StrCallID =="#a20"){
          s3Switch.send(2839721, 24); 
          delay(1000); 
          digitalWrite(led1, LOW);
          Serial.println("Hall Light off");
    }else if (StrCallID =="#a21"){
          s3Switch.send(2839714, 24);
          digitalWrite(led1, HIGH);
          Serial.println("Hall Light On"); 
    }else if (StrCallID =="#b00"){
          sSwitch.send(4534566, 24);
          digitalWrite(led2, LOW);
          Serial.println("DAB Radio off");
    }else if (StrCallID =="#b01"){
          sSwitch.send(4534574, 24);
          digitalWrite(led2, HIGH);
          Serial.println("DAB Radio on");
    }else if (StrCallID =="#c00"){
          sSwitch.send(16776961, 24);
          digitalWrite(led3, LOW);
          Serial.println("Hall Light On/Off");
    }else if (StrCallID =="#c01"){
          sSwitch.send(16776970, 24);
          digitalWrite(led3, HIGH);
          Serial.println("Hall Light Colour Plus");
    }else if (StrCallID =="#d00"){
          sSwitch.send(16776973, 24);
          digitalWrite(led4, LOW);
          Serial.println("Hall Light Colour Minus");
    }else if (StrCallID =="#d01"){
          sSwitch.send(16776968, 24);
          digitalWrite(led4, HIGH);
          Serial.println("Hall Light Demo");
    }else if (StrCallID =="#e00"){
      if (conTrolF == true){
              delay(250);
              int analogV = analogRead(A0);
              String analogS = "";
              analogS += analogV;
              delay(250);
              Serial.println("SMS Content : " + analogS);
            //  M590.print("AT+CMGS=\"" + StrCallID1 + "\"\r\n");  // Number to reply to from incomming message
            //  delay(2000);
            //  M590.print("Sensor Value = " + analogS);//message content
            //  delay(1000);
            //  M590.write(0x1a);     // send out SMS
            //  delay(2000);
      }
    }else if (StrCallID =="#e01"){
      if (conTrolF == true){
             Serial.println("delete all SMS....");
             M590.println("AT+CMGD=1,4"); // delete all SMS
             Serial.println("All SMS Deleted"); // delete all SMS  
      }
    }
  callID="";
  }
} 
 
void loop() 
{
  while(M590.available()){
    smsG();
  }
  conTrolF = false;
  LiNe = 0;
    //This is where to do some interesting stuff......
}
