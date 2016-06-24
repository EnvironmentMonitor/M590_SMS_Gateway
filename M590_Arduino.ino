//
//  Rewrite for M590 GPRS module
//  http://www.banggood.com/GSM-GPRS-SIM900-1800MHz-Short-Message-Service-m590-SMS-Module-DIY-Kit-For-Arduino-p-1043437.html
//  
//  Added Nunber Verification
//  The Power Supply needs to be able to produce 5v 2A for RF to function(After Drop over diode ~4.5v)
//  If there are any problems, the PSU is the most likely cause, 
//  Only UK Networks tested
//  the SIM choice can also cause issues, EE caused numerous failures
//  but O2 seems to connect every time, although the SMS Delivery can take Longer 
 
#include <SoftwareSerial.h> 
char inchar;                    // Will hold the incoming character from the GSM shield
String conTrol = "00000000000"; // Only allow control from this number....
SoftwareSerial M590(7, 8);      // Rx, Tx
String callID = "";
String StrCallID = "";
String StrCallID1 = "";
int led1 = 10;
int led2 = 11;
int led3 = 12;
int led4 = 13;
 
void setup()
{ // initialize both serial ports:
 Serial.begin(9600);

 // set up the digital pins to control
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
 
  // wake up the GSM shield
M590.begin(9600);
 // give time to log on to network.
delay(5500);
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

M590.flush();
delay(2500);
Serial.flush();
delay(2500);
while(Serial.available() > 0) {
    char t = Serial.read();
  }
while(M590.available() > 0) {
    char t = M590.read();
  }
  Serial.println("Ready...");
}
 
void loop() 
{
  if(M590.available()>0)
    {
    inchar=M590.read(); 
    //Serial.write(inchar);
    callID += inchar;
    if (inchar == '\n'){// Data +CMT: "+440000000000"
     Serial.println(callID);
     StrCallID = callID.substring(1,4);
    if (StrCallID =="CMT"){
      StrCallID1 = "0";
      StrCallID1 += callID.substring(10,20);
      Serial.print("The Caller ID to Verify : ");
      Serial.println(StrCallID1);
    if (StrCallID1 == conTrol){
      Serial.println("Number Verified.....");
    }else{
    Serial.println("Verification Failed, Control Not Permitted");
    }
      callID="";
      }
      callID="";
      }
  if (StrCallID1 == conTrol){
    if (inchar=='#')
    {
      delay(10);
 
      inchar=M590.read(); 
      if (inchar=='a')
      {
        delay(10);       
       inchar=M590.read();
        if (inchar=='0')
        {
          digitalWrite(led1, LOW);
          Serial.println("LED1 off");
        } 
        else if (inchar=='1')
        {
          digitalWrite(led1, HIGH);
          Serial.println("LED1 on");
        }
        delay(10);
         }
       else if (inchar=='b')
        {
      inchar=M590.read(); 
          if (inchar=='0')
          {
            digitalWrite(led2, LOW);
            Serial.println("LED2 off");
          } 
          else if (inchar=='1')
          {
            digitalWrite(led2, HIGH);
            Serial.println("LED2 on");
          }
          delay(10);
          }
         else if (inchar=='c')
          {
      inchar=M590.read(); 
            if (inchar=='0')
            {
              digitalWrite(led3, LOW);
              Serial.println("LED3 off");
            } 
            else if (inchar=='1')
            {
              digitalWrite(led3, HIGH);
              Serial.println("LED3 on");
            }
            delay(10);
          }
          else if (inchar=='d')
          {
       inchar=M590.read(); 
              if (inchar=='0')
              {
                digitalWrite(led4, LOW);
                Serial.println("LED4 off");
              } 
              else if (inchar=='1')
              {
                digitalWrite(led4, HIGH);
                Serial.println("LED4 on");
              }
              delay(10);
            }
             M590.println("AT+CMGD=1,4"); // delete all SMS
             Serial.println("delete all SMS"); // delete all SMS   
        }
      } 
    }
}
    
  
