/*************************************************************************/
/*                              TimerOne.h                               */
/*                              **********                               */
/*                                                                       */
/* Includes the source code for the timer used in the ISR checkLine      */
/*                                                                       */
/*************************************************************************/

#include <TimerOne.h>

/*************************************************************************/
/*                             global variables                          */
/*                             ****************                          */
/*                                                                       */
/* interruptMessage: an int that represents which message to send based  */
/*                  on the toggling of the hardware lines                */
/*                                                                       */
/* commandMode: an int that represents whether interrupts are disabled,  */
/*                  a negative value means interrupts are on (default)   */
/*                                                                       */
/* msg, msg2, msg3, msg4: Strings to be sent over serial to the APFx     */
/*                                                                       */
/* msgLen, msg2Len, msg3Len, msg4Len: ints used to specify length of the */
/*                  respective messages, needed when sending as bytes    */
/*                                                                       */
/* cmdMode, pts, pt, p: arrays of bytes sent over serial, the arrays are */
/*                  created by converting their corresponding strings to */
/*                  bytes using a built-in function                      */
/*                                                                       */
/*************************************************************************/

int interruptMessage = 0;
int commandMode = -1;
int cpMode = -1;
long count = 0;

String msg = "SBE 41CP UW. V 2.0\n\rS>";
int msgLen = msg.length()+1;
byte cmdMode[100];

String msg2;
int msg2Len;
byte pts[100];

String msg3;
int msg3Len;
byte pt[100];

String msg4;
int msg4Len;
byte p[100];


/*************************************************************************/
/*                            function prototypes                        */
/*                            *******************                        */
/*                                                                       */
/*************************************************************************/

String getReadingFromPiston(int);
String floatToString(float);
int debounce(int);
void checkLine();
void runTimer(int);
void setup();
void loop();


/*************************************************************************/
/*                              checkline                                */
/*                              *********                                */
/*                                                                       */
/* Attached to a rising logic level on pin 2                             */
/* paramaters: none                                                      */
/* returns: none                                                         */
/*                                                                       */
/* After the initial change, wait 200ms, then check the request line. If */
/* still high, send the APF9 the fw rev and turn command mode on. If the */
/* request line is low, then check the mode line quickly. If the mode    */
/* line is high and the Rx line is low, send the APF9 a P,T,S sample.    */
/* If the request line is low and the mode line is low, then check the   */
/* Rx line. If the Rx line is high, sent a P,T sample. If the Rx line is */
/* low, send a P sample. The samples are determined by the               */
/* getPTSfromPiston, getPTfromPiston, getPfromPiston functions. The      */
/* messages are created by taking a string and converting it to an array */
/* of bytes then writing the array of bytes over serial.                 */
/*                                                                       */
/*           ***************************************************         */
/*           **  Request  *    Rx    *   Mode   *   Message   **         */
/*           ***************************************************         */
/*           **   HIGH    *   ----   *   ----   *   Serial #  **         */
/*           ***************************************************         */
/*           **    LOW    *   ----   *   HIGH   *    P,T,S    **         */
/*           ***************************************************         */
/*           **    LOW    *   HIGH   *    LOW   *     P,T     **         */
/*           ***************************************************         */
/*           **    LOW    *    LOW   *    LOW   *      P      **         */
/*           ***************************************************         */
/*                                                                       */
/*************************************************************************/


void checkLine(){
  
  //start the timer, once  the current time is ~200ms more than the first reading, continue to check the hardware lines
  long time;
  Timer1.start();
  long timeLast = Timer1.read();
  int i = 0;
  for(i = 0; i < 100000; i++){
    time = Timer1.read();
    if (time > (timeLast + 199900)){
      Timer1.stop();
      break;
    }
  }
  
  //if the request line is still high, choose message 1 (get serial number / firmware rev) 
  if(digitalRead(2)==HIGH){
    interruptMessage = 1;
    commandMode = 1;
  }
  
  //if the request line is low, check the other two lines
  else if(digitalRead(2)==LOW){
    
    //if the mode line is high, choose message 2 (get pts)
    if(digitalRead(3)==HIGH){
      interruptMessage = 2;
    }
    
    //if the mode line (3) is low, and the Rx line (19) is high (check with debounce to be safe)\
    //choose message 3 (get pt)
    else if((debounce(19)>0)&&(debounce(3)<0)){
      interruptMessage = 3;
    }
    
    //if the mode line (3) is low, and the Rx line (19) is low (check with debounce to be safe)
    //choose message 4 (get p)
    else if((debounce(19)<0)&&(debounce(3)<0)){//get p
      interruptMessage = 4;
    }
    
    //else don't do anything
    
    else{
    }
  }
}

/*************************************************************************/
/*                                 setup                                 */
/*                                 *****                                 */
/*                                                                       */
/* parameters: none                                                      */
/* returns: none                                                         */
/*                                                                       */
/* Part of programming in Arduino, used to set up the board for the      */
/* program. Enables serial communication over ports 1 and 2, at a baud   */
/* rate of 9600 for both. It also configures pins 8 as an ouput.         */
/* It configures pins 2, 3, and A0 as inputs. Pin 2 and 3 are digital    */
/* inputs. A0 is an analog input. It sets the reference voltage for      */
/* analog input at 2.56V as its max. It attaches an interrupt to pin 2   */
/* that will run the function checkLines if it is triggered by a rising  */
/* edge. It also initializes the timer with a period of 1 second.        */
/*                                                                       */
/*************************************************************************/

void setup(){
  //initialize serial ports at 9600 baud 8-N-1 
  Serial.begin(9600);
  Serial1.begin(9600);
  
  //sets pin 8 as an output
  pinMode(8, OUTPUT);
  
  //sets pins 2 (digital), 3 (digital), and A0 (analog) as inputs
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(A0, INPUT);
  
  //sets the analog reference (max) voltage at 2.56V
  analogReference(INTERNAL2V56);
  
  //if there is a rising edge on pin 2, the function checkLine will be called
  attachInterrupt(0, checkLine, RISING);
  
  //initializes the timer with a period of 1 sec
  Timer1.initialize(1000000);
}

/*************************************************************************/
/*                                 loop                                  */
/*                                 ****                                  */
/*                                                                       */
/* parameters: none                                                      */
/* returns: none                                                         */
/*                                                                       */
/* Part of programming in Arduino, used as the main loop. It performs a  */
/* digitalWrite to pin 8 which is connected to an LED to let the tester  */
/* know that the device is on and ready. The loop then enters a switch   */
/* statement to handle the sending of messages based on the value of     */
/* interruptMessage which is changed by the itnerrupt on pin 2. The rest */
/* of the function handles receiving and sending correct serial messages */
/* over the Serial1 Tx and Rx lines. It does so by sending response      */
/* strings as arrays of bytes.                                           */
/*                                                                       */
/*************************************************************************/


void loop(){
  
  //turn on status LED attached to pin 8
  digitalWrite(8, HIGH);
  
  //interruptMessage will be zero unless changed during the ISR
  switch(interruptMessage){
    
    //if it is 0, do nothing and just leave the loop
    case 0:
      break;
      
    //if it is 1, convert the global string msg to the global byte array cmdMode
    //then send the array over Serial1, reset interruptMessage to 0, then leave the loop
    case 1:
      msg.getBytes(cmdMode, msgLen);
      Serial1.write(cmdMode, msgLen);
      interruptMessage = 0;
      break;
      
    //if it is 2, clear any junk analog values on A0 before getting the p,t,s value based on the analog 
    //value on pin A0, set msg2 equal to this value, then convert the global string msg2 to the global 
    //byte array pts, then send the array over Serial1, reset interruptMessage to 0, then leave the loop
    case 2:
      analogRead(A0);
      msg2 = getReadingFromPiston(2);
      msg2Len = msg2.length()+1;
      msg2.getBytes(pts, msg2Len);
      Serial1.write(pts, msg2Len);
      interruptMessage = 0;
      break;
    
    //if it is 3, clear any junk analog values on A0 before getting the p,t value based on the analog
    //value on pin A0, set msg3 eqaul to this value, then convert the global string msg3 to the global 
    //byte array pt, then send the array over Serial1, reset interruptMessage to 0, then leave the loop
    case 3:
      analogRead(A0);
      msg3 = getReadingFromPiston(3);
      msg3Len = msg3.length()+1;
      msg3.getBytes(pt, msg3Len);
      Serial1.write(pt, msg3Len);
      interruptMessage = 0;
      break;

    //if it is 4, clear any junk analog values on A0 before getting the p value based on the analog
    //value on pin A0, set msg4 eqaul to this value, then convert the global string msg4 to the global 
    //byte array p, then send the array over Serial1, reset interruptMessage to 0, then leave the loop
    case 4:
      analogRead(A0);
      msg4 = getReadingFromPiston(4);
      msg4Len = msg4.length()+1;
      msg4.getBytes(p, msg4Len);
      Serial1.write(p, msg4Len);
      interruptMessage = 0;
      break;
  }
  
  
  /*************************************************************/
  /*                 CONTINUOUS PROFILING MODE                 */
  /*************************************************************/
  while(cpMode == 1){
    //change this code
    long time;
    Timer1.start();
    long timeLast = Timer1.read();
    int i = 0;
    for(i = 0; i < 100000; i++){
      time = Timer1.read();
      if (time > (timeLast + 999900)){
        Timer1.stop();
        count += 1;
        break;
      }
    }
    if(Serial1.available()>0){
      String input = "";
      while(1){
        if(Serial1.available()>0){  
          char temp;
          temp = char(Serial1.read());
          input+=temp;
          if((input.equals('\r'))||(input.equals("stopprofile"))){
            break;
          }
        }
      }
      if(input.equals("stopprofile")){
        String exitcp = "\n\rS>stopprofile";
        int exitcpLen = exitcp.length()+1;
        byte exitcpBuffer[100];
        exitcp.getBytes(exitcpBuffer, exitcpLen);
        Serial1.write(exitcpBuffer, exitcpLen);
        cpMode = -1;
      }
      else if(input.equals("\r")){
        String cmdMode = "\n\rS>";
        int cmdModeLen = cmdMode.length()+1;
        byte cmdModeBuffer[100];
        cmdMode.getBytes(cmdModeBuffer, cmdModeLen);
        Serial1.write(cmdModeBuffer, cmdModeLen);
      }
    }
  }
  
  
  /*************************************************************/
  /*                        COMMAND MODE                       */
  /*************************************************************/
  if(commandMode == 1){
    
    //ignore interrupts on pin 2 (only want to do this once)
    detachInterrupt(0);
    
    //stay in command mode until the value of commandMode changes 
    while(commandMode == 1){
      
      //check for a message in Serial1, it there is, create a blank string, then add each character in the 
      //Serial1 input buffer to the input string. Wait until a carriage return to make sure a command
      //is actually sent, if it is not the carriage return, wait for the next character
      if(Serial1.available()>0){
        String input = "";
        while(1){
          if(Serial1.available()>0){  
            char temp;
            temp = char(Serial1.read());
            input+=temp;
            if((temp=='\r')||(input.equals("startprofile"))||(input.equals("stopprofile"))){
              break;
            }
          }
        }
        
        //if the input is a carriage return, send back the sbe command prompt (S>) as a series of byes
        if(input.equals("\r")){
          String cmdMode = "\n\rS>";
          int cmdModeLen = cmdMode.length()+1;
          byte cmdModeBuffer[100];
          cmdMode.getBytes(cmdModeBuffer, cmdModeLen);
          Serial1.write(cmdModeBuffer, cmdModeLen);
        }
        
        //if the input is the ds command, send back all of the information as a series of bytes (uses generic
        //info based on an actual seabird (can edit field in this string if necessary)
        else if(input.equals("ds\r")){
          String ds = "ds\n\rSBE 41CP UW V 2.0  SERIAL NO. 4242\n\rfirmware compilation date: 18 December 2007 09:20\n\rstop profile when pressure is less than = 2.0 decibars\n\rautomatic bin averaging at end of profile disabled\n\rnumber of samples = 0\n\rnumber of bins = 0\n\rtop bin interval = 2\n\rtop bin size = 2\n\rtop bin max = 10\n\rmiddle bin interval = 2\n\rmiddle bin size = 2\n\rmiddle bin max = 20\n\rbottom bin interval = 2\n\rbottom bin size = 2\n\rdo not include two transition bins\n\rinclude samples per bin\n\rpumped take sample wait time = 20 sec\n\rreal-time output is PTS\n\rS>";
          int dsLen = ds.length()+1;
          byte dsBuffer[1000];
          ds.getBytes(dsBuffer, dsLen);
          Serial1.write(dsBuffer, dsLen);
        }
        
        //if the input is the dc command, send back all of the information as a series of bytes (uses generic
        //info based on an actual seabird (can edit field in this string if necessary)
        else if(input.equals("dc\r")){
          String dc = "dc\n\rSBE 41CP UW V 2.0  SERIAL NO. 4242\n\rtemperature:  19-dec-10    \n\rTA0 =  4.882851e-05    \n\rTA1 =  2.747638e-04    \n\rTA2 = -2.478284e-06    \n\rTA3 =  1.530870e-07\n\rconductivity:  19-dec-10    \n\rG = -1.013506e+00    \n\rH =  1.473695e-01    \n\rI = -3.584262e-04    \n\rJ =  4.733101e-05    \n\rCPCOR = -9.570001e-08    \n\rCTCOR =  3.250000e-06    \n\rWBOTC =  2.536509e-08\n\rpressure S/N = 3212552, range = 2900 psia:  14-dec-10    \n\rPA0 =  6.297445e-01    \n\rPA1 =  1.403743e-01    \n\rPA2 = -3.996384e-08    \n\rPTCA0 =  6.392568e+01    \n\rPTCA1 =  2.642689e-01    \n\rPTCA2 = -2.513274e-03    \n\rPTCB0 =  2.523900e+01    \n\rPTCB1 = -2.000000e-04    \n\rPTCB2 =  0.000000e+00    \n\rPTHA0 = -7.752968e+01    \n\rPTHA1 =  5.141199e-02    \n\rPTHA2 = -7.570264e-07    \n\rPOFFSET =  0.000000e+00\n\rS>";
          int dcLen = dc.length()+1;
          byte dcBuffer[1000];
          dc.getBytes(dcBuffer, dcLen);
          Serial1.write(dcBuffer, dcLen);
        }
        
        //if the input is startprofileN, recognize that it is the start profile command,
        // then send back that the profile has started
        else if(input.equals("startprofile")){
          String cp = "\n\rS>startprofile\n\rprofile started, pump delay = 0 seconds";
          int cpLen = cp.length()+1;
          byte cpBuffer[100];
          cp.getBytes(cpBuffer, cpLen);
          Serial1.write(cpBuffer, cpLen);
          cpMode = 1;
          delay(100);
          attachInterrupt(0, checkLine, RISING);
        }
        
        else if(input.equals("stopprofile")){
          String exitcp = "\n\rS>stopprofile";
          int exitcpLen = exitcp.length()+1;
          byte exitcpBuffer[100];
          exitcp.getBytes(exitcpBuffer, exitcpLen);
          Serial1.write(exitcpBuffer, exitcpLen);
          cpMode = -1;
        }
        
        else if(input.equals("binaverage\r")){
          String binAvg = "\n\rS>samples =";
          int binAvgLen = binAvg.length()+1;
          byte binAvgBuffer[100];
          binAvg.getBytes(binAvgBuffer, binAvgLen);
          Serial1.write(binAvgBuffer, binAvgLen);
        }
        
        else if(input.equals("da\r")){
          detachInterrupt(0);
          int ii = 0;
          for(ii = 0; ii < count; ii+26){
            long time;
            Timer1.start();
            long timeLast = Timer1.read();
            int i = 0;
            for(i = 0; i < 100000; i++){
              time = Timer1.read();
              if (time > (timeLast + 999900)){
                Timer1.stop();
                break;
              }
            }
            String sample = samples[ii];
            int sampleLen = sample.length()+1;
            byte sampleBuffer[100];
            sample.getBytes(sampleBuffer, sampleLen);
            Serial1.write(sampleBuffer, sampleLen);
          }
          String done = "\n\rdone\r"
          int doneLen = done.length()+1;
          byte doneBuffer[100];
          done.getBytes(doneBuffer, doneLen);
          Serial1.write(doneeBuffer, doneLen);
          delay(100);
          attachInterrupt(0, checkLine, RISING);
        }
        
        //if the input is qsr, send back that the seabird is powering down as a series of bytes, 
        //leave command mode, and turn the interrupt back on (will send a junk value when turned back on)
        else if(input.equals("qsr\r")){
          String cmdMode = "\n\rS>qsr\n\rpowering down\n\rS>";
          int cmdModeLen = cmdMode.length()+1;
          byte cmdModeBuffer[100];
          cmdMode.getBytes(cmdModeBuffer, cmdModeLen);
          Serial1.write(cmdModeBuffer, cmdModeLen);
          commandMode = -1;
          delay(100);
          attachInterrupt(0, checkLine, RISING);
        }
      }
    }
  }
}



/*************************************************************************/
/*                                debounce                               */
/*                                ********                               */
/*                                                                       */
/* parameters: pin, an integer value representing the pin that is        */
/*                 connected to the input being toggled                  */
/* returns: an integer value that will be positive (1) if the pin is     */
/*                 high and negative (-1) if the pin is low              */
/*                                                                       */
/* This function checks the logic level of a pin 6 times (once / ~50ms)  */
/* and determines if it is high or low                                   */
/*                                                                       */
/*************************************************************************/

int debounce(int pin){
  int highOrLow = 0;
  int highOrLowTotal = 0;
  int i = 0;
  
  //check the given pin, if it is high, add 1 to the total, if it is low add 0 to the total.
  //repeat this 6 times for accuracy, waiting ~50ms between each read of the pin.
  for(i = 0; i < 6; i++){
    if(digitalRead(pin)==HIGH){
      highOrLow = 1;
    }
    else{
      highOrLow = 0;
    }
    long time;
    Timer1.start();
    long timeLast = Timer1.read();
    int i = 0;
    for(i = 0; i < 100000; i++){
      time = Timer1.read();
      if (time > (timeLast + 49900)){
        Timer1.stop();
        break;
      }
    }
    highOrLowTotal+=highOrLow;
  }
  
  //if it is considered high (based on value of total after loop), return 1
  if(highOrLowTotal >=2){
    return 1;
  }
  
  //if it is considered low (based on value of total after loop), return -1
  else{
    return -1;
  }
}

/*************************************************************************/
/*                             getReadingFromPiston                      */
/*                             ********************                      */
/*                                                                       */
/* parameters: select, an int value that represents which string will be */
/*                  returned (PTS, PT, or P reading)                     */
/* returns: String representing the PTS, PT, or P reading                */
/*                                                                       */
/* This function converts a reading from the analog input pin A0 to a    */
/* string that represents P,T,S sample. This is achieved by manipulating */
/* the input value and fitting it to generic, general values tested by   */
/* Hugh Fargher. In general, we used 3 linear models to represent 3      */
/* ranges of depth (2000m-1000m, 1000m-500m, 500m-0m) with different     */
/* slopes and offsets. From these pressure values, we assume another     */
/* linear relationship to temperature (as pressure increases linerarly,  */
/* temperature decreases linearly). Lastly, we can assume one last       */
/* linear relationship between pressure and salinity (as pressure        */
/* increases linearly, salinity increases linearly). The values of these */
/* strings are appended to one another and formatted to match a regex    */
/* pattern expected by the APF board on the float. Then use the select   */
/* to choose which string (PTS, PT, or P to send to the APF board.       */
/*                                                                       */
/*************************************************************************/

String getReadingFromPiston(int select){
  
  //original calculated values as floats
  float pressure;
  float temperature;
  float salinity;
  
  //represent the values as longs that are either 100 or 1000 times larger than the floats
  long pressureLong;
  long temperatureLong;
  long salinityLong;
  
  //the string representation of the pressure, temperature, salinity, all 3 together,
  //a combination of pressure and temperature, then the message to be sent
  String pStr;
  String tStr;
  String sStr;
  String ptsStr;
  String ptStr;
  String sendMessage;
  
  //read an analog value on pin 1, use it for the calculations 1023=2.56V
  int voltage = analogRead(A0);
  Serial.println(String(voltage));
  
  //technically out of range, but use it to go to a pressure greater than 2000dbar, min change = 5dbar
  if(voltage<72){
    pressure = 2000+1*(voltage-72);
  }
  
  //for pressures between 2000-1000dbar, 72 = 2000dbar, 293 = 1000dbar, min change = 4.5045dbar
  else if((voltage>=72)&&(voltage<294)){
    pressure = ((4.5045)*(222-(voltage-72)))+1000.00;
  }
  
  //for pressures between 1000-500dbar, 294 = 1000dbar, 453 = 500dbar, min change = 3.125dbar
  else if((voltage>=294)&&(voltage<454)){
    pressure = ((3.125)*(160-(voltage-294)))+500.00;
  }
  
  //for pressures between 500-0dbar, 454 = 500dbar, 1023 = 0dbar, min change = 0.878dbar
  else if((voltage>=454)&&(voltage<1024)){
    pressure = ((0.878)*(569-(voltage-454)));
  }
  
  //adjust for hardware that amplifies the signal by approximately 1.1, then convert the float 
  //to a string using the floatToString function
  pressure = pressure * 1.08;
  pStr = floatToString(pressure);
  
  //calculate a float temperature value based on the pressure, assume linearity with the maximum
  //temperature of 20 deg C and minimum of 5 deg C. then convert the float 
  //to a string using the floatToString function
  temperature = 20-(((pressure)*(15.00))/2000.00);
  tStr = floatToString(temperature);
  
  //calculate a float salinty value based on the pressure, assume linearity with the maximum
  //salinity of 37.5 and minimum of 33.5. then convert the float 
  //to a string using the floatToString function
  salinity = (((pressure)*(4.00))/2000) + 33.5;
  sStr = floatToString(salinity);
  
  //add all of the strings to create one string that represents a p,t,s reading
  ptsStr = pStr+", "+tStr+", "+sStr+"\r\n";
  
  //add the pressure and temperature strings to create one string that represents a p,t reading
  ptStr = pStr+", "+tStr+"\r\n";
  
  
  //choose which string you want to return based on the select value
  switch(select){
    case 0:
      sendMessage = "";
      break;
    case 1:
      sendMessage = "";
      break;
    case 2:
      sendMessage = ptsStr;
      break;
    case 3:
      sendMessage = ptStr;
      break;
    case 4:
      sendMessage = (pStr+"\r\n");
      break;
  }
  
  //return the given string
  return sendMessage;
}

/*************************************************************************/
/*                             floatToString                             */
/*                             *************                             */
/*                                                                       */
/* parameters: aFloat, a float value representing the float that is      */
/*                 going to be converted to a string                     */
/* returns: an string value that will represent the float as a string    */
/*                                                                       */
/* This function creates a string that will look like a float by         */
/* splitting it into its whole and decimal parts, then adding them as    */
/* two strings with the appropriate formatting for a p, t ,or s value    */
/*                                                                       */
/*************************************************************************/

String floatToString(float aFloat){
  
  //long int values
  long floatLong;
  int floatInt;
  int floatDec;
  
  //string to be returned
  String floatStr;
  
  //calculate the whole number and decimal number
  floatLong = 1000*aFloat;
  floatInt = floatLong/1000;
  floatDec = floatLong - (floatInt*1000);
  Serial.println(String(floatInt));
  Serial.println(String(floatDec));
  
  //handle case for losing the 0 in a number less than 10 (i.e. get 09 instead of 9)
  if(floatDec<10){
    floatStr = String(floatInt)+".00"+String(floatDec);
  }
  else if((floatDec < 100)&&(floatDec >= 10)){
    floatStr = String(floatInt)+".0"+String(floatDec);
  }
  else{
    floatStr = String(floatInt)+'.'+String(floatDec);
  }
  
  //return the formatted string
  return floatStr;
}

/*************************************************************************/
/*                                runTimer                               */
/*                                ********                               */
/*                                                                       */
/* parameters: timeout, an int representing the desired runtime in ns    */
/* returns: none                                                         */
/*                                                                       */
/* This function runs a timer for the given interval of time, uses code  */
/* from TimerOne.h                                                       */
/*                                                                       */
/*************************************************************************/
void runTimer(int timeOut){
  long time;
  Timer1.start();
  long timeLast = Timer1.read();
  long i = 0;
  for(i = 0; i < 10000000; i++){
    time = Timer1.read();
    if (time > (timeLast + timeOut)){
      Timer1.stop();
      break;
    }
  }
}
25530	1002	2000	25.53

String samples[1002] = "0, 0, 0, 0",
"2.410000086, 22.27300072, 36.45550156,	16",
"4.010000229, 22.27759933, 36.45510101,	25",
"6.059999943, 22.27409935, 36.45439911,	29",
"8.029999733, 22.27199936, 36.45420074,	41",
"10.01000023, 22.27449989, 36.45420074, 46",
"12.01000023, 22.27389908, 36.45420074, 42",
"14.01000023, 22.25659943, 36.4530983, 46",
"16.22999954, 22.22039986, 36.45199966, 74",
"17.90999985, 22.17420006, 36.4510994, 113",
"19.97999954, 22.10420036, 36.4510994, 133",
"21.88999939, 21.76530075, 36.46419907, 75",
"23.92000008, 21.21500015, 36.50379944, 62",
"25.96999931, 21.1019001, 36.52349854, 42",
"27.97999954, 21.01409912, 36.54330063, 38",
"30, 20.96380043, 36.5481987, 35",
"32, 20.76429939, 36.51480103, 31",
"33.97999954, 20.59320068, 36.49599838,	30",
"35.97000122, 20.52519989, 36.50450134,	28",
"38, 20.43899918, 36.50859833, 27",
"40, 20.36030006, 36.50970078, 25",
"42, 20.31710052, 36.51100159, 26",
"43.99000168, 20.28269959, 36.51240158,	24",
"45.95000076, 20.23590088, 36.51399994,	25",
"47.97000122, 20.18880081, 36.50960159,	24",
"50.00999832, 20.09309959, 36.52059937,	25",
"52.02999878, 20.05970001, 36.52000046,	25",
"54.02000046, 20.00880051, 36.51910019,	24",
"56.00999832, 19.94930077, 36.52709961,	25",
"58, 19.91740036, 36.5257988, 27",
"59.97999954, 19.86650085, 36.52679825,	30",
"61.97999954, 19.83139992, 36.52769852,	30",
"63.99000168, 19.7961998, 36.53139877, 30",
"65.98999786, 19.78980064, 36.53829956, 30",
"68, 19.75169945, 36.54700089, 29",
"69.98999786, 19.66839981, 36.54370117,	27",
"71.97000122, 19.64539909, 36.54570007,	28",
"73.97000122, 19.63809967, 36.54959869,	27",
"75.97000122, 19.63010025, 36.55469894,	26",
"77.97000122, 19.61630058, 36.56060028,	26",
"80, 19.56990051, 36.55390167, 26",
"82.01000214, 19.51910019, 36.54629898,	24",
"83.98000336, 19.4932003, 36.54230118, 25",
"86.01000214, 19.48349953, 36.54380035,	24",
"87.98999786, 19.46030045, 36.54249954,	23",
"89.98999786, 19.40180016, 36.53490067,	24",
"92, 19.39819908, 36.53720093, 23",
"93.98999786, 19.39290047, 36.5379982, 23",
"95.97000122, 19.38809967, 36.53829956,	23",
"97.98999786, 19.33720016, 36.52569962,	24",
"100.0199966, 19.25919914, 36.50709915,	26",
"101.9899979, 19.22890091, 36.50049973,	26",
"104, 19.19849968, 36.49710083,	26",
"105.9899979, 19.15970039, 36.49300003,	25",
"107.9800034, 19.12849998, 36.4919014, 25",
"110.0100021, 19.11940002, 36.49340057,	26",
"112.0299988, 19.10320091, 36.49349976,	25",
"113.9899979, 19.08359909, 36.49250031,	25",
"115.9800034, 19.05999947, 36.49209976,	25",
"117.9899979, 19.05360031, 36.49219894,	26",
"119.9899979, 19.03310013, 36.4905014, 25",
"121.9700012, 19.02149963, 36.49209976,	26",
"124.0100021, 19.02840042, 36.50099945,	28",
"126.0199966, 19.05130005, 36.51580048,	32",
"128.0099945, 19.05559921, 36.51850128,	33",
"129.9799957, 19.05780029, 36.51990128,	32",
"132, 19.03479958, 36.5163002, 33",
"134.0099945, 18.99780083, 36.51079941,	31",
"136.0099945, 18.99230003, 36.51240158,	31",
"137.9900055, 19.00300026, 36.52019882,	30",
"139.9900055, 19.00429916, 36.52360153,	30",
"142.0099945, 18.99609947, 36.52460098,	29",
"143.9900055, 18.99900055, 36.52899933,	28",
"146.0099945, 18.98629951, 36.52709961,	31",
"147.9900055, 18.9803009, 36.52859879,	34",
"149.9900055, 18.97660065, 36.53110123,	33",
"152, 18.97019959, 36.53229904,	31",
"153.9900055, 18.95829964, 36.53540039,	30",
"156.0200043, 18.94869995, 36.53649902,	30",
"158.0099945, 18.92740059, 36.53379822,	28",
"160.0099945, 18.91309929, 36.53279877,	29",
"162, 18.90929985, 36.53250122,	27",
"163.9900055, 18.8927002, 36.53110123, 27",
"165.9700012, 18.85779953, 36.52840042,	26",
"167.9799957, 18.83510017, 36.52769852,	26",
"170.0200043, 18.84160042, 36.53530121,	26",
"172.0399933, 18.84250069, 36.5381012,	25",
"174, 18.83989906, 36.54240036,	24",
"176, 18.80859947, 36.54050064,	25",
"178.0299988, 18.78580093, 36.53789902,	24",
"180, 18.77479935, 36.53760147,	23",
"181.9900055, 18.75469971, 36.53739929,	24",
"183.9900055, 18.74679947, 36.53969955,	23",
"185.9600067, 18.75550079, 36.54529953,	23",
"188, 18.7378006, 36.54679871,	24",
"190, 18.70490074, 36.54359818,	22",
"191.9700012, 18.68029976, 36.5401001, 23",
"193.9799957, 18.65740013, 36.53559875,	23",
"196.0099945, 18.62310028, 36.52959824,	23",
"198.0099945, 18.5951004, 36.52539825,	22",
"200, 18.58390045, 36.52360153,	23",
"202, 18.57360077, 36.52159882,	22",
"203.9700012, 18.5583992, 36.51950073, 22",
"205.9700012, 18.54129982, 36.51869965,	22",
"207.9700012, 18.52109909, 36.52000046,	22",
"209.9799957, 18.50939941, 36.52180099,	22",
"211.9700012, 18.5102005, 36.52330017, 22",
"213.9799957, 18.51110077, 36.52470016,	22",
"215.9799957, 18.50219917, 36.52510071,	22",
"217.9799957, 18.46999931, 36.52330017,	22",
"220.0099945, 18.46459961, 36.52700043,	23",
"222.0399933, 18.45709991, 36.52859879,	22",
"224, 18.43330002, 36.52629852,	22",
"225.9900055, 18.41010094, 36.52410126,	23",
"228.0299988, 18.39900017, 36.5245018, 23",
"230.0299988, 18.38339996, 36.52610016, 22",
"231.9900055, 18.3757, 36.52740097, 22",
"234, 18.36179924, 36.5284996,	23",
"236, 18.35160065, 36.52980042,	22",
"237.9900055, 18.35050011, 36.53010178,	23",
"240.0099945, 18.33950043, 36.53099823,	23",
"242.0299988, 18.33379936, 36.53079987,	23",
"243.9900055, 18.33250046, 36.53070068,	22",
"245.9700012, 18.3295002, 36.5304985, 23",
"247.9799957, 18.30610085, 36.53039932,	23",
"249.9900055, 18.27840042, 36.5306015, 23",
"252.0200043, 18.26440048, 36.5306015, 24",
"254.0299988, 18.2609005, 36.53039932, 23",
"256.0299988, 18.2553997, 36.5298996, 24",
"258, 18.24300003, 36.52930069,	23",
"259.9700012, 18.22809982, 36.52830124,	24",
"261.9700012, 18.21030045, 36.52669907,	24",
"263.9899902, 18.16390038, 36.5237999,	24",
"265.980011, 18.13100052, 36.52170181,	24",
"267.9599915, 18.10549927, 36.52090073,	24",
"269.9599915, 18.07929993, 36.51869965,	24",
"271.9599915, 18.05120087, 36.51660156,	24",
"274, 18.02659988, 36.51319885,	25",
"276.0400085, 18.0170002, 36.51850128,	24",
"278.019989, 18.02210045, 36.52159882,	24",
"280.0100098, 18.02020073, 36.52180099,	24",
"282.0100098, 18.01580048, 36.52140045,	25",
"284, 18.01040077, 36.52149963,	25",
"286, 18.00849915, 36.52180099,	25",
"287.9899902, 18.00219917, 36.51990128,	25",
"290, 17.99620056, 36.51919937,	26",
"292.0100098, 17.97859955, 36.51660156,	25",
"294.0299988, 17.94939995, 36.51110077,	28",
"295.9899902, 17.93639946, 36.50899887,	29",
"297.9899902, 17.92880058, 36.50740051,	30",
"300.019989, 17.89979935, 36.50230026, 29",
"302.019989, 17.84770012, 36.49509811, 30",
"304.0100098, 17.83839989, 36.4939003, 30",
"305.9899902, 17.82550049, 36.49250031,	30",
"308, 17.79829979, 36.48910141,	30",
"310.0100098, 17.78109932, 36.48669815,	29",
"312.0100098, 17.77199936, 36.48619843,	29",
"314.0100098, 17.75790024, 36.48460007,	28",
"316.0100098, 17.74760056, 36.48279953,	30",
"318.0100098, 17.73450089, 36.48070145,	33",
"319.9899902, 17.71310043, 36.4776001, 32",
"321.9899902, 17.6916008, 36.47409821, 32",
"324.0100098, 17.67480087, 36.47060013,	31",
"326.019989, 17.64839935, 36.46580124, 30",
"328.0100098, 17.6291008, 36.46279907, 29",
"330.0100098, 17.61289978, 36.46020126,	29",
"332.019989, 17.60169983, 36.45849991, 28",
"334.0100098, 17.58410072, 36.45529938,	27",
"336, 17.56909943, 36.45289993,	27",
"338.0100098, 17.56240082, 36.45140076,	26",
"340.0100098, 17.53120041, 36.44660187,	26",
"342, 17.51059914, 36.44290161,	25",
"344, 17.49410057, 36.4396019,	25",
"346.0100098, 17.46430016, 36.43399811,	24",
"347.9899902, 17.41740036, 36.42580032,	24",
"349.9899902, 17.37109947, 36.41849899,	24",
"352, 17.35400009, 36.41559982,	24",
"353.9899902, 17.3458004, 36.41419983, 23",
"356.0100098, 17.34079933, 36.41339874,	26",
"358.0100098, 17.34070015,36.41320038, 27",
"359.980011, 17.3390007, 36.41270065, 27",
"362.0100098, 17.33300018, 36.41159821,	28",
"364.019989, 17.32970047, 36.41090012, 27",
"366.0100098, 17.32150078, 36.40940094,	27",
"368.0100098, 17.29990005, 36.40570068,	27",
"369.980011, 17.2772007, 36.40230179, 26",
"371.980011, 17.25810051, 36.39929962, 27",
"373.9899902, 17.24399948, 36.39680099,	27",
"376.019989, 17.22809982, 36.39379883, 27",
"378.0400085, 17.20770073, 36.39009857,	26",
"380.0100098, 17.19280052, 36.38679886,	25",
"381.9899902, 17.15710068, 36.38000107,	26",
"383.9899902, 17.10610008, 36.3708992, 25",
"386.0100098, 17.0692997, 36.36289978, 26",
"388, 17.04949951, 36.35879898,	24",
"389.9899902, 17.02260017, 36.35419846,	25",
"392.0100098, 16.98069954, 36.34579849,	24",
"393.980011, 16.9673996, 36.3429985, 24",
"395.9899902, 16.94750023, 36.33969879,	25",
"398.0100098, 16.94490051, 36.33890152,	24",
"400, 16.93000031, 36.3362999,	24",
"402, 16.91139984, 36.33340073,	24",
"404.019989, 16.88059998, 36.32820129, 24",
"406, 16.87010002, 36.32559967,	23",
"407.9899902, 16.83909988, 36.31959915,	23",
"409.9899902, 16.8076992, 36.31380081, 23",
"412.0100098, 16.78429985, 36.30989838,	23",
"414.019989, 16.75300026, 36.30419922, 23",
"416.019989, 16.7404995, 36.30160141, 23",
"417.9899902, 16.71940041, 36.29759979,	22",
"419.9899902, 16.68689919, 36.29119873,	23",
"421.9899902, 16.61660004, 36.27880096,	22",
"423.980011, 16.56889915, 36.270401, 23",
"425.9899902, 16.54739952, 36.2663002, 23",
"428, 16.52869987, 36.26259995,	23",
"430.0100098, 16.50259972, 36.25780106,	23",
"432.0100098, 16.45249939, 36.24959946,	23",
"434.0100098, 16.42449951, 36.24430084,	23",
"435.9899902, 16.40439987, 36.24000168,	23",
"438.0100098, 16.37829971, 36.23550034,	24",
"440.0100098, 16.35490036, 36.23160172,	23",
"442.0100098, 16.32979965, 36.22700119,	24",
"444.0299988, 16.30220032, 36.22180176,	23",
"446.0100098, 16.27479935, 36.21680069,	23",
"448, 16.24850082, 36.2120018, 23",
"449.980011, 16.23159981, 36.20959854, 23",
"452, 16.22739983, 36.20880127,	24",
"454.019989, 16.21859932, 36.20729828, 23",
"455.980011, 16.20940018, 36.20539856, 23",
"457.980011, 16.20000076, 36.20299912, 24",
"460, 16.18989944, 36.20050049,	24",
"461.9899902, 16.15399933, 36.19409943,	23",
"463.980011, 16.12809944, 36.18899918, 24",
"466.0100098, 16.08230019, 36.18159866,	24",
"468.0100098, 16.05979919, 36.17750168,	23",
"470, 16.04789925, 36.17520142,	24",
"472.019989, 16.0284996, 36.17129898, 24",
"474, 15.98690033, 36.16389847,	23",
"475.9899902, 15.93150043, 36.15499878,	24",
"478.019989, 15.91899967, 36.15169907, 24",
"480.019989, 15.85239983, 36.1405983, 23",
"482.0100098, 15.78670025, 36.1291008, 24",
"484.0100098, 15.73770046, 36.12099838,	23",
"486, 15.70230007, 36.11460114,	24",
"488, 15.6864996, 36.11119843, 23",
"490, 15.64309978, 36.10390091,	24",
"492.0100098, 15.59780025, 36.09659958,	23",
"494, 15.55090046, 36.08860016,	23",
"495.9899902, 15.48530006, 36.07780075,	24",
"497.9899902, 15.45540047, 36.07189941,	27",
"500.0100098, 15.43169975, 36.06760025,	27",
"502.019989, 15.40639973, 36.0632019, 27",
"504.0100098, 15.38510036, 36.05920029,	27",
"506.0100098, 15.34010029, 36.05210114,	27",
"508.0299988, 15.30020046, 36.04510117,	27",
"510.0100098, 15.27169991, 36.03939819,	26",
"511.980011, 15.21290016, 36.03020096, 26",
"513.9699707, 15.16839981, 36.02330017,	26",
"515.9899902, 15.15509987, 36.020401, 27",
"518.0300293, 15.1171999, 36.01350021, 26",
"520, 15.05770016, 36.0041008, 25",
"522.0100098, 15.01990032, 35.99819946,	26",
"524.0200195, 15.00249958, 35.99480057,	25",
"526.0200195, 14.98029995, 35.99110031,	25",
"528, 14.97189999, 35.98899841,	24",
"529.9799805, 14.93360043, 35.98249817,	24",
"531.9699707, 14.88010025, 35.97399902,	24",
"533.9699707, 14.8281002, 35.9654007, 24",
"535.9899902, 14.7663002, 35.95550156, 24",
"537.9899902, 14.72889996, 35.94929886,	24",
"539.9899902, 14.67619991, 35.94070053,	24",
"541.9699707, 14.64690018, 35.9353981, 24",
"543.960022, 14.62370014, 35.93090057, 24",
"545.9699707, 14.5770998, 35.92309952, 24",
"548, 14.50020027, 35.9109993, 24",
"550.0100098, 14.44400024, 35.89970016,	23",
"551.9799805, 14.39449978, 35.88959885,	23",
"553.9899902, 14.36730003, 35.88430023,	24",
"556, 14.29930019, 35.87469864, 23",
"558, 14.27369976, 35.8708992, 24",
"560.0100098, 14.24279976, 35.86660004,	23",
"561.9799805, 14.20300007, 35.85960007,	23",
"563.9699707, 14.1583004, 35.85250092, 24",
"566.0100098, 14.08310032, 35.8404007, 24"
"568, 14.05770016, 35.83560181, 23",
"570, 14.0255003, 35.82979965, 24",
"572, 13.99009991, 35.82310104, 23",
"574.0100098, 13.95740032, 35.81689835, 24",
"576.0200195, 13.88329983, 35.80680084,	23",
"577.9899902, 13.8451004, 35.80179977, 23",
"579.9899902, 13.80679989, 35.79639816,	24",
"582.0100098, 13.78689957, 35.7928009, 24",
"584.0200195, 13.76130009, 35.78789902,	26",
"586.0100098, 13.73760033, 35.78379822,	28",
"588.0100098, 13.70849991, 35.78010178,	27",
"589.9899902, 13.68999958, 35.77750015,	27",
"591.9799805, 13.66639996, 35.77410126,	27",
"593.9699707, 13.62010002, 35.76729965,	27",
"595.9899902, 13.53079987, 35.75450134,	27",
"598.0100098, 13.49339962, 35.7480011, 27",
"599.9799805, 13.47039986, 35.74309921,	26",
"602, 13.40390015, 35.73289871, 27",
"604, 13.30980015, 35.7195015, 26",
"605.9799805, 13.2507, 35.71099854, 27",
"608.0200195, 13.22140026, 35.70640183,	29",
"610.0100098, 13.17150021, 35.69900131,	32",
"611.9899902, 13.10400009, 35.6894989, 31",
"613.9799805, 13.05389977, 35.68230057,	31",
"616, 13.02040005, 35.67720032,	31",
"618.0100098, 12.98309994, 35.67150116,	30",
"619.9899902, 12.9612999, 35.66749954, 29",
"622, 12.9307003, 35.66180038,	30",
"624.0100098, 12.87160015, 35.65140152,	29",
"626, 12.76099968, 35.63539886,	27",
"627.9899902, 12.72599983, 35.62879944,	27",
"630.0100098, 12.65550041, 35.61819839,	26",
"631.9899902, 12.58339977, 35.60689926,	25",
"633.9899902, 12.51589966, 35.59749985,	25",
"636, 12.43519974, 35.58750153,	24",
"638, 12.40460014, 35.58319855,	23",
"640.0200195, 12.36450005, 35.5780983, 23",
"642, 12.33290005, 35.57389832,	21",
"643.9799805, 12.32330036, 35.57239914,	22",
"645.9899902, 12.30049992, 35.56940079,	21",
"647.9899902, 12.27140045, 35.56560135,	21",
"649.9899902, 12.25619984, 35.56309891,	20",
"651.960022, 12.23700047, 35.56029892, 20",
"653.9899902, 12.21249962, 35.5564003, 21",
"656.0100098, 12.18029976, 35.55250168,	20",
"657.9799805, 12.16479969, 35.54930115,	20",
"659.9699707, 12.12819958, 35.54240036,	20",
"661.9899902, 11.99670029, 35.52629852,	20",
"663.9899902, 11.91860008, 35.51689911,	19",
"665.960022, 11.90089989, 35.51449966, 19",
"667.960022, 11.89019966, 35.51319885, 19",
"669.9699707, 11.88490009, 35.51190186,	19",
"671.9699707, 11.87339973, 35.50979996,	19",
"674, 11.83870029, 35.50479889,	20",
"676.0300293, 11.7826004, 35.49720001, 19",
"677.9899902, 11.7258997, 35.48960114, 19",
"679.9799805, 11.65439987, 35.48070145,	20",
"682.0200195, 11.60639954, 35.47430038,	20",
"684.0100098, 11.54660034, 35.46640015,	19",
"686, 11.5163002, 35.46229935, 20",
"687.9899902, 11.50930023, 35.4612999, 19",
"689.960022, 11.50599957, 35.46070099, 20",
"691.9699707, 11.50339985, 35.45999908,	20",
"693.9799805, 11.4939003, 35.45809937, 20",
"695.9699707, 11.44190025, 35.45159912,	20",
"697.9799805, 11.40719986, 35.44710159,	21",
"699.9899902, 11.40190029, 35.44599915,	20",
"701.9799805, 11.39159966, 35.44380188,	21",
"703.9799805, 11.37240028, 35.44100189,	21",
"705.9799805, 11.34780025, 35.43650055,	22",
"708.0100098, 11.26669979, 35.42570114,	22",
"710.0300293, 11.12390041, 35.41080093,	22",
"712.0200195, 11.08839989, 35.40729904,	22",
"713.9899902, 11.06719971, 35.40439987,	23",
"716.0100098, 11.02429962, 35.39870071,	24",
"718.0200195, 10.97210026, 35.39239883,	24",
"720.0200195, 10.90939999, 35.38499832,	25",
"722.039978, 10.8135004, 35.37609863, 25",
"724.0100098, 10.75469971, 35.36980057,	24",
"725.9799805, 10.72200012, 35.36840057,	25",
"728.0100098, 10.68150043, 35.36370087,	27",
"730.039978, 10.61970043, 35.35680008, 30",
"732, 10.56869984, 35.35319901,	30",
"733.9699707, 10.48330021, 35.34669876,	31",
"735.9899902, 10.43519974, 35.34019852,	31",
"738.0100098, 10.35350037, 35.3260994, 31",
"740, 10.25399971, 35.31090164,	29",
"741.9799805, 10.15250015, 35.29990005,	29",
"744.0100098, 10.12040043, 35.29899979,	29",
"746.0200195, 10.13259983, 35.30599976,	28",
"748.0200195, 10.13710022, 35.30960083,	28",
"750.0200195, 10.09679985, 35.30559921,	28",
"752.0300293, 10.04179955, 35.29869843,	31",
"754, 10.00109959, 35.29359818, 34",
"756.0100098, 9.971599579, 35.28969955,	33",
"758.0100098, 9.934000015, 35.28419876,	32",
"760.0100098, 9.860199928, 35.27569962,	32",
"762.0200195, 9.795499802, 35.26879883,	30",
"763.9899902, 9.772100449, 35.26689911,	30",
"765.9899902, 9.752900124, 35.2663002, 30",
"767.9899902, 9.73029995, 35.26350021, 28",
"769.9799805, 9.68500042, 35.2582016, 28",
"771.9899902, 9.649600029, 35.2533989, 27",
"773.9899902, 9.567500114, 35.24229813,	27",
"776, 9.484499931, 35.23099899,	25",
"777.9699707, 9.43789959, 35.22510147, 25",
"779.9799805, 9.418399811, 35.22180176,	26",
"782, 9.401399612, 35.21920013,	25",
"783.9799805, 9.371999741, 35.21569824,	25",
"785.9699707, 9.342000008, 35.21210098,	25",
"787.9899902, 9.318900108, 35.20890045,	25",
"790.0100098, 9.252400398, 35.20240021,	24",
"792, 9.167900085, 35.19459915,	23",
"794, 9.077699661, 35.18730164,	24",
"796.0200195, 9.038200378, 35.1833992, 23",
"798.0200195, 9.006899834, 35.18099976,	23",
"800.0300293, 8.989100456, 35.18000031,	23",
"802, 8.963999748, 35.17760086, 22",
"803.9799805, 8.91380024, 35.17710114, 23",
"805.9799805, 8.888600349, 35.17670059,	22",
"808, 8.876600266, 35.17639923,	23",
"810, 8.864700317, 35.17599869,	22",
"812.0100098, 8.858099937, 35.17559814,	23",
"814.0200195, 8.85130024, 35.17599869, 22",
"816, 8.85490036, 35.17720032, 23",
"818.0200195, 8.8604002, 35.1792984, 23",
"820.0200195, 8.855999947, 35.18040085,	23",
"822.0100098, 8.852600098, 35.18220139,	23",
"823.9899902, 8.831199646, 35.1875, 23",
"825.9799805, 8.817899704, 35.19509888,	23",
"828, 8.789099693, 35.19480133,	24",
"830, 8.742899895, 35.19120026,	23",
"831.9899902, 8.713299751, 35.18849945,	24",
"833.9899902, 8.685400009, 35.18529892,	24",
"836.0300293, 8.616000175, 35.17890167,	27",
"838.0100098, 8.576000214, 35.17570114,	28",
"840, 8.527999878, 35.17269897,	30",
"842, 8.490400314, 35.16989899,	30",
"843.9899902, 8.448699951, 35.16550064,	31",
"846, 8.34319973, 35.15779877, 30",
"848, 8.282699585, 35.15330124,	30",
"850.0100098, 8.231499672, 35.15039825,	29",
"852.0100098, 8.181500435, 35.14680099,	29",
"854.0100098, 8.163499832, 35.14490128,	28",
"855.9799805, 8.153599739, 35.14289856,	27",
"858, 8.08520031, 35.1371994, 28",
"860.0200195, 7.965899944, 35.13040161,	30",
"862.0100098, 7.932199955, 35.1291008, 32",
"864.0200195, 7.908199787, 35.12829971,	30",
"865.9899902, 7.87319994, 35.12639999, 29",
"867.9799805, 7.842199802, 35.12490082,	29",
"869.9699707, 7.817399979, 35.12379837,	28",
"871.9799805, 7.802800179, 35.12260056,	28",
"874, 7.780200005, 35.12060165,	27",
"875.9799805, 7.72480011, 35.11849976, 26",
"877.9799805, 7.635900021, 35.11619949,	26",
"879.9799805, 7.579800129, 35.11600113,	25",
"881.9699707, 7.572100163, 35.11539841,	25",
"884, 7.558899879, 35.11510086,	26",
"886, 7.524400234, 35.11380005,	24",
"887.9899902, 7.478000164, 35.11299896,	25",
"890.0300293, 7.447599888, 35.11349869,	25",
"892.039978, 7.432499886, 35.11190033, 26",
"894, 7.412899971, 35.10979843,	29",
"896.0100098, 7.393400192, 35.10789871,	28",
"898.0200195, 7.37130022, 35.1053009, 28",
"900.0200195, 7.310599804, 35.10079956,	27",
"901.9899902, 7.244299889, 35.09830093,	27",
"903.9899902, 7.178800106, 35.09749985,	27",
"906.0100098, 7.158800125, 35.09730148,	27",
"908.0200195, 7.150899887, 35.09740067,	26",
"910, 7.138700008, 35.09769821,	26",
"911.9799805, 7.118100166, 35.09759903,	26",
"913.9799805, 7.102600098, 35.09700012,	26",
"915.9799805, 7.078800201, 35.0965004, 26",
"917.9899902, 7.058899879, 35.09550095,	26",
"919.9799805, 7.027400017, 35.09529877,	25",
"922, 6.998099804, 35.09389877,	26",
"924.0100098, 6.972099781, 35.09170151,	24",
"926, 6.938300133, 35.09090042,	25",
"927.9899902, 6.909599781, 35.09080124,	24",
"929.9799805, 6.875699997, 35.0890007, 25",
"931.9899902, 6.840899944, 35.08620071,	25",
"934, 6.83589983, 35.08530045, 25",
"936, 6.824100018, 35.08390045,	24",
"938.0200195, 6.799099922, 35.08169937,	25",
"940.0200195, 6.773499966, 35.08010101,	24",
"942.0300293, 6.743999958, 35.07989883,	26",
"944.0100098, 6.713399887, 35.08300018,	28",
"946, 6.707099915, 35.08359909,	29",
"948.0100098, 6.703999996, 35.08399963,	28",
"950.0200195, 6.704199791, 35.08549881,	28",
"952.0200195, 6.702700138, 35.0862999,	28",
"954, 6.68200016, 35.08599854, 27",
"956, 6.638599873, 35.08599854,	28",
"957.9899902, 6.621399879, 35.0862999, 26",
"959.9799805, 6.601500034, 35.08670044,	27",
"962, 6.58589983, 35.08679962, 26",
"964, 6.564799786, 35.0870018, 26",
"965.9899902, 6.553899765, 35.08670044,	26",
"968.0100098, 6.533699989, 35.08589935,	30",
"970.0100098, 6.504199982, 35.08330154,	31",
"972, 6.467599869, 35.08050156,	29",
"974, 6.416399956, 35.07550049,	30",
"976.0200195, 6.371500015, 35.0707016, 29",
"978.0100098, 6.360599995, 35.06990051,	28",
"979.9799805, 6.354599953, 35.07089996,	28",
"981.9899902, 6.333300114, 35.07099915,	28",
"984, 6.324399948, 35.07049942,	27",
"986, 6.322800159, 35.07020187,	27",
"987.9899902, 6.317299843, 35.06959915,	26",
"989.9799805, 6.312799931, 35.0705986,	26",
"992, 6.297999859, 35.07040024,	26",
"994.0100098, 6.260000229, 35.06650162,	25",
"996.0100098, 6.231200218, 35.06299973,	25",
"997.9899902, 6.18569994, 35.06019974, 24",
"999.9899902, 6.165699959, 35.05929947,	25",
"1002.02002, 6.162199974, 35.05899811, 24",
"1004.02002, 6.154500008, 35.0583992, 24",
"1006.039978, 6.140799999, 35.05780029,	24",
"1008.01001, 6.126699924, 35.05709839, 23",
"1010.01001, 6.090600014, 35.05690002, 24",
"1012.030029, 6.078299999, 35.05690002,	23",
"1014.02002, 6.06400013, 35.05690002, 23",
"1016.01001, 6.054399967, 35.05670166, 23",
"1018.030029, 6.031599998, 35.05879974,	23",
"1020.01001, 6.00880003, 35.05830002, 22",
"1021.98999, 5.984000206, 35.05830002, 23",
"1024.01001, 5.974999905, 35.05810165, 23",
"1026.02002, 5.972000122, 35.05789948, 23",
"1028.01001, 5.962999821, 35.05759811, 22",
"1029.98999, 5.956200123, 35.05699921, 23",
"1032.02002, 5.944900036, 35.05659866, 23",
"1034.01001, 5.938000202, 35.05649948, 22",
"1035.97998, 5.933899879, 35.05659866, 23",
"1038, 5.928599834, 35.05699921, 23",
"1040, 5.922599792, 35.05770111, 23",
"1042.01001, 5.917699814, 35.05830002, 23",
"1043.98999, 5.91410017, 35.05889893, 23",
"1046, 5.91079998, 35.0591011, 23",
"1048.01001, 5.897999763, 35.05979919, 22",
"1049.97998, 5.869699955, 35.05879974, 22",
"1051.959961, 5.847099781, 35.05749893,	22",
"1053.959961, 5.838399887, 35.05749893,	22",
"1056	5.833399773, 35.05820084, 23",
"1058.02002, 5.82889986, 35.05879974, 22",
"1060, 5.818900108, 35.05789948, 22",
"1061.97998, 5.793200016, 35.05630112, 22",
"1063.97998, 5.779399872, 35.05609894, 22",
"1065.969971, 5.773799896, 35.0564003, 22",
"1067.97998, 5.772200108, 35.0564003, 22",
"1069.959961, 5.771399975, 35.05649948,	22",
"1071.98999, 5.772399902, 35.05690002, 23",
"1074.030029, 5.756400108, 35.05730057,	22",
"1076, 5.733699799, 35.05590057, 21",
"1077.959961, 5.723899841, 35.05519867,	22",
"1079.97998, 5.716100216, 35.05459976,	22",
"1082, 5.70690012, 35.05369949,	22",
"1083.97998, 5.693699837, 35.05509949, 21",
"1085.959961, 5.678100109, 35.05569839,	22",
"1088, 5.669499874, 35.05630112, 22",
"1090.02002, 5.665599823, 35.0564003, 22",
"1092.030029, 5.660099983, 35.05670166,	22",
"1094.030029, 5.651400089, 35.05789948,	22",
"1096.030029, 5.644299984, 35.05799866,	22",
"1098.02002, 5.633500099, 35.05770111, 22",
"1100.030029, 5.608799934, 35.05820084,	22",
"1102.02002	5.595799923	35.05820084	22
"1103.98999	5.594399929	35.05820084	22
"1105.97998	5.593299866	35.05820084	23
"1107.97998	5.592700005	35.05820084	22
"1110.01001	5.59100008	35.05799866	23
"1112.040039	5.586599827	35.05770111	24
"1114.02002	5.577600002	35.05709839	26
"1116.01001	5.56400013	35.05680084	26
"1118	5.54489994	35.05789948	26
"1119.97998	5.535099983	35.05780029	26
"1121.98999	5.521399975	35.05699921	27
"1124	5.490699768	35.05490112	26
"1125.98999	5.47179985	35.05369949	26
"1128	5.463500023	35.05289841	26
"1130.02002	5.450500011	35.05160141	26
"1132	5.442399979	35.05049896	25
"1134	5.431399822	35.04919815	26
"1136	5.420599937	35.04779816	25
"1138.01001	5.412199974	35.04669952	26
"1139.98999	5.399400234	35.04539871	24
"1142	5.394800186	35.0447998	26
"1144.040039	5.387899876	35.04399872	25
"1146.030029	5.377699852	35.04249954	25
"1148.01001	5.346099854	35.0387001	24
"1150	5.302599907	35.03450012	25
"1151.98999	5.268000126	35.03139877	24
"1153.97998	5.262000084	35.0306015	25
"1156	5.243800163	35.02880096	24
"1158.01001	5.22300005	35.02690125	24
"1160.01001	5.203000069	35.02529907	24
"1161.98999	5.183599949	35.0245018	23
"1163.98999	5.176700115	35.02429962	24
"1166.02002	5.171800137	35.02410126	23
"1168.01001	5.170400143	35.02420044	23
"1170	5.170000076	35.02410126	23
"1171.97998	5.169099808	35.02410126	23
"1174.01001	5.167399883	35.02429962	24
"1176.040039	5.165800095	35.0243988	23
"1178.030029	5.161600113	35.0237999	23
"1179.98999	5.148900032	35.02249908	22
"1181.959961	5.128499985	35.02030182	23
"1183.97998	5.117000103	35.01929855	23
"1186.01001	5.10340023	35.0182991	23
"1188.030029	5.097400188	35.01800156	23
"1190.01001	5.092100143	35.01750183	23
"1191.969971	5.079599857	35.01689911	26
"1194	5.074699879	35.01639938	27
"1196	5.061900139	35.01570129	25
"1197.97998	5.040999889	35.01509857	26
"1200	5.021999836	35.01459885	26
"1201.97998	5.006800175	35.01350021	25
"1203.97998	4.999800205	35.01300049	26
"1205.97998	4.996600151	35.01269913	25
"1207.97998	4.993199825	35.01240158	26
"1210.01001	4.990799904	35.0121994	26
"1212.01001	4.988900185	35.01200104	25
"1214.02002	4.984300137	35.01139832	26
"1216	4.978899956	35.01100159	25
"1217.97998	4.96750021	35.01070023	26
"1219.97998	4.963200092	35.01089859	25
"1221.97998	4.939499855	35.01050186	25
"1223.98999	4.914800167	35.00910187	25
"1225.98999	4.908199787	35.00859833	25
"1227.98999	4.90500021	35.0080986	25
"1230.02002	4.897999763	35.00780106	25
"1232.030029	4.895500183	35.00770187	24
"1233.98999	4.892700195	35.00740051	24
"1236	4.890900135	35.00730133	25
"1238.030029	4.889999866	35.00709915	25
"1240.02002	4.885700226	35.00680161	24
"1242.01001	4.880000114	35.00659943	25
"1244.040039	4.876599789	35.00630188	24
"1246	4.875100136	35.00600052	23
"1247.969971	4.873000145	35.00600052	24
"1250.01001	4.866300106	35.00550079	25
"1252.030029	4.856299877	35.00500107	27
"1254.02002	4.841899872	35.00519943	28
"1256.01001	4.836500168	35.00500107	27
"1258	4.835800171	35.00519943	27
"1260.030029	4.835299969	35.00540161	27
"1262.040039	4.831799984	35.00579834	26
"1264.040039	4.828000069	35.00559998	26
"1266.040039	4.824500084	35.00540161	26
"1268.030029	4.816500187	35.00450134	26
"1270	4.810500145	35.00360107	26
"1271.969971	4.810299873	35.0033989	26
"1273.959961	4.804800034	35.00270081	26
"1276.01001	4.794700146	35.00220108	28
"1278.01001	4.790599823	35.00189972	30
"1279.97998	4.783599854	35.00149918	31
"1282	4.777299881	35.00099945	31
"1284	4.763100147	35.00030136	29
"1285.969971	4.75150013	35	29
"1287.969971	4.750800133	35	29
"1289.97998	4.751200199	34.99990082	28
"1291.97998	4.75	34.99990082	28
"1293.969971	4.745999813	35.00019836	27
"1295.97998	4.753799915	35.00230026	27
"1298.02002	4.770400047	35.00619888	27
"1300.030029	4.773200035	35.00790024	26
"1302.02002	4.783199787	35.01060104	26
"1304.02002	4.810900211	35.01670074	26
"1306	4.83370018	35.02190018	25
"1307.98999	4.852300167	35.0257988	25
"1310	4.856800079	35.02719879	25
"1312	4.859099865	35.0279007	24
"1313.98999	4.861400127	35.0284996	24
"1316	4.864699841	35.02939987	25
"1318.030029	4.866600037	35.03020096	28
"1320.030029	4.864900112	35.0304985	29
"1322.030029	4.863999844	35.03030014	28
"1324.030029	4.861499786	35.03010178	28
"1326	4.858799934	35.02970123	27
"1327.97998	4.854899883	35.02909851	27
"1329.97998	4.839300156	35.02750015	27
"1332.01001	4.830299854	35.02640152	27
"1334.030029	4.828999996	35.02569962	26
"1336.030029	4.816400051	35.02339935	26
"1338	4.782299995	35.01850128	25
"1340	4.750100136	35.01409912	26
"1342.030029	4.73570013	35.01169968	25
"1343.98999	4.718900204	35.00889969	24
"1345.97998	4.701200008	35.00579834	25
"1348.01001	4.666600227	35.00130081	25
"1350.02002	4.643899918	34.99840164	24
"1352	4.635300159	34.99710083	24
"1354	4.608300209	34.99430084	24
"1356	4.598899841	34.99349976	24
"1357.97998	4.597899914	34.99340057	23
"1359.97998	4.597300053	34.99340057	24
"1362	4.595200062	34.99330139	23
"1363.98999	4.593500137	34.99319839	23
"1366	4.591899872	34.99330139	23
"1368.01001	4.591700077	34.99319839	24
"1370.02002	4.591800213	34.99290085	27
"1371.98999	4.583499908	34.99290085	27
"1373.97998	4.577700138	34.99290085	27
"1375.98999	4.573500156	34.99280167	27
"1377.97998	4.570199966	34.99280167	26
"1380	4.56829977	34.99269867	27
"1382	4.565899849	34.99269867	26
"1384.01001	4.56430006	34.99269867	27
"1385.98999	4.562900066	34.99259949	25
"1387.98999	4.557300091	34.99250031	27
"1390.02002	4.552299976	34.99259949	26
"1392.02002	4.549600124	34.99250031	26
"1394.02002	4.549499989	34.99259949	25
"1395.97998	4.549600124	34.99250031	25
"1397.959961	4.548799992	34.99250031	25
"1399.97998	4.543000221	34.99290085	25
"1402	4.539700031	34.99290085	25
"1404.02002	4.539299965	34.99300003	25
"1406	4.538799763	34.99300003	24
"1408	4.538000107	34.99290085	25
"1410	4.537199974	34.99290085	24
"1412	4.534999847	34.99280167	25
"1413.98999	4.532999992	34.99280167	24
"1415.97998	4.532999992	34.99280167	25
"1418	4.533100128	34.99269867	25
"1420.01001	4.530900002	34.99269867	25
"1421.98999	4.527699947	34.99259949	24
"1423.97998	4.52670002	34.99250031	25
"1425.98999	4.525000095	34.99240112	24
"1427.969971	4.523499966	34.99219894	24
"1430	4.517199993	34.99169922	25
"1432.030029	4.513500214	34.99150085	24
"1434.02002	4.508999825	34.99219894	24
"1436	4.508500099	34.99269867	24
"1438	4.507400036	34.99280167	24
"1440	4.500999928	34.99269867	24
"1442	4.499499798	34.99280167	24
"1444.02002	4.499800205	34.99280167	24
"1446	4.500199795	34.99290085	23
"1447.97998	4.499899864	34.99309921	24
"1450.01001	4.499499798	34.99319839	24
"1452.02002	4.498899937	34.99359894	23
"1454.02002	4.501200199	34.99420166	24
"1456.02002	4.511099815	34.99639893	23
"1457.97998	4.514900208	34.99720001	23
"1459.97998	4.518199921	34.9980011	24
"1462	4.522900105	34.99940109	23
"1463.97998	4.532400131	35.00180054	23
"1465.98999	4.537099838	35.00289917	24
"1468	4.535299778	35.00270081	23
"1470.01001	4.534699917	35.00260162	24
"1472.01001	4.52670002	35.00189972	23
"1473.97998	4.521500111	35.00120163	23
"1475.969971	4.5145998	35.00059891	23
"1477.959961	4.498600006	34.99900055	23
"1480.01001	4.492400169	34.99819946	24
"1482	4.490699768	34.99769974	22
"1483.959961	4.484300137	34.99679947	23
"1485.969971	4.475200176	34.99530029	23
"1487.98999	4.456999779	34.99300003	23
"1489.98999	4.434700012	34.99079895	23
"1492.01001	4.425700188	34.98970032	23
"1494.040039	4.422599792	34.98910141	23
"1496.02002	4.419199944	34.98860168	22
"1498.01001	4.410399914	34.98759842	23
"1500	4.403399944	34.98680115	22
"1502	4.39620018	34.98609924	23
"1504	4.394100189	34.98590088	22
"1505.97998	4.39139986	34.9858017	22
"1508	4.388299942	34.98590088	23
"1510.02002	4.381299973	34.98609924	22
"1511.97998	4.381899834	34.98720169	22
"1514	4.3822999	34.98759842	23
"1516	4.381299973	34.98749924	22
"1517.97998	4.380000114	34.98740005	22
"1519.98999	4.376999855	34.98720169	23
"1522.030029	4.374899864	34.98709869	23
"1524.01001	4.373799801	34.98709869	22
"1525.98999	4.373099804	34.98699951	23
"1527.98999	4.366799831	34.9858017	23
"1530	4.349500179	34.9836998	23
"1532	4.333199978	34.98189926	23
"1534.01001	4.321499825	34.98070145	23
"1536.02002	4.314499855	34.98009872	23
"1538.01001	4.311500072	34.97980118	23
"1540	4.308100224	34.97949982	23
"1542.01001	4.301599979	34.97919846	23
"1544.01001	4.29820013	34.97909927	22
"1546.01001	4.297699928	34.97900009	24
"1548.030029	4.297399998	34.97900009	25
"1549.98999	4.295800209	34.97900009	26
"1552	4.294000149	34.97890091	27
"554.02002	4.291800022	34.97869873	26
"1556.030029	4.287399769	34.97880173	26
"1558	4.28429985	34.97869873	25
"1559.97998	4.281000137	34.97880173	26
"1562.01001	4.277699947	34.97859955	26
"1564.02002	4.271800041	34.97840118	25
"1565.98999	4.265299797	34.97809982	25
"1567.97998	4.261400223	34.97800064	25
"1570	4.258100033	34.97790146	25
"1572	4.252799988	34.9776001	24
"1574.02002	4.250299931	34.97740173	27
"1575.97998	4.247700214	34.97729874	28
"1577.97998	4.243599892	34.97710037	29
"1580	4.241399765	34.97689819	28
"1582.02002	4.239900112	34.97669983	28
"1584.030029	4.235199928	34.97650146	27
"1585.98999	4.229599953	34.97629929	26
"1587.959961	4.222799778	34.97650146	27
"1589.97998	4.220799923	34.97650146	27
"1592	4.220399857	34.97650146	26
"1593.97998	4.220399857	34.97650146	26
"1595.969971	4.219399929	34.97650146	26
"1597.97998	4.218699932	34.97639847	26
"1600	4.214000225	34.97610092	26
"1602	4.20359993	34.97579956	25
"1604.01001	4.198699951	34.97539902	26
"1606.02002	4.196800232	34.97539902	25
"1608.01001	4.195499897	34.97529984	25
"1610.01001	4.193999767	34.97499847	25
"1611.98999	4.193500042	34.97510147	24
"1613.97998	4.192900181	34.97499847	25
"1615.98999	4.191699982	34.97480011	24
"1617.97998	4.188000202	34.97460175	25
"1620.01001	4.178999901	34.97430038	25
"1622.02002	4.175600052	34.97409821	24
"1624.02002	4.172500134	34.97399902	26
"1625.98999	4.171500206	34.97399902	29
"1627.97998	4.171599865	34.97409821	29
"1629.98999	4.17140007	34.97430038	29
"1632	4.16960001	34.97449875	28
"1633.98999	4.167500019	34.97439957	28
"1635.98999	4.166500092	34.97439957	28
"1638	4.165800095	34.97430038	27
"1639.98999	4.162899971	34.97409821	27
"1642.01001	4.155600071	34.97380066	27
"1644.02002	4.153299809	34.97370148	26
"1646.01001	4.151899815	34.97340012	26
"1648.01001	4.14839983	34.97330093	26
"1650	4.145800114	34.97299957	25
"1651.969971	4.142700195	34.97280121	25
"1653.959961	4.137499809	34.97259903	25
"1655.949951	4.133600235	34.97240067	25
"1657.97998	4.127399921	34.97230148	26
"1660.01001	4.126599789	34.97219849	25
"1662.02002	4.125699997	34.97219849	25
"1664.02002	4.125599861	34.97200012	25
"1666.02002	4.118999958	34.97190094	27
"1667.97998	4.116700172	34.97190094	29
"1670	4.115300179	34.97190094	29
"1671.97998	4.114500046	34.97180176	27
"1673.969971	4.113500118	34.97159958	28
"1675.98999	4.105500221	34.97159958	27
"1677.98999	4.10340023	34.9715004	27
"1680.01001	4.102200031	34.9715004	27
"1682	4.101799965	34.9715004	26
"1684	4.099699974	34.9715004	27
"1686	4.099299908	34.97140121	25
"1687.949951	4.098999977	34.97140121	26
"1689.98999	4.098800182	34.97129822	26
"1691.98999	4.096600056	34.97119904	25
"1693.97998	4.094500065	34.97109985	25
"1695.97998	4.09250021	34.97100067	25
"1697.98999	4.090799809	34.97090149	25
"1700.01001	4.088699818	34.97079849	25
"1702.01001	4.086800098	34.97060013	24
"1704.01001	4.084700108	34.97050095	25
"1706.02002	4.083300114	34.97040176	24
"1708	4.07889986	34.97019959	24
"1709.97998	4.075500011	34.97000122	24
"1711.97998	4.072000027	34.97000122	24
"1713.97998	4.066999912	34.96960068	24
"1715.97998	4.064700127	34.9695015	24
"1717.98999	4.063300133	34.9693985	24
"1720.02002	4.062099934	34.96929932	24
"1722.01001	4.060900211	34.96910095	23
"1724.01001	4.058300018	34.96920013	25
"1726.02002	4.0552001	34.96900177	26
"1727.98999	4.05369997	34.96900177	29
"1730	4.052400112	34.96889877	27
"1732	4.050799847	34.96879959	28
"1734.030029	4.048399925	34.96860123	27
"1736.01001	4.045800209	34.96860123	26
"1737.98999	4.04460001	34.96849823	27
"1740	4.042600155	34.96849823	27
"1742.02002	4.041800022	34.96839905	27
"1744.01001	4.039899826	34.96829987	26
"1746.01001	4.036499977	34.96820068	27
"1748	4.033599854	34.9681015	25
"1749.97998	4.033299923	34.9679985	26
"1751.98999	4.031899929	34.96789932	25
"1753.969971	4.028600216	34.96770096	25
"1756	4.025700092	34.96760178	26
"1758.050049	4.025599957	34.96760178	25
"1760.030029	4.025800228	34.96749878	24
"1761.98999	4.022900105	34.96749878	24
"1763.98999	4.02159977	34.96749878	25
"1765.98999	4.019800186	34.96749878	24
"1767.98999	4.017499924	34.9673996	25
"1769.98999	4.016600132	34.96749878	24
"1771.959961	4.015500069	34.9673996	24
"1773.949951	4.013199806	34.96730042	24
"1775.969971	4.011700153	34.96730042	24
"1777.98999	4.011099815	34.96720123	24
"1780	4.008900166	34.96720123	23
"1781.98999	4.007999897	34.96720123	24
"1784.02002	4.007900238	34.96720123	24
"1786.030029	4.007400036	34.96709824	23
"1788.01001	4.00579977	34.96709824	23
"1790.01001	4.004899979	34.96699905	23
"1792.01001	4.000599861	34.96689987	23
"1794	3.996000052	34.96680069	23
"1796	3.993900061	34.96670151	23
"1798.02002	3.992899895	34.96670151	23
"1800.030029	3.991100073	34.96649933	23
"1802	3.985399961	34.96630096	22
"1803.98999	3.980499983	34.96609879	23
"1806.02002	3.978600025	34.96609879	23
"1808.01001	3.977699995	34.9659996	22
"1809.98999	3.976500034	34.9659996	23
"1812.030029	3.976099968	34.9659996	23
"1814.030029	3.976200104	34.9659996	22
"1815.97998	3.975600004	34.96590042	22
"1817.97998	3.971100092	34.96590042	23
"1820	3.967099905	34.96590042	22
"1821.969971	3.966099977	34.96590042	22
"1824	3.965699911	34.96590042	23
"1826.030029	3.965300083	34.96580124	22
"1828.01001	3.96510005	34.96569824	22
"1829.98999	3.961899996	34.96569824	22
"1831.97998	3.961299896	34.96559906	22
"1833.959961	3.961299896	34.96569824	22
"1835.98999	3.961499929	34.96559906	23
"1838.01001	3.960400105	34.9654007	22
"1839.97998	3.954900026	34.96530151	22
"1841.98999	3.950099945	34.96509933	23
"1844.02002	3.949500084	34.96500015	23
"1846.02002	3.948499918	34.96490097	23
"1848.01001	3.946799994	34.96480179	23
"1850	3.941600084	34.96459961	22
"1851.969971	3.939100027	34.96459961	22
"1853.959961	3.935800076	34.96440125	22
"1855.98999	3.935600042	34.96440125	23
"1858.01001	3.936000109	34.96429825	22
"1859.98999	3.935600042	34.96429825	22
"1861.98999	3.932300091	34.96409988	23
"1864.01001	3.930299997	34.96409988	23
"1866	3.929500103	34.9640007	22
"1868	3.925600052	34.96390152	24
"1870.02002	3.921000004	34.96369934	26
"1872.01001	3.918600082	34.96360016	27
"1874	3.918100119	34.96360016	26
"1875.98999	3.914499998	34.96340179	27
"1877.98999	3.913599968	34.96319962	26
"1879.969971	3.911600113	34.96300125	26
"1881.98999	3.904799938	34.96279907	27
"1884.01001	3.90109992	34.96269989	26
"1886.01001	3.898400068	34.96260071	26
"1888.01001	3.897700071	34.96260071	26
"1890	3.896100044	34.96250153	26
"1892.01001	3.894399881	34.96250153	26
"1894.02002	3.893899918	34.96250153	25
"1895.97998	3.894200087	34.96250153	24
"1897.97998	3.892999887	34.96250153	25
"1900	3.89260006	34.96239853	25
"1902.030029	3.891799927	34.96229935	25
"1904.030029	3.889400005	34.96239853	25
"1906	3.886100054	34.96220016	24
"1907.97998	3.881500006	34.96210098	25
"1910	3.876199961	34.96179962	25
"1912.02002	3.870100021	34.96179962	25
"1914.02002	3.866899967	34.96149826	25
"1915.98999	3.859400034	34.96120071	24
"1917.97998	3.851200104	34.96089935	25
"1920	3.848799944	34.96080017	25
"1921.97998	3.84739995	34.96070099	24
"1923.98999	3.84190011	34.96030045	26
"1926.02002	3.830499887	34.95999908	25
"1928.01001	3.824700117	34.95980072	24
"1929.98999	3.823600054	34.95980072	25
"1931.97998	3.823400021	34.95980072	25
"1933.97998	3.82310009	34.95959854	25
"1936	3.821199894	34.95959854	25
"1938	3.819400072	34.95940018	25
"1939.97998	3.818500042	34.95940018	25
"1941.97998	3.814800024	34.95940018	26
"1944.02002	3.812799931	34.95909882	26
"1945.98999	3.805599928	34.95890045	25
"1947.97998	3.788300037	34.95869827	25
"1949.97998	3.786200047	34.95859909	25
"1952	3.784600019	34.95859909	25
"1954.01001	3.783400059	34.95849991	24
"1956	3.781699896	34.95840073	24
"1957.98999	3.778199911	34.95830154	24
"1960.01001	3.77670002	34.95819855	24
"1962	3.776499987	34.95809937	23
"1964.01001	3.776200056	34.95800018	24
"1966.01001	3.774499893	34.95800018	22
"1967.97998	3.770800114	34.95780182	23
"1970	3.768699884	34.95759964	23
"1971.97998	3.767199993	34.95750046	22
"1973.969971	3.766000032	34.95750046	23
"1976	3.765399933	34.95740128	23
"1978	3.761399984	34.9571991	22
"1979.969971	3.760799885	34.9571991	22
"1981.98999	3.759399891	34.95709991	23
"1984.01001	3.756999969	34.95690155	22
"1985.98999	3.753499985	34.95669937	22
"1987.97998	3.747900009	34.95660019	22
"1990.02002	3.746400118	34.95650101	23
"1992.040039	3.745300055	34.95640183	25
"1994.030029	3.744999886	34.95640183	29
"1995.98999	3.741400003	34.95619965	34
"1998	3.735100031	34.95589828	35
"2000.02002	3.73210001	34.95589828	36
"2002.050049	3.727699995	34.95569992	77



