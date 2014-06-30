/*************************************************************************/
/*                             finished_code.ino                         */
/*                             *****************                         */
/*                                                                       */
/* Written by: Sean P. Murphy                                            */
/*                                                                       */
/*************************************************************************/




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
/* cpMode: an int that represents whether the simulator is in continuous */
/*                  profiling, a negative value means no (default)       */
/*                                                                       */
/* count: an int that represents the number of samples taken while in    */
/*                  continuous profile mode                              */
/*                                                                       */
/* maxPress, minPress: float values that represent that max and min      */
/*                  pressure calculated during continuous profiling      */
/*                                                                       */
/* nBins, samplesLeft: int values that represent the total number of     */
/*                  bins and the number of samples left after            */
/*                  subtracting the samples used for one bin             */
/*                                                                       */
/* da: an int that represents whether a bin average has been taken, if a */
/*                  binaverage hasn't been calculated, it's -1 (default) */
/*                                                                       */
/* inc: an int used to increment the pressure value for the calculated   */
/*                  data of continuous profiling, is incremented in the  */
/*                  binaverage function then reset after dumping the     */
/*                  data from the profile                                */
/*                                                                       */
/* last, first: int values used to indicate whether the given sample is  */
/*                  the first or last sample of the profile. both are    */
/*                  -1 by default, and will be set to 1 once per profile */
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
int count;

float maxPress = 0;
float minPress = 10000;
int nBins;
int samplesLeft;
int da = -1;
int inc = 0;
int last = -1;
int first = -1;

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

String binaverage();

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
  
  /*************************************************************************/
  /*                             command mode                              */
  /*************************************************************************/
  
  //command mode is turned on by the ISR checkLine, it will disable external interrupts
  //on pin 2 at the beginning and will handle the data being transmitted and received
  //over the serial ports. To be in continuous profiling mode, the simulator needs to be
  //in command mode, so the loop for continuous profiling is also handled here
  if(commandMode == 1){
    
    //ignore the interrupts on pin 2 once at the beginning
    detachInterrupt(0);
    
    /*************************************************************************/
    /*                      continuous profiling mode                        */
    /*************************************************************************/
    
    //enter the while loop to stay in command mode
    while(commandMode == 1){
      
      //contiuous profiling mode is turned on by the startprofile command over serial
      while(cpMode == 1){
        
        //only take sample once every 1 sec (delay .95 sec)
        delay(950);
        
        //clear out any junk value on pin A0
        analogRead(A0);
        
        //create an array of bytes (a PTS reading) based on the value of the pin A0
        //then send it over Serial1
        byte cpStrBuffer[100];
        String cpStr = getReadingFromPiston(2);
        int cpStrLen = cpStr.length()+1;
        cpStr.getBytes(cpStrBuffer, cpStrLen);
        Serial1.write(cpStrBuffer, cpStrLen);
        
        //record that you have taken 1 sample
        count+=1;
        
        //leave continuous profiling mode if the pressure is less than 2 dbar
        if(minPress<=2){
          cpMode = -1;
        }
        
        //leave continuous profiling mode if the stopprofile command is received
        if(Serial1.available()>0){
          String input = "";
          while(Serial1.available()>0){  
            char temp;
            temp = char(Serial1.read());
            input+=temp;
            if(input.equals("stopprofile")){
              cpMode = -1;
            }
          }
        }
      }
      
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
        
        //if the input is startprofile, recognize that it is the start profile command,
        //then send back that the profile has started, reattach interrupt to pin2, and 
        //turn on continuous profiling mode
        else if(input.equals("startprofile")){
          String cp = "\n\rstartprofile\n\rprofile started, pump delay = 0 seconds\n\rS>";
          int cpLen = cp.length()+1;
          byte cpBuffer[100];
          cp.getBytes(cpBuffer, cpLen);
          Serial1.write(cpBuffer, cpLen);
          attachInterrupt(0, checkLine, RISING);
          cpMode = 1;
        }
        
        //if the input is stopprofile, recognize that it is the stop profile command,
        //then send back that the profile has stopped, ignore the external interrupt
        //on pin2, and turn off continuous profiling mode
        else if(input.equals("stopprofile")){
          String exitcp = "\n\rS>stopprofile";
          int exitcpLen = exitcp.length()+1;
          byte exitcpBuffer[100];
          exitcp.getBytes(exitcpBuffer, exitcpLen);
          Serial1.write(exitcpBuffer, exitcpLen);
          detachInterrupt(0);
          cpMode = -1;
        }
        
        //if the input is binaverage, return the values parsed from the data sent
        //during continuous profiling mode. set da to 1 which will allow for the
        //da command to be run (make sure there is actual data to dump when requested)
        else if(input.equals("binaverage\r")){
          nBins = (int(maxPress)/2) + 1;
          String binavg = "\n\rS>binaverage\n\rsamples = "+String(count)+", maxPress = "+floatToString(maxPress)+"\n\rrd: 0\n\ravg: 0\n\n\rdone, nbins = "+String(nBins)+"\n\rS>";
          int binavgLen = binavg.length()+1;
          byte binavgBuffer[100];
          binavg.getBytes(binavgBuffer, binavgLen);
          Serial1.write(binavgBuffer, binavgLen);
          da = 1;
        }

        //if the inpt is da, send bins in the format "p, t, s, b" (pressure,
        //temperature, salinity, number of samples) over Serial1. then send that 
        //the upload is done. then reinitialize all of the global variables used 
        //for bin averaging then dumping the values        
        else if((input.equals("da\r"))&&(da==1)){
          first = 1;
          int ii;
          for(ii=0; ii < nBins; ii++){
            if(ii == nBins - 1){
              last = 1;
            }
            String bin = binaverage();
            int binLen = bin.length()+1;
            byte binBuffer[100];
            bin.getBytes(binBuffer, binLen);
            Serial1.write(binBuffer, binLen);
          }
          String complete = "\n\rupload complete\n\rS>";
          int completeLen = complete.length()+1;
          byte completeBuffer[100];
          complete.getBytes(completeBuffer, completeLen);
          Serial1.write(completeBuffer, completeLen);
          maxPress = 0;
          minPress = 10000;
          nBins = 0;
          da = -1;
          inc = 0;
          count = 0;
        }
          
        
        //if the input is qsr, send back that the seabird is powering down as a series of bytes 
        //(the simulator will just stay on and wait for the next interaction with the APFx)
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
  //just pressure and temperature, then the message to be sent
  String pStr;
  String tStr;
  String sStr;
  String ptsStr;
  String ptStr;
  String sendMessage;
  
  //read an analog value on pin 1, use it for the calculations 1023=2.56V
  int voltage = analogRead(A0);
  
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
  if(cpMode == 1){
    if(pressure >= maxPress){
      maxPress = pressure;
    }
    if(pressure <= minPress){
      minPress = pressure;
    }
  }
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
  
  
  //choose which string you want to return
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
 
  //handle case for losing the 0 in a number less than 10 (i.e. get 09 instead of 9)
  // or losing two 0's in a number less than 100 (i.e. get 009 instead of 9)
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

/*************************************************************************/
/*                              binaverage                               */
/*                              **********                               */
/*                                                                       */
/* parameters: none                                                      */
/* returns: String representing the bin in the format of avg P, avg T,   */
/*              avg S, then the number of bins                           */
/*                                                                       */
/* This function runs a timer for the given interval of time, uses code  */
/* from TimerOne.h                                                       */
/*                                                                       */
/*************************************************************************/

String binaverage(){
  
  //string containing the p,t,s,b values
  String returnStr;
  
  //float values
  float pressure = 0;
  float temperature;
  float salinity;
  
  //int value
  int samplesUsed = 0;
  
  //create a pressure value that increases by approximately 2
  pressure += (inc*2);
  pressure += float((float(random(100,500))/float(1500)));
  pressure -= float(float((random(200,600))/float(1600)));
  
  //calculate temperature and salinty values the same way as usual
  temperature = 20-(((pressure)*(15.00))/2000.00);
  salinity = (((pressure)*(4.00))/2000) + 33.5;
  
  //if the loop is in its first iteration, the total number of samples
  //is the number of samples originally taken (count)
  if(first == 1){
    samplesLeft = count;
    first = -1;
  }
  
  //calculate a random value for the number of samples per bin
  samplesUsed = ((samplesLeft%5) + random(0,30));
  
  //ensure that there are not too many samples used
  if(samplesLeft-samplesUsed <= 0){
    samplesUsed = 0;
  }
  
  //use the remaining number of samples for the last bin
  if(last==1){
    samplesUsed = samplesLeft;
    last = -1;
  }
  
  //if the incremented value of pressure is lower than the lowest measured value
  // then set all of the values equal to zero. or if there are no samples, set all
  // of the values for that bin equal to zero
  if((pressure <= minPress)||(samplesUsed==0)){
    pressure = 0;
    temperature = 0;
    salinity = 0;
    samplesUsed = 0;
  }
  
  //increment the pressure by 2 (1 pressure increment = inc *2)
  inc += 1;
  
  //decrease the number of samples remaining
  samplesLeft -= samplesUsed;

  //create the string to be returned in the format:
  //"pppp.pppp, tt.tttt, ss.ssss, bb"
  returnStr = floatToString(pressure)+", "+floatToString(temperature)+", "+floatToString(salinity)+", "+String(samplesUsed)+"\n\r";
  
  //return the string
  return returnStr;
  }
