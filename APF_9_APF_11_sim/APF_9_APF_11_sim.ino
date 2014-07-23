/*************************************************************************/
/*                            APF-9_APF-11_sim.ino                       */
/*                            ********************                       */
/*                                                                       */
/* Written by: Sean P. Murphy                                            */
/*                                                                       */
/* Version [1.0] supports simulation of APF-9 and APF-11. Currently      */
/* will support getting P, T (11 only), PT, or PTS readings; querrying   */
/* firmware / serial number (9 only); configuration; continuous profile, */
/* binaverage; ice avoidance; displaying calibration coefficients.       */
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
/* pOrPTS: an array of strings that determine whether the output is just */
/*                  a p reading or a pts reading, necessary to pass test */
/*                  during configuration                                 */
/*                                                                       */
/* pOrPTSsel: an int that represents the array index for the pOrPTS      */
/*                  array, 1 (default) means PTS, 0 means P. this will   */
/*                  be changed based on serial messages received from    */
/*                  APF board                                            */
/*                                                                       */
/* iceAvoidance: an int that represents which ice avoidance protocol is  */
/*                  in effect, -1:none, 1:detect, 2:cap, 3:breakup       */
/*                                                                       */
/* missionMode: an int that represents whether the simulator should be   */
/*                  simulating a mission, 0:no, 1:mission set, but not   */
/*                  started, 108>=: mission in progress(incremnted by 1  */
/*                  each time a parameter is set, by 100 when mission    */
/*                  is started                                           */
/*                                                                       */
/* phase: an int that represents which phase of the mission the is being */
/*                  simulated, 0:descent, 1:park, 2:deep descent,        */
/*                  3:ascent                                             */
/* parkPressure, deepProfilePressure: ints that represent the pressures  */
/*                  expected at different states of the mission          */
/*                                                                       */
/* parkDescentTime, downTime, deepProfileDescentTime, ascentTimeOutOut:  */
/*                  ints that represent the time in seconds it take for  */
/*                  the float to reach different states of the mission   */
/*                                                                       */
/* currentTime: an int representing the current time in the mission, can */
/*                  be set manually in sys chat or will be incremented   */
/*                  by timer automatically every ~5 seconds              */
/*                                                                       */
/*************************************************************************/

int interruptMessage = 0;
int commandMode = -1;
int cpMode = -1;
int count = 0;

float maxPress = 0;
float minPress = 10000;
int nBins = 0;
int samplesLeft;
int da = -1;
int inc = 0;
int last = -1;
int first = -1;

String msg = "SBE 41CP UW. V 2.0\r\nS>";
int msgLen = msg.length()+1;
byte msgBuffer[100];

String msg2;
int msg2Len;
byte pts[150];

String msg3;
int msg3Len;
byte pt[100];

String msg4;
int msg4Len;
byte p[100];

String pOrPTS[2] = {"P only", "PTS"};

int pOrPTSsel = 1;

int iceAvoidance = -1;

int icePressure = 20;

String stopprofile = "stop";

int missionMode = 0;

int phase = 0;

int parkPressure = 1000, deepProfilePressure = 2000;

long lastUpdate = 0, update = 0;

long startOffset = 0, pauseOffset = 0;

long pauseTime = 0, resumeTime = 0;

long currentTime = 0, parkDescentTime = 18000000, downTime = 86400000, deepProfileDescentTime = 18000000, ascentTimeOut = 36000000;

long currentTimeDisplay = 0, parkDescentTimeDisplay = 300, downTimeDisplay = 1440, deepProfileDescentTimeDisplay = 300, ascentTimeOutDisplay = 600;

/*************************************************************************/
/*                            function prototypes                        */
/*                            *******************                        */
/*                                                                       */
/*************************************************************************/

String getReadingFromPiston(int);

String getDynamicReading(int, int);

String pressureToString(float);

String tempOrSalinityToString(float);

String binaverage();

int debounce(int);

void checkLine();

void runTimer(int);

void writeBytes(String);

long updateTime();

void setup();

void loop();


/*************************************************************************/
/*                              checkline                                */
/*                              *********                                */
/*                                                                       */
/* Attached to a rising logic level on pin 2                             */
/*                                                                       */
/* paramaters: none                                                      */
/*                                                                       */
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
/*                                                                       */
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
  millis();
}

/*************************************************************************/
/*                                 loop                                  */
/*                                 ****                                  */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
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
      delay(1300);
      writeBytes(msg);
      interruptMessage = 0;
      break;
      
    //if it is 2, clear any junk analog values on A0 before getting the p,t,s value based on the analog 
    //value on pin A0, set msg2 equal to this value, then convert the global string msg2 to the global 
    //byte array pts, then send the array over Serial1, reset interruptMessage to 0, then leave the loop
    case 2:
      analogRead(A0);
      if(missionMode < 107){
        msg2 = getReadingFromPiston(2);
      }
      else if(missionMode >= 107){
        msg2 = getDynamicReading(2, phase);
      }
      writeBytes(msg2);
      interruptMessage = 0;
      break;
    
    //if it is 3, clear any junk analog values on A0 before getting the p,t value based on the analog
    //value on pin A0, set msg3 eqaul to this value, then convert the global string msg3 to the global 
    //byte array pt, then send the array over Serial1, reset interruptMessage to 0, then leave the loop
    case 3:
      analogRead(A0);
     if(missionMode < 107){
        msg3 = getReadingFromPiston(3);
      }
      else if(missionMode >= 107){
        msg3 = getDynamicReading(3, phase);
      }
      writeBytes(msg3);
      interruptMessage = 0;
      break;

    //if it is 4, clear any junk analog values on A0 before getting the p value based on the analog
    //value on pin A0, set msg4 eqaul to this value, then convert the global string msg4 to the global 
    //byte array p, then send the array over Serial1, reset interruptMessage to 0, then leave the loop
    case 4:
      analogRead(A0);
      if(missionMode < 107){
        msg4 = getReadingFromPiston(4);
      }
      else if(missionMode >= 107){
        msg4 = getDynamicReading(4, phase);
      }
      writeBytes(msg4);
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
    
    //enter the while loop to stay in command mode
    while(commandMode == 1){
      
      /*************************************************************************/
      /*                      continuous profiling mode                        */
      /*************************************************************************/
        
      //contiuous profiling mode is turned on by the startprofile command over serial
      while(cpMode == 1){
        //only take sample once every 1 sec (delay .95 sec)
        delay(950);
        
        //clear out any junk value on pin A0
        analogRead(A0);
        
        //create an array of bytes (a PTS reading) based on the value of the pin A0
        //then send it over Serial1
        byte cpStrBuffer[100];
        String cpStr;
        if(missionMode < 107){
          if(pOrPTSsel == 1){
            cpStr = getReadingFromPiston(2);
          }
          else if(pOrPTSsel == 0){
            cpStr = getReadingFromPiston(4);
          }
        }
        else if(missionMode >= 107){
          if(pOrPTSsel == 1){
            cpStr = getDynamicReading(2, phase);
          }
          else if(pOrPTSsel == 0){
            cpStr =getDynamicReading(4, phase);
          }
        }
        writeBytes(cpStr);
        //record that you have taken 1 sample
        count+=1;
        
        //leave continuous profiling mode if the stopprofile command is received
        if(Serial1.available()>0){
          String input = "";
          while(Serial1.available()>0){  
            char temp;
            temp = char(Serial1.read());
            if(temp=='s'){
              input+=temp;
            }
            if(temp=='t'){
              input+=temp;
            }
            if(temp=='o'){
              input+=temp;
            }
            if(temp=='p'){
              input+=temp;
            }
            if(input.equals(stopprofile)){
              String exitcp = "profile stopped";
              writeBytes(exitcp);
              detachInterrupt(0);
              cpMode = -1;
            }
          }
        }
      }
      
      /*************************************************************************/
      /*                     end continuous profiling mode                     */
      /*************************************************************************/
        
      
      //check for a message in Serial1, if there is, create a blank string, then add each character in the 
      //Serial1 input buffer to the input string. Wait until a carriage return to make sure a command
      //is actually sent, if it is not the carriage return, wait for the next character, two exceptions 
      //are the input strings startprofile and stopprofile (used for continuous profiling mode)
      if(Serial1.available()>0){
        String input = "";
        while(1){
          if(Serial1.available()>0){  
            char temp;
            temp = char(Serial1.read());
            input+=temp;
            if((temp=='\r')||(input.equals("startprofile"))||(input.equals("stopprofile"))||(input.equals("parkDescentTime="))
                           ||(input.equals("parkPressure="))||(input.equals("downTime="))||(input.equals("deepProfileDescentTime="))
                           ||(input.equals("deepProfilePressure="))||(input.equals("ascentTimeOut="))
                           ||(input.equals("currentTime="))||(input.equals("id@"))||(input.equals("ic@"))){
              break;
            }
          }
        }
        
        //if the input is a carriage return, send back the sbe command prompt (S>) as a series of byes
        if(input.equals("\r")){
          String cmdMode = "\r\nS>";
          writeBytes(cmdMode);
        }
        
         else if(input.equals("mission\r")){
          String m_config = "mission\r\nPlease enter values for the following parameters based on your misssion...\r\n"
          "\r\n"
          "\r\nPark Pressure (dbar): parkPressure=int"
          "\r\nPark Descent Time (minutes): parkDescentTime=int"
          "\r\nDown Time(minutes): downTime=int"
          "\r\nDeep Profile Pressure: deepProfilePressure=int"
          "\r\nDeep Profile Descent Time(minutes): deepProfileDescentTime=int"
          "\r\nAscent Time Out(minutes): ascentTimeOut=int"
          "\r\nCurrent Time(seconds) (optional- use to change simulation time): currentTime=int"
          "\r\nS>";
          writeBytes(m_config);
          missionMode += 1;
        }
        
        else if(input.equals("list parameters\r")){
          updateTime();
          String listParams = "list parameters\r\nPark Pressure: " + String(parkPressure) +  
          "\r\nPark Descent Time: " + String(parkDescentTimeDisplay) +
          "\r\nDown Time: " + String(downTimeDisplay) +
          "\r\nDeep Profile Pressure: " + String(deepProfilePressure) +
          "\r\nDeep Profile Descent Time: " + String(deepProfileDescentTimeDisplay) +
          "\r\nAscent Time Out: " + String(ascentTimeOutDisplay) + 
          "\r\ncurrentTime: "+String(currentTimeDisplay)+" ("+String((currentTimeDisplay/60))+" minutes)\r\nS>";
          writeBytes(listParams);
          missionMode += 6;
        }
        
        else if(input.equals("start descent\r")){
          String start = "descent started\r\nS>";
          writeBytes(start);
          startOffset = millis();
          pauseOffset = 0;
          currentTime += startOffset;
          parkDescentTime += startOffset;
          downTime += startOffset;
          deepProfileDescentTime += startOffset;
          ascentTimeOut += startOffset;
          lastUpdate = startOffset;
          missionMode+=100;
        }
        
        else if(input.equals("end mission\r")){
          String m_end = "mission ended\r\nS>";
          writeBytes(m_end);
          missionMode = 0;
          parkDescentTime = 18000000;
          parkPressure = 2000;
          downTime = 86400000;
          deepProfileDescentTime = 18000000;
          deepProfilePressure = 4000;
          ascentTimeOut = 36000000;
          currentTime = 0;
          startOffset = 0; 
          pauseOffset = 0;
        }
        
        else if(input.equals("pause mission\r")){
          String m_paused = "mission paused\r\nS>";
          writeBytes(m_paused);
          pauseTime = millis();
          missionMode = 0;
        }
        
        else if(input.equals("resume mission\r")){
          String m_resume = "resuming mission\r\nS>";
          writeBytes(m_resume);
          resumeTime = millis();
          pauseOffset = resumeTime-pauseTime;
          currentTime += pauseOffset;
          parkDescentTime += pauseOffset;
          downTime += pauseOffset;
          deepProfileDescentTime += pauseOffset;
          ascentTimeOut += pauseOffset;
          missionMode = 108;
          
        }
        
        //if the input is parkDescentTime=, check the Serial port for the value to be used as the park descent time,
        //calculate the value of the park descent time in milliseconds, then send the value of park descent time as 
        //a series of bytes, confirm that the value has actually changed by using the global variable value 
        //in this echo
        else if(input.equals("parkDescentTime=")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              parkDescentTimeDisplay = input.toInt();
              parkDescentTime=((input.toInt())*60000) + startOffset + pauseOffset;
              break;
            }
          }
          String parkDescentTimeStr = "\r\nS>parkDescentTime="+String(parkDescentTimeDisplay)+"\r\nS>";
          writeBytes(parkDescentTimeStr);
          missionMode++;
        }
        
        //if the input is parkPressure=, check the Serial port for the value to be used as the park pressure,
        //calculate the value of the park pressure in dbar, then send the value of park pressure as 
        //a series of bytes, confirm that the value has actually changed by using the global variable value 
        //in this echo
        else if(input.equals("parkPressure=")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              parkPressure=input.toInt();
              break;
            }
          }
          String parkPressureStr = "\r\nS>parkPressure="+String(parkPressure)+"\r\nS>";
          writeBytes(parkPressureStr);
          missionMode++;
        }
        
        //if the input is downTime=, check the Serial port for the value to be used as the down time,
        //calculate the value of the down time in milliseconds, then send the value of down time as 
        //a series of bytes, confirm that the value has actually changed by using the global variable value 
        //in this echo
        else if(input.equals("downTime=")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              downTimeDisplay = input.toInt();
              downTime=((input.toInt())*60000) + startOffset + pauseOffset;
              break;
            }
          }
          String downTimeStr = "\r\nS>downTime="+String(downTimeDisplay)+"\r\nS>";
          writeBytes(downTimeStr);
          missionMode++;
        }

        //if the input is deepProfileDescentTime=, check the Serial port for the value to be used as the
        //deep profile descent time, calculate the value of the deep profile descent time in milliseconds,
        //then send the value of deep profile descent time as a series of bytes, confirm that the value 
        //has actually changed by using the global variable value in this echo
        else if(input.equals("deepProfileDescentTime=")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              deepProfileDescentTimeDisplay = input.toInt();
              deepProfileDescentTime=((input.toInt())*60000) + startOffset + pauseOffset;
              break;
            }
          }
          String deepProfileDescentTimeStr = "\r\nS>deepProfileDescentTime="+String(deepProfileDescentTimeDisplay)+"\r\nS>";
          writeBytes(deepProfileDescentTimeStr);
          missionMode++;
        }
        
        //if the input is deepProfilePressure=, check the Serial port for the value to be used as the deep profile pressure,
        //calculate the value of the deep profile pressure in dbar, then send the value of deep profile pressure as 
        //a series of bytes, confirm that the value has actually changed by using the global variable value 
        //in this echo
        else if(input.equals("deepProfilePressure=")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              deepProfilePressure=input.toInt();
              break;
            }
          }
          String deepProfilePressureStr = "\r\nS>deepProfilePressure="+String(deepProfilePressure)+"\r\nS>";
          writeBytes(deepProfilePressureStr);
          missionMode++;
        }
        
        //if the input is ascentTimeOut=, check the Serial port for the value to be used as the ascent timeout,
        //calculate the value of the ascent timeout in milliseconds, then send the value of current time as 
        //a series of bytes, confirm that the value has actually changed by using the global variable value 
        //in this echo
        else if(input.equals("ascentTimeOut=")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              ascentTimeOutDisplay = input.toInt();
              ascentTimeOut=((input.toInt())*60000) + startOffset + pauseOffset;
              break;
            }
          }
          String ascentTimeOutStr = "\r\nS>ascentTimeOut="+String(ascentTimeOutDisplay)+"\r\nS>";
          writeBytes(ascentTimeOutStr);
          missionMode++;
        }

        //if the input is currentTime=, check the Serial port for the value to be used as the current time,
        //calculate the value of the current time in milliseconds, then send the value of current time as 
        //a series of bytes, confirm that the value has actually changed by using the global variable value 
        //in this echo
        else if(input.equals("currentTime=")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              currentTimeDisplay = input.toInt();
              currentTime=((input.toInt())*1000)+ startOffset + pauseOffset;
              lastUpdate = millis();
              break;
            }
          }
          String currentTimeStr = "\r\nS>currentTime="+String(currentTimeDisplay)+" ("+String((currentTimeDisplay/60))+" minutes)\r\nS>";
          writeBytes(currentTimeStr);
          missionMode++;
        }
        
        //if the input is the ds command, send back all of the information as a series of bytes (uses generic
        //info based on an actual seabird, can edit field in this string if necessary), there should be 3 
        //fields that will vary: the number of bins, number of samples, and whether it is expecting only P
        //or pts for real time output
        else if(input.equals("ds\r")){
          String countStr = String(count);
          String nBinsStr = String(nBins);
          String ds = "ds\r\nSBE 41CP UW V 2.0  SERIAL NO. 4242"
          "\r\nfirmware compilation date: 18 December 2007 09:20"
          "\r\nstop profile when pressure is less than = 2.0 decibars"
          "\r\nautomatic bin averaging at end of profile disabled"
          "\r\nnumber of samples = "+countStr+
          "\r\nnumber of bins = "+nBinsStr+
          "\r\ntop bin interval = 2"
          "\r\ntop bin size = 2"
          "\r\ntop bin max = 10"
          "\r\nmiddle bin interval = 2"
          "\r\nmiddle bin size = 2"
          "\r\nmiddle bin max = 20"
          "\r\nbottom bin interval = 2"
          "\r\nbottom bin size = 2"
          "\r\ndo not include two transitions bins"
          "\r\ninclude samples per bin"
          "\r\npumped take sample wait time = 20 sec"
          "\r\nreal-time output is "+pOrPTS[pOrPTSsel]+"\r\nS>";
          int dsLen = ds.length()+1;
          byte dsBuff[750];
          ds.getBytes(dsBuff, dsLen);
          Serial1.write(dsBuff, dsLen);
        }
        
        //if the input is the dc command, send back all of the information as a series of bytes (uses generic
        //info based on an actual seabird (can edit field in this string if necessary)
        else if(input.equals("dc\r")){
          String dc = "dc\r\nSBE 41CP UW V 2.0  SERIAL NO. 4242"
          "\r\ntemperature:  19-dec-10"
          "\r\n    TA0 =  4.882851e-05"
          "\r\n    TA1 =  2.747638e-04"
          "\r\n    TA2 = -2.478284e-06"
          "\r\n    TA3 =  1.530870e-07"
          "\r\nconductivity:  19-dec-10"
          "\r\n    G = -1.013506e+00"
          "\r\n    H =  1.473695e-01"
          "\r\n    I = -3.584262e-04"
          "\r\n    J =  4.733101e-05"
          "\r\n    CPCOR = -9.570001e-08"
          "\r\n    CTCOR =  3.250000e-06"
          "\r\n    WBOTC =  2.536509e-08"
          "\r\npressure S/N = 3212552, range = 2900 psia:  14-dec-10    "
          "\r\nPA0 =  6.297445e-01"
          "\r\n    PA1 =  1.403743e-01"
          "\r\n    PA2 = -3.996384e-08"
          "\r\n    PTCA0 =  6.392568e+01"
          "\r\n    PTCA1 =  2.642689e-01"
          "\r\n    PTCA2 = -2.513274e-03"
          "\r\n    PTCB0 =  2.523900e+01"
          "\r\n    PTCB1 = -2.000000e-04"
          "\r\n    PTCB2 =  0.000000e+00"
          "\r\n    PTHA0 = -7.752968e+01"
          "\r\n    PTHA1 =  5.141199e-02"
          "\r\n    PTHA2 = -7.570264e-07"
          "\r\n    POFFSET =  0.000000e+00"
          "\r\nS>";
          int dcLen = dc.length()+1;
          byte dcBuff[750];
          dc.getBytes(dcBuff, dcLen);
          Serial1.write(dcBuff, dcLen);
        }
        
        //if the input is startprofile, recognize that it is the start profile command,
        //then send back that the profile has started, reattach interrupt to pin2, and 
        //turn on continuous profiling mode
        else if(input.equals("startprofile")){
          String cp = "\r\nS>startprofile\r\nprofile started, pump delay = 0 seconds\r\nS>";
          writeBytes(cp);
          attachInterrupt(0, checkLine, RISING);
          cpMode = 1;
        }
        //if the input is startprofile, recognize that it is the start profile command,
        //then send back that the profile has started, reattach interrupt to pin2, and 
        //turn on continuous profiling mode
        else if(input.equals("startprofile")){
          String cp = "\r\nS>startprofile\r\nprofile started, pump delay = 0 seconds\r\nS>";
          writeBytes(cp);
          cpMode = 1;
        }
        
        //if the input is stopprofile, recognize that it is the stop profile command,
        //then send back that the profile has stopped, ignore the external interrupt
        //on pin2, and turn off continuous profiling mode
        else if(input.equals("stopprofile")){
          String exitcp = "profile stopped";
          writeBytes(exitcp);
          cpMode = -1;
        }
        
        //if the input is binaverage, return the values parsed from the data sent
        //during continuous profiling mode. set da to 1 which will allow for the
        //da command to be run (makes sure there is actual data to dump when requested)
        else if(input.equals("binaverage\r")){
          nBins = (int(maxPress)/2) + 1;
          String countStr2 = String(count);
          String maxPressStr = pressureToString(maxPress);
          String nBinsStr2 = String(nBins);
          String binavg = "\r\nS>binaverage\r\nsamples = "+countStr2+", maxPress = "+maxPressStr+"\r\nrd: 0\r\navg: 0\r\n\ndone, nbins = "+nBinsStr2+"\r\nS>";
          writeBytes(binavg);
          da = 1;
        }

        //if the inpt is da, send bins in the format "p, t, s, b" (pressure,
        //temperature, salinity, number of samples) over Serial1. then send that 
        //the upload is done. then reinitialize all of the global variables used 
        //for bin averaging and dumping the values        
        else if((input.equals("da\r"))&&(da==1)){
          first = 1;
          int ii;
          
          //send all of the samples over serial
          for(ii=0; ii < nBins; ii++){
            if(ii == nBins - 1){
              last = 1;
            }
            String bin = binaverage();
            writeBytes(bin);
          }
          
          //send upload complete at end of all samples
          String complete = "\r\nupload complete\r\nS>";
          writeBytes(complete);
          
          //reinitalize values
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
          String cmdMode = "\r\nS>qsr\r\npowering down\r\nS>";
          writeBytes(cmdMode);
          commandMode = -1;
          delay(100);
          attachInterrupt(0, checkLine, RISING);
        }
        
        //if the input is autobinavg=n, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("autobinavg=n\r")){
          delay(10);
          String aba = "\r\nS>autobinavg=n";
          writeBytes(aba);
        }
        
        //if the input is pcutoff=2.0, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("pcutoff=2.0\r")){
          delay(10);
          String pcutoff = "\r\nS>pcutoff=2.0";
          writeBytes(pcutoff);
        }
        
        //if the input is outputpts=y, send back the command prompt and echo the input as a series of bytes
        //and change pOrPTSsel to 1 so that the ds command will display pts
        else if(input.equals("outputpts=y\r")){
          delay(10);
          String optsy = "\r\nS>outputpts=y";
          writeBytes(optsy);
          pOrPTSsel = 1;
        }
        
        //if the input is tswait=20, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("tswait=20\r")){
          delay(10);
          String tsw = "\r\nS>tswait=20";
          writeBytes(tsw);
          pOrPTSsel = 1;
        }
        
        //if the input is top_bin_interval=2, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("top_bin_interval=2\r")){
          delay(10);
          String tbi = "\r\nS>top_bin_interval=2";
          writeBytes(tbi);
        }
        
        //if the input is top_bin_size=2, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("top_bin_size=2\r")){
          delay(10);
          String tbs = "\r\nS>top_bin_size=2";
          writeBytes(tbs);
        }
        
        //if the input is top_bin_max=10, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("top_bin_max=10\r")){
          delay(10);
          String tbm = "\r\nS>top_bin_max=10";
          writeBytes(tbm);
        }
        
        //if the input is middle_bin_interval=2, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("middle_bin_interval=2\r")){
          delay(10);
          String mbi = "\r\nS>middle_bin_interval=2";
          writeBytes(mbi);
        }
        
        //if the input is middle_bin_size=2, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("middle_bin_size=2\r")){
          delay(10);
          String mbs = "\r\nS>middle_bin_size=2";
          writeBytes(mbs);
        }
        
        //if the input is middle_bin_max=20, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("middle_bin_max=20\r")){
          delay(10);
          String mbm = "\r\nS>middle_bin_max=20";
          writeBytes(mbm);
        }
        
        //if the input is bottom_bin_interval=2, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("bottom_bin_interval=2\r")){
          delay(10);
          String bbi = "\r\nS>bottom_bin_interval=2";
          writeBytes(bbi);
        }
        
        //if the input is bottom_bin_size=2, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("bottom_bin_size=2\r")){
          delay(10);
          String bbs = "\r\nS>bottom_bin_size=2";
          writeBytes(bbs);
        }
        
        //if the input is includetransitionbin=n, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("includetransitionbin=n\r")){
          delay(10);
          String itb = "\r\nS>includetransitionbin=n";
          writeBytes(itb);
        }
        
        //if the input is includenbin=y, send back the command prompt and echo the input as a series of bytes
        else if(input.equals("includenbin=y\r")){
          delay(10);
          String ib = "\r\nS>includenbin=y";
          writeBytes(ib);
        }
        
        //if the input is outputpts=n, send back the command prompt and echo the input as a series of bytes
        //and set pOrPTSsel to 0 so that the ds command will display p only
        else if(input.equals("outputpts=n\r")){
          delay(10);
          String optsn = "\r\nS>outputpts=n";
          writeBytes(optsn);
          pOrPTSsel = 0;
        }
        
        //if the input is id, send back that the seabird is in ice detect mode as a series of bytes 
        //change the global variable ice avoidance to 1, which is detect mode. also assume ice will
        //be detected at 20 dbar
        else if((input.equals("id\r"))||(input.equals("id on\r"))){
          iceAvoidance = 1;
          icePressure = 20;
          String icedMode = "\r\nice detect mode on\r\nS>";
          writeBytes(icedMode);
        }
        
        //if the input is id@<val>, send back that the seabird is in ice detect mode as a series of bytes 
        //change the global variable ice avoidance to 1, which is detect mode. also set ice to be detected 
        //at the given input pressure
        else if(input.equals("id@")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              icePressure=input.toInt();
              break;
            }
          }
          iceAvoidance = 1;
          String icedaMode = "\r\nice detect mode on, will detect ice at "+String(icePressure)+"dbar\r\nS>";
          writeBytes(icedaMode);
        }
        
        //if the input is ic, send back that the seabird is in ice cap mode as a series of bytes 
        //change the global variable ice avoidance to 2, which is cap mode
        else if((input.equals("ic\r"))||(input.equals("ic on\r"))){
          icePressure = 4;
          iceAvoidance = 2;
          String icecMode = "\r\nice cap mode on\r\nS>";
          writeBytes(icecMode);
        }
        
        //if the input is ic@<val>, send back that the seabird is in ice cap mode as a series of bytes 
        //change the global variable ice avoidance to 2, which is cap mode. also set ice to be detected 
        //at the given input pressure
        else if(input.equals("ic@")){
          while(1){
            if(Serial1.available()>0){
              String input = "";
              while(1){
                if(Serial1.available()>0){  
                  char temp;
                  temp = char(Serial1.read());
                  if(temp=='\r'){
                    break;
                  }
                  if(temp!=' '){
                    input+=temp;
                  }
                }
              }
              icePressure=input.toInt();
              break;
            }
          }
          iceAvoidance = 2;
          String icecaMode = "\r\nice cap mode on, will detect ice at "+String(icePressure)+"dbar\r\nS>";
          writeBytes(icecaMode);
        }
        
        //if the input is ib, send back that the seabird is in ice breakup mode as a series of bytes 
        //change the global variable ice avoidance to 3, which is breakup mode
        else if((input.equals("ib\r"))||(input.equals("ib on\r"))){
          iceAvoidance = 1;
          String icebMode = "\r\nice breakup mode on\r\nS>";
          writeBytes(icebMode);
        }
        
        //if the input is id off, send back that ice detect mode is off as a series of bytes 
        //change the global variable ice avoidance to -1, which is normal mode
        else if(input.equals("id off\r")){
          iceAvoidance = -1;
          String icedModeOff = "\r\nice detect mode off\r\nS>";
          writeBytes(icedModeOff);
          icePressure=20;
        }
        
        //if the input is ic off, send back that ice cap mode is off as a series of bytes 
        //change the global variable ice avoidance to -1, which is normal mode
        else if(input.equals("ic off\r")){
          iceAvoidance = -1;
          String icecModeOff = "\r\nice cap mode off\r\nS>";
          writeBytes(icecModeOff);
          icePressure=20;
        }
        
        //if the input is ib off, send back that ice breakup mode is off as a series of bytes 
        //change the global variable ice avoidance to -1, which is normal mode
        else if(input.equals("ib off\r")){
          iceAvoidance = -1;
          String icebModeOff = "\r\nice breakup mode off\r\nS>";
          writeBytes(icebModeOff);
        }
        
        //if the input is ?, list the simulation type and all of the options for commands
        else if(input.equals("?\r")){
          String list = "?\r\nAPF-9 & APF-11 Iridium SBE41cp Simulator"
          "\r\nid"
          "\r\nid on"
          "\r\nid@<value>"
          "\r\nid off"
          "\r\nic"
          "\r\nic on"
          "\r\nic@<value>"
          "\r\nic off"
          "\r\nib"
          "\r\nib on"
          "\r\nib off"
          "\r\nqsr"
          "\r\nda"
          "\r\nds"
          "\r\ndc"
          "\r\nstartprofile"
          "\r\nstopprofile"
          "\r\nbinaverage"
          "\r\nmission"
          "\r\nlist parameters"
          "\r\nstart descent"
          "\r\nparkPressure=<value>"
          "\r\nparkDescentTime=<value>"
          "\r\ndeepProfilePressure=<value>"
          "\r\ndeepProfileDescentTime=<value>"
          "\r\ndownTime=<value>"
          "\r\nascentTimeOut=<value>"
          "\r\ncurrentTime=<value>"
          "\r\npause mission"
          "\r\nresume mission"
          "\r\nend mission\r\nS>";
          writeBytes(list);
        }
        
        else{
        }   
      }
    }
  }
}


/*************************************************************************/
/*                             getReadingFromPiston                      */
/*                             ********************                      */
/*                                                                       */
/* parameters: select, an int value that represents which string will be */
/*                  returned (PTS, PT, or P reading)                     */
/*                                                                       */
/* returns: String representing the PTS, PT, or P reading                */
/*                                                                       */
/* This function converts a reading from the analog input pin A0 to a    */
/* string that represents P,T,S sample. This is achieved by manipulating */
/* the input value and fitting it to generic, general values tested by   */
/* Hugh Fargher. In general, we used 3 linear models to represent 3      */
/* ranges of depth (2000m-1000m, 1000m-500m, 500m-0m) with different     */
/* slopes and offsets. From these pressure values, we need to handle any */
/* ice avoidance scenarios then we assume a polynomail relationship to   */
/* temperature. Lastly, we can assume one last polynomail relationship   */
/* between temperature and salinity (the same ratio as pressure to       */
/* temperature, so use temperature to calculate). The values of these    */
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
  //to a string using the pressureToString function
  pressure = pressure * 1.08;
  if(cpMode == 1){
    if(pressure >= maxPress){
      maxPress = pressure;
    }
    if(pressure <= minPress){
      minPress = pressure;
    }
  }
  
  pStr = pressureToString(pressure);
  
  //determine if in ice detect, ice cap, ice breakup, or normal mode
  //then calculate temperature based on criteria
  
  //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
  if((iceAvoidance == 1)&&(pressure < 50)){
    float midway = float((50-icePressure)/2) + icePressure;
    temperature = float((pressure-midway)/midway) - 1.8;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == 2)&&(pressure < 20)){
   float midway = float((20-icePressure)/2) + icePressure;
   temperature = float((pressure-midway)/midway) - 1.8;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == 3)&&(pressure <50)){
    temperature = 23.2-float(pressure*0.0088);
  }
  
  //calculate temperature normally
  else{ 
    temperature = 23.2-float(pressure*0.0088);
  }
  
  tStr = tempOrSalinityToString(temperature);
  
  //calculate a float salinty value based on the pressure, assume linearity with the 
  //minimum salinity of 33.5. then convert the float 
  //to a string using the tempOrSalinityToString function
  salinity = temperature*0.1+ 34.9;
  sStr = tempOrSalinityToString(salinity);
  
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
      sendMessage = (pStr)+"\r\n";
      break;
  }
  
  //return the given string
  return sendMessage;
}


/*************************************************************************/
/*                               getDynamicReading                       */
/*                             ********************                      */
/*                                                                       */
/* parameters: select, an int value that represents which string will be */
/*                  returned (PTS, PT, or P reading)                     */
/*                                                                       */
/*             phase, an int that represents what phase of the mission   */
/*                  the apf expects to be in                             */
/*                                                                       */
/* returns: String representing the PTS, PT, or P reading                */
/*                                                                       */
/* This function calculates a pressure based on the time it has spent in */
/* a given phase during a mission and produce a string that represents   */
/* a P; P,T; or P,T,S sample. From these pressure values, we need to     */
/* handle any ice avoidance scenarios. Then we use a relationship to     */
/* temperature. Lastly, we can assume one last polynomail relationship   */
/* between temperature and salinity (the same ratio as pressure to       */
/* temperature, so use temperature to calculate). The values of these    */
/* strings are appended to one another and formatted to match a regex    */
/* pattern expected by the APF board on the float. Then use the select   */
/* to choose which string (PTS, PT, or P) to send to the APF board. The  */
/* phase of the mission is determined by the global variable phase.      */
/*                                                                       */
/*************************************************************************/

String getDynamicReading(int select, int phase){
  
  updateTime();
  
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
  
  //calculate a pressure based on the current phase: 0 = descent, 1 = park, 2 = deep descent, 3 = ascent
  switch(phase){
    case 0:
      pressure = float(float(currentTime)*parkPressure/float(parkDescentTime));
      break;
    case 1:
      pressure = parkPressure + float(float(random(100))/float(10)) - float(float(random(100))/float(10));
      break;
    case 2:
      pressure = parkPressure + float(float((currentTime-downTime))*(deepProfilePressure-parkPressure)/float(deepProfileDescentTime));
      break;
    case 3:
      pressure = deepProfilePressure - float(float(((currentTime-deepProfileDescentTime)-downTime))*deepProfilePressure/float((ascentTimeOut)));
      break;
  }
  
  if(cpMode == 1){
    if(pressure >= maxPress){
      maxPress = pressure;
    }
    if(pressure <= minPress){
      minPress = pressure;
    }
  }
  
  pStr = pressureToString(pressure);
  
  //determine if in ice detect, ice cap, ice breakup, or normal mode
  //then calculate temperature based on criteria
  
  //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
  if((iceAvoidance == 1)&&(pressure < 50)){
    float midway = float((50-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == 2)&&(pressure < 20)){
    float midway = float((20-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == 3)&&(pressure <55)){
    temperature = 23.2-float(pressure*0.0088);
  }
  
  //calculate temperature normally
  else{ 
    temperature = 23.2-float(pressure*0.0088);
  }
  
  tStr = tempOrSalinityToString(temperature);
  
  //calculate a float salinty value based on the pressure, assume linearity with the 
  //minimum salinity of 33.5. then convert the float 
  //to a string using the tempOrSalinityToString function
  salinity = temperature*0.1+ 34.9;
  sStr = tempOrSalinityToString(salinity);
  
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
      sendMessage = (pStr)+"\r\n";
      break;
  }
  
  //return the given string
  return sendMessage;
}

/*************************************************************************/
/*                              binaverage                               */
/*                              **********                               */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: String representing the bin in the format of avg P, avg T,   */
/*              avg S, then the number of bins                           */
/*                                                                       */
/* This function will create a string that represents the data requested */
/* by the binaverage command of the APFx. The function creates a         */
/* pressure value that is approximately 2 greater than the last, then    */
/* calculates the temperature and salinity according to the same         */
/* algorithm as getReadingFromPiston. It also calculates a random number */
/* of samples per bin between 0 and 35. It handles the cases of the      */
/* first and last bin differently. The first bin is used to initialize   */
/* total number of bins available. The last bin is used to ensure that   */
/* the sum of the random bins equals the total number of bins. If the    */
/* pressure calculated in the function is less than the minimum pressure */
/* calculated or if the number of samples in a bin is 0, then all of the */
/* other fields are equal to 0. It then returns a string value that      */
/* matches the expected output ("pppp.pp, tt.tttt, ss.ssss, bb"). This   */
/* function is meant to be called repeatedly when sending data to the    */
/* APFx after receiving the 'da' command.                                */
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
  
  //determine if in ice detect, ice cap, ice breakup, or normal mode
  //then calculate temperature based on criteria
  
  //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
  if((iceAvoidance == 1)&&(pressure < 50)){
    float midway = float((50-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == 2)&&(pressure < 20)){
    float midway = float((20-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == 3)&&(pressure <55)){
    temperature = 23.2-float(pressure*0.0088);
  }
  
  //calculate temperature normally
  else{ 
    temperature = 23.2-float(pressure*0.0088);
  }
  
  //calculate salinity normally
  salinity = temperature*0.1 + 34.9;
  
  //if the loop is in its first iteration, the total number of samples
  //is the number of samples originally taken (count)
  if(first == 1){
    samplesLeft = count;
    first = -1;
  }
  
  //calculate a random value for the number of samples per bin
  samplesUsed = ((samplesLeft%5) + random(10,30));
  
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
  //"pppp.pp, tt.tttt, ss.ssss, bb"
  returnStr = pressureToString(pressure)+", "+tempOrSalinityToString(temperature)+", "+tempOrSalinityToString(salinity)+", "+String(samplesUsed)+"\r\n";
  
  //return the string
  return returnStr;
}

/*************************************************************************/
/*                           pressureToString                            */
/*                           ****************                            */
/*                                                                       */
/* parameters: aFloat, a float value representing the float that is      */
/*                 going to be converted to a string                     */
/*                                                                       */
/* returns: an string value that will represent the float as a string    */
/*                                                                       */
/* This function creates a string that will look like a float by         */
/* splitting it into its whole and decimal parts, then adding them as    */
/* two strings with the appropriate formatting for a pressure value      */
/*                                                                       */
/*************************************************************************/

String pressureToString(float aFloat){
  
  //long int values
  long floatLong;
  int floatInt;
  int floatDec;
  
  //string to be returned
  String floatStr;
  
  //calculate the whole number and decimal number
  floatLong = 100*aFloat;
  floatInt = floatLong/100;
  floatDec = abs(floatLong - (floatInt*100));
 
  //handle case for losing the 0 in a number less than 10 (i.e. get 09 instead of 9)
  if(floatDec<10){
    floatStr = " "+String(floatInt)+".0"+String(floatDec);
  }
  else{
    floatStr = " "+String(floatInt)+"."+String(floatDec);
  }
  
  //return the formatted string
  return floatStr;
}

/*************************************************************************/
/*                        tempOrSalinityToString                         */
/*                        **********************                         */
/*                                                                       */
/* parameters: aFloat, a float value representing the float that is      */
/*                 going to be converted to a string                     */
/*                                                                       */
/* returns: an string value that will represent the float as a string    */
/*                                                                       */
/* This function creates a string that will look like a float by         */
/* splitting it into its whole and decimal parts, then adding them as    */
/* two strings with the appropriate formatting for a temperature or      */
/* salinity value.                                                       */
/*                                                                       */
/*************************************************************************/

String tempOrSalinityToString(float aFloat){
  
  //long int values
  long floatLong;
  int floatInt;
  int floatDec;
  
  //string to be returned
  String floatStr;
  //calculate the whole number and decimal number
  floatLong = 10000*aFloat;
  floatInt = floatLong/10000;
  floatDec = floatLong - (floatInt*10000);
  if(floatDec < 0){
    floatDec = floatDec - floatDec*2;
  }
 
  //handle case for losing three 0's in a number less than 10 (i.e. get .0009 instead of .9000)
  //or losing two 0's in a number less than 100 but greater than 10 (i.e. .0091 rather than .9100)
  // or losing a 0 in a number less than 1000 but greater than 100 (i.e. .0991 rather than .9110
  if(floatDec<10){
    floatStr = " "+String(floatInt)+".000"+String(floatDec);
  }
  else if((floatDec<100)&&(floatDec>=10)){
    floatStr = " "+String(floatInt)+".00"+String(floatDec);
  }
  else if((floatDec<1000)&&(floatDec>=100)){
    floatStr = " "+String(floatInt)+".0"+String(floatDec);
  }
  else{
    floatStr = " "+String(floatInt)+'.'+String(floatDec);
  }
  
  //return the formatted string
  return floatStr;
}


/*************************************************************************/
/*                                debounce                               */
/*                                ********                               */
/*                                                                       */
/* parameters: pin, an integer value representing the pin that is        */
/*                 connected to the input being toggled                  */
/*                                                                       */
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
/*                                runTimer                               */
/*                                ********                               */
/*                                                                       */
/* parameters: timeout, an int representing the desired runtime in ns    */
/*                                                                       */
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
/*                               updateTime                              */
/*                               **********                              */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: a long integer value that represents the current time in the */
/*                 mission                                               */
/*                                                                       */
/* This function updates the current time of the mission and determines  */
/* which part of the mission the simulator should be in, currently, the  */
/* default order is park descent, park, deep descent, ascent, done       */
/*                                                                       */
/*************************************************************************/
long updateTime(){
  if(missionMode >= 107){
    phase = 0;
    update = millis();
    currentTime += (update - lastUpdate);
    currentTimeDisplay += ((update - lastUpdate)/1000);
    lastUpdate = update;
    if(currentTime>=parkDescentTime){
      phase = 1;
      if(currentTime>=downTime){
        phase = 2;
        if(currentTime>=(downTime+deepProfileDescentTime)){
          phase = 3;
          if(currentTime>=(downTime+deepProfileDescentTime+ascentTimeOut)){
            missionMode = 0;
            parkDescentTime = 18000000;
            parkPressure = 1000;
            downTime = 86400000;
            deepProfileDescentTime = 18000000;
            deepProfilePressure = 2000;
            ascentTimeOut = 36000000;
            currentTime = 0;
            startOffset = 0;
            pauseOffset = 0;
          }
        }  
      }
    }
  }
  return currentTime;
}

/*************************************************************************/
/*                               writeBytes                              */
/*                               **********                              */
/*                                                                       */
/* parameters: aString, a string value to be written over Serial1 as a   */
/*                 series of bytes                                       */
/*                                                                       */
/* returns: none                                                         */
/*                                                                       */
/* This function calculates the string length, creates a byte buffer,    */
/* converts the string to a series of bytes, then sends the bytes over   */
/* Serial1                                                               */
/*                                                                       */
/*************************************************************************/      
void writeBytes(String aString){
  int strLen = aString.length()+1;
  byte strBuffer[500];
  aString.getBytes(strBuffer, strLen);
  Serial1.write(strBuffer, strLen);
}
