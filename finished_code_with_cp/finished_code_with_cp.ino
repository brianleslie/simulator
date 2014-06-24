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

String getPfromPiston();
String getPTfromPiston();
String getPTSfromPiston();
int debounce(int);
void checkLine();
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
      msg2 = getPTSfromPiston();
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
      msg3 = getPTfromPiston();
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
      msg4 = getPfromPiston();
      msg4Len = msg4.length()+1;
      msg4.getBytes(p, msg4Len);
      Serial1.write(p, msg4Len);
      interruptMessage = 0;
      break;
  }
<<<<<<< HEAD
  while(cpMode > 0){
    String msgArray[4500];
    int i = 0;
    for(i = 0; i < 4500; i++){
      delay(700);
      msgArray[i] = getPTSfromPiston();
    }
=======
  
  if(commandMode == 1){
    detachInterrupt(0);
>>>>>>> f6cd823e7f4049695b852a94e261866826883813
<<<<<<< HEAD
  }
  
  while(cpMode == 1){
    long time;
    Timer1.start();
    long timeLast = Timer1.read();
    int i = 0;
    for(i = 0; i < 100000; i++){
      time = Timer1.read();
      if (time > (timeLast + 99900)){
        Timer1.stop();
        break;
      }
    }
    int p = analogRead(A0);
    String ptsStr = getPTSfromPiston();
    
    if(p > 1010){
      break;
    }
    
    
=======
>>>>>>> 152f61a92a183191fd36c3d226e3934d6eedc449
  }
  
  //check for a message in Serial1, it there is, create a blank string, then add each character in the 
  //Serial1 input buffer to the input string. Wait until a carriage return to make sure a command
  //is actually sent, if it is not the carriage return, wait for the next character
  if(Serial1.available()>0){
    String input = "";
    while(Serial1.available()>0){
      char temp;
      temp = char(Serial1.read());
      input+=temp;
      if(temp=='\r'){
        Serial.println(input);
        break;
      }
      else{
        delay(1000);
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
      String dc = "dc\n\rSBE 41CP UW V 2.0  SERIAL NO. 3616\n\rtemperature:  19-dec-10    \n\rTA0 =  4.882851e-05    \n\rTA1 =  2.747638e-04    \n\rTA2 = -2.478284e-06    \n\rTA3 =  1.530870e-07\n\rconductivity:  19-dec-10    \n\rG = -1.013506e+00    \n\rH =  1.473695e-01    \n\rI = -3.584262e-04    \n\rJ =  4.733101e-05    \n\rCPCOR = -9.570001e-08    \n\rCTCOR =  3.250000e-06    \n\rWBOTC =  2.536509e-08\n\rpressure S/N = 3212552, range = 2900 psia:  14-dec-10    \n\rPA0 =  6.297445e-01    \n\rPA1 =  1.403743e-01    \n\rPA2 = -3.996384e-08    \n\rPTCA0 =  6.392568e+01    \n\rPTCA1 =  2.642689e-01    \n\rPTCA2 = -2.513274e-03    \n\rPTCB0 =  2.523900e+01    \n\rPTCB1 = -2.000000e-04    \n\rPTCB2 =  0.000000e+00    \n\rPTHA0 = -7.752968e+01    \n\rPTHA1 =  5.141199e-02    \n\rPTHA2 = -7.570264e-07    \n\rPOFFSET =  0.000000e+00\n\rS>";
      int dcLen = dc.length()+1;
      byte dcBuffer[1000];
      dc.getBytes(dcBuffer, dcLen);
      Serial1.write(dcBuffer, dcLen);
    }
    
    //if the input is startprofileN, recognize that it is the start profile command,
    // then send back that the profile has started
    else if(input.equals("startprofile")){
      String cp = "\n\rstartprofile\n\rprofile started, pump delay = 0 seconds";
      int cpLen = cp.length()+1;
      byte cpBuffer[100];
      cp.getBytes(cpBuffer, cpLen);
      Serial1.write(cpBuffer, cpLen);
    }
    
    //if the input is qsr, send back that the seabird is powering down as a series of bytes 
    //(the simulator will just stay on and wait for the next interaction with the APFx)
    else if(input.equals("qsr\r")){
      String cmdMode = "qsr\n\rpowering down\n\rS>";
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


/*************************************************************************/
/*                                debounce                               */
/*                                ********                               */
/*                                                                       */
/* parameters: pin, an integer value representing the pin that is        */
/*                 connected to the input being toggled                  */
/* returns: an integer value that will be positive (1) if the pin is     */
/*                 high and negative (-1) if the pin is low              */
/*                                                                       */
/* This function checks the logic level of a pin 6 times (once / ~500ms) */
/* and determines if it is high or low                                   */
/*                                                                       */
/*************************************************************************/

int debounce(int pin){
  int highOrLow = 0;
  int highOrLowTotal = 0;
  int i = 0;
  
  //check the given pin, if it is high, add 1 to the total, if it is low add 0 to the total.
  //repeat this 6 times for accuracy, waiting ~500ms between each read of the pin.
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
    int j = 0;
    for(j = 0; j < 100000; j++){
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
  else if(highOrLowTotal < 2){
    return -1;
  }
}

/*************************************************************************/
/*                               getPTSfromPiston                        */
/*                               ****************                        */
/*                                                                       */
/* parameters: none                                                      */
/* returns: String representing the P, T, S values                       */
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
/* pattern expected by the APF board on the float.                       */
/*                                                                       */
/*************************************************************************/

String getPTSfromPiston(){
  
  //original calculated values as floats
  float pressure;
  float temperature;
  float salinity;
  
  //represent the values as longs that are either 100 or 1000 times larger than the floats
  long pressureLong;
  long temperatureLong;
  long salinityLong;
  
  //represent the whole number part of the float values
  int pressureInt;
  int temperatureInt;
  int salinityInt;
  
  //represent the decimal part of the float values
  int pressureDec;
  int temperatureDec;
  int salinityDec;
  
  //the string representation of the pressure, temperature, and salinity, then all 3 together
  String pStr;
  String tStr;
  String sStr;
  String ptsStr;
  
  
  //read an analog value on pin 1, use it for the calculations 1023=2.56V
  int voltage = analogRead(A0);
  
  
  //technically out of range, but use it to go to a pressure greater than 2000dbar, min change = 5dbar
  if(voltage<72){
    pressure = 2000+5*(voltage-72);
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
  
  //adjust for hardware that amplifies the signal by approximately 1.1, then convert the int
  //to two different ints that represent the whole number and the decimal, then add them 
  //together as strings to create one string to look like a float that is the pressure
  pressure = pressure * 1.08;
  pressureLong = 100*pressure;
  pressureInt = pressureLong/100;
  pressureDec = pressureLong-pressureInt*100;
  pStr = String(pressureInt)+'.'+String(pressureDec);
  
  //calculate a int temperature value based on the pressure, assume linearity with the maximum
  //temperature of 20 deg C and minimum of 5 deg C. then convert the int to two different ints 
  //that represent the whole number and the decimal, then add them together as strings to create
  //one string to look like a float that is the temperature
  temperature = 20-(((pressure)*(15.00))/2000.00);
  temperatureLong = 100*temperature;
  temperatureInt = temperatureLong/100;
  temperatureDec = temperatureLong-temperatureInt*100;
  tStr = String(temperatureInt)+'.'+String(temperatureDec);
  
  //calculate a int salinty value based on the pressure, assume linearity with the maximum
  //salinity of 37.5 and minimum of 33.5. then convert the int to two different ints 
  //that represent the whole number and the decimal, then add them together as strings to create
  //one string to look like a float that is the salinty
  salinity = (((pressure)*(4.00))/2000) + 33.5;
  salinityLong = 10000*salinity;
  salinityInt = salinityLong/10000;
  salinityDec = salinityLong - salinityInt*10000;
  sStr = String(salinityInt)+'.'+String(salinityDec);
  
  //add all of the strings to create one string that represents a p,t,s reading
  ptsStr = pStr+", "+tStr+", "+sStr+"\r\n";
  
  //return the pressure, temperature, and salinity string
  return ptsStr;
}

/*************************************************************************/
/*                               getPTfromPiston                         */
/*                               ***************                         */
/*                                                                       */
/* parameters: none                                                      */
/* returns: String representing the P, T values                          */
/*                                                                       */
/* This function converts a reading from the analog input pin A0 to a    */
/* string that represents P,T sample. This is achieved by manipulating   */
/* the input value and fitting it to generic, general values tested by   */
/* Hugh Fargher. In general, we used 3 linear models to represent 3      */
/* ranges of depth (2000m-1000m, 1000m-500m, 500m-0m) with different     */
/* slopes and offsets. From these pressure values, we assume another     */
/* linear relationship to temperature (as pressure increases linerarly,  */
/* temperature decreases linearly).  The values of these strings are     */
/* appended to one another and formatted to match a regex pattern        */
/* expected by the APF board on the float.                               */
/*                                                                       */
/*************************************************************************/


String getPTfromPiston(){
  
  //original calculated values as floats
  float pressure;
  float temperature;  
  
  //represent the values as longs that are either 100 or 1000 times larger than the floats
  long pressureLong;
  long temperatureLong;  
  
  //represent the whole number part of the float values
  int pressureInt;
  int temperatureInt;
  
  //represent the decimal part of the float values
  int pressureDec;
  int temperatureDec;
  
  //the string representation of the pressure and temperature, then both together
  String pStr;
  String tStr;
  String ptStr;
  
  
  //read an analog value on pin 1, use it for the calculations 1023=2.56V
  int voltage = analogRead(A0);
  
  //technically out of range, but use it to go to a pressure greater than 2000dbar, min change = 5dbar
  if(voltage<72){
    pressure = 2000+5*(voltage-72);
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
  
  //adjust for hardware that amplifies the signal by approximately 1.1, then convert the int
  //to two different ints that represent the whole number and the decimal, then add them 
  //together as strings to create one string to look like a float that is the pressure
  pressure = pressure * 1.08;
  pressureLong = 100*pressure;
  pressureInt = pressureLong/100;
  pressureDec = pressureLong-pressureInt*100;
  pStr = String(pressureInt)+'.'+String(pressureDec);
  
  //calculate a int temperature value based on the pressure, assume linearity with the maximum
  //temperature of 20 deg C and minimum of 5 deg C. then convert the int to two different ints 
  //that represent the whole number and the decimal, then add them together as strings to create
  //one string to look like a float that is the temperature
  temperature = 20-(((pressure)*(15.00))/2000.00);
  temperatureLong = 100*temperature;
  temperatureInt = temperatureLong/100;
  temperatureDec = temperatureLong-temperatureInt*100;
  tStr = String(temperatureInt)+'.'+String(temperatureDec);
  
  //add both of the strings to create one string that represents a p,t reading
  ptStr = pStr+", "+tStr+"\r\n";
  
  //return the pressure and temperature string
  return ptStr;
}

/*************************************************************************/
/*                               getPTfromPiston                         */
/*                               ***************                         */
/*                                                                       */
/* parameters: none                                                      */
/* returns: String representing the P value                              */
/*                                                                       */
/* This function converts a reading from the analog input pin A0 to a    */
/* string that represents P,T sample. This is achieved by manipulating   */
/* the input value and fitting it to generic, general values tested by   */
/* Hugh Fargher. In general, we used 3 linear models to represent 3      */
/* ranges of depth (2000m-1000m, 1000m-500m, 500m-0m) with different     */
/* slopes and offsets.  The values of these strings are formatted to     */
/* match a regex pattern expected by the APF board on the float.         */
/*                                                                       */
/*************************************************************************/

String getPfromPiston(){
  
  
  //original calculated value as a float
  float pressure; 
  
  //represent the value as a long that is either 100 or 1000 times larger than the float
  long pressureLong;
  
  //represent the whole number part of the float value
  int pressureInt;
  
  //represent the decimal part of the float value
  int pressureDec;
  
  //the string representation of the pressure 
  String pStr;
  
 //read an analog value on pin 1, use it for the calculations 1023=2.56V
  int voltage = analogRead(A0);
  
  //technically out of range, but use it to go to a pressure greater than 2000dbar, min change = 5dbar
  if(voltage<72){
    pressure = 2000+5*(voltage-72);
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
  
  //adjust for hardware that amplifies the signal by approximately 1.1, then convert the int
  //to two different ints that represent the whole number and the decimal, then add them 
  //together as strings to create one string to look like a float that is the pressure
  pressure = pressure * 1.08;
  pressureLong = 100*pressure;
  pressureInt = pressureLong/100;
  pressureDec = pressureLong-pressureInt*100;
  pStr = String(pressureInt)+'.'+String(pressureDec);
  
  //return the pressure string
  return pStr;
}


