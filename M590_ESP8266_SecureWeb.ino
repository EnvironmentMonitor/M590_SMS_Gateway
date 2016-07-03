//
//  Download the CSS and data directory in a zip....
//  https://github.com/downtrodden/monitor/blob/gh-pages/data.zip?raw=true
//


#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Wire.h>
#include <SoftwareSerial.h> 
SoftwareSerial M590(13, 12);  // Rx, Tx
extern "C" 
{
#include "user_interface.h"
}
// Only USE if nothing connected to ADC !!!!!!!!!
// ADC_MODE(ADC_VCC);
/* Set these to your desired credentials. */
const char *ssid = "Your-SSID";
const char *password = "Yor-Passphrase";

ESP8266WebServer server(80);

char inchar;                     // Will hold the incoming character from the GSM shield
String conTrol = "XXXXXXXXXXXX"; // Only allow control from this number....
String callID = "";
String StrCallID = "";
String StrCallID1 = "";
int led1 = 15;

const char* _passcode = "1234";//note: the field "passcode in the html form "frm1" must match the passcode lenght
char passcodeOK='0';//start Code is blank...
String readString;
int analogReadOut;
int dispcont = 0;

// Create placeholders fot MPU6050..........
int16_t AcX, AcY, AcZ, Tmpt, Tmp, GyX, GyY, GyZ, AcXo, AcYo, AcZo, GyXo, GyYo, GyZo;
#define serverPort 80
// define (map) GPIO to HTML Fields (buttons/Indicators), note: not using 0 as base index deliberately
#define input1 0
#define output1 2 


void smsG()
{
    inchar=M590.read(); 
    //Serial.write(inchar);
    callID += inchar;
    if (inchar == '\n'){// Data +CMT: "+440000000000"
     Serial.println(callID);
     StrCallID = callID.substring(1,4);
    if (StrCallID =="CMT"){
      StrCallID1 = "";
      StrCallID1 += callID.substring(8,20);
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
      callID = "";
      delay(200);
      inchar=M590.read(); 
      delay(200);
      if (inchar=='a')
          {
              delay(200);
              inchar=M590.read(); 
              delay(200);
              if (inchar=='0')
              {
                digitalWrite(led1, LOW);
                Serial.println("LED off");
              } 
              else if (inchar=='1')
              {
                digitalWrite(led1, HIGH);
                Serial.println("LED on");
              }
            }
            else if (inchar=='b')
            {
              delay(200);
              inchar=M590.read(); 
              delay(200);
              if (inchar=='0')
              {
              delay(250);
              int analogV = analogRead(A0);
              String analogS = "";
              analogS += analogV;
              delay(250);
              // These lines are an example to generate "Random" passwords for the webpage, reply via SMS....
              int x = random(0,999);
              int y = random(0,999);
              int I_passcode = x + y + 1000; // Minimum 4 digits
              String PassCode = "";
              PassCode += I_passcode; // SMS to Reply with..... PassCode (String)
              //_passcode = PassCode.c_str(); // Commented this out to allow the page to run with the Declared password....
              // To Here.......
              Serial.println(analogS);
             // M590.print("AT+CMGS=\"" + StrCallID1 + "\"\r\n");  // Number to reply to from incomming message Only Verified No's !!!
              delay(1500);
             // M590.print("Sensor Value = " + analogS);//message content
              delay(500);
             // M590.write(0x1a);     // send out SMS
              delay(1000);
              } 
              else if (inchar=='1')
              {
             Serial.println("delete all SMS....");
             M590.println("AT+CMGD=1,4"); // delete all SMS
             Serial.println("All SMS Deleted"); // delete all SMS   
             }
          }
      StrCallID1 = "";
      }
   }
}


void returnOK() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "");
}

void returnOKs() {
  server.sendHeader("Connection", "refresh,20");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}


bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}

void handleNotFound(){
  if(loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

// send back GPIO Data Refresh
void readInputs() {
  WiFiClient client = server.client();
  client.println("\r\n<p>");//dont miss to wrap the request with some tag, mandatory for ajax to work properly
//  Do not use if LDR Fitted
//  analogReadOut = ESP.getVcc();  
delay(50);
 analogReadOut = analogRead(A0);
delay(50);
// Accelerometer 0x43
// Gyro 0x3B
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);                                  // starting with register 0x43 (GYRO_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 14, true);                  // request a total of 6 registers or What you need  
  GyX = Wire.read() << 8 | Wire.read(); 
  GyY = Wire.read() << 8 | Wire.read(); 
  GyZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L) 
  AcX = Wire.read() << 8 | Wire.read(); 
  AcY = Wire.read() << 8 | Wire.read(); 
  AcZ = Wire.read() << 8 | Wire.read();
  Wire.endTransmission(true);
  Tmpt = Tmp/340.00+36.53;    
  if (analogReadOut < 30){
    client.print("<p>Light Level is Low:<span style='background-color:#FF0000; font-size:18pt'>");
    client.print(analogReadOut);
    client.println("</span></p>");
  }else{
    client.print("<p>Light Level is:<span style='background-color:#00FF00; font-size:18pt'>");
    client.print(analogReadOut);
    client.println("</span></p>");
  }
  //________________________________Secured Data Start______________________________________________
  // this will ensure sensor condition or user verification before displaying this section
  //  The use of SMS Data can either unlock the page or reply with a password...
    if ((digitalRead(input1))&&(passcodeOK== '0')) {   
      }else{
    client.println("<BR>Attitude is relative to facing the Device LED<BR><BR>"); 
    if (Tmpt < 20){
    client.print("<p>Body Temperature:<span style='background-color:#FF0000; font-size:18pt'>");
    client.print(Tmpt);
    client.println("C</span></p>");
  }else{
    client.print("<p>Body Temperature:<span style='background-color:#00FF00; font-size:18pt'>");
    client.print(Tmpt);
    client.println("C</span></p>");
  }
  GyXo = GyX/=100;
  if (GyXo < 0){
    client.print("<p>Device is Upside Down :<span style='background-color:#FF0000; font-size:18pt'>");
    client.print(GyXo);
    client.println("</span></p>");
  }else{
    client.print("<p>Right Side Up 180 is Verticle:<span style='background-color:#00FF00; font-size:18pt'>");
    client.print(GyXo);
    client.println("</span></p>");
  }
  GyYo = GyY/=100;
  if (GyYo < 0){
    client.print("<p>Left Inclination:<span style='background-color:#0000FF; font-size:18pt'>");
    client.print(GyYo);
    client.println("</span></p>");
  }else{
    client.print("<p>Right Inclination:<span style='background-color:#00FF00; font-size:18pt'>");
    client.print(GyYo);
    client.println("</span></p>");
  }
  GyZo = GyZ/=100;
  if (GyZo < 0){
    client.print("<p>Backward Inclination:<span style='background-color:#0000FF; font-size:18pt'>");
    client.print(GyZo);
    client.println("</span></p>");
  }else{
    client.print("<p>Forward Inclination:<span style='background-color:#00FF00; font-size:18pt'>");
    client.print(GyZo);
    client.println("</span></p>");
  }
    passcodeOK='0';
  }
 // ______________________________________secured data end___________________________________________
  client.println("<br />");
  if (digitalRead(input1)) {
    client.println("<p>Sensor ONE is: <span style='background-color:#00FF00; font-size:18pt'>ALL OK</span></p>");
  }else{
    client.println("<p>Sensor ONE is: <span style='background-color:#FF0000; font-size:18pt'>Assistance</span></p>");
  }
  client.println("<br />");
/*  if (digitalRead(input2)) {
    client.println("<p>Sensor TWO is: <span style='background-color:#00FF00; font-size:18pt'>ALL OK</span></p>");
  }else{
    client.println("<p>Sensor TWO is: <span style='background-color:#FF0000; font-size:18pt'>Assistance</span></p>");
  }
    client.println("<br />");
  if (digitalRead(input3)) {
    client.println("<p>Flash Button is: <span style='background-color:#00FF00; font-size:18pt'> ALL OK</span></p>");
  }else{
    client.println("<p>Flash Button is: <span style='background-color:#0000FF; font-size:18pt'>Assistance</span></p>");
  }*/
  client.println("</p>");//this tag completes the request wrap
}

void falld(){
  WiFiClient client = server.client();
           client.println("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n");
    String diagdat="";     
           diagdat=("<html><head><TITLE>EM Fall Detector 1</TITLE>\r\n<link href=\"/style.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen\" /></head><body>");
           diagdat+=("<div id=\"menu\">\n<ul>\n<li class=\"current_page_item\"><a href=\"#\">Fall Detector</a></li>\n<li><a href=\"/\">Blog</a></li>\n<li><a href=\"/photos/\">Photos</a></li>\n<li>");
           diagdat+=("<a href=\"/links/\">Links</a></li>\n<li><a href=\"/contact/\">Contact</a></li>\n<li><a href=\"/diag\">Diagnostics</a></li>\n<li><a href=\"/fall\">Fall</a></li>\n</ul>\n</div>\n<font color=\"#DFDFDF\"><body bgcolor=\"#a0dFfe\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">");
           client.println(diagdat);
               if (server.hasArg("passcode")){
                 if (server.arg(0)=="1234"){// This is the password compare from the returned data
                passcodeOK='1';
            }else{
                passcodeOK='0';
            }
               }
            // simple javascritp code to intercept anchor clientick and trick HREF field with the Button (or action) ID and add the password from the input field
            client.println("\r\n<script>\r\nfunction SndCommand( obx,swtchNO ){ \r\nobx.href=\"button\"+swtchNO+\"&passcode=\"+document.getElementById(\"frm1\").elements[0].value;\r\n}\r\n</script>\r\n");
            // not so simple script to allow sensor data to be automatically updated
            client.println("\r\n<script>\r\nfunction GetSensorUpd() {\r\nnocache = \"&nocache=\" + Math.random() * 1000000;");
            client.println("var ajaxreq = new XMLHttpRequest();");
            client.println("ajaxreq.open(\"GET\", \"sensorupdrq\" + nocache, true);");
            client.println("ajaxreq.onreadystatechange = function() {");
            client.println("if (ajaxreq.readyState == 4 && ajaxreq.status == 200) {");
            client.println("document.getElementById(\"readX\").innerHTML = ajaxreq.responseText;");
            client.println("}\r\n}\r\najaxreq.send();");
            client.println("setTimeout('GetSensorUpd()', 5000);\r\n}\r\n</script>\r\n");//updates each half second

            client.println("</HEAD>");
//            client.println("<BODY onload=\"GetSensorUpd()\">");//comment this out if you do not want automatic sensor update
            client.println("<BODY>");//uncomment this if you do not want auto update
            client.println("<BR><BR><BR><H1>Display the Motion Data of MPU6050 Securely<br />With<br />  Open Sensor/Switch Reads on the MCU</H1>");
            client.println("<br />");  
            client.print("<div id=\"passx\"><form id=\"frm1\">Please Input Password<BR>To Unlock Data:<BR><input name=\"passcode\" size=\"5\" maxlength=\"4\" type=\"password\" value=\"");
            if (passcodeOK== '1') client.print(_passcode);
            client.println("\" style='font-size:24pt'></form></div>");
               
            client.print("<div id=\"readX\">");
            readInputs();  
            client.println("</div>");

            
            // first ON/OFF Buttons pair
            client.println("<BR /><a href=\"\" onclick=\"SndCommand(this,11)\">Output 1 On</a>");
            //SndCommand is a javascript code running on the clientiet (aka ajax) Parameters are outputIDx and State 
            client.println("<a href=\"\" onclick=\"SndCommand(this,10)\">Output 1 Off</a><br /><br />");   
            
            // then repeat for every gpio as output to control
           
           // client.println("<a href=\"\" onclick=\"SndCommand(this,21)\">Output 2 On</a>");
           // client.println("<a href=\"\" onclick=\"SndCommand(this,20)\">Output 2 Off</a><br /><br />");  
  
           // client.println("<a href=\"\" onclick=\"SndCommand(this,31)\">Output 3 On</a>");
           // client.println("<a href=\"\" onclick=\"SndCommand(this,30)\">Output 3 Off</a><br /><br />");  
                       
            client.println("<BR>the use of static html with data refreshed dynamically<BR><BR><IMG SRC=\"/images/esp-201.jpg\" WIDTH=\"420\" HEIGHT=\"300\" BORDER=\"1\"></BODY><BR><BR>");
            client.println("<BR><FONT SIZE=-2>environmental.monitor.log@gmail.com<BR><FONT SIZE=-1>ESP8266-12  Mini Server With SPI Flash HTML<BR><FONT SIZE=-2>Compiled Using ver 2.3.0<BR></HTML>");
            //Processing client request
            // Button0
            if (readString.indexOf("button11") >0 && passcodeOK== '1'){
            }
            if (readString.indexOf("button10")>0 && passcodeOK== '1'){
            }
            if (readString.indexOf("button21") >0 && passcodeOK== '1'){
            }
            if (readString.indexOf("button20")>0 && passcodeOK== '1'){
            }
            if (readString.indexOf("button31") >0 && passcodeOK== '1'){
            }
            if (readString.indexOf("button30")>0 && passcodeOK== '1'){

            }     
          }


void diags(){ 
  WiFiClient client = server.client(); 
//   float servolt1 = ESP.getVcc();
//   float servolt1 = analogRead(A0);
     unsigned long  spdcount = ESP.getCycleCount();
     delay(1);
     unsigned long  spdcount1 = ESP.getCycleCount();
     unsigned long  speedcnt = spdcount1-spdcount; 
     FlashMode_t ideMode = ESP.getFlashChipMode();
                                String duration1 = " ";
                                int hr,mn,st;
                                st = millis() / 1000;
                                mn = st / 60;
                                hr = st / 3600;
                                st = st - mn * 60;
                                mn = mn - hr * 60;
                                if (hr<10) {duration1 += ("0");}
                                duration1 += (hr);
                                duration1 += (":");
                                if (mn<10) {duration1 += ("0");}
                                duration1 += (mn);
                                duration1 += (":");
                                if (st<10) {duration1 += ("0");}
                                duration1 += (st);
                         String diagdat="";     
                                diagdat=("<html><head><title>Mini-Server-Diagnostics</title><link href=\"/style.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen\" /></head><body>");
                                diagdat+=("<div id=\"menu\">\n<ul>\n<li class=\"current_page_item\"><a href=\"#\">Diagnostics</a></li>\n<li><a href=\"/\">Blog</a></li>\n<li><a href=\"/photos/\">Photos</a></li>\n<li>");
                                diagdat+=("<a href=\"/links/\">Links</a></li>\n<li><a href=\"/contact/\">Contact</a></li>\n<li><a href=\"/diag\">Diagnostics</a></li>\n</ul>\n</div>\n<font color=\"#DFEFDF\"><body bgcolor=\"#a0dFfe\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">");
                                diagdat+=("<BR><h1><BR>Mini Server Gateway<BR>SDK Diagnostic Information</h1>");
                                diagdat+=("<BR><a href=\"/fall\">Fall Detector Page</a><BR>");
                                diagdat+="<BR>  WiFi Station Hostname = ";
                                diagdat+=wifi_station_get_hostname();
                                diagdat+="<BR>  Free RAM = ";
                                client.print(diagdat);
                                client.print((uint32_t)system_get_free_heap_size()/1024);
                                diagdat=" KBytes<BR>  SDK Version = ";                                 
                                diagdat+=ESP.getSdkVersion();
                                diagdat+="<BR>  Boot Version = ";
                                diagdat+=ESP.getBootVersion();
                                diagdat+="<BR>  Free Sketch Space  = ";
                                diagdat+=ESP.getFreeSketchSpace()/1024;
                                diagdat+=" KBytes<BR>  Sketch Size  = ";
                                diagdat+=ESP.getSketchSize()/1024;
                                diagdat+=" KBytes<BR>";
                                client.print(diagdat);
                                client.printf("  Flash Chip id = %08X\n", ESP.getFlashChipId());
                                client.print("<BR>");
                                client.printf("  Flash Chip Mode = %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));
                                diagdat="<BR>  Flash Size By ID = ";
                                diagdat+=ESP.getFlashChipRealSize()/1024;
                                diagdat+=" KBytes<BR>  Flash Size (IDE) = "; 
                                diagdat+=ESP.getFlashChipSize()/1024;
                                diagdat+=" KBytes<BR>  Flash Speed = ";
                                diagdat+=ESP.getFlashChipSpeed()/1000000;
                                diagdat+=" MHz<BR>  ESP8266 CPU Speed = ";
                                diagdat+=ESP.getCpuFreqMHz();
                                diagdat+=" MHz<BR>";
                                client.print(diagdat);
                                client.printf("  ESP8266 Chip id = %08X\n", ESP.getChipId());
                                diagdat="<BR>  System Instruction Cycles Per Second = ";
                                diagdat+=speedcnt*1000;  
                                diagdat+="<BR>  Last System Restart Reason = ";
                                diagdat+=ESP.getResetInfo();                                                              
                                //diagdat+="<BR>  System VCC = ";
                                //diagdat+=servolt1/1000, 3; 
                                diagdat+="<BR>  System Uptime = ";
                                diagdat+=duration1;
                                client.print(diagdat);
                                client.print("<BR><FONT SIZE=-2>environmental.monitor.log@gmail.com<BR><FONT SIZE=-1>ESP8266-12  Mini Server With SPI Flash HTML<BR><FONT SIZE=-2>Compiled Using ver 2.0.0-rc1<BR>");
                                client.println("<IMG SRC=\"/images/ESP-12E2.jpg\" WIDTH=\"420\" HEIGHT=\"280\" BORDER=\"1\"></body></html>");
                                diagdat = "";
                                client.stop();
                                duration1 = "";  
}

void setup() {
	delay(1000);
  Wire.begin(5,4);
  pinMode(led1, OUTPUT);
  randomSeed(analogRead(A0));
  digitalWrite(led1, LOW);
	 Serial.begin(9600);
    SPIFFS.begin();
        Wire.begin();
        Wire.beginTransmission(0x68);                       // 0x68 I2C address of the MPU-6050
        Wire.write(0x6B);                                   // PWR_MGMT_1 register
        Wire.write(0);                                      // set to zero (wakes up the MPU-6050)
        Wire.endTransmission(true); 
	Serial.println();
	Serial.print("Configuring access point...");
  WiFi.mode(WIFI_STA);
  delay(500);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
   Serial.print("IP= ");  
  Serial.print(WiFi.localIP());
        server.on("/diag", HTTP_GET, diags); 
        server.on("/fall", HTTP_GET, falld);
        server.on("/sensorupdrq",  HTTP_POST,  [](){ readInputs(); }); 
        server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");
  Serial.println("Initialize SMS Modem...");
M590.begin(9600);
delay(5000);
M590.print("ATE0\r"); // Echo Off
// Use AT+CFUN=15 to Reset Modem, returns +PBREADY on Succesful network connection.
delay(500);
M590.print("AT+CMGF=1\r");  // set SMS mode to text
Serial.println("set SMS mode to txt");  // set SMS mode to text
delay(2500);

M590.print("AT+CSCS=\"GSM\"");
M590.print("\r");
delay(2500);

Serial.println("set SMS mode to txt");  // set SMS mode to text
M590.print("AT+CMGF=1\r");  // set SMS mode to text
delay(2500);

  // blurt out contents of new SMS upon receipt to the GSM shield's serial out
Serial.println("blurt out contents of new SMS upon receipt to the GSM shield's serial out"); 
M590.print("AT+CNMI=2,2,0,0,0\r"); 
delay(2500);

M590.println("AT+CMGD=1,4"); // delete all SMS
Serial.println("delete all SMS"); // delete all SMS
delay(2500);  
Serial.println("Ready...");
}

void loop() {
	server.handleClient();
  while(M590.available()){smsG();}
}
