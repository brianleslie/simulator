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
/* in order to compile, you must download this library from the arduino  */
/* playground online                                                     */
/*                                                                       */
/*************************************************************************/

#include <TimerOne.h>

/*************************************************************************/
/*                                  #defines                             */
/*                                  ********                             */
/*                                                                       */
/*************************************************************************/


//used to define phase/state of mission
#define PRESSUREACTIVATION 0
#define PRELUDE 1
#define PARKDESCENT 2
#define PARK 3
#define DEEPDESCENT 4
#define ASCENTTOPARK 5
#define ASCENTTOSURFACE 6
#define SURFACE 7

//used to define the type of message to send back as a result of a request
//over the hardware toggle lines
#define SERNO 1
#define PTS 2
#define PT 3
#define P 4

//used to define the different options for ice detection
#define NOICE -1
#define ICEDETECT 1
#define ICECAP 2
#define ICEBREAKUP 3

//used to define the commands to set the mission parameters
#define DOWNTIME "Mtd"
#define ASCENTTIMEOUT "Mta"
#define DEEPPROFILEDESCENTTIME "Mtj"
#define PARKDESCENTTIME "Mtk"
#define PRELUDETIME "Mtp"
#define PARKPRESSURE "Mk"
#define DEEPPROFILEPRESSURE "Mj"
#define MISSIONTIME "i*t"
#define DOWNTIME2 "mtd"
#define ASCENTTIMEOUT2 "mta"
#define DEEPPROFILEDESCENTTIME2 "mtj"
#define PARKDESCENTTIME2 "mtk"
#define PRELUDETIME2 "mtp"
#define PARKPRESSURE2 "mk"
#define DEEPPROFILEPRESSURE2 "mj"

#define OFF -69

/*************************************************************************/
/*                             global variables                          */
/*                             ****************                          */
/*                                                                       */
/* interruptMessage: an int that represents which message to send based  */
/*                  on the toggling of the hardware lines                */
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
/* nBins, samplesLeft, samplesused: int values that represent the total  */
/*                  number of bins, the number of samples left after     */
/*                  subtracting the samples used for one bin, and the    */
/*                  the number of samples used in a single bin           */
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
/* icePressure: an int that represents the pressure at which ice will be */
/*                  detected, set by typing one of ice (id/b/c) commands */
/*                                                                       */
/* missionMode: an int that represents whether the simulator should be   */
/*                  simulating a mission, 0: no mission, 100>=: mission  */
/*                                                                       */
/* currentPosition, lastPosition: ints that represent the current  and   */
/*                  previous values being read from the piston pot       */
/*                                                                       */
/* currentPhase, lastPhase: ints that represent which phase of the       */
/*                  mission the is currently being simulated and the     */
/*                  previous phase that was just simulated, both at      */
/*                  start (prelude) initially                            */
/* phase: an array of strings that represent the various phases that a   */
/*                  mission might be in, used for the UI while running a */
/*                  mission                                              */
/*                                                                       */
/* update, lastUpdate: ints that represent the mission time in the       */
/*                  mission (relative to the last update) and the last   */
/*                  time that the current mission time was updated       */
/*                                                                       */
/* parkPressure, deepProfilePressure: ints that represent the pressures  */
/*                  expected at different states of the mission          */
/*                                                                       */
/* parkDescentTime, parkDescentTimeDisplay, parkDescentTimeCopy: ints    */
/*                  that represent the time in ms, minutes, and a copy   */
/*                  in ms, it takes for the float to go through the      */
/*                  entire park descent                                  */
/*                                                                       */
/* downTime, downTimeDisplay, downTimeCopy: ints that represent the time */
/*                  in ms, minutes, and a copy in ms, it takes for the   */
/*                  float to go through the entire park descent, park,   */
/*                  and deep profile descent phases combined             */
/*                                                                       */
/* deepProfileDescentTime, deepProfileDescentTimeDisplay,                */
/*                  deepProfileDescentCopy: ints that represent the time */
/*                  in ms, seconds, and a copy in ms, it takes for the   */
/*                  float to go through the entire deep profile descent  */
/*                                                                       */
/* ascentTimeOut, ascentTimeOutDisplay, ascentTimeOutCopy: ints that     */
/*                  represent the time in ms, minutes, and a copy in ms, */
/*                  it takes for the float to go through the ascent      */
/*                                                                       */
/* missionTime: an int representing the mission time in the mission, can */
/*                  be set manually in sys chat or will be incremented   */
/*                  by timer automatically every ~5 seconds              */
/*                                                                       */
/* ascentTime: ascentTimeCopy: ints that represent the actual time in ms */
/*                  that it takes the float to ascend to the surface, it */
/*                  is based off of the ascent rate and the depth of the */
/*                  deep profile descent                                 */
/*          w                                                             */
/* ascentToPark, ascentToParkCopy: ints that represent the actual time   */
/*                  in ms that it takes the float to ascent from deep    */
/*                  profile pressure to park pressure, based on ascent   */
/*                  time, and the difference between the park pressure   */
/*                  and the deep profile pressure                        */
/*                                                                       */
/* ascentToSurface, ascentToSurfaceCopy: ints that represent the actual  */
/*                  time in ms that it takes the float to ascent from    */
/*                  deep profile pressure to park pressure, based on     */
/*                  ascent time, and the difference between the park     */
/*                  pressure and the deep profile pressure               */
/*                                                                       */
/* ascentRate: an int that represents how quickly a float will ascend in */
/*                  cm/sec during ascent phase of a mission              */
/*                                                                       */
/*************************************************************************/

volatile int interruptMessage = 0;
int cpMode;
int count = 0;

float maxPress = 0;
float minPress = 10000;

int nBins = 0;
int samplesLeft;
int da = -1;

int inc = 0;

int last = -1;
int first = -1;

String msg = "SBE 41CP UW. V 2.0";

String msg2;

String msg3;

String msg4;

String pOrPTS[2] = {"P only", "PTS"};

int pOrPTSsel = 1;

int iceAvoidance = -1;

int icePressure = 20;

int missionMode = 0;

int currentPosition = 0, lastPosition = 0;

int currentPhase = PRESSUREACTIVATION, lastPhase = PRESSUREACTIVATION;

String phase[8]={"Pressure Activation","Prelude","Park Descent","Park","Deep Profile Descent","Ascent","Ascent","Surface"};

long lastUpdate = 0, update = 0;

int parkPressure = 1000, deepProfilePressure = 2000;

long prelude = -1, preludeDisplay = 200;

long parkDescentTime = 18000000, parkDescentTimeCopy = 18000000, parkDescentTimeDisplay = 300;

long downTime = 86400000, downTimeCopy = 86400000, downTimeDisplay = 1440;

long deepProfileDescentTime = 18000000, deepProfileDescentTimeCopy = 18000000, deepProfileDescentTimeDisplay = 300;

long ascentTimeOut = 36000000, ascentTimeOutCopy = 36000000, ascentTimeOutDisplay = 600;

long missionTime = 0, missionTimeCopy = 0, missionTimeDisplay = 0;

long phaseChange = 0, newTime, newTimeDisplay;

long ascentTime = 25000000, ascentTimeCopy = 25000000;

long ascentToPark = 12500000, ascentToParkCopy = 12500000;

long ascentToSurface = 12500000, ascentToSurfaceCopy = 12500000;

int ascentRate = 8;

int constantP, constant = -1;;

/*************************************************************************/
/*                            function prototypes                        */
/*                            *******************                        */
/*                                                                       */
/*************************************************************************/

String getReadingFromPiston(int);

String getDynamicReading(int);

String pressureToString(float);

String tempOrSalinityToString(float);

String binaverage(void);

String binaverageHex(void);

long int getInput(void);

int debounce(int);

void checkLine(void);

void checkPistonSurfce(void);

void checkPistonPrelude(void);

void checkPistonPark(void);

void checkPistonAscent(void);

void continuousProfile(void);

void runTimer(int);

void writeBytes(String);

long updateTime(void);

void setup(void);

void loop(void);


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


void checkLine(void){
  //start the timer, once  the mission time is ~200ms more than the first reading, continue to check the hardware lines
  long time;
  Timer1.start();
  long timeLast = Timer1.read();
  int i = 0;
  for(i = 0; i < 100000; i++){
    time = Timer1.read();
    if (time > (timeLast + 199999)){
      Timer1.stop();
      break;
    }
  }
  
  //if the request line is still high, choose message 1 (get serial number / firmware rev) 
  if(digitalRead(2)==HIGH){
    interruptMessage = SERNO;
  }
  
  //if the request line is low, check the other two lines
  else if(digitalRead(2)==LOW){
    
    //if the mode line is high, choose message 2 (get pts)
    if(digitalRead(3)==HIGH){
      interruptMessage = PTS;
    }
    
    //if the mode line (3) is low, and the Rx line (19) is high (check with debounce to be safe)\
    //choose message 3 (get pt)
    else if((debounce(19)>0)&&(debounce(3)<0)){
      interruptMessage = PT;
    }
    
    //if the mode line (3) is low, and the Rx line (19) is low (check with debounce to be safe)
    //choose message 4 (get p)
    else if((debounce(19)<0)&&(debounce(3)<0)){//get p
      interruptMessage = P;
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

void setup(void){
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


void loop(void){
  
  digitalWrite(8,HIGH);
  
  //if in continuous profiling mode
  if(cpMode==1){
    //once a second
    if(millis()%1000==1){
      
      //perform all functions associated with continuous profiling (getting reading
      //every second based on real-time output and comparing it to p cut off)
      continuousProfile();
    }
  }
    
  //interruptMessage will be zero unless changed during the ISR, this section of code
  //will handle if there is an interrupt as well as checking for the piston position
  //changing the phase
  
  //if interrupt message has not changed
  switch(interruptMessage){
    case 0:
      //if the simulator is running a mission
      if(missionMode >= 100){
          
          //check the piston position every 15sec
          if((millis())%15000 < 1){
            
            //handle the piston position changing
            switch(currentPhase){
              
              
              case SURFACE:
                checkPistonSurface();
                break;
                
              //handle the float descending for a short park or leaving park earlier than anticipated
              //by checking the piston position every 15 sceonds, this will start 2min into park
              case PARK:
                if(missionTime>(parkDescentTime+120000)){
                  checkPistonPark();
                }
                break;
              
              //handle the float detecting ice by checking the piston position every 15 seconds during the ascent
              //once the pressure is less than 60dbar
              case ASCENTTOSURFACE:
                if((parkPressure - float(float((missionTime-downTime-ascentToPark))*(parkPressure)/float(ascentToSurface))) < 60){
                  checkPistonAscent();
                }
                break;
            } 
          }
        }
        else{
          if(prelude == 1){
            if((millis())%15000 < 1){
            //handle the float descending at any point during the designated prelude time by checking
            //for a decreasing piston position every 15 seconds, this will start 15min into prelude
              checkPistonPrelude();
            }
          }
        }
        
      break;
    
    //if interrupt message is 1, convert the global string msg to the global byte 
    //array cmdMode then send the array over Serial1, reset interruptMessage to 0  
    case SERNO:
      if(cpMode == -1){
        Serial.println("SERNO");
        delay(1240);
        writeBytes(msg);
        detachInterrupt(0);
        interruptMessage = 0;
      }
      break;
    
    //if it is 2, clear any junk analog values on A0 before getting the p,t,s value based on the analog 
    //value on pin A0, set msg2 equal to this value, then convert the global string msg2 to the global 
    //byte array pts, then send the array over Serial1, reset interruptMessage to 0
    case PTS:
      Serial.println("PTS");
      analogRead(A0);
      if(missionMode < 100){
        msg2 = getReadingFromPiston(PTS);
      }
      else if(missionMode >= 100){
        msg2 = getDynamicReading(PTS);
      }
      writeBytes(msg2);
      interruptMessage = 0;
      break;
  
    //if it is 3, clear any junk analog values on A0 before getting the p,t value based on the analog
    //value on pin A0, set msg3 eqaul to this value, then convert the global string msg3 to the global 
    //byte array pt, then send the array over Serial1, reset interruptMessage to 0
    case PT:
      Serial.println("PT");
      analogRead(A0);
      if(missionMode < 100){
        msg3 = getReadingFromPiston(PT);
      }
      else if(missionMode >= 100){
        msg3 = getDynamicReading(PT);
      }
      writeBytes(msg3);
      interruptMessage = 0;
      break;

    //if it is 4, clear any junk analog values on A0 before getting the p value based on the analog
    //value on pin A0, set msg4 eqaul to this value, then convert the global string msg4 to the global 
    //byte array p, then send the array over Serial1, reset interruptMessage to 0
    case P:
      Serial.println("P");
      analogRead(A0);
      if(missionMode < 100){
        msg4 = getReadingFromPiston(P);
      }
      else if(missionMode >= 100){
        msg4 = getDynamicReading(P);
      }
      writeBytes(msg4);
      interruptMessage = 0;
      break;
  }
  

  //check for a message in Serial1, if there is, create a blank string, then add each character in the 
  //Serial1 input buffer to the input string. Wait until a carriage return to make sure a command
  //is actually sent, if it is not the carriage return, wait for the next character, 3 sec. timeout
  if(Serial1.available()>0){
    Serial.println("found serial");
    String input = "";
    int start;
    while(1){
      if(Serial1.available()>0){
        start = millis();  
        char temp;
        temp = char(Serial1.read());
        input+=temp;
        if((temp=='\r')||(input.equals("startprofile"))||(input.equals(PARKDESCENTTIME))
                       ||(input.equals(PARKPRESSURE))||(input.equals(DOWNTIME))||(input.equals(DEEPPROFILEDESCENTTIME))
                       ||(input.equals(DEEPPROFILEPRESSURE))||(input.equals(ASCENTTIMEOUT))||(input.equals(PRELUDETIME))
                       ||(input.equals(MISSIONTIME))||(input.equals("ascentRate="))||(input.equals("id@"))
                       ||(input.equals("ic@"))||(input.equals("show "))){
          break;
        }
      }
      int timeout = millis();
      
      //wait 3 seconds for the new character, if not found, time out
      if((timeout-start)>3000){
        break;
      }
    }
    
    //if the input is a carriage return, send back the sbe command prompt (S>)
    if((input.equals("\r"))&&(cpMode!=1)){
        String cmdMode = "\r\nS>";
        writeBytes(cmdMode);
    }
    
    //if the input is the i*l command, send back a list of the mission parameters
    else if((input.equals("i*l\r"))||(input.equals("i*l"))){
      updateTime();
      String m_config = "\r\n"+ String(preludeDisplay) +" Prelude (minutes): Mtp<val>"
      "\r\n"+ String(parkPressure) +" Park Pressure (dbar): Mk<val>"
      "\r\n"+ String(parkDescentTimeDisplay) +" Park Descent Time (minutes): Mtk<val>"
      "\r\n"+ String(downTimeDisplay) +" Down Time(minutes): Mtd<val>"
      "\r\n"+ String(deepProfilePressure) +" Deep Profile Pressure: Mj<val>"
      "\r\n"+ String(deepProfileDescentTimeDisplay) +" Deep Profile Descent Time(minutes): Mtj<val>"
      "\r\n"+ String(ascentTimeOutDisplay) +" Ascent Time Out(minutes): Mta<val>"
      "\r\n"+ String(missionTimeDisplay) +" Mission Time(seconds): i*t<val>"
      "\r\nS>";
      int m_configLen = m_config.length()+1;
      byte m_configBuff[750];
      m_config.getBytes(m_configBuff, m_configLen);
      Serial1.write(m_configBuff, m_configLen);
    }
    
    //if the input is the i*s command, send back a list of the times for the phases
    //in seconds
    else if((input.equals("i*s\r"))||(input.equals("i*s"))){
      updateTime();
        
      String listParams = "\r\nMission Time: "+String(missionTime/1000) +
      "\r\nPhase of mission cycle: "+ phase[currentPhase] +
      "\r\nPark Descent: " + String(((parkDescentTime)/1000)) +
      "\r\nPark: " + String(((downTime-deepProfileDescentTime)/1000)) +
      "\r\nDeep Descent: " + String(((downTime)/1000)) +
      "\r\nAscent: " + String(((downTime+ascentTime)/1000)) + 
      "\r\nS>";
      writeBytes(listParams);
    }
    
    //if the input is the e command, start the mission
    else if((input.equals("e\r"))||(input.equals("e"))){
      String start = "\r\nstart mission\r\nS>";
      writeBytes(start);
      prelude = 1;
      currentPhase = PRELUDE;
    }
    
    else if(input.equals("start\r")){
      String manualStart = "\r\nmanual start activated\r\nS>";
      writeBytes(manualStart);
      lastUpdate = millis() - 80000;
      
      missionMode+=110;
      
      prelude = -1;
    }
 
    //if the input is the k command, end the mission and reset all of the parameters
    else if((input.equals("k\r"))||(input.equals("k"))){
      String m_end = "\r\nmission ended\r\nS>";
      writeBytes(m_end);
      missionMode = 0;
      currentPhase = PRESSUREACTIVATION;
      parkDescentTime = 18000000;
      parkDescentTimeDisplay = 300;
      parkPressure = 1000;
      downTime = 86400000;
      downTimeDisplay = 1440;
      deepProfileDescentTime = 18000000;
      deepProfileDescentTimeDisplay = 300;
      deepProfilePressure = 2000;
      ascentTimeOut = 36000000;
      ascentTimeOutDisplay = 600;
      ascentTime = 25000000;
      ascentToPark = 12500000;
      ascentToSurface = 12500000;
      missionTime = 0;
      missionTimeDisplay=0;
    }
    
    //if the input is mtk, check the Serial port for the value to be used as the park descent time,
    //calculate the value of the park descent time in milliseconds, then send the value of park descent time as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if((input.equals(PARKDESCENTTIME))||(input.equals(PARKDESCENTTIME2))){
      while(1){
        if(Serial1.available()>0){
          parkDescentTimeDisplay = getInput();
          parkDescentTime=(parkDescentTimeDisplay*60000);
          parkDescentTimeCopy = parkDescentTime;
          break;
        }
      }
      String parkDescentTimeStr = "\r\nS>parkDescentTime="+String(parkDescentTimeDisplay)+"\r\nS>";
      writeBytes(parkDescentTimeStr);
    }
    
    //if the input is mtp, check the Serial port for the value to be used as the park pressure,
    //calculate the value of the park pressure in dbar, then send the value of park pressure as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if((input.equals(PRELUDETIME))||(input.equals(PRELUDETIME2))){
      while(1){
        if(Serial1.available()>0){
          preludeDisplay=getInput();
          break;
        }
      }
      String preludeStr = "\r\nS>prelude="+String(preludeDisplay)+"\r\nS>";
      writeBytes(preludeStr);
    }
    
    //if the input is mk, check the Serial port for the value to be used as the park pressure,
    //calculate the value of the park pressure in dbar, then send the value of park pressure as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if((input.equals(PARKPRESSURE))||(input.equals(PARKPRESSURE2))){
      while(1){
        if(Serial1.available()>0){
          parkPressure = getInput();
          break;
        }
      }
      String parkPressureStr = "\r\nS>parkPressure="+String(parkPressure)+"\r\nS>";
      writeBytes(parkPressureStr);
    }
    
    //if the input is mtd, check the Serial port for the value to be used as the down time,
    //calculate the value of the down time in milliseconds, then send the value of down time as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if((input.equals(DOWNTIME))||(input.equals(DOWNTIME2))){
      while(1){
        if(Serial1.available()>0){
          downTimeDisplay = getInput();
          downTime=(downTimeDisplay*60000);
          downTimeCopy = downTime;
          break;
        }
      }
      String downTimeStr = "\r\nS>downTime="+String(downTimeDisplay)+"\r\nS>";
      writeBytes(downTimeStr);
    }

    //if the input is mtj, check the Serial port for the value to be used as the
    //deep profile descent time, calculate the value of the deep profile descent time in milliseconds,
    //then send the value of deep profile descent time as a series of bytes, confirm that the value 
    //has actually changed by using the global variable value in this echo
    else if((input.equals(DEEPPROFILEDESCENTTIME))||(input.equals(DEEPPROFILEDESCENTTIME2))){
      while(1){
        if(Serial1.available()>0){
          deepProfileDescentTimeDisplay = getInput();
          deepProfileDescentTime = (deepProfileDescentTimeDisplay*60000);
          deepProfileDescentTimeCopy = deepProfileDescentTime;
          
          break;
        }
      }
      String deepProfileDescentTimeStr = "\r\nS>deepProfileDescentTime="+String(deepProfileDescentTimeDisplay)+"\r\nS>";
      writeBytes(deepProfileDescentTimeStr);
    }
    
    //if the input is mj, check the Serial port for the value to be used as the deep profile pressure,
    //calculate the value of the deep profile pressure in dbar, then send the value of deep profile pressure as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if((input.equals(DEEPPROFILEPRESSURE))||(input.equals(DEEPPROFILEPRESSURE2))){
      while(1){
        if(Serial1.available()>0){
          deepProfilePressure = getInput();
          ascentTime = deepProfilePressure*100000/ascentRate;
          ascentTimeCopy = ascentTime;
          ascentToSurface = float(float(parkPressure)/float(deepProfilePressure))*ascentTime;
          ascentToSurfaceCopy = ascentToSurface;
          ascentToPark = ascentTime - ascentToSurface;
          ascentToParkCopy = ascentToPark;
          break;
        }
      }
      String deepProfilePressureStr = "\r\nS>deepProfilePressure="+String(deepProfilePressure)+"\r\nS>";
      writeBytes(deepProfilePressureStr);
    }
    
    //if the input is mta, check the Serial port for the value to be used as the ascent timeout,
    //calculate the value of the ascent timeout in milliseconds, then send the value of mission time as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if(((input.equals(ASCENTTIMEOUT))||input.equals(ASCENTTIMEOUT2))){
      while(1){
        if(Serial1.available()>0){
          ascentTimeOutDisplay = getInput();
          ascentTimeOut=(ascentTimeOutDisplay*60000);
          ascentTimeOutCopy = ascentTimeOut;
          break;
        }
      }
      String ascentTimeOutStr = "\r\nS>ascentTimeOut="+String(ascentTimeOutDisplay)+"\r\nS>";
      writeBytes(ascentTimeOutStr);
    }

    //if the input is i*t, check the Serial port for the value to be used as the mission time,
    //calculate the value of the mission time in milliseconds, then send the value of mission time as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if(input.equals(MISSIONTIME)){
      while(1){
        if(Serial1.available()>0){
          newTimeDisplay = getInput();
          newTime = (newTimeDisplay*1000);
          Serial.println(String(newTimeDisplay));
          if(newTime >= 0){
            phaseChange = PRELUDE;
            if(newTime > 0){
              phaseChange = PARKDESCENT;
              if(newTime>(parkDescentTime)){
                phaseChange = PARK;
                if(newTime>(downTime-deepProfileDescentTime)){
                  phaseChange = DEEPDESCENT;
                  if(newTime>(downTime)){
                    phaseChange = ASCENTTOPARK;
                    if(newTime>(downTime+ascentToPark)){
                      phaseChange = ASCENTTOSURFACE;
                      if(newTime>(downTime+ascentTime)){
                        phaseChange = SURFACE;
                      }
                    }
                  }
                }
              }
            }
          }
          Serial.println(phaseChange);
          Serial.println(currentPhase);
          if(phaseChange==currentPhase){
            missionTimeDisplay = newTimeDisplay;
            missionTime=(newTimeDisplay*1000);
            lastUpdate = millis();
            String missionTimeStr = "\r\nS>missionTime="+String(missionTimeDisplay)+" ("+String((missionTimeDisplay/60))+" minutes)\r\nS>";
            writeBytes(missionTimeStr);
            break;  
          }
          else{
            String no = "\r\nS>Cannot move time to a different phase";
            writeBytes(no);
            break;
          }
        }
      }
    }
    
    //if the input is ascentRate=, check the Serial port for the value to be used as the ascent rate,
    //then send the value of mission time as a series of bytes, confirm that the value has actually
    //changed by using the global variable value in this echo
    else if(input.equals("ascentRate=")){
      while(1){
        if(Serial1.available()>0){
          ascentRate = getInput();
          ascentTime = deepProfilePressure*100000/ascentRate;
          ascentTimeCopy = ascentTime;
          ascentToSurface = float(float(parkPressure)/float(deepProfilePressure))*ascentTime;
          ascentToSurfaceCopy = ascentToSurface;
          ascentToPark = ascentTime - ascentToSurface;
          ascentToParkCopy = ascentToPark;
          break;
        }
      }
      String ascentRateStr = "\r\nS>ascentRate="+pressureToString(ascentRate)+" cm/s\r\nS>";
      writeBytes(ascentRateStr);
    }
    
    //if the input is show , check the Serial port for the value to be used as the mission time,
    //calculate the value of the mission time in milliseconds, then send the value of mission time as 
    //a series of bytes, confirm that the value has actually changed by using the global variable value 
    //in this echo
    else if(input.equals("show ")){
      updateTime();
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
          String str; 
          if(input.equals("missionTime")){
           str= "\r\nS>"+input+"="+String(missionTime)+
           "\r\nS>"+input+"="+String(missionTimeCopy);
          }
          else if(input.equals("parkPressure")){
           str= "\r\nS>"+input+"="+String(parkPressure);
          }
          else if(input.equals("deepProfilePressure")){
           str= "\r\nS>"+input+"="+String(deepProfilePressure);
          }
          else if(input.equals("parkDescentTime")){
           str= "\r\nS>"+input+"="+String(parkDescentTime)+
           "\r\nS>"+input+"="+String(parkDescentTimeCopy);
          }
          else if(input.equals("downTime")){
           str= "\r\nS>"+input+"="+String(downTime)+
           "\r\nS>"+input+"="+String(downTimeCopy);
          }
          else if(input.equals("prelude")){
           str= "\r\nS>"+input+"="+String(preludeDisplay);
          }
          else if(input.equals("deepProfileDescentTime")){
           str= "\r\nS>"+input+"="+String(deepProfileDescentTime)+
           "\r\nS>"+input+"="+String(deepProfileDescentTimeCopy);
          }
          else if(input.equals("ascentTimeOut")){
           str= "\r\nS>"+input+"="+String(ascentTimeOut)+
           "\r\nS>"+input+"="+String(ascentTimeOut);
          }
          writeBytes(str);
          break;
        }
      }
      missionMode++;
    }
    
    
    //if the input is the ds command, send back all of the information as a series of bytes (uses generic
    //info based on an actual seabird, can edit field in this string if necessary), there should be 3 
    //fields that will vary: the number of bins, number of samples, and whether it is expecting only P
    //or pts for real time output
    else if(((input.equals("ds\r"))||(input.equals("ds")))&&(cpMode!=1)){
      if(maxPress!=0){
        if(minPress!=10000){
        count = ((maxPress-minPress)*100)/ascentRate;
        }
      }
      nBins = (int(maxPress)/2)+1;
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
    else if(((input.equals("dc\r"))||(input.equals("dc")))&&(cpMode!=1)){
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
      updateTime();
      //reinitalize values
      maxPress = 0;
      minPress = 10000;
      nBins = 0;
      da = -1;
      inc = 0;
      count = 0;
      cpMode = 1;
      String cp = "\r\nS>startprofile";
      String cp2 = "\r\nprofile started, pump delay = 0 seconds\r\nS>";
      writeBytes(cp);
      delay(500);
      writeBytes(cp2);
      attachInterrupt(0, checkLine, RISING);
    }
    
    //if the input is stopprofile, recognize that it is the stop profile command,
    //then send back that the profile has stopped, ignore the external interrupt
    //on pin2, and turn off continuous profiling mode
    else if(input.equals("stopprofile\r")){
      cpMode = -1;
      String exitcp = "profile stopped";
      writeBytes(exitcp);
    }
    
    //if the input is binaverage, return the values parsed from the data sent
    //during continuous profiling mode. set da to 1 which will allow for the
    //da command to be run (makes sure there is actual data to dump when requested)
    else if((input.equals("binaverage\r"))&&(cpMode!=1)){
      if(maxPress!=0){
        if(minPress!=10000){
          count = ((maxPress-minPress)*100)/ascentRate;
        }
      }
      nBins = (int(maxPress)/2) + 1;
      String countStr2 = String(count);
      String maxPressStr = pressureToString(maxPress);
      String nBinsStr2 = String(nBins);
      String binavg = "\r\nS>binaverage\r\nsamples = "+countStr2+", maxPress = "+maxPressStr+"\r\nrd: 0\r\navg: 0\r\n\r\ndone, nbins = "+nBinsStr2+"\r\nS>";
      writeBytes(binavg);
      da = 1;
    }

    //if the inpt is da, send bins in the format "p, t, s, b" (pressure,
    //temperature, salinity, number of samples) over Serial1. then send that 
    //the upload is done. then reinitialize all of the global variables used 
    //for bin averaging and dumping the values        
    else if(((input.equals("da\r"))||(input.equals("da")))&&(cpMode!=1)){
      first = 1;
      int ii;
      String data = "da\r\n";
      writeBytes(data);
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
      inc = 0;
    }
    
    //if the inpt is da, send bins in the format "p, t, s, b" (pressure,
    //temperature, salinity, number of samples) over Serial1. then send that 
    //the upload is done. then reinitialize all of the global variables used 
    //for bin averaging and dumping the values        
    else if(((input.equals("dah\r"))||(input.equals("dah")))&&(cpMode!=1)){
      first = 1;
      int ii;
      String data = "dah\r\n";
      writeBytes(data);
      //send all of the samples over serial
      for(ii=0; ii < nBins; ii++){
        if(ii == nBins - 1){
          last = 1;
        }
        String bin = binaverageHex();
        writeBytes(bin);
      }
      
      //send upload complete at end of all samples
      String complete = "\r\nupload complete\r\nS>";
      writeBytes(complete);
      inc = 0;
      
    }
    
    else if(((input.equals("dd\r"))||(input.equals("dd")))&&(cpMode!=1)){
      String rawData ="dd\r\n" ;
      String append = "";
      int d;
      float aPressure;
      if(maxPress!=0){
        if(minPress!=10000){
          count = ((maxPress-minPress)*100)/ascentRate;
        }
      }
      float increment = (maxPress-minPress)/count;
      float temperature;
      float salinity; 
      float midway;
      for(d = count; d >= 0; d--){
        aPressure = d*increment;
        //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
        if((iceAvoidance == ICEDETECT)&&(aPressure < 50)){
          midway = float((50-icePressure)/2) + icePressure;
          temperature = (aPressure-midway)/midway - 1.8;
        }
        
        //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
        else if((iceAvoidance == ICECAP)&&(aPressure < 20)){
          midway = float((20-icePressure)/2) + icePressure;
          temperature = (aPressure-midway)/midway - 1.8;
        }
        
        //ice breakup mode, need a temp of > -1.78 C the whole way up
        else if((iceAvoidance == ICEBREAKUP)&&(aPressure <55)){
          temperature = 23.2-float(aPressure*0.0088);
        }
        
        //calculate temperature normally
        else{ 
          temperature = 23.2-float(aPressure*0.0088);
        }
        
        //calculate salinity normally
        salinity = temperature*0.1 + 34.9;
        if(pOrPTSsel==0){
          append = pressureToString(aPressure)+"\r\n";
        }
        else if(pOrPTSsel==1){
          append = pressureToString(aPressure)+", "+tempOrSalinityToString(temperature)+", "+tempOrSalinityToString(salinity)+"\r\n";
        }
        rawData+=append;
      }
      rawData+="upload complete\r\nS>";
      writeBytes(rawData);
    } 
    
    //if the input is qsr, send back that the seabird is powering down as a series of bytes 
    //(the simulator will just stay on and wait for the next interaction with the APFx)
    else if(((input.equals("qsr\r"))||(input.equals("qsr")))&&(cpMode!=1)){
      //delay(200);
      String cmdMode = "\r\nS>qsr\r\npowering down\r\nS>";
      writeBytes(cmdMode);
      delay(700);
      attachInterrupt(0, checkLine, RISING);
      
    }
    
    //if the input is autobinavg=n, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("autobinavg=n\r"))&&(cpMode!=1)){
      delay(10);
      String aba = "\r\nS>autobinavg=n";
      writeBytes(aba);
    }
    
    //if the input is pcutoff=2.0, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("pcutoff=2.0\r"))&&(cpMode!=1)){
      delay(10);
      String pcutoff = "\r\nS>pcutoff=2.0";
      writeBytes(pcutoff);
    }
    
    //if the input is outputpts=y, send back the command prompt and echo the input as a series of bytes
    //and change pOrPTSsel to 1 so that the ds command will display pts
    else if((input.equals("outputpts=y\r"))&&(cpMode!=1)){
      delay(10);
      String optsy = "\r\nS>outputpts=y";
      writeBytes(optsy);
      pOrPTSsel = 1;
    }
    
    //if the input is tswait=20, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("tswait=20\r"))&&(cpMode!=1)){
      delay(10);
      String tsw = "\r\nS>tswait=20";
      writeBytes(tsw);
    }
    
    //if the input is top_bin_interval=2, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("top_bin_interval=2\r"))&&(cpMode!=1)){
      delay(10);
      String tbi = "\r\nS>top_bin_interval=2";
      writeBytes(tbi);
    }
    
    //if the input is top_bin_size=2, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("top_bin_size=2\r"))&&(cpMode!=1)){
      delay(10);
      String tbs = "\r\nS>top_bin_size=2";
      writeBytes(tbs);
    }
    
    //if the input is top_bin_max=10, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("top_bin_max=10\r"))&&(cpMode!=1)){
      delay(10);
      String tbm = "\r\nS>top_bin_max=10";
      writeBytes(tbm);
    }
    
    //if the input is middle_bin_interval=2, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("middle_bin_interval=2\r"))&&(cpMode!=1)){
      delay(10);
      String mbi = "\r\nS>middle_bin_interval=2";
      writeBytes(mbi);
    }
    
    //if the input is middle_bin_size=2, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("middle_bin_size=2\r"))&&(cpMode!=1)){
      delay(10);
      String mbs = "\r\nS>middle_bin_size=2";
      writeBytes(mbs);
    }
    
    //if the input is middle_bin_max=20, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("middle_bin_max=20\r"))&&(cpMode!=1)){
      delay(10);
      String mbm = "\r\nS>middle_bin_max=20";
      writeBytes(mbm);
    }
    
    //if the input is bottom_bin_interval=2, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("bottom_bin_interval=2\r"))&&(cpMode!=1)){
      delay(10);
      String bbi = "\r\nS>bottom_bin_interval=2";
      writeBytes(bbi);
    }
    
    //if the input is bottom_bin_size=2, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("bottom_bin_size=2\r"))&&(cpMode!=1)){
      delay(10);
      String bbs = "\r\nS>bottom_bin_size=2";
      writeBytes(bbs);
    }
    
    //if the input is includetransitionbin=n, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("includetransitionbin=n\r"))&&(cpMode!=1)){
      delay(10);
      String itb = "\r\nS>includetransitionbin=n";
      writeBytes(itb);
    }
    
    //if the input is includenbin=y, send back the command prompt and echo the input as a series of bytes
    else if((input.equals("includenbin=y\r"))&&(cpMode!=1)){
      delay(10);
      String ib = "\r\nS>includenbin=y";
      writeBytes(ib);
    }
    
    //if the input is outputpts=n, send back the command prompt and echo the input as a series of bytes
    //and set pOrPTSsel to 0 so that the ds command will display p only
    else if((input.equals("outputpts=n\r"))&&(cpMode!=1)){
      delay(10);
      String optsn = "\r\nS>outputpts=n";
      writeBytes(optsn);
      pOrPTSsel = 0;
    }
    
    else if(input.equals("constantP=")){
      while(1){
        if(Serial1.available()>0){
          constantP=getInput();
        }
      }
      if(constantP==OFF){
        constant = -1;
      }
      else{
        constant = 1;
      }
    }
    
    //if the input is id, send back that the seabird is in ice detect mode as a series of bytes 
    //change the global variable ice avoidance to 1, which is detect mode. also assume ice will
    //be detected at 20 dbar
    else if((input.equals("id\r"))||(input.equals("id"))||(input.equals("id on\r"))||(input.equals("id on"))){
      iceAvoidance = ICEDETECT;
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
          icePressure=getInput();
          break;
        }
      }
      iceAvoidance = ICEDETECT;
      String icedaMode = "\r\nice detect mode on, will detect ice at "+String(icePressure)+"dbar\r\nS>";
      writeBytes(icedaMode);
    }
    
    //if the input is ic, send back that the seabird is in ice cap mode as a series of bytes 
    //change the global variable ice avoidance to 2, which is cap mode
    else if((input.equals("ic\r"))||(input.equals("ic"))||(input.equals("ic on\r"))||(input.equals("ic on"))){
      icePressure = 4;
      iceAvoidance = ICECAP;
      String icecMode = "\r\nice cap mode on\r\nS>";
      writeBytes(icecMode);
    }
    
    //if the input is ic@<val>, send back that the seabird is in ice cap mode as a series of bytes 
    //change the global variable ice avoidance to 2, which is cap mode. also set ice to be detected 
    //at the given input pressure
    else if(input.equals("ic@")){
      while(1){
        if(Serial1.available()>0){
          icePressure=getInput();
          break;
        }
      }
      iceAvoidance = ICECAP;
      String icecaMode = "\r\nice cap mode on, will detect ice at "+String(icePressure)+"dbar\r\nS>";
      writeBytes(icecaMode);
    }
    
    //if the input is ib, send back that the seabird is in ice breakup mode as a series of bytes 
    //change the global variable ice avoidance to 3, which is breakup mode
    else if((input.equals("ib\r"))||(input.equals("ib"))||(input.equals("ib on\r"))||(input.equals("ib on"))){
      iceAvoidance = ICEBREAKUP;
      String icebMode = "\r\nice breakup mode on\r\nS>";
      writeBytes(icebMode);
    }
    
    //if the input is id off, send back that ice detect mode is off as a series of bytes 
    //change the global variable ice avoidance to -1, which is normal mode
    else if(input.equals(("id off\r"))||(input.equals("id off"))){
      iceAvoidance = NOICE;
      String icedModeOff = "\r\nice detect mode off\r\nS>";
      writeBytes(icedModeOff);
      icePressure=20;
    }
    
    //if the input is ic off, send back that ice cap mode is off as a series of bytes 
    //change the global variable ice avoidance to -1, which is normal mode
    else if(input.equals(("ic off\r"))||(input.equals("ic off"))){
      iceAvoidance = NOICE;
      String icecModeOff = "\r\nice cap mode off\r\nS>";
      writeBytes(icecModeOff);
      icePressure=20;
    }
    
    //if the input is ib off, send back that ice breakup mode is off as a series of bytes 
    //change the global variable ice avoidance to -1, which is normal mode
    else if(input.equals(("ib off\r"))||(input.equals("ib off"))){
      iceAvoidance = NOICE;
      String icebModeOff = "\r\nice breakup mode off\r\nS>";
      writeBytes(icebModeOff);
    }
    
    //if the input is ?, list the simulation type and all of the options for commands
    else if((input.equals("?\r"))||(input.equals("?"))){
      String list = "?\r\nAPF-9 & APF-11 Iridium SBE41cp Simulator"
      "\r\nid"
      "\r\nid@<value>"
      "\r\nid off"
      "\r\nic"
      "\r\nic@<value>"
      "\r\nic off"
      "\r\nib"
      "\r\nib off"
      "\r\nqsr"
      "\r\nda"
      "\r\nds"
      "\r\ndc\r\nS>";
      writeBytes(list);
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
/* to choose which string (PTS, PT, or P) to send to the APF board.      */
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
  
  //convert the float to a string using the pressureToString function
  if(cpMode == 1){
    if(pressure >= maxPress){
      maxPress = pressure;
    }
    if(pressure <= minPress){
      minPress = pressure;
    }
  }
  
  if(constant==1){
    pressure = float(constantP);
  }
  
  pStr = pressureToString(pressure);
  
  //determine if in ice detect, ice cap, ice breakup, or normal mode
  //then calculate temperature based on criteria
  
  //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
  if((iceAvoidance == ICEDETECT)&&(pressure < 50)){
    float midway = float((50-icePressure)/2) + icePressure;
    temperature = float((pressure-midway)/midway) - 1.8;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == ICECAP)&&(pressure < 20)){
   float midway = float((20-icePressure)/2) + icePressure;
   temperature = float((pressure-midway)/midway) - 1.8;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == ICEBREAKUP)&&(pressure <50)){
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
  
  if((cpMode==1)&&(pOrPTSsel==1)){
    select=PTS;
  }
  else if((cpMode==1)&&(pOrPTSsel==0)){
    select=P;
  }
  //choose which string you want to return
  switch(select){
    case PTS:
      sendMessage = ptsStr;
      break;
    case PT:
      sendMessage = ptStr;
      break;
    case P:
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

String getDynamicReading(int select){
  
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
  
  //calculate a pressure based on the current phase
  switch(currentPhase){
    case PRELUDE:
      pressure = 0;
      break;
    case PARKDESCENT:
      pressure = float(float(missionTime)*(parkPressure)/float(parkDescentTime));
      if((iceAvoidance == 1)||(iceAvoidance == 2)){
        pressure = icePressure + float(float(missionTime)*(parkPressure)/float(parkDescentTime));
      }
      break;
    case PARK:
      pressure = parkPressure;
      break;
    case DEEPDESCENT:
      pressure = parkPressure + float(float((missionTime-(downTime-deepProfileDescentTime)))*(deepProfilePressure-parkPressure)/float(deepProfileDescentTime));
      break;
    case ASCENTTOPARK:
      pressure = deepProfilePressure - float(float((missionTime-downTime))*(deepProfilePressure-parkPressure)/float(ascentToPark));
      break;
    case ASCENTTOSURFACE:
      pressure = parkPressure - float(float((missionTime-(downTime+ascentToPark)))*(parkPressure)/float((ascentToSurface)));
      break;
    case SURFACE:
      pressure = 0;
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
  
  if(constant == 1){
    pressure = float(constantP);
  }
  
  pStr = pressureToString(pressure);
  
  //determine if in ice detect, ice cap, ice breakup, or normal mode
  //then calculate temperature based on criteria
  
  //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
  if((iceAvoidance == ICEDETECT)&&(pressure < 50)){
    float midway = float((50-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == ICECAP)&&(pressure < 20)){
    float midway = float((20-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == ICEBREAKUP)&&(pressure <55)){
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
  
  if((cpMode==1)&&(pOrPTSsel==1)){
    select=PTS;
  }
  else if((cpMode==1)&&(pOrPTSsel==0)){
    select=P;
  } 
 
  //choose which string you want to return
  switch(select){
    case PTS:
      sendMessage = ptsStr;
      break;
    case PT:
      sendMessage = ptStr;
      break;
    case P:
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

String binaverage(void){
  
//string containing the p,t,s,b values
  String returnStr;
  
  //float values
  float pressure = 0;
  float temperature;
  float salinity;
  
  int samplesUsed;
  
  //create a pressure value that increases by approximately 2
  pressure += (inc*2);
  
  //determine if in ice detect, ice cap, ice breakup, or normal mode
  //then calculate temperature based on criteria
  
  //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
  if((iceAvoidance == ICEDETECT)&&(pressure < 50)){
    float midway = float((50-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == ICECAP)&&(pressure < 20)){
    float midway = float((20-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == ICEBREAKUP)&&(pressure <55)){
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
   if(maxPress!=0){
      if(minPress!=10000){
        count = ((maxPress-minPress)*100)/ascentRate;
      }
    }
    first = -1;
  }
  
  samplesUsed = 200/ascentRate + random(0,5) - random(0,5);
  
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
    
  returnStr = " "+pressureToString(pressure)+", "+ tempOrSalinityToString(temperature)+", "+ tempOrSalinityToString(salinity)+", "+ samplesUsed+"\r\n";
  
  //return the string
  return returnStr;
}



/*************************************************************************/
/*                            binaverageHex                              */
/*                            *************                              */
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

String binaverageHex(void){
  
  //string containing the p,t,s,b values
  String returnStr;
  
  //float values
  float pressure = 0;
  float temperature;
  float salinity;
  
  int samplesUsed;
  
  //create a pressure value that increases by approximately 2
  pressure += (inc*2);
  
  //determine if in ice detect, ice cap, ice breakup, or normal mode
  //then calculate temperature based on criteria
  
  //ice detect mode, need median temp of <= -1.78 C for 20-50dbar range
  if((iceAvoidance == ICEDETECT)&&(pressure < 50)){
    float midway = float((50-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == ICECAP)&&(pressure < 20)){
    float midway = float((20-icePressure)/2) + icePressure;
    temperature = (pressure-midway)/midway - 1.8;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == ICEBREAKUP)&&(pressure <55)){
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
   if(maxPress!=0){
      if(minPress!=10000){
        count = ((maxPress-minPress)*100)/ascentRate;
      }
    }
    first = -1;
  }
  
  samplesUsed = 200/ascentRate + random(0,5) - random(0,5);
  
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
  
  if(samplesUsed!=0){
    //create the string to be returned in the format:
    //"pppp.pp, tt.tttt, ss.ssss, bb"
    int pressureInt = pressure*10;
    int temperatureInt = temperature*100;
    int salinityInt = salinity*100;
    String pressureHex = String(pressureInt, HEX);
    String temperatureHex = String(temperatureInt, HEX);
    String salinityHex = String(salinityInt, HEX);
    String samplesHex = String(samplesUsed, HEX);
    pressureHex.toUpperCase();
    temperatureHex.toUpperCase();
    salinityHex.toUpperCase();
    samplesHex.toUpperCase();
    if(samplesHex.length()<2){
      samplesHex = '0'+samplesHex;
    }
    while(pressureHex.length()<4){
      pressureHex = '0'+pressureHex;
    }
    while(temperatureHex.length()<4){
      temperatureHex = '0'+temperatureHex;
    }
    while(salinityHex.length()<4){
      salinityHex = '0'+salinityHex;
    }
    
    returnStr = pressureHex+temperatureHex+salinityHex+samplesHex+"\n\r";
  }
  if((pressure <= minPress)||(samplesUsed==0)){
    returnStr = "00000000000000\n\r";
  }
  
  
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
/* returns: a long integer value that represents the mission time in the */
/*                 mission                                               */
/*                                                                       */
/* This function updates the mission time of the mission and determines  */
/* which part of the mission the simulator should be in, currently, the  */
/* default order is prelude, park descent, park, deep descent, ascent,   */
/* surface mode, then repeat                                             */
/*                                                                       */
/*************************************************************************/

long updateTime(void){
  
  Serial.println("update");
  
  //int that will be used if millis() rolls over
  int rollover;
  
  //update the last phase
  lastPhase = currentPhase;
  
  //only update time while in a mission
  if(missionMode >= 100){
    
    //if surface mode, don't update time either
    if(currentPhase == SURFACE){
      if(missionTime <  (downTime + ascentTime)){
        currentPhase = PRELUDE;
      } 
    }
    
    //in any other part of the mission
    else{
      
      update = millis();
      
      //overflow has occurred
      if(update < lastUpdate){
        
        //create an interval of time that is equal to the difference
        //between the last update and the largest value possible
        rollover = 2147483647-lastUpdate;
        
        //add it to the value of the update, then add it to missionTime
        missionTime += (update+rollover);
        missionTimeDisplay +=((update+rollover)/1000);
      }
      
      //overflow has not occurred
      else{
        
        //update both the mission time and mission time display by adding
        //the update interval to both
        missionTime += (update - lastUpdate);
        missionTimeDisplay += ((update - lastUpdate)/1000);
      }
      
      //set the value of lastUpdate to now
      lastUpdate = update;
      
      //the initial value of currentPhase is PRELUDE (-1)
      currentPhase = PRELUDE;
      
      //if the mission time is past the time it takes for the prelude
      if(missionTime>0){
        
        //the current phase is park descent (0)
        currentPhase = PARKDESCENT;
        
        //if the mission time is past the time it takes for the prelude and 
        //the park descent
        if(missionTime>=parkDescentTime){
          
          //the current phase is park (1)
          currentPhase = PARK;
          
          //if the mission time is past the time it takes for prelude, park
          //descent, and park (downtime - deep profile time = park descent + park)
          if(missionTime>=(downTime-deepProfileDescentTime)){
            
            //the current phase is deep descent (2)
            currentPhase = DEEPDESCENT;
            
            //if the mission time is past the time it takes for prelude, park
            //descent, park, and deep descent (down time + prelude)
            if(missionTime>=downTime){
              
              //the current phase is ascent to park (3), the ascent is split into
              //two phases to account for missions that do not go into deep 
              //descent and ascent straight from park
              currentPhase = ASCENTTOPARK;
              
              //if the mission time is past the time it takes for prelude, park
              //descent, park, deep descent, and half of the ascent (down time 
              //+prelude + 1/2ascent)
              if(missionTime>downTime+ascentToPark){
                
                //the current phase is ascent to surface (4), this second half of
                //ascent will bring the float from park to the surface or from the 
                //first half of the ascent to the surface
                currentPhase = ASCENTTOSURFACE;
                
                //if the mission time is past the time it takes for all of the
                //phases to be complete
                if(missionTime>=(downTime+ascentTime)){
                  
                  //it is in surface / telemetry mode
                  currentPhase = SURFACE;
                }
              }
            }  
          }
        }
      }
    }
  }
  if(lastPhase!=currentPhase){
    String phaseChange = phase[lastPhase]+" -> "+phase[currentPhase];
    Serial.println(phaseChange);
  }
  Serial.println(phase[currentPhase]);
  return missionTime;
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
  Serial1.flush();
  int strLen = aString.length()+1;
  byte strBuffer[550];
  aString.getBytes(strBuffer, strLen);
  Serial1.write(strBuffer, strLen);
}



/*************************************************************************/
/*                            continuousProfile                          */
/*                            *****************                          */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: none                                                         */
/*                                                                       */
/* This function writes a P or PTS reading once every second while in cp */
/* mode based on the whether it is in mission mode and whether the       */
/* real-time output is set to P or PTS, also handle turning off          */
/* continuous profiling if the pressure is less than p cut off           */
/*                                                                       */
/*************************************************************************/

void continuousProfile(void){

  float aPressure;
  
  //calculate a float value for the pressure as well as send a reading over serial
  //as a series of bytes
  
  //if in mission
  if(missionMode>=100){
    
    //if the phase is ascent to park
    if(currentPhase==ASCENTTOPARK){
      
      //calculate the pressure based on the ascent to park algorithm
      aPressure = deepProfilePressure - float(float((missionTime-downTime))*(deepProfilePressure-parkPressure)/float(ascentToPark));
    }
          
    //if the phase is ascent to surface
    else if(currentPhase==ASCENTTOSURFACE){
      
      //calculate the pressure based on the ascent to surface algorithm
      aPressure = parkPressure - float(float((missionTime-(downTime+ascentToPark)))*(parkPressure)/float((ascentToSurface)));
    }
    
    //send a reading over serial
    
    //if real-time output is P only
    if(pOrPTSsel==0){
      writeBytes(getDynamicReading(P));
    }
    
    //if real-time output is PTS
    else{
      writeBytes(getDynamicReading(PTS));
    }
  }
  
  //if not in a mission
  else{
    
    //calculate the pressure based on the piston position
    int voltage = analogRead(A0);
  
    //technically out of range, but use it to go to a pressure greater than 2000dbar, min change = 5dbar
    if(voltage<72){
      aPressure = 2000+1*(voltage-72);
    }
    
    //for pressures between 2000-1000dbar, 72 = 2000dbar, 293 = 1000dbar, min change = 4.5045dbar
    else if((voltage>=72)&&(voltage<294)){
      aPressure = ((4.5045)*(222-(voltage-72)))+1000.00;
    }
    
    //for pressures between 1000-500dbar, 294 = 1000dbar, 453 = 500dbar, min change = 3.125dbar
    else if((voltage>=294)&&(voltage<454)){
      aPressure = ((3.125)*(160-(voltage-294)))+500.00;
    }
    
    //for pressures between 500-0dbar, 454 = 500dbar, 1023 = 0dbar, min change = 0.878dbar
    else if((voltage>=454)&&(voltage<1024)){
      aPressure = ((0.878)*(569-(voltage-454)));
    }
    
    //send a reading over serial
    
    //if real-time output is P only
    if(pOrPTSsel==0){
      writeBytes(getReadingFromPiston(P));
    }
    
    //if real-time output is PTS
    else{
      writeBytes(getReadingFromPiston(PTS));
    }
  }
     
  //if the pressure calculated through the desired algorithm is less than 2 (which
  //is the default p cut off), exit continuous profiling
  if(aPressure < 2){
    cpMode = -1;
    String exitcp = "profile stopped";
    writeBytes(exitcp);
  }
}


/*************************************************************************/
/*                           checkPistonSurface                          */
/*                           ******************                          */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: none                                                         */
/*                                                                       */
/* This function determines whether the simulator should leave prelude   */
/* based on whether the piston position is decreasing. If it is, set the */
/* prelude equal to the current mission time. This will effectively end  */
/* prelude phase in terms of calculating in the code. Also, set the last */
/* update to the timer value. Last, reset the values of last postion and */
/* current position if the phase is changing.                            */
/*                                                                       */
/*************************************************************************/

void checkPistonSurface(void){
  updateTime();
  
  //get the value of the potentiometer
  currentPosition = analogRead(A0);
  Serial.println(String(currentPosition));
  
  //wait until there are 2 readings to compare
  if(lastPosition!=0){
    
    //if it is descending
    if(currentPosition < lastPosition - 5){
      
      //set it to park descent phase
      currentPhase = PARKDESCENT;
      
      //reset all of the parameters to their values from the 
      //start of the mission by setting them equal to their copy values
      //and get rid of the prelude by setting the prelude time = 0
      preludeDisplay = 0;
      missionTime = 0;
      missionTimeDisplay = 80;
      parkDescentTime = parkDescentTimeCopy;
      parkDescentTimeDisplay = parkDescentTime/60000;
      downTime = downTimeCopy;
      downTimeDisplay = downTime/60000;
      deepProfileDescentTime = deepProfileDescentTimeCopy;
      deepProfileDescentTimeDisplay = deepProfileDescentTime/60000;
      ascentTimeOut = ascentTimeOutCopy;
      ascentTime = ascentTimeCopy;
      ascentTimeOutDisplay = ascentTimeOut/60000;
       
      //set last update
      lastUpdate = millis() - 80000;
      
      //reset values of currentPosition and lastPosition
      currentPosition = 0;
    }
  }
  lastPosition=currentPosition;
}

/*************************************************************************/
/*                             checkPistonPark                           */
/*                             ***************                           */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: none                                                         */
/*                                                                       */
/* This function determines whether the simulator should leave prelude   */
/* based on whether the piston position is decreasing. If it is, set the */
/* prelude equal to the current mission time. This will effectively end  */
/* prelude phase in terms of calculating in the code. Also, set the last */
/* update to the timer value. Last, reset the values of last postion and */
/* current position if the phase is changing.                            */
/*                                                                       */
/*************************************************************************/

void checkPistonPark(void){
  updateTime();
  analogRead(A0);
  currentPosition= analogRead(A0);
  Serial.println(String(currentPosition));
  if(lastPosition!=0){
    
    //if it is ascending
    if(currentPosition > lastPosition + 5){
      
      //store the normal value of downTime to be reset later
      downTimeCopy = downTime;
      
      //store the normal value of deepProfileDescentTime to be reset later
      deepProfileDescentTimeCopy = deepProfileDescentTime;
      
      //store the normal value of ascentToPark to be reset later
      ascentToParkCopy = ascentToPark;
      
      //set deepProfileDescentTime to 0 so that the phase is skipped for this cycle
      deepProfileDescentTime = 0;
      deepProfileDescentTimeCopy = 0;
      
      //set ascentToPark to 0 so that the phase is skipped for this cycle
      ascentToPark = 0;
      
      //set downTime 
      downTime = missionTime-20000;
      downTimeDisplay = downTime/60000;
      
      lastUpdate = millis();
    }
    
    //if it is descending
    else if(currentPosition < lastPosition - 5){
      
      //store the normalValue of downTime to be reset later
      downTimeCopy = downTime;
      
      //set downTime
      downTime = missionTime+deepProfileDescentTime - 15000;
      downTimeDisplay = downTime/60000;
            
      //set last update
      lastUpdate = millis();
      
      //reset values of currentPosition and lastPosition
      currentPosition = 0;
    }
  }
  lastPosition=currentPosition;
}


/*************************************************************************/
/*                           checkPistonAscent                           */
/*                           *****************                           */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: none                                                         */
/*                                                                       */
/* This function determines whether the simulator should leave ascent    */
/* based on whether the piston position is decreasing. If it is, reset   */
/* all of the mission parameters. This functionality is in place to      */
/* simulate the behaviour during ice avoidance. Reset all of the         */
/* parameters by setting them equal to their copies. Also, set the last  */
/* update to the timer value. Last, reset the values of last postion and */
/* current position if the phase is changing.                            */
/*                                                                       */
/*************************************************************************/
void checkPistonAscent(void){
  updateTime();
  currentPosition = analogRead(A0);
  Serial.println(String(currentPosition));
  if(lastPosition!=0){
    
    //if it is descending
    if(currentPosition < lastPosition - 5){
      
      //reset all parameters, but shorten descent time
      //will handle pressure during park by adding offset
      //in getDynamicReading function
      preludeDisplay = 0;
      missionTime = 0;
      missionTimeDisplay = 0;
      parkDescentTime = ((parkPressure-icePressure)/parkPressure)*parkDescentTimeCopy;
      parkDescentTimeDisplay = parkDescentTime/60000;
      downTime = downTimeCopy;
      downTimeDisplay = downTime/60000;
      deepProfileDescentTime = deepProfileDescentTimeCopy;
      deepProfileDescentTimeDisplay = deepProfileDescentTime/60000;
      ascentTimeOut = ascentTimeOutCopy;
      ascentTimeOutDisplay = ascentTimeOut/60000;
       
      //set last update
      lastUpdate = millis();
      
      //reset values of currentPosition and lastPosition
      currentPosition = 0;
    }
  }
  lastPosition = currentPosition;
}


/*************************************************************************/
/*                           checkPistonPrelude                          */
/*                           ******************                          */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: none                                                         */
/*                                                                       */
/* This function determines whether the simulator should leave prelude   */
/* based on whether the piston position is decreasing. If it is, set the */
/* prelude equal to the current mission time. This will effectively end  */
/* prelude phase in terms of calculating in the code. Also, set the last */
/* update to the timer value. Last, reset the values of last postion and */
/* current position if the phase is changing.                            */
/*                                                                       */
/*************************************************************************/

void checkPistonPrelude(void){
  updateTime();
  currentPosition = analogRead(A0);
  if(lastPosition!=0){
    
    //if it is descending
    if(currentPosition < lastPosition - 5){
      
      //set last update
      lastUpdate = millis() - 85000;
      
      missionMode+=110;
      
      prelude = -1;
      
      //reset values of currentPosition and lastPosition
      currentPosition = 0;
    }
  }
  lastPosition = currentPosition;
}

/*************************************************************************/
/*                                getInput                               */
/*                                ********                               */
/*                                                                       */
/* parameters: none                                                      */
/*                                                                       */
/* returns: a long integer value that represents the value entered       */
/*                                                                       */
/* This funciton waits for the user to enter a value, it will be used to */
/* allow the user to change mission parameters such as ice detection and */
/* mission depth and timing.                                             */
/*                                                                       */
/*************************************************************************/

long int getInput(void){
  long int inputInt;
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
  if(input=="off\r"){
    inputInt=OFF;
  }
  else{
    inputInt = input.toInt();
  }
  return inputInt;
}

