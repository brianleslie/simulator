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
/* msg, msg2, msg3, msg4: Strings to be sent over serial to the APFx     */
/*                                                                       */
/* msgLen, msg2Len, msg3Len, msg4Len: ints used to specify length of the */
/*                  respective messages, needed when sending as bytes    */
/*                                                                       */
/* cmdMode, pts, pt, p: arrays of bytes sent over serial, the arrays are */
/*                  created by converting their corresponding Strings to */
/*                  bytes using a built-in function                      */
/*                                                                       */
/*************************************************************************/

int interruptMessage = 0;

String msg = "\rSBE 41CP UW. V 2.0\n\rS>";
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
/*                              checklines                               */
/*                              **********                               */
/*                                                                       */
/* Attached to a rising logic level on pin 2                             */
/* paramaters: none                                                      */
/* returns: none                                                         */
/*                                                                       */
/* After the initial change, wait 200ms, then check the request line. If */
/* still high, then check that the other two lines (Rx and mode) are     */
/* also high. If they are, send the APF9 the serial number. If the       */
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
  analogRead(A0);
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
  if(digitalRead(2)==HIGH){
    interruptMessage = 1;
    
  }
  else if(digitalRead(2)==LOW){
    if(digitalRead(3)==HIGH){//get pts
      interruptMessage = 2;
    }
    else if((debounce(19)>0)&&(debounce(3)<0)){//get pt
      interruptMessage = 3;
    }
    else if((debounce(19)<0)&&(debounce(3)<0)){//get p
      interruptMessage = 4;
    }
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
/* It configures pins 2, 3, and A0 as inputs. It sets the reference      */
/* voltage for analog input at 2.56V. It attaches an interrupt to pin 2  */
/* that will run the function checkLines if it is triggered by a rising  */
/* edge. It also initializes the timer with a period of 1 second.        */
/*                                                                       */
/*************************************************************************/

void setup(){
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(8, OUTPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(A0, INPUT);
  analogReference(INTERNAL2V56);
  attachInterrupt(0, checkLine, RISING);
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
/* Strings as arrays of bytes.                                           */
/*                                                                       */
/*************************************************************************/


void loop(){
  digitalWrite(8, HIGH);
  switch(interruptMessage){
    case 0:
      break;
    case 1:
      msg.getBytes(cmdMode, msgLen);
      Serial1.write(cmdMode, msgLen);
      interruptMessage = 0;
      break;
    case 2:
      msg2 = getPTSfromPiston();
      msg2Len = msg2.length()+1;
      msg2.getBytes(pts, msg2Len);
      Serial1.write(pts, msg2Len);
      interruptMessage = 0;
      break;
    case 3:
      msg3 = getPTfromPiston();
      msg3Len = msg3.length()+1;
      msg3.getBytes(pt, msg3Len);
      Serial1.write(pt, msg3Len);
      interruptMessage = 0;
      break;
    case 4:
      msg4 = getPfromPiston();
      msg4Len = msg4.length()+1;
      msg4.getBytes(p, msg4Len);
      Serial1.write(p, msg4Len);
      interruptMessage = 0;
      break;
  }
  if(Serial1.available()>0){
    String input = "";
    while(Serial1.available()>0){
      digitalWrite(11, HIGH);
      char temp;
      temp = char(Serial1.read());
      input+=temp;
      delay(5);
      digitalWrite(11, LOW);
    }
    if(input.equals("\r")){
      String cmdMode = "S>\n\r";
      int cmdModeLen = cmdMode.length()+1;
      byte cmdModeBuffer[100];
      cmdMode.getBytes(cmdModeBuffer, cmdModeLen);
      Serial1.write(cmdModeBuffer, cmdModeLen);
    }
     else if((input.equals("ds\r"))||(input.equals("ds"))){
      String sn = "SBE 41CP UW V 2.0  SERIAL NO. 4242\n\rfirmware compilation date: 18 December 2007 09:20\n\rstop profile when pressure is less than = 2.0 decibars\n\rautomatic bin averaging at end of profile disabled\n\rnumber of samples = 0\n\rnumber of bins = 0\n\rtop bin interval = 2\n\rtop bin size = 2\n\rtop bin max = 10\n\rmiddle bin interval = 2\n\rmiddle bin size = 2\n\rmiddle bin max = 20\n\rbottom bin interval = 2\n\rbottom bin size = 2\n\rdo not include two transition bins\n\rinclude samples per bin\n\rpumped take sample wait time = 20 sec\n\rreal-time output is PTS\n\rS>";
      int snLen = sn.length()+1;
      byte snBuffer[1000];
      sn.getBytes(snBuffer, snLen);
      Serial1.write(snBuffer, snLen);
     }
     else if((input.equals("qsr\r"))||(input.equals("qsr"))){
      String cmdMode = "\n\rpowering down";
      int cmdModeLen = cmdMode.length()+1;
      byte cmdModeBuffer[100];
      cmdMode.getBytes(cmdModeBuffer, cmdModeLen);
      Serial1.write(cmdModeBuffer, cmdModeLen);
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
/* This function checks the logic level of a pin 6 times (once a second) */
/* and determines if it is high or low                                   */
/*                                                                       */
/*************************************************************************/

int debounce(int pin){
  int highOrLow = 0;
  int highOrLowTotal = 0;
  int i = 0;
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
      if (time > (timeLast + 99900)){
        Timer1.stop();
        break;
      }
    }
    highOrLowTotal+=highOrLow;
  }
  if(highOrLowTotal >=2){
    return 1;
  }
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
/* String that represents P,T,S sample. This is achieved by manipulating */
/* the input value and fitting it to generic, general values tested by   */
/* Hugh Fargher. In general, we used 3 linear models to represent 3      */
/* ranges of depth (2000m-1000m, 1000m-500m, 500m-0m) with different     */
/* slopes and offsets. From these pressure values, we assume another     */
/* linear relationship to temperature (as pressure increases linerarly,  */
/* temperature decreases linearly). Lastly, we can assume one last       */
/* linear relationship between pressure and salinity (as pressure        */
/* increases linearly, salinity increases linearly). The values of these */
/* Strings are appended to one another and formatted to match a regex    */
/* pattern expected by the APF board on the float.                       */
/*                                                                       */
/*************************************************************************/

String getPTSfromPiston(){
  
  float pressure;
  float temperature;
  float salinity;
  
  long pressureLong;
  long temperatureLong;
  long salinityLong;
  
  int pressureInt;
  int temperatureInt;
  int salinityInt;
  
  int pressureDec;
  int temperatureDec;
  int salinityDec;
  
  String pStr;
  String tStr;
  String sStr;
  String ptsStr;
  
  int voltage = analogRead(A0);
  
  if(voltage<72){
    pressure = 2000+5*(voltage-72);
  }
  else if((voltage>=72)&&(voltage<294)){
    pressure = ((4.5045)*(222-(voltage-72)))+1000.00;
  }
  else if((voltage>=294)&&(voltage<454)){
    pressure = ((3.125)*(160-(voltage-294)))+500.00;
  }
  else if((voltage>=454)&&(voltage<1024)){
    pressure = ((0.878)*(569-(voltage-454)));
  }
  
  pressure = pressure * 1.08;
  pressureLong = 100*pressure;
  pressureInt = pressureLong/100;
  pressureDec = pressureLong-pressureInt*100;
  pStr = String(pressureInt)+'.'+String(pressureDec);
  
  
  temperature = 20-(((pressure)*(15.00))/2000.00);
  temperatureLong = 100*temperature;
  temperatureInt = temperatureLong/100;
  temperatureDec = temperatureLong-temperatureInt*100;
  tStr = String(temperatureInt)+'.'+String(temperatureDec);
  
  
  salinity = (((pressure)*(4.00))/2000) + 33.5;
  salinityLong = 10000*salinity;
  salinityInt = salinityLong/10000;
  salinityDec = salinityLong - salinityInt*10000;
  sStr = String(salinityInt)+'.'+String(salinityDec);
  
  
  ptsStr = pStr+", "+tStr+", "+sStr+"\r\n";
  
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
/* String that represents P,T sample. This is achieved by manipulating   */
/* the input value and fitting it to generic, general values tested by   */
/* Hugh Fargher. In general, we used 3 linear models to represent 3      */
/* ranges of depth (2000m-1000m, 1000m-500m, 500m-0m) with different     */
/* slopes and offsets. From these pressure values, we assume another     */
/* linear relationship to temperature (as pressure increases linerarly,  */
/* temperature decreases linearly).  The values of these Strings are     */
/* appended to one another and formatted to match a regex pattern        */
/* expected by the APF board on the float.                               */
/*                                                                       */
/*************************************************************************/


String getPTfromPiston(){
  
  float pressure;
  float temperature;
  
  long pressureLong;
  long temperatureLong;
  
  int pressureInt;
  int temperatureInt;
  
  int pressureDec;
  int temperatureDec;
  
  String pStr;
  String tStr;
  String ptStr;
  
  int voltage = analogRead(A0);
  
  if(voltage<72){
    pressure = 2000+5*(voltage-72);
  }
  else if((voltage>=72)&&(voltage<294)){
    pressure = ((4.5045)*(222-(voltage-72)))+1000.00;
  }
  else if((voltage>=294)&&(voltage<454)){
    pressure = ((3.125)*(160-(voltage-294)))+500.00;
  }
  else if((voltage>=454)&&(voltage<1024)){
    pressure = ((0.878)*(569-(voltage-454)));
  }
  
  pressure = pressure * 1.08;
  pressureLong = 100*pressure;
  pressureInt = pressureLong/100;
  pressureDec = pressureLong-pressureInt*100;
  pStr = String(pressureInt)+'.'+String(pressureDec);
  
  temperature = 20-(((pressure)*(15.00))/2000.00);
  temperatureLong = 100*temperature;
  temperatureInt = temperatureLong/100;
  temperatureDec = temperatureLong-temperatureInt*100;
  tStr = String(temperatureInt)+'.'+String(temperatureDec);
  
  ptStr = pStr+", "+tStr+"\r\n";
  
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
/* String that represents P,T sample. This is achieved by manipulating   */
/* the input value and fitting it to generic, general values tested by   */
/* Hugh Fargher. In general, we used 3 linear models to represent 3      */
/* ranges of depth (2000m-1000m, 1000m-500m, 500m-0m) with different     */
/* slopes and offsets.  The values of these Strings are formatted to     */
/* match a regex pattern expected by the APF board on the float.         */
/*                                                                       */
/*************************************************************************/

String getPfromPiston(){
    
  float pressure;
  long pressureLong;
  int pressureInt;
  int pressureDec;
  String pStr;
  
  int voltage = analogRead(A0);
  
  if(voltage<72){
    pressure = 2000+5*(voltage-72);
  }
  else if((voltage>=72)&&(voltage<294)){
    pressure = ((4.5045)*(222-(voltage-72)))+1000.00;
  }
  else if((voltage>=294)&&(voltage<454)){
    pressure = ((3.125)*(160-(voltage-294)))+500.00;
  }
  else if((voltage>=454)&&(voltage<1024)){
    pressure = ((0.878)*(569-(voltage-454)));
  }
 
 pressure = pressure * 1.08;
  pressureLong = 100*pressure;
  pressureInt = pressureLong/100;
  pressureDec = pressureLong-pressureInt*100;
  pStr = String(pressureInt)+'.'+String(pressureDec)+"\r\n";
  
  return pStr;
}


