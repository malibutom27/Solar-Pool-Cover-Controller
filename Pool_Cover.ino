// program for pool cover revision 22  03-01-19:

#include <EEPROM.h>
int coverPosition; // will be pin A0
int upperLimit; // will be the value of the upper limit read in from EEPROM
int lowerLimit; // will be the value of the lower limit read in from EEPROM
int switchUpPosition; // will be pin 2
int switchDownPosition;  // will be pin 3
int modeNormalOrCalibrate;  // will be pin 5
int calibrateSwitchUp;  // will be pin 6
int calibrateSwitchDown; // will be pin 9
int calibrateUpSet = 0;  // used to see if the calibrate switch has been moved to up
int calibrateDownSet = 0;  // used to see if the calibrate switch has been moved to down
int calibrationUp = 0;  // 0 if not done, 1 if done
int calibrationDown = 0;  // 0 if not done, 1 if done
int nextDirection = 0;  // 0 if just powered up, 1 if going down, 2 if going up, 3 if an error has occured
int timerLong = 0;  // the variable that will be used to hold the long time
int timerSetLong = 0;  // 0 if the tim0eout timer has not been started, 1 if the timer has started
int timerShort = 0;  // the variable that will be used to hold the short time
int timerSetShort = 0;  // 0 if the timeout time has not been started, 1 if the time has started
// need to set longTime, shortTime, and clicksPerIntervalxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx:
long longTime = 42250;  // the duration of the long timer before it forces a shut down
long shortTime = 2000;  // the length of time for a checkup on distance traveled by the position sensor
int positionStart = 0;  // the number of clicks the position sensor has traveled in the current time phase
int clicksPerInterval = 30;  // the number of clicks traveled per interval (per 1 second)
int digitalread2 = 0;  // scratch variable
float one;  // use to break position into bytes
float ten;  // use to break position into bytes
float hund;  // use to break position into bytes
float thou;  // use to break position into bytes
int moniter = 0;  // if on, it will dispay the monitor lines when usb is connected 
int errorCode = 0;  // 0 = no errors, 1 = going up and overall time expired, 2 = going up and 2 second time expired
// 3 = going down and overall time expired, 4 = going down and 2 second time expired
int errorCodeCounter = 1;  // the counter for the for/next loop in the error section of the program

void setup()
{
  digitalWrite (4, HIGH);  // pin 4 is power to power the cover up
  digitalWrite (7, HIGH);  // pin 7 is power to power the cover down
  pinMode (4, OUTPUT);  // because relays or on when input is low and because the input would be momentarily
  pinMode (7, OUTPUT);  // low at power up, must do a digitalWrite (high), then assign pinMode as output
  pinMode (2, INPUT);  // high if cover is going up
  pinMode (3, INPUT);  // high if cover is going down
  pinMode (5, INPUT);  // high if in the calibrate mode
  pinMode (6, INPUT);  // high if calibrating the upper position
  pinMode (8, OUTPUT);  // was used on relay shield, not used on 2 card relay
  pinMode (9, INPUT);  // high if calibrating the lower position
  pinMode (12, OUTPUT);  // used for LED power and error light 
  pinMode (13,INPUT);  // make high to turn the serial monitor on 
  Serial.begin(9600);

  moniter = digitalRead (13);
  digitalWrite (12,HIGH);  // turn LED power light on
  if (moniter ==1)
    {Serial.println ("doing first pass and delaying 1 second");delay (2000);}
  lowerLimit = EEPROM.read(0); timerSetLong = EEPROM.read(1); lowerLimit = lowerLimit + (timerSetLong * 10);
  timerSetLong = EEPROM.read(2); lowerLimit = lowerLimit + (timerSetLong * 100);
  timerSetLong = EEPROM.read(3); lowerLimit = lowerLimit + (timerSetLong * 1000);
  upperLimit = EEPROM.read(4); timerSetLong = EEPROM.read(5); upperLimit = upperLimit + (timerSetLong * 10);
  timerSetLong = EEPROM.read(6); upperLimit = upperLimit + (timerSetLong * 100);
  timerSetLong = EEPROM.read(7); upperLimit = upperLimit + (timerSetLong * 1000);timerSetLong = 0;
  if (moniter == 1)
    {
      Serial.print ("upperlimit after load = ");Serial.println (upperLimit);
      Serial.print ("lowerLimit after load = ");Serial.println (lowerLimit);
    }
  digitalread2 = digitalRead (2);
  if (moniter == 1) {Serial.print ("up = ");Serial.println (digitalread2);delay(2000);}
  if (digitalread2 == 1) { nextDirection = 1;}
  digitalread2 = digitalRead (3);
  if (moniter == 1) {Serial.print ("down = ");Serial.println (digitalread2);}
  if (digitalread2 == 1) { nextDirection = 2;}
}

void loop()
{ // begining of main loop
   if ( moniter == 1 ){Serial.print ("nextDirection = ");Serial.println (nextDirection);
   Serial.println ("if nextDirection = 3, its in error mode");delay(2000);}
  if (nextDirection < 3)
  {
    //this is the beginning of the whole loop after the first pass check;
      // this is the start of the calibrate routine;
     if (moniter == 1) {Serial.println ("bypassed the first pass routine and delaying 2 seconds");
      delay (2000);}
      modeNormalOrCalibrate = digitalRead (5);
      if (modeNormalOrCalibrate == 1)
      {  
        delay (2000);
        modeNormalOrCalibrate = digitalRead (5);
        if (modeNormalOrCalibrate == 1)
        {  // this is the beginning of the calibrate routine **********************************************
          if (moniter == 1){
          Serial.println ("the calibrate switch is on, and delaying 2 seconds");
          Serial.print ("modeNormalOrCalibrate = ");
          Serial.println (modeNormalOrCalibrate);
          delay (2000);}
          calibrationDown = 0; calibrationUp = 0;
          while  (modeNormalOrCalibrate == 1)  // if true, its in the calibrate mode
          {
            if (moniter == 1){
            Serial.print ("in the while routine.  modeNormalOrCalibrate before read = ");
            Serial.println (modeNormalOrCalibrate);}
            modeNormalOrCalibrate = digitalRead (5);
            if (moniter == 1){
            Serial.print ("modeNormalOrCalibrate after read = ");
            Serial.println (modeNormalOrCalibrate);delay (2000);}
            calibrateSwitchUp = digitalRead (6);
            if (calibrateSwitchUp == 1)
            {
              delay (1000);
              calibrateSwitchUp = digitalRead (6);
              if (calibrateSwitchUp == 1)
              {
                if (moniter == 1){
                Serial.println ("the calibrate switch is in the up position, and delaying 2 seconds");}
                upperLimit = analogRead (A0);
                if (moniter == 1){
                Serial.print ("the upper limit is set to ");
                Serial.println (upperLimit);delay (2000);}
                calibrationUp = 1;
              }
            }
            calibrateSwitchDown = digitalRead (9);
            if (calibrateSwitchDown == 1)
            {
              delay (1000);
              calibrateSwitchDown = digitalRead (9);
              if (calibrateSwitchDown == 1)
              {
                if (moniter == 1){
                Serial.println ("the calibrate switch is in the down position, and delaying 2 seconds");}
                lowerLimit = analogRead (A0);
                if (moniter == 1){
                Serial.print ("after read the lower limit is set to ");
                Serial.println (lowerLimit);delay (2000);}
                calibrationDown = 1;
              }
            }
          }
          if (calibrationDown == 1)
          {
            if (moniter == 1){
            Serial.println("now saving the calibration down number and delay 2 seconds.  LowerLimit = ");
            Serial.print (lowerLimit);}
            thou = lowerLimit / 1000;
            hund = (lowerLimit / 100) - (thou * 10);
            ten =  (lowerLimit / 10) - ((thou * 100) + (hund * 10));
            one = lowerLimit % 10;
            if (moniter == 1){
            Serial.println (one);
            Serial.println (ten);
            Serial.println (hund);
            Serial.println (thou);
            delay (2000);}
            EEPROM.write (0,one);EEPROM.write (1,ten);EEPROM.write (2,hund);EEPROM.write (3,thou);
          }

          if (calibrationUp == 1)
          {
            if (moniter == 1){
            Serial.println("now saving the calibration up number and delay 2 seconds.  upperLimit =");
            Serial.println (upperLimit);}
            thou = upperLimit / 1000;
            hund = (upperLimit / 100) - (thou * 10);
            ten =  (upperLimit / 10) - ((thou * 100) + (hund * 10));
            one = upperLimit % 10;
            if (moniter == 1){
            Serial.println (one);
            Serial.println (ten);
            Serial.println (hund);
            Serial.println (thou);
            delay (2000);}
            EEPROM.write (4,one);EEPROM.write (5,ten);EEPROM.write (6,hund);EEPROM.write (7,thou);
          }
          calibrationDown = 0; calibrationUp = 0;
        }  //this is the end of the calibrate routine *******************************************************************
      }

      // this is the main loop

      // routine to raise cover
      if (moniter == 1){Serial.println("in main loop, waiting for a signal to go up or down");
      Serial.println ("nextDirection = ");Serial.println (nextDirection);delay (2000);}
      if (nextDirection == 0 || nextDirection == 1)
        {
          switchUpPosition = digitalRead (2);
          if (moniter == 1){Serial.print ("switchUpPosition = ");
            Serial.println (switchUpPosition);
            delay (2000);}
          if (switchUpPosition == 1)
            {
              coverPosition = analogRead (A0);Serial.print ("upper ");Serial.print(upperLimit);
              Serial.print("   ");Serial.println (coverPosition);
              if (moniter == 1){Serial.println("now in the raise cover loop ");Serial.println ("cover position = ");
               Serial.println (coverPosition);delay (2000);Serial.println ("upperLimit = ");Serial.println (upperLimit);}
              if (coverPosition < upperLimit)
                {
                  digitalWrite (4, LOW); // turns up relay on
                  if (timerSetLong == 0)
                    {
                      timerLong = millis();
                      timerSetLong = longTime * ((upperLimit - coverPosition) / (upperLimit - lowerLimit));
                    }
                  else if ( millis() > timerLong + timerSetLong )
                    {
                      digitalWrite (4, HIGH); nextDirection = 3; timerSetLong = 0; errorCode = 2;
                      timerSetShort = 0;
                      if (moniter == 1){Serial.println("an error has occured.  The overall time has expired");
                       Serial.println ("millis   timerLong    longTime = ");
                       Serial.println (millis());Serial.println (timerLong);Serial.println (longTime);
                       delay (5000);}
                    }  // error, the time has expired
            if (timerSetShort == 0) {
              timerSetShort = 1;
              timerShort = millis();
              positionStart = analogRead (A0);
            }
            else if ( millis() > timerShort + shortTime)
              // 2 seconds has transpired, now check to see if distance is too short
            {
            if (moniter == 1)
            {
              Serial.println ("millis = timerShort = shortTime =");
              Serial.println (millis()   );Serial.println (timerShort);Serial.println (shortTime);
              }
             if (moniter == 1){ Serial.println ("coverPosition  PositionStart   ClicksPerInterval");
              Serial.println (coverPosition);Serial.println (positionStart);Serial.println (clicksPerInterval);
             delay (2000);}
              if (coverPosition < positionStart + clicksPerInterval)
              {
                digitalWrite (4, HIGH); nextDirection = 3; timerSetShort = 0; timerSetLong = 0; errorCode = 3;
                if (moniter == 1){Serial.println ("an error has occured.  The cover didn't move far enough in 1 second");
                delay (2000);}
              }
              else {
                timerSetShort = 0;
              }
            }
          }
          else 
            {
              digitalWrite (4, HIGH);
              nextDirection = 2;
              timerSetLong == 0;
              timerSetShort = 0;
            }
        }
        else 
            {
              digitalWrite (4, HIGH);
              nextDirection = 2;
              timerSetLong == 0;
              timerSetShort = 0;
            }
      }

      // routine to lower cover
      if (nextDirection == 0 || nextDirection == 2)
      {
        switchDownPosition = digitalRead (3);
         if (moniter == 1){Serial.println ("switchDownPosition = ");
        Serial.println (switchDownPosition);delay (2000);}
        if (switchDownPosition == 1)
        {
           coverPosition = analogRead (A0);
           Serial.print ("lower ");Serial.print(lowerLimit);
           Serial.print("   ");Serial.println (coverPosition);
           if (moniter == 1){Serial.println("now in the lower cover loop ");Serial.println ("cover position = ");
        Serial.println (coverPosition);Serial.println ("lowerLimit = ");Serial.println (lowerLimit);delay (2000);}
          if (coverPosition > lowerLimit)
          {
          if (moniter == 1)
           {   
              Serial.println ("timerLong before = ");Serial.println (timerLong);
           }
              digitalWrite (7, LOW);digitalWrite (4, LOW); // turns up & down relay on
              if (moniter == 1)
              {
              Serial.println ("timerSetLong = ");Serial.println (timerSetLong);delay(5000);
              }
            if (timerSetLong == 0) {
              timerLong = millis();
            if (moniter == 1)
              {
              Serial.println ("timerLong after = ");Serial.println (timerLong);
              }
              timerSetLong = longTime * ((coverPosition - lowerLimit)      /     (upperLimit - lowerLimit));;
            }
            else if ( millis() > timerLong + timerSetLong ) {
              digitalWrite (7, HIGH);digitalWrite (4, HIGH); nextDirection = 3; timerSetLong = 0;
              timerSetShort = 0; errorCode = 4;
              if (moniter == 1){Serial.println("an error has occured.  The overall time has expired");
              Serial.println ("millis   timerLong    longTime = ");
              Serial.println (millis());Serial.println (timerLong);Serial.println (longTime);
              delay (5000);}
            }  // error, the time has expired
            if (timerSetShort == 0) {
              timerSetShort = 1;
              timerShort = millis();
              positionStart = analogRead (A0);
            }
            else if ( millis() > timerShort + shortTime)
              // 1 second has transpired, now check to see if distance is too short
            {
             if (moniter == 1){ Serial.println ("coverPosition  PositionStart   ClicksPerInterval");
              Serial.println (coverPosition);Serial.println (positionStart);Serial.println (clicksPerInterval);
             delay (2000);}
              if (coverPosition > positionStart - clicksPerInterval)
              {
                digitalWrite (7, HIGH);digitalWrite (4, HIGH); nextDirection = 3; timerSetShort = 0; timerSetLong = 0;
                 errorCode = 5;
                if (moniter == 1){Serial.println ("an error has occured.  The cover didn't move far enough in 2 seconds");
                delay (2000);}
              }
              else {
                timerSetShort = 0;
              }
            }
          }
          else {
            digitalWrite (7, HIGH);digitalWrite (4, HIGH);
            nextDirection = 1;
            timerSetLong == 0;
            timerSetShort = 0;
          }
        }
        else 
            {
              digitalWrite (7, HIGH);digitalWrite (4, HIGH);
              nextDirection = 1;
              timerSetLong == 0;
              timerSetShort = 0;
            }
        }
  }
  else
     {
    digitalWrite (12,LOW); delay (3000);
      if (moniter == 1 & errorCode == 2) { Serial.println ("error code = 2: going up and total time exceeded");}
      if (moniter == 1 & errorCode == 3) { Serial.println ("error code = 3: going up and 2 second time exceeded");}
      if (moniter == 1 & errorCode == 4) { Serial.println ("error code = 4: going down and total time exceeded");}
      if (moniter == 1 & errorCode == 5) { Serial.println ("error code = 5: going down and 2 second time exceeded");}
    while (errorCode > 0)
      {
    for ( errorCodeCounter = 1; errorCodeCounter < errorCode; errorCodeCounter++)
      {
      digitalWrite (12,HIGH); delay (1000);digitalWrite (12,LOW); delay (1000);
      }
      digitalWrite (12,HIGH); delay (1000);digitalWrite (12,LOW); delay (8000);
      }
     } 
  }  // end of main loop
