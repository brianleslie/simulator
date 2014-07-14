/*************************************************************************/
/*                            finished_code_9.ino                        */
/*                            *******************                        */
/*                                                                       */
/* Written by: Sean P. Murphy                                            */
/*                                                                       */
/* APF-9 differs from APF-11 in the way it exits continuous profiling    */
/* mode.                                                                 */
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
/* iceAvoidance: an int that represents which ice avoidance protocol is  */
/*                  in effect, -1:none, 1:detect, 2:cap, 3:breakup       */
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

String msg = "SBE 41CP UW. V 2.0";
int msgLen = msg.length()+1;
byte cmdMode[100];

String msg2;
int msg2Len;
byte pts[150];

String msg3;
int msg3Len;
byte pt[100];

String msg4;
int msg4Len;
byte p[100];

String cMode = "\r\nS>";
int cModeLen;
byte cModeBuffer[100];

String pOrPTS[2] = {"P only", "PTS"};

int pOrPTSsel = 1;

int iceAvoidance = -1;

/*************************************************************************/
/*                            function prototypes                        */
/*                            *******************                        */
/*                                                                       */
/*************************************************************************/

String getReadingFromPiston(int);

String pressureToString(float);

String tempOrSalinityToString(float);

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
      delay(1300);
      cModeLen = cMode.length()+1;
      cMode.getBytes(cModeBuffer, cModeLen);
      Serial1.write(cModeBuffer, cModeLen);
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
      Serial.println(msg2);
      delay(500);
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
    
    //enter the while loop to stay in command mode
    while(commandMode == 1){
      
      /*************************************************************************/
      /*                      continuous profiling mode                        */
      /*************************************************************************/
        
      //contiuous profiling mode is turned on by the startprofile command over serial
      if(cpMode == 1){
        
        attachInterrupt(0, checkLine, RISING);
        
        while(cpMode == 1){
          
          //only take sample once every 1 sec (delay .95 sec)
          //start the timer, once  the current time is ~200ms more than the first reading, continue to check the hardware lines
          long time;
          Timer1.start();
          long timeLast = Timer1.read();
          int i = 0;
          for(i = 0; i < 100000; i++){
            time = Timer1.read();
            if (time > (timeLast + 949900)){
              Timer1.stop();
              break;
            }
          }
          
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
            String exitcp = "profile stopped";
            int exitcpLen = exitcp.length()+1;
            byte exitcpBuffer[100];
            exitcp.getBytes(exitcpBuffer, exitcpLen);
            Serial1.write(exitcpBuffer, exitcpLen);
            detachInterrupt(0);
            cpMode = -1;
          }
          
          //leave continuous profiling mode if the stopprofile command is received
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
            if(input.equals("stopprofile")){
              String exitcp = "profile stopped";
              int exitcpLen = exitcp.length()+1;
              byte exitcpBuffer[100];
              exitcp.getBytes(exitcpBuffer, exitcpLen);
              Serial1.write(exitcpBuffer, exitcpLen);
              detachInterrupt(0);
              cpMode = -1;
            }
          }
        }
      }
      
      /*************************************************************************/
      /*                     end continuous profiling mode                     */
      /*************************************************************************/
        
      
      //check for a message in Serial1, it there is, create a blank string, then add each character in the 
      //Serial1 input buffer to the input string. Wait until a carriage return to make sure a command
      //is actually sent, if it is not the carriage return, wait fƒor the next character
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
        //the command prompt should also be returned during the config command as a response to each
        //one of the parameters being set, also need to handle the configuring of pts output during
        //cp mode (the very last "outputpts=y" control statement), this will also return the command prompt 
        if(input.equals("\r")){
          String cmdMode = "\r\nS>";
          int cmdModeLen = cmdMode.length()+1;
          byte cmdModeBuffer[100];
          cmdMode.getBytes(cmdModeBuffer, cmdModeLen);
          Serial1.write(cmdModeBuffer, cmdModeLen);
        }
        else if(input.equals("pcutoff=2.0\r")){
          delay(10);
          String pcutoff = "\r\nS>pcutoff=2.0";
          int pcutoffLen = pcutoff.length()+1;
          byte pcutoffBuffer[100];
          pcutoff.getBytes(pcutoffBuffer, pcutoffLen);
          Serial1.write(pcutoffBuffer, pcutoffLen);
        }
        else if(input.equals("outputpts=y\r")){
          delay(10);
          String optsy = "\r\nS>outputpts=y";
          int optsyLen = optsy.length()+1;
          byte optsyBuffer[100];
          optsy.getBytes(optsyBuffer, optsyLen);
          Serial1.write(optsyBuffer, optsyLen);
          pOrPTSsel = 1;
        }
        else if(input.equals("tswait=20\r")){
          delay(10);
          String tsw = "\r\nS>tswait=20";
          int tswLen = tsw.length()+1;
          byte tswBuffer[100];
          tsw.getBytes(tswBuffer, tswLen);
          Serial1.write(tswBuffer, tswLen);
          pOrPTSsel = 1;
        }
        
        //if the input is the ds command, send back all of the information as a series of bytes (uses generic
        //info based on an actual seabird, can edit field in this string if necessary)
        else if(input.equals("ds\r")){
          String ds = "\r\nSBE 41CP UW V 2.0  SERIAL NO. 4242"
          "\r\nfirmware compilation date: 18 December 2007 09:20"
          "\r\nstop profile when pressure is less than = 2.0 decibars"
          "\r\nautomatic bin averaging at end of profile disabled"
          "\r\nnumber of samples = "+String(count)+
          "\r\nnumber of bins = "+String(nBins)+
          "\r\ntop bin interval = 2"
          "\r\ntop bin size = 2"
          "\r\ntop bin max = 10"
          "\r\nmiddle bin interval = 2"
          "\r\nmiddle bin size = 2"
          "\r\nmiddle bin max = 20"
          "\r\nbottom bin interval = 2"
          "\r\nbottom bin size = 2"
          "\r\ndo not include two transition bins"
          "\r\ninclude samples per bin"
          "\r\npumped take sample wait time = 20 sec"
          "\r\nreal-time output is "+pOrPTS[pOrPTSsel]+"\r\nS>";
          int dsLen = ds.length()+1;
          byte dsBuffer[1000];
          ds.getBytes(dsBuffer, dsLen);
          Serial1.write(dsBuffer, dsLen);
        }
        
        //if the input is the dc command, send back all of the information as a series of bytes (uses generic
        //info based on an actual seabird (can edit field in this string if necessary)
        else if(input.equals("dc\r")){
          String dc = "\r\nSBE 41CP UW V 2.0  SERIAL NO. 4242"
          "\r\ntemperature:  19-dec-10    "
          "\r\nTA0 =  4.882851e-05    "
          "\r\nTA1 =  2.747638e-04    "
          "\r\nTA2 = -2.478284e-06    "
          "\r\nTA3 =  1.530870e-07"
          "\r\nconductivity:  19-dec-10    "
          "\r\nG = -1.013506e+00    "
          "\r\nH =  1.473695e-01    "
          "\r\nI = -3.584262e-04    "
          "\r\nJ =  4.733101e-05    "
          "\r\nCPCOR = -9.570001e-08    "
          "\r\nCTCOR =  3.250000e-06    "
          "\r\nWBOTC =  2.536509e-08"
          "\r\npressure S/N = 3212552, range = 2900 psia:  14-dec-10    "
          "\r\nPA0 =  6.297445e-01    "
          "\r\nPA1 =  1.403743e-01    "
          "\r\nPA2 = -3.996384e-08    "
          "\r\nPTCA0 =  6.392568e+01    "
          "\r\nPTCA1 =  2.642689e-01    "
          "\r\nPTCA2 = -2.513274e-03    "
          "\r\nPTCB0 =  2.523900e+01    "
          "\r\nPTCB1 = -2.000000e-04    "
          "\r\nPTCB2 =  0.000000e+00    "
          "\r\nPTHA0 = -7.752968e+01    "
          "\r\nPTHA1 =  5.141199e-02    "
          "\r\nPTHA2 = -7.570264e-07    "
          "\r\nPOFFSET =  0.000000e+00"
          "\r\nS>";
          int dcLen = dc.length()+1;
          byte dcBuffer[1000];
          dc.getBytes(dcBuffer, dcLen);
          Serial1.write(dcBuffer, dcLen);
        }
        
        //if the input is startprofile, recognize that it is the start profile command,
        //then send back that the profile has started, reattach interrupt to pin2, and 
        //turn on continuous profiling mode
        else if(input.equals("startprofile")){
          String cp = "\r\nS>startprofile\r\nprofile started, pump delay = 0 seconds\r\nS>";
          int cpLen = cp.length()+1;
          byte cpBuffer[100];
          cp.getBytes(cpBuffer, cpLen);
          Serial1.write(cpBuffer, cpLen);
          cpMode = 1;
        }
        
        //if the input is stopprofile, recognize that it is the stop profile command,
        //then send back that the profile has stopped, ignore the external interrupt
        //on pin2, and turn off continuous profiling mode
        else if(input.equals("stopprofile")){
          String exitcp = "profile stopped";
          int exitcpLen = exitcp.length()+1;
          byte exitcpBuffer[100];
          exitcp.getBytes(exitcpBuffer, exitcpLen);
          Serial1.write(exitcpBuffer, exitcpLen);
          detachInterrupt(0);
          cpMode = -1;
        }
        
        //if the input is binaverage, return the values parsed from the data sent
        //during continuous profiling mode. set da to 1 which will allow for the
        //da command to be run (makes sure there is actual data to dump when requested)
        else if(input.equals("binaverage\r")){
          digitalWrite(8, LOW);
          nBins = (int(maxPress)/2) + 1;
          String binavg = "\n\rS>binaverage\n\rsamples = "+String(count)+", maxPress = "+pressureToString(maxPress)+"\n\rrd: 0\n\ravg: 0\n\n\rdone, nbins = "+String(nBins)+"\n\rS>";
          int binavgLen = binavg.length()+1;
          byte binavgBuffer[100];
          binavg.getBytes(binavgBuffer, binavgLen);
          Serial1.write(binavgBuffer, binavgLen);
          delay(200);
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
            int binLen = bin.length()+1;
            byte binBuffer[100];
            bin.getBytes(binBuffer, binLen);
            Serial1.write(binBuffer, binLen);
          }
          
          //send upload complete at end of all samples
          String complete = "\r\nupload complete\r\nS>";
          int completeLen = complete.length()+1;
          byte completeBuffer[100];
          complete.getBytes(completeBuffer, completeLen);
          Serial1.write(completeBuffer, completeLen);
          
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
          int cmdModeLen = cmdMode.length()+1;
          byte cmdModeBuffer[100];
          cmdMode.getBytes(cmdModeBuffer, cmdModeLen);
          Serial1.write(cmdModeBuffer, cmdModeLen);
          commandMode = -1;
          delay(100);
          attachInterrupt(0, checkLine, RISING);
        }
        
        else if(input.equals("autobinavg=n\r")){
          delay(10);
          String aba = "\r\nS>autobinavg=n";
          int abaLen = aba.length()+1;
          byte abaBuffer[100];
          aba.getBytes(abaBuffer, abaLen);
          Serial1.write(abaBuffer, abaLen);
        }
        else if(input.equals("top_bin_interval=2\r")){
          delay(10);
          String tbi = "\r\nS>top_bin_interval=2";
          int tbiLen = tbi.length()+1;
          byte tbiBuffer[100];
          tbi.getBytes(tbiBuffer, tbiLen);
          Serial1.write(tbiBuffer, tbiLen);
        }
        else if(input.equals("top_bin_size=2\r")){
          delay(10);
          String tbs = "\r\nS>top_bin_size=2";
          int tbsLen = tbs.length()+1;
          byte tbsBuffer[100];
          tbs.getBytes(tbsBuffer, tbsLen);
          Serial1.write(tbsBuffer, tbsLen);
        }
        else if(input.equals("top_bin_max=10\r")){
          delay(10);
          String tbm = "\r\nS>top_bin_max=10";
          int tbmLen = tbm.length()+1;
          byte tbmBuffer[100];
          tbm.getBytes(tbmBuffer, tbmLen);
          Serial1.write(tbmBuffer, tbmLen);
        }
        else if(input.equals("middle_bin_interval=2\r")){
          delay(10);
          String mbi = "\r\nS>middle_bin_interval=2";
          int mbiLen = mbi.length()+1;
          byte mbiBuffer[100];
          mbi.getBytes(mbiBuffer, mbiLen);
          Serial1.write(mbiBuffer, mbiLen);
        }
        else if(input.equals("middle_bin_size=2\r")){
          delay(10);
          String mbs = "\r\nS>middle_bin_size=2";
          int mbsLen = mbs.length()+1;
          byte mbsBuffer[100];
          mbs.getBytes(mbsBuffer, mbsLen);
          Serial1.write(mbsBuffer, mbsLen);
        }
        else if(input.equals("middle_bin_max=20\r")){
          delay(10);
          String mbm = "\r\nS>middle_bin_max=20";
          int mbmLen = mbm.length()+1;
          byte mbmBuffer[100];
          mbm.getBytes(mbmBuffer, mbmLen);
          Serial1.write(mbmBuffer, mbmLen);
        }
        else if(input.equals("bottom_bin_interval=2\r")){
          delay(10);
          String bbi = "\r\nS>bottom_bin_interval=2";
          int bbiLen = bbi.length()+1;
          byte bbiBuffer[100];
          bbi.getBytes(bbiBuffer, bbiLen);
          Serial1.write(bbiBuffer, bbiLen);
        }
        else if(input.equals("bottom_bin_size=2\r")){
          delay(10);
          String bbs = "\r\nS>bottom_bin_size=2";
          int bbsLen = bbs.length()+1;
          byte bbsBuffer[100];
          bbs.getBytes(bbsBuffer, bbsLen);
          Serial1.write(bbsBuffer, bbsLen);
        }
        else if(input.equals("includetransitionbin=n\r")){
          delay(10);
          String itb = "\r\nS>includetransitionbin=n";
          int itbLen = itb.length()+1;
          byte itbBuffer[100];
          itb.getBytes(itbBuffer, itbLen);
          Serial1.write(itbBuffer, itbLen);
        }
        else if(input.equals("includenbin=y\r")){
          delay(10);
          String ib = "\r\nS>includenbin=y";
          int ibLen = ib.length()+1;
          byte ibBuffer[100];
          ib.getBytes(ibBuffer, ibLen);
          Serial1.write(ibBuffer, ibLen);
        }
        else if(input.equals("outputpts=n\r")){
          delay(10);
          String optsn = "\r\nS>outputpts=n";
          int optsnLen = optsn.length()+1;
          byte optsnBuffer[100];
          optsn.getBytes(optsnBuffer, optsnLen);
          Serial1.write(optsnBuffer, optsnLen);
          pOrPTSsel = 0;
        }
        
        //if the input is id, send back that the seabird is in ice detect mode as a series of bytes 
        //change the global variable ice avoidance to 1, which is detect mode
        else if((input.equals("id\r"))||(input.equals("id on\r"))){
          iceAvoidance = 1;
          String icedMode = "\r\nice detect mode on\r\nS>";
          int icedModeLen = icedMode.length()+1;
          byte icedModeBuffer[100];
          icedMode.getBytes(icedModeBuffer, icedModeLen);
          Serial1.write(icedModeBuffer, icedModeLen);
        }
        
        //if the input is ic, send back that the seabird is in ice cap mode as a series of bytes 
        //change the global variable ice avoidance to 2, which is cap mode
        else if((input.equals("ic\r"))||(input.equals("ic on\r"))){
          iceAvoidance = 2;
          String icecMode = "\r\nice cap mode on\r\nS>";
          int icecModeLen = icecMode.length()+1;
          byte icecModeBuffer[100];
          icecMode.getBytes(icecModeBuffer, icecModeLen);
          Serial1.write(icecModeBuffer, icecModeLen);
        }
        
        //if the input is ib, send back that the seabird is in ice breakup mode as a series of bytes 
        //change the global variable ice avoidance to 3, which is breakup mode
        else if((input.equals("ib\r"))||(input.equals("ib on\r"))){
          iceAvoidance = 1;
          String icebMode = "\r\nice breakup mode on\r\nS>";
          int icebModeLen = icebMode.length()+1;
          byte icebModeBuffer[100];
          icebMode.getBytes(icebModeBuffer, icebModeLen);
          Serial1.write(icebModeBuffer, icebModeLen);
        }
        
        //if the input is id off, send back that ice detect mode is off as a series of bytes 
        //change the global variable ice avoidance to -1, which is normal mode
        else if(input.equals("id off\r")){
          iceAvoidance = -1;
          String iceModeOff = "\r\nice detect mode off\r\nS>";
          int iceModeOffLen = iceModeOff.length()+1;
          byte iceModeOffBuffer[100];
          iceModeOff.getBytes(iceModeOffBuffer, iceModeOffLen);
          Serial1.write(iceModeOffBuffer, iceModeOffLen);
        }
        
        //if the input is ic off, send back that ice cap mode is off as a series of bytes 
        //change the global variable ice avoidance to -1, which is normal mode
        else if(input.equals("ic off\r")){
          iceAvoidance = -1;
          String icecModeOff = "\r\nice cap mode off\r\nS>";
          int icecModeOffLen = icecModeOff.length()+1;
          byte icecModeOffBuffer[100];
          icecModeOff.getBytes(icecModeOffBuffer, icecModeOffLen);
          Serial1.write(icecModeOffBuffer, icecModeOffLen);
        }
        
        //if the input is ib off, send back that ice breakup mode is off as a series of bytes 
        //change the global variable ice avoidance to -1, which is normal mode
        else if(input.equals("ib off\r")){
          iceAvoidance = -1;
          String icebModeOff = "\r\nice breakup mode off\r\nS>";
          int icebModeOffLen = icebModeOff.length()+1;
          byte icebModeOffBuffer[100];
          icebModeOff.getBytes(icebModeOffBuffer, icebModeOffLen);
          Serial1.write(icebModeOffBuffer, icebModeOffLen);
        }
        
        //if the input is build send back that the simulator is setup for APF-9 simulation
        //by returning APF-9 as a series of bytes of serial
        else if(input.equals("build\r")){
          String build = "\r\nAPF-9\r\nS>";
          int buildLen = build.length()+1;
          byte buildBuffer[100];
          build.getBytes(buildBuffer, buildLen);
          Serial1.write(buildBuffer, buildLen);
        }
        
        else{
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
  if((iceAvoidance == 1)&&(pressure < 55)){
    temperature = -2.00;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == 2)&&(pressure < 20)){
    temperature = -2.00;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == 3)&&(pressure <55)){
    temperature = 23.2-float(pressure*0.0175)-float(0.000000002*pressure*pressure*pressure);
  }
  
  //calculate temperature normally
  else{ 
    temperature = 23.2-float(pressure*0.0175)-float(0.000000002*pressure*pressure*pressure);
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
/*                           pressureToString                            */
/*                           ****************                            */
/*                                                                       */
/* parameters: aFloat, a float value representing the float that is      */
/*                 going to be converted to a string                     */
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
  floatDec = floatLong - (floatInt*100);
 
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
/* returns: an string value that will represent the float as a string    */
/*                                                                       */
/* This function creates a string that will look like a float by         */
/* splitting it into its whole and decimal parts, then adding them as    */
/* two strings with the appropriate formatting for a temperature or      */
/* salinity value.
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
  if((iceAvoidance == 1)&&(pressure < 55)){
    temperature = -2.00;
  }
  
  //ice cap mode, need a temp of <= -1.78 C for surface (or after 20dbar)
  else if((iceAvoidance == 2)&&(pressure < 20)){
    temperature = -2.00;
  }
  
  //ice breakup mode, need a temp of > -1.78 C the whole way up
  else if((iceAvoidance == 3)&&(pressure <55)){
    temperature = 23.2-float(pressure*0.0175)-float(0.000000002*pressure*pressure*pressure);
  }
  
  //calculate temperature normally
  else{ 
    temperature = 23.2-float(pressure*0.0175)-float(0.000000002*pressure*pressure*pressure);
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
