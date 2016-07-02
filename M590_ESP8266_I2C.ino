//
//  Rewrite for M590 GPRS module
//  http://www.banggood.com/GSM-GPRS-SIM900-1800MHz-Short-Message-Service-m590-SMS-Module-DIY-Kit-For-Arduino-p-1043437.html
//  
//  Added Nunber Verification (Full International Number required)
//  Added Response to incomming number, useful for returning sensor data etc...
//  Added Example sensor reply......
//  Only 2 LED's when using I2C, MPU6050/HMC5883L example code copied from Library Examples.....
//  The Power Supply needs to be able to produce 5v 2A for RF to function(After Drop over diode ~4.5v)
//  If there are any problems, the PSU is the most likely cause, 
//  Only UK Networks tested
//  the SIM choice can also cause issues, EE caused numerous failures, Vodafone failed about 20%
//  but Tesco/O2 seems to connect every time, although the SMS Delivery can take Longer 
//
//  Uncomment the 3 Lines to send sms, saving your credit....Serial print of data provided for setup.....
//  Send text: #a0 led 1 off(or 1 for on), #b0 led 2 off(or 1 for on), #c0 Send SMS, #c1 Clear SMS bank 1.
//  I2C Library Example Code from....
//
//  HMC5883L Triple Axis Digital Compass + MPU6050 (GY-86 / GY-87). Compass Example.
//  Read more: http://www.jarzebski.pl/arduino/czujniki-i-sensory/3-osiowy-magnetometr-hmc5883l.html
//  GIT: https://github.com/jarzebski/Arduino-HMC5883L
//  Web: http://www.jarzebski.pl
//  (c) 2014 by Korneliusz Jarzebski




#include <Wire.h>
#include <HMC5883L.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>

SoftwareSerial M590(12, 13);    // Rx, Tx
HMC5883L compass;
MPU6050 mpu;

char inchar;                     // Will hold the incoming character from the GSM shield
String conTrol = "XXXXXXXXXXXX"; // Only allow control from this number....Set to Your No.....
String callID = "";
String StrCallID = "";
String StrCallID1 = "";
int lIne=0;
int led1 = 14;
int led2 = 15;
 
void setup()
{ // initialize serial port + I2C:
 Serial.begin(9600);
 Wire.begin(4,5);
  // Initialize MPU6050
  Serial.println("Initialize MPU6050");
  while(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  // HMC5883L attached to MPU6050 secondary I2C Xsda Xscl
  mpu.setI2CMasterModeEnabled(false);
  mpu.setI2CBypassEnabled(true) ;
  mpu.setSleepEnabled(false);
  // Set measurement range
  compass.setRange(HMC5883L_RANGE_1_3GA);

  // Set measurement mode
  compass.setMeasurementMode(HMC5883L_CONTINOUS);

  // Set data rate
  compass.setDataRate(HMC5883L_DATARATE_30HZ);

  // Set number of samples averaged
  compass.setSamples(HMC5883L_SAMPLES_8);

  // Set calibration offset. See HMC5883L_calibration.ino
  compass.setOffset(0, 0); 
  // Calibrate gyroscope. The calibration must be at rest.
  // If you don't want calibrate, comment this line.
  mpu.calibrateGyro();

  // Set threshold sensivty. Default 3.
  // If you don't want use threshold, comment this line or set 0.
  mpu.setThreshold(3);
  Vector normGyro = mpu.readNormalizeGyro();
  Serial.print(" Xnorm = ");
  Serial.print(normGyro.XAxis);
  Serial.print(" Ynorm = ");
  Serial.print(normGyro.YAxis);
  Serial.print(" Znorm = ");
  Serial.println(normGyro.ZAxis);
 // set up the digital pins to control
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
 
  // wake up the GSM shield
M590.begin(9600);
 // give time to log on to network.
delay(5500);
M590.print("ATE0\r");
delay(500);
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

void smsG()
{
    inchar=M590.read(); 
    //Serial.write(inchar);
    callID += inchar;
    if (inchar == '\n'){// Data +CMT: "+440000000000"
     Serial.println(callID);
     if (lIne==0){  // Limit "Spoofing"
     StrCallID = callID.substring(1,4);
     }
    if (StrCallID =="CMT"){
      StrCallID1 = callID.substring(8,20);
      StrCallID="";
      Serial.print("The Caller ID to Verify : ");
      Serial.println(StrCallID1);
    if (StrCallID1 == conTrol){
      Serial.println("Number Verified.....");
    }else{
    Serial.println("Verification Failed, Control Not Permitted");
    }
      callID="";
      lIne=1;
      }
      callID="";
      }
  if (StrCallID1 == conTrol){
delay(50);
   if (inchar=='#')
    {
      delay(50);
      inchar=M590.read(); 
      if (inchar=='a')
      {     
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
          }
            else if (inchar=='c')
            {
              inchar=M590.read(); 
              if (inchar=='0')
              {
              delay(100);
                // Read normalized values 
  Vector normAccel = mpu.readNormalizeAccel();

  // Calculate Pitch & Roll
  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis*normAccel.YAxis + normAccel.ZAxis*normAccel.ZAxis))*180.0)/M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis)*180.0)/M_PI;
  Vector norm = compass.readNormalize();

  // Calculate heading
  float heading = atan2(norm.YAxis, norm.XAxis);

  // Set declination angle on your location and fix heading
  // You can find your declination on: http://magnetic-declination.com/
  // (+) Positive or (-) for negative
  // For Bytom / Poland declination angle is 4'26E (positive)
  // Formula: (deg + (min / 60.0)) / (180 / M_PI);
  float declinationAngle = (4.0 + (26.0 / 60.0)) / (180 / M_PI);
  heading += declinationAngle;

  // Correct for heading < 0deg and heading > 360deg
  if (heading < 0)
  {
    heading += 2 * PI;
  }
 
  if (heading > 2 * PI)
  {
    heading -= 2 * PI;
  }

  // Convert to degrees
  float headingDegrees = heading * 180/M_PI;
              String SensorStr = "Sensor Values, Pitch : ";
              SensorStr += pitch;
              SensorStr += ", Roll : ";
              SensorStr += roll;
              SensorStr += ", Heading : ";
              SensorStr += heading;
              SensorStr += ", Degrees : ";
              SensorStr += headingDegrees;
              Serial.println(SensorStr);
              delay(250);
        //  The next 3 commented out lines send the sms.......
            //  M590.print("AT+CMGS=\"" + StrCallID1 + "\"\r\n");  // Number to reply to from incomming message
              delay(2000);
            //  M590.print(SensorStr);//message content
              delay(1000);
            //  M590.write(0x1a);     // send out SMS
              delay(1000);
              // Alternatively post the data online when triggered, no cost when using wifi.....(ESP8266...)
              // Unlock a local webpage via sms to have the MCU verify the commands...
              } 
              else if (inchar=='1')
              {
             Serial.println("delete all SMS....");
             M590.println("AT+CMGD=1,4"); // delete all SMS
             Serial.println("All SMS Deleted"); // delete all SMS   
             }
          }      
      callID = "";
      StrCallID1="";
      }
   }
} 
 
void loop() 
{
  while(M590.available()){smsG();}
  lIne=0; // reset the line counter to only read caller id from first line of Serial data/sms
 //This is where to do some interesting stuff......
}
    
  
