#ifndef SBE41_H
#define SBE41_H (0x0008U)

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * $Id: sbe41.c,v 1.33 2010/03/22 15:39:15 swift Exp $
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* Copyright University of Washington.   Written by Dana Swift.
 *
 * This software was developed at the University of Washington using funds
 * generously provided by the US Office of Naval Research, the National
 * Science Foundation, and NOAA.
 *  
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
/** RCS log of revisions to the C source code.
 *
 * \begin{verbatim}
 * $Log: sbe41.c,v $
 * Revision 1.33  2010/03/22 15:39:15  swift
 * Updates to the nan.c to avoid conflicts with native C lib functions.
 *
 * Revision 1.32  2010/03/21 00:11:29  swift
 * Modify Sbe41EnterCmdMode() to work with various reply formats.
 *
 * Revision 1.31  2008/07/25 14:11:02  swift
 * Modifications to API to account for SeaBird's new controller and firmware.
 *
 * Revision 1.30  2008/05/05 15:04:48  swift
 * API modifications for use with socketed-eeprom SBE controller and FwRev 3.0.
 *
 * Revision 1.29  2008/03/20 20:56:54  swift
 * Merged code for Sbe43i.  Added functions to configure the SBE41 and to query
 * the SBE41 for configuration information.
 *
 * Revision 1.28  2007/07/16 21:38:41  swift
 * Added author and funding attribution to the main comment section.
 *
 * Revision 1.27  2006/09/27 19:19:46  swift
 * Changed the way that the pump-time for IDO CTDs is computed.
 *
 * Revision 1.26  2006/04/21 13:37:41  swift
 * Changed copyright attribute.
 *
 * Revision 1.25  2005/08/17 23:00:08  swift
 * Modifications to accomodate reimplemented low-level activation of P, PT,
 * PTS, and PTSO samples.
 *
 * Revision 1.24  2005/06/02 15:15:46  swift
 * Modified the pedantic regex for O2 to accomodate the SBE43i.
 *
 * Revision 1.23  2005/05/01 21:39:46  swift
 * Implemented an API to the low-power PT sampling feature of the SBE41.
 *
 * Revision 1.22  2005/01/20 20:10:22  swift
 * Fixed bugs in chat() that caused garbage characters to be output to console.
 *
 * Revision 1.21  2005/01/04 21:16:07  swift
 * Modifications of enabling/disabling conio and ctdio.
 *
 * Revision 1.20  2005/01/04 19:27:44  swift
 * Modified regex pattern for SBE41 serial number recognition.
 *
 * Revision 1.19  2005/01/04 01:32:08  swift
 * Simplify the Sbe41ExitCmdMode() function and fixed some trouble spots with
 * ConioEnable() and CtdEnableIo().
 *
 * Revision 1.18  2005/01/03 19:40:14  swift
 * Added a function Sbe41FwRev() to extract the firmware revision from an SBE41.
 *
 * Revision 1.17  2004/12/29 19:47:32  swift
 * Added facility to use the SBE41's low-power PT mode.
 *
 * Revision 1.16  2004/12/09 01:20:02  swift
 * Modified LogEntry() to use strings stored in the CODE segment.  This saves
 * lots of space in the DATA segment and significantly speeds code start-up.
 *
 * Revision 1.15  2004/11/02 19:43:16  swift
 * Fixed garbage-producing log entries.
 *
 * Revision 1.14  2004/10/29 23:40:56  swift
 * Modifications to print log-entries to the console during CTD operation.
 *
 * Revision 1.13  2004/06/07 21:20:55  swift
 * Enable console IO in Sbe41Pt().
 *
 * Revision 1.12  2004/05/03 18:07:08  swift
 * Modifications to make Sbe41GetPt() to work correctly.
 *
 * Revision 1.11  2004/04/23 23:43:37  swift
 * Changes to SbeLogCal() to account for differing baud rates between ports 1 & 2.
 *
 * Revision 1.10  2003/12/20 19:48:12  swift
 * Fixed a regex bug that ignored negative signs in STP measurements and caused
 * the parse results to be always non-negative.
 *
 * Revision 1.9  2003/12/17 20:42:55  swift
 * Included statically linked special version of chat() in order to work around
 * timing problems caused by the 20mA current loop serial interface.
 *
 * Revision 1.8  2003/12/03 00:17:35  swift
 * Modifications to chat() arguments to account for changes in chat() protocol.
 *
 * Revision 1.7  2003/11/20 18:59:42  swift
 * Added GNU Lesser General Public License to source file.
 *
 * Revision 1.6  2003/11/12 22:24:28  swift
 * Added Sbe43fPumpTime() to compute the SBE43F pump-time for the adaptive
 * pumping scheme.
 *
 * Revision 1.5  2003/10/21 17:59:43  swift
 * Changed TimeOut period of Sbe41GetPtso() from 30 seconds to 60 seconds to
 * accomodate SeaBird's change in protocol for the SBE43F oxygen sensor.
 *
 * Revision 1.4  2003/07/16 23:52:23  swift
 * Implement the master/slave model in a more robust way especially when in
 * communications mode.  Also, IEEE definitions of NaN were exploited in order
 * to write and use functions that set and detect NaN and Inf.
 *
 * Revision 1.3  2003/07/05 21:11:13  swift
 * Eliminated 'Sbe41LogCal()' because it would require too large of a FIFO
 * buffer for the serial port.  The console serial port of the APF9 is limited
 * to 4800 baud while the CTD serial port operates at 9600 baud.  Since no
 * hardware or software handshaking is implemented then the FIFO has to buffer
 * the differing rates.
 *
 * In 'Sbe41InitiateSample()', some of the code was refactored away from being
 * hardware independent toward being hardware dependent.  This was a concession
 * to reliability and robustness that was caused by the slow processing speed
 * of the APF9.  Its prior factorization suffered from too many function calls
 * to low-level hardware dependent primitives.  These calls threw off timing
 * enough to reduce the reliability communications and sequencing of SBE41
 * control.
 *
 * Revision 1.2  2003/07/03 22:47:46  swift
 * Major revisions.  This revision is not yet stable.
 *
 * Revision 1.1  2003/06/29 01:36:06  swift
 * Initial revision
 * \end{verbatim}
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#define sbe41ChangeLog "$RCSfile: sbe41.c,v $ $Revision: 1.33 $ $Date: 2010/03/22 15:39:15 $"

#include <serial.h>

/* function prototypes */
int Sbe41Config(int PtPump);
int Sbe41EnterCmdMode(void);
int Sbe41ExitCmdMode(void);
int Sbe41FwRev(char *buf,unsigned int bufsize);
int Sbe41GetP(float *p);
int Sbe41GetPt(float *p, float *t);
int Sbe41GetPts(float *p, float *t, float *s);
int Sbe41GetPtso(float *p, float *t, float *s, float *o);
int Sbe41Status(unsigned int *serno,int *ptpump, int *density, int *delay);
int Sbe41LogCal(void);
int Sbe41SerialNumber(void);
int Sbe43Config(int *Ido);
int Sbe43Status(float *Ns,float *Nf,float *Tau20,int *Ido);
time_t Sbe43PumpTime(float p, float t, float Tau1P, int N);

/* define the return states of the SBE41 API */
extern const char Sbe41ChatFail;        /* Failed chat attempt. */
extern const char Sbe41NoResponse;      /* No response received from SBE41. */
extern const char Sbe41RegExceptn;      /* Response received, regexec() exception */
extern const char Sbe41NullArg;         /* Null function argument. */
extern const char Sbe41RegexFail;       /* response received, regex no-match */
extern const char Sbe41Fail;            /* general failure */
extern const char Sbe41Ok;              /* response received, regex match */
extern const char Sbe41PedanticFail;    /* response received, pedantic regex no-match */
extern const char Sbe41PedanticExceptn; /* response received, pedantic regex exception */

#endif /* SBE41_H */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctdio.h>
#include <logger.h>
#include <regex.h>
#include <extract.h>
#include <assert.h>
#include <time.h>
#include <nan.h>
#include <math.h>
#include <snprintf.h>

#ifdef _XA_
   #include <apf9.h>
#else
   #define WatchDog()
   #define StackOk() 1
#endif /* _XA_ */

/* define the maximum length of the SBE41 response */
#define MAXLEN 80

/* define a buffer for communications with the CTD serial port */
char buf[MAXLEN+1];

/* define a nonpedantic regex pattern for a field */
#define FIELD "([^,]*)"

/* define nonpedantic regex pattern for float */
#define FLOAT "[^-+0-9.]*([-+0-9.]+)[^0-9]*"

/* define pedantic regex patterns for P, T, and S */
#define P "[ ]+(-?[0-9]{1,4}\\.[0-9]{2})"
#define T "[ ]+(-?[0-9]{1,2}\\.[0-9]{4})"
#define S "[ ]+(-?[0-9]{1,2}\\.[0-9]{4})"
#define O "[ ]+([0-9]{1,5})"
  
/* define the return states of the SBE41 API */
const char Sbe41ChatFail        = -4; /* Failed chat attempt. */
const char Sbe41NoResponse      = -3; /* No response received from SBE41. */
const char Sbe41RegExceptn      = -2; /* Response received, regexec() exception */
const char Sbe41NullArg         = -1; /* Null function argument. */
const char Sbe41RegexFail       =  0; /* response received, regex no-match */
const char Sbe41Fail            =  0; /* general failure */
const char Sbe41Ok              =  1; /* response received, regex match */
const char Sbe41PedanticFail    =  2; /* response received, pedantic regex no-match */
const char Sbe41PedanticExceptn =  3; /* response received, pedantic regex exception */

/* functions with static linkage */
static int chat(const struct SerialPort *port, const char *cmd,
                const char *expect, time_t sec);
 
/*------------------------------------------------------------------------*/
/* function to configure the SBE41                                        */
/*------------------------------------------------------------------------*/
/**
   This function configures the SBE41 by entering the SBE41's command mode
   and executing commands that set the configuration parameters.

      \begin{verbatim}
      input:

         PtPump...If nonzero then the pump will run for 0.25seconds during
                  collection of PT samples.  If zero then PT samples will be
                  unpumped.
      
      output:

         This function returns a positive number if the configuration
         attempt was successful.  Zero is returned if a response was
         received but it failed the regex match.  A negative return value
         indicates failure due to an exceptional condition.  Here are the
         possible return values of this function:

         Sbe41ChatFail..........Execution of configuration commands failed.
         Sbe41NullArg...........Null function argument.
         Sbe41Fail..............Post-configuration verification failed.
         Sbe41Ok................Configuration attempt was successful.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe41Config(int PtPump)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41Config()";
   
   /* initialize the return value */
   int status=Sbe41NullArg;

   /* define the timeout period */
   const time_t TimeOut = 2;
   
   /* pet the watchdog timer */
   WatchDog();
   
   /* stack-check assertion */
   assert(StackOk());

   /* SBE41 serial number is the return value of Sbe41EnterCmdMode() */
   if ((status=Sbe41EnterCmdMode())>0)
   {
      #define MaxBufLen 31
      char buf[MaxBufLen+1];
      int ptpump=0,density=0,delay=0;
      unsigned int serno=0;
      
      /* reinitialize the return value */
      status=Sbe41Ok;

      /* write the command to set the pressure cutoff */
      snprintf(buf,MaxBufLen,"pumpfastpt=%c\r",((PtPump)?'y':'n'));
     
      /* initialize the control parameters of the SBE41 */
      if (chat(&ctdio, buf,                        "S>", TimeOut)<=0 ||
          chat(&ctdio, "dsreplyformat=s\r",        "S>", TimeOut)<=0 ||
          chat(&ctdio, "outputdensity=n\r",        "S>", TimeOut)<=0 ||
          chat(&ctdio, "addtimingdelays=n\r",      "S>", TimeOut)<=0  )
      {
         /* create the message */
         static cc msg[]="chat() failed.\n";

         /* log the configuration failure */
         ConioEnable(); LogEntry(FuncName,msg);

         /* indicate failure */
         status=Sbe41ChatFail;
      }

      /* analyze the query response to verify expected configuration */
      else if ((status=Sbe41Status(&serno, &ptpump, &density, &delay))>0)
      { 
         /* verify the configuration parameters */
         if ((PtPump && !ptpump) || (!PtPump && ptpump) || density || delay)
         {
               /* create the message */
               static cc msg[]="Configuration failed.\n";
               
               /* log the configuration failure */
               ConioEnable(); LogEntry(FuncName,msg);

               /* indicate failure */
               status=Sbe41Fail;
         }

         /* log the configuration success */
         else if (debuglevel>=2 || (debugbits&SBE41_H))
         {
            /* create the message */
            static cc msg[]="Configuration successful.\n";
  
            /* log the configuration failure */
            ConioEnable(); LogEntry(FuncName,msg);
         }
      }
   }
  
   /* put the SBE41 back to sleep */
   if (CtdEnableIo()>0 && chat(&ctdio,"\r","S>",2)>0) Sbe41ExitCmdMode();

   /* enable console IO */
   CtdDisableIo(); ConioEnable(); sleep(2);

   if (status<=0)
   {
      /* create the message */
      static cc msg[]="Attempt to set up SBE41 failed.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   return status;
   
   #undef NSUB
   #undef MaxBufLen
}

/*------------------------------------------------------------------------*/
/* function to query the SBE41 for configuration/status parameters        */
/*------------------------------------------------------------------------*/
/**
   This function executes an SBE41 status query and then extracts various
   configuration/status parameters from the response.
   
     \begin{verbatim}
     output:

        serno.....The serial number of the SBE41.

        ptpump....The value zero indicates that PT samples are unpumped.
                  Nonzero values indicate that PT samples are preceeded by a
                  0.25 second pump period.

        density...A nonzero value indicates that density (sigma-theta)
                  will be included with PTS samples.

        delay.....A nonzero value indicates that timing delays will be added
                  to emulate previous SBE41 behavior

     This function returns a positive return value on success and a zero or
     negative value on failure.  Here are the possible return values of this
         function:

         Sbe41NoResponse........No response received from SBE41.
         Sbe41Fail..............Attempt to start profile failed.
         Sbe41Ok................Attempt to start profile was successful.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
   \end{verbatim}
*/
int Sbe41Status(unsigned int *serno,int *ptpump, int *density, int *delay)
{
   #define MaxBufLen 79
   char *p,buf[MaxBufLen+1];

   /* define the parameter tokens */
   const char *const token[]=
   {
      "SERIAL NO.",
      "do not pump before faspt measurement",
      "pump 0.25 sec before faspt measurement",
      "add timing delays = no",
      "add timing delays = yes",
      "output density = no",
      "output density = yes",
   };

   /* initialize the return value */
   int i,status=Sbe41NoResponse;

   /* define the timeout period */
   const time_t TimeOut = 2;

   /* pet the watchdog timer */
   WatchDog();
   
   /* stack-check assertion */
   assert(StackOk());

   /* initialize the function parameters */
   if (serno)   *serno   = (unsigned int)(-1);
   if (ptpump)  *ptpump  = -1;
   if (delay)   *delay   = -1;
   if (density) *density = -1;

   for (i=0; i<3 && status!=Sbe41Ok; i++)
   {
      /* flush the IO queues */
      if (ctdio.ioflush) {Wait(10); ctdio.ioflush(); Wait(10);}

      if (chat(&ctdio,"\r","S>",2)>0)
      {
         /* query the SBE41 for its current configuration */
         pputs(&ctdio,"ds\r",TimeOut,"");

         /* analyze the query response to verify expected configuration */
         while (pgets(&ctdio,buf,MaxBufLen,TimeOut,"\r\n")>0)
         {
            if      (serno   && (p=strstr(buf,token[ 0]))) {*serno   = atoi(p+strlen(token[ 0]));} 
            else if (ptpump  && (p=strstr(buf,token[ 1]))) {*ptpump  = 0;}
            else if (ptpump  && (p=strstr(buf,token[ 2]))) {*ptpump  = 1;}
            else if (delay   && (p=strstr(buf,token[ 3]))) {*delay   = 0;} 
            else if (delay   && (p=strstr(buf,token[ 4]))) {*delay   = 1;} 
            else if (density && (p=strstr(buf,token[ 5]))) {*density = 0;} 
            else if (density && (p=strstr(buf,token[ 6]))) {*density = 1;} 
            
            if (strstr(buf,"output density")) break;
            
            status=Sbe41Ok;
         }
      }
   }
   
   /* validate each requested parameter */
   if (status>0)
   {
      if (ptpump  && (*ptpump  == (unsigned)(-1))) status=Sbe41Fail;
      if (delay   && (*delay   == (unsigned)(-1))) status=Sbe41Fail;
      if (density && (*density == (unsigned)(-1))) status=Sbe41Fail;
      Wait(100);
   }
   
   return status;

   #undef MaxBufLen
}

/*------------------------------------------------------------------------*/
/* function to enter the SBE41's command mode                             */
/*------------------------------------------------------------------------*/
/**
   This function wakes the SBE41 and places it in command mode.  It does
   this by asserting the wake pin for 1 full second in order to induce the
   SBE41 into command mode.  Experience shows that the mode-select line must
   be low when this command is executed or else it initiates a full CTD
   sample.  This will waste energy and throw off timing.

      \begin{verbatim}
      output:

         On success, this function returns a positive number that is the
         SBE41 serial number if the response matched a regex for the float
         serial number.  Zero is returned if a response was received but it
         failed the regex match.  A negative return value indicates failure
         due to an exceptional condition.  Here are the possible return
         values of this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                   an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not match the
                                   regex pattern for the serial number.
      \end{verbatim}
*/
int Sbe41EnterCmdMode(void)
{ 
   /* define the logging signature */
   static cc FuncName[] = "Sbe41EnterCmdMode()";
   
   /* initialize the return value */
   int status = Sbe41NullArg;

   /* pet the watchdog */
   WatchDog();

   /* stack-check assertion */
   assert(StackOk());
 
   /* validate the CTD serial ports ioflush() function */
   if (!(ctdio.ioflush))
   {
      /* create the message */
      static cc msg[]="NULL ioflush() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   else
   {
      /* define the number of subexpressions in the regex pattern */
      #define NSUB 1

      /* define objects needed for regex matching */
      regex_t regex; regmatch_t regs[NSUB+1]; int errcode;

      /* define the the nonpedantic pattern that will match a float */
      const char *pattern = "SERIAL NO\\.[^0-9]*([0-9]{4})";
      
      /* initialize the communications timeout periods */
      time_t To=time(NULL); const time_t TimeOut=30, timeout=2;
    
      /* compile the regex pattern */
      assert(!regcomp(&regex,pattern,REG_EXTENDED|REG_NEWLINE));

      /* protect against segfaults */
      assert(NSUB==regex.re_nsub);

      /* reinitialize the return value */
      status=Sbe41NoResponse;

      /* clear the mode-select line and enable IO from CTD serial port */
      CtdClearModePin(); CtdEnableIo(); 

      /* fault tolerance loop - keep trying if ctd is busy */
      for (status=0; status<=0 && difftime(time(NULL),To)<TimeOut;)
      {
         /* initiate the wake-up cycle */
         CtdAssertWakePin(); sleep(1); CtdClearWakePin();
    
         /* get the SBE41 command prompt to confirm that SBE41 is ready for commands */
         if ((status=chat(&ctdio,"\r","S>",2))>0)
         {
            /* initialize the reference time */
            const time_t To=time(NULL);

            /* flush the IO buffers of the CTD serial port */
            ctdio.iflush();
      
            /* send the command to display status */
            pputs(&ctdio,"ds",timeout,"\r");

            /* get the response */
            while (pgets(&ctdio,buf,MAXLEN,timeout,"\r\n")>0 && difftime(time(NULL),To)<TimeOut)
            {
               /* log the string received from the CTD serial port */
               if (debuglevel>=4)
               {
                  /* create the message */
                  static cc format[]="[%s]\n";

                  /* log the message */
                  LogEntry(FuncName,format,buf);
               }
               
               /* check the current response against the regex */
               if (!(errcode=regexec(&regex,buf,regex.re_nsub+1,regs,0)))
               {
                  /* extract the serial number from the response */
                  status = atoi(extract(buf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));

                  break;
               }

               /* indicate that the response did not match the regex pattern */
               else  {status = (errcode==REG_NOMATCH) ? Sbe41RegexFail : Sbe41RegExceptn;}
            }

            /* flush the IO queues */
            chat(&ctdio,"\r","S>",2);
            
            break;
         }
      }
   
      /* clean up the regex pattern buffer */
      regfree(&regex);
   }
   
   return status;

   #undef NSUB
}

/*------------------------------------------------------------------------*/
/* function to exit the SBE41's command mode                              */
/*------------------------------------------------------------------------*/
/**
   This function sends the SBE41 a command to exit command mode and power
   down.  Experience shows that the mode-select line must be low when this
   command is executed or else it initiates a full CTD sample.  This will
   waste energy and throw off timing.
   
      \begin{verbatim}
      output:
         This function returns a positive value on success and zero on
         failure.  A negative value indicates an exceptional error.
      \end{verbatim}
*/
int Sbe41ExitCmdMode(void)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41ExitCmdMode()";

   /* initialize the return value */
   int status = -1;
   
   /* pet the watchdog */
   WatchDog();

   /* stack-check assertion */
   assert(StackOk());
 
   /* validate the CTD serial ports ioflush() function */
   if (!(ctdio.ioflush))
   {
      /* create the message */
      static cc msg[]="NULL ioflush() function for serial port.\n";

      /* log the message */
      ConioEnable(); LogEntry(FuncName,msg);
   }

   else
   {
      /* initialize the communications timeout periods */
      const time_t To=time(NULL), TimeOut=30, timeout=2;

      /* set the mode-select line low and enable IO on the CTD port */
      CtdClearModePin(); CtdEnableIo();
 
      /* fault tolerance loop - keep trying if ctd is busy */
      for (status=0; status<=0 && difftime(time(NULL),To)<TimeOut;)
      {
         /* flush the CTD's IO buffers */
         ctdio.ioflush();
      
         /* get the SBE41 command prompt to confirm that SBE41 is ready for commands */
         if (chat(&ctdio,"\r","S>",timeout)>0)
         {
            /* send the command to shut down */
            pputs(&ctdio,"qs",2,"\r"); status=1; 
         }

         /* toggle the wake pin to initiate a wake-up cycle */
         else {CtdAssertWakePin(); sleep(1); CtdClearWakePin();}
      }
   
      /* disable IO, flush the IO buffers for the CTD serial port */
      CtdDisableIo(); sleep(1); ctdio.ioflush();

      if (status<=0)
      {
         /* create the message */
         static cc msg[]="Command to power down the SBE41 failed.\n";

         /* log the message */
         ConioEnable(); LogEntry(FuncName,msg);
      }
   }
   
   return status;
}

/*------------------------------------------------------------------------*/
/* function to query the SBE41 for its firmware revision                  */
/*------------------------------------------------------------------------*/
/**
   This function queries the SBE41 for its firmware revision.  It parses the
   response to a 'ds' command using a regex and extracts the firmware
   revision.  

      \begin{verbatim}
      input:

         size.....This is the maximum number of bytes (including the 0x0
                  string terminator) that will be written into the FwRev
                  function argument.

      output:

         FwRev....The firmware revision string will be written into this
                  buffer.

         On success, this function returns a positive value and stores the
         firmware revision in its function argument.  Zero is returned if a
         response was received but it failed the regex match.  A negative
         return value indicates failure due to an exceptional condition.
         Here are the possible return values of this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                   an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not match the
                                   regex pattern for the serial number.
      \end{verbatim}
*/
int Sbe41FwRev(char *FwRev,unsigned int size)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41FwRev()";

   int status=Sbe41NullArg;

   /* pet the watchdog */
   WatchDog();

   /* stack-check assertion */
   assert(StackOk());

   /* validate the function argument */
   if (!FwRev || !size)
   {
      /* create the message */
      static cc msg[]="NULL function argument.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* validate the CTD serial ports ioflush() function */
   else if (!(ctdio.iflush))
   {
      /* create the message */
      static cc msg[]="NULL iflush() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* enter SBE41 command mode */
   else if ((status=Sbe41EnterCmdMode())>0)
   {
      /* create a temporary buffer to receive the SBE41 response string */
      char buf[64];

      /* initialize the return values */
      status=Sbe41NoResponse; FwRev[0]=0;
      
      /* get the SBE41 command prompt to confirm that SBE41 is ready for commands */
      if (chat(&ctdio,"ds\r","SBE 41",2)>0 && pgets(&ctdio,buf,sizeof(buf),3,"\r\n")>0)
      {
         /* define the number of subexpressions in the regex pattern */
         #define NSUB 3

         /* define objects needed for regex matching */
         regex_t regex; regmatch_t regs[NSUB+1]; int errcode;

         /* define the the nonpedantic pattern that will match a float */
         const char *pattern = "(ALACE)|(STD).*[ ]+V[ ]+([^ ]+)";

         /* compile the regex pattern */
         assert(!regcomp(&regex,pattern,REG_EXTENDED|REG_NEWLINE));

         /* protect against segfaults */
         assert(regex.re_nsub==NSUB);

         /* check if the current line matches the regex */
         if (!(errcode=regexec(&regex,buf,regex.re_nsub+1,regs,0)))
         {
            /* determine the number of bytes to copy */
            unsigned int n = regs[3].rm_eo-regs[3].rm_so; if(n>=size) n=size-1;

            /* copy the firmware revision from the SBE41 response */
            strncpy(FwRev,buf+regs[3].rm_so,n); FwRev[n]=0;

            /* indicate success */
            status=Sbe41Ok;
         }

         /* indicate that the response did not match the regex pattern */
         else  {status = (errcode==REG_NOMATCH) ? Sbe41RegexFail : Sbe41RegExceptn;}
         
         /* clean up the regex pattern buffer */
         regfree(&regex); 
      }
   }
   
   /* put the SBE41 back to sleep */
   if (CtdEnableIo()>0 && chat(&ctdio,"\r","S>",2)>0) Sbe41ExitCmdMode();
   
   /* enable console IO */
   ConioEnable();
   
   return status;

   #undef NSUB
}

/*------------------------------------------------------------------------*/
/* function to get one pressure measurement from the SBE41                */
/*------------------------------------------------------------------------*/
/**
   This function queries the SBE41 CTD for a single pressure measurement.

      \begin{verbatim}
      input:
         ctdio....A structure that contains pointers to machine dependent
                  primitive IO functions.  See the comment section of the
                  SerialPort structure for details.  The function checks to
                  be sure this pointer is not NULL.

      output:
         p........This is where the pressure will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).

         This function returns a positive number if the response received
         from the CTD serial port matched a regex for a float.  Zero is
         returned if a response was received but it failed the regex match.
         A negative return value indicates failure due to an exceptional
         condition.  Here are the possible return values of this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not match the
                                regex pattern for a float.
         Sbe41Ok................Response received that matched the pedantic
                                regex pattern.
         Sbe41PedanticFail......Response received that matched a float but
                                failed to match the pedantic regex pattern.
         Sbe41PedanticExceptn...Response received that matched a float but
                                the pedantic regex experienced an exception.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe41GetP(float *p)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41GetP()";

   int status=Sbe41NullArg;

   /* pet the watchdog */
   WatchDog();

   /* stack-check assertion */
   assert(StackOk());

   /* make sure that floats are 4 bytes in length */
   assert(sizeof(float)==4);
   
   /* validate the function argument */
   if (!p)
   {
      /* create the message */
      static cc msg[]="NULL function argument.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* validate the CTD serial ports ioflush() function */
   else if (!(ctdio.iflush))
   {
      /* create the message */
      static cc msg[]="NULL iflush() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   else
   {
      int errcode;

      /* reinitialize the return values */
      status=Sbe41Ok;

      /* initialize the return value of 'p' to IEEE NaN */
      *p = NaN();

      /* activate the SBE41 with the hardware control lines */
      errcode=CtdPSample(buf,MAXLEN);

      /* log the data received from the SBE41 */
      if (debuglevel>=4)
      {
         /* create the message */
         static cc format[]="Received: [%s]\n";
         
         /* log the message */
         LogEntry(FuncName,format,buf);
      }
      
      /* check if an error was detected */
      if (errcode<=0)
      {
         /* create the message */
         static cc msg[]="No response from SBE41.\n";
         
         /* log the message */
         LogEntry(FuncName,msg);

         /* indicate failure */
         status=Sbe41NoResponse;
      }

      /* subject the SBE41 response to lexical analysis */
      else
      {
         /* define the number of subexpressions in the regex pattern */
         #define NSUB 1

         /* define objects needed for regex matching */
         regex_t regex; regmatch_t regs[NSUB+1]; int errcode;

         /* define the the nonpedantic pattern that will match a float */
         const char *pattern = "^" FLOAT;
         
         /* compile the regex pattern */
         assert(!regcomp(&regex,pattern,REG_EXTENDED|REG_NEWLINE));

         /* protect against segfaults */
         assert(NSUB==regex.re_nsub);

         /* check if the current line matches the regex */
         if (!(errcode=regexec(&regex,buf,regex.re_nsub+1,regs,0)))
         {
            /* extract the pressure from the buffer */
            *p = atof(extract(buf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
               
            /* define the pedantic form of the expected response */
            pattern = "^" P "$";

            /* jettison the nonpedantic regex */
            regfree(&regex);

            /* compile the regex pattern */
            assert(!regcomp(&regex,pattern,REG_EXTENDED|REG_NEWLINE));

            /* protect against segfaults */
            assert(NSUB==regex.re_nsub);

            /* check if the response matches exactly the expected form */
            if ((errcode=regexec(&regex,buf,regex.re_nsub+1,regs,0)))
            {
               /* create the message */
               static cc format[]="Violation of pedantic regex: \"%s\\r\\n\"\n";

               /* log the message */
                LogEntry(FuncName,format,buf);

               /* reinitialize the return value */
               status = (errcode==REG_NOMATCH) ? Sbe41PedanticFail : Sbe41PedanticExceptn;
            }
         }

         else
         {
            /* create the message */
            static cc format[]="Violation of nonpedantic regex: [%s\\r\\n]\n";

            /* log the message */
            LogEntry(FuncName,format,buf);
              
            /* indicate that the response from the SBE41 violated even the nonpedantic regex */
            status = (errcode==REG_NOMATCH) ? Sbe41RegexFail : Sbe41RegExceptn; 
         }
         
         /* clean up the regex pattern buffer */
         regfree(&regex); 
      }
   }

   return status;
   
   #undef NSUB
}

/*------------------------------------------------------------------------*/
/* function to get a low-power PT sample from the SBE41                   */
/*------------------------------------------------------------------------*/
/**
   This function queries the SBE41 CTD for a low-power PT measurement.

      \begin{verbatim}
      input:
         ctdio....A structure that contains pointers to machine dependent
                  primitive IO functions.  See the comment section of the
                  SerialPort structure for details.  The function checks to
                  be sure this pointer is not NULL.

      output:
      
         p........This is where the pressure will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).
      
         t........This is where the temperature will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).

         This function returns a positive number if the response received
         from the CTD serial port matched (for each of p and t) a regex for
         a float.  Zero is returned if a response was received but it failed
         the regex match.  A negative return value indicates failure due to
         an exceptional condition.  Here are the possible return values of
         this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not match (for
                                each of p, t, and s) the regex pattern for a
                                float.
         Sbe41Ok................Response received that matched the pedantic
                                regex pattern.
         Sbe41PedanticFail......Response received that matched a float but
                                failed to match the pedantic regex pattern.
         Sbe41PedanticExceptn...Response received that matched a float but
                                the pedantic regex experienced an exception.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe41GetPt(float *p, float *t)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41GetPt()";

   int status=Sbe41NullArg;

   /* pet the watchdog timer */
   WatchDog();
      
   /* stack-check assertion */
   assert(StackOk());

   /* make sure that floats are 4 bytes in length */
   assert(sizeof(float)==4);
   
   /* validate the function argument */
   if (!p || !t)
   {
      /* create the message */
      static cc msg[]="NULL function argument(s).\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* validate the CTD serial ports ioflush() function */
   else if (!(ctdio.ioflush))
   {
      /* create the message */
      static cc msg[]="NULL ioflush() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   else
   {
      int errcode;

      /* reinitialize the return values */
      status=Sbe41Ok;

      /* initialize the return value of 'p' and 't' to IEEE NaN */
      *p=NaN(); *t=NaN();

      /* activate the SBE41 with the hardware control lines */
      errcode=CtdPtSample(buf,MAXLEN);

      /* log the data received from the SBE41 */
      if (debuglevel>=4)
      {
         /* create the message */
         static cc format[]="Received: [%s]\n";

         /* log the message */
         LogEntry(FuncName,format,buf);
      }
      
      /* check if an error was detected */
      if (errcode<=0)
      {
         /* create the message */
         static cc msg[]="No response from SBE41.\n";
         
         /* log the message */
         LogEntry(FuncName,msg);

         /* indicate failure */
         status=Sbe41NoResponse;
      }
      
      /* subject the SBE41 response to lexical analysis */
      else
      {
         /* define the number of subexpressions in the regex pattern */
         #define NSUB 2
          
         /* define objects needed for regex matching */
         regex_t regex; regmatch_t regs[NSUB+1]; int errcode;

         /* define the the nonpedantic pattern that will match a float */
         const char *pattern = "^" FIELD "," FIELD;
         
         /* compile the regex pattern */
         assert(!regcomp(&regex,pattern,REG_EXTENDED|REG_NEWLINE));

         /* protect against segfaults */
         assert(regex.re_nsub==NSUB);

         /* log the string received from the CTD */
         if (debuglevel>=4)
         {
            /* create the message */
            static cc format[]="[%s]\n";
         
            /* log the message */
            LogEntry(FuncName,format,buf);
         }
         
         /* check if the current line matches the regex */
         if (!(errcode=regexec(&regex,buf,regex.re_nsub+1,regs,0)))
         {
            /* initialize pointers to the P,T fields */
            const char *pbuf=0, *tbuf=0;

            /* define the pedantic form of the expected response */
            pattern = "^" P "," T "$"; 

            /* jettison the nonpedantic regex */
            regfree(&regex);

            /* compile the pedantic regex pattern */
            assert(!regcomp(&regex,pattern,REG_NOSUB|REG_EXTENDED|REG_NEWLINE));

            /* check if the response matches exactly the expected form */
            if ((errcode=regexec(&regex,buf,0,0,0)))
            {
               /* create the message */
               static cc format[]="Violation of pedantic regex: [%s\\r\\n]\n";
               
               /* log the message */
               LogEntry(FuncName,format,buf);

               /* reinitialize the return value */
               status = (errcode==REG_NOMATCH) ? Sbe41PedanticFail : Sbe41PedanticExceptn;
            }

            /* segment the buffer into separate P and T fields */
            if (regs[1].rm_so>=0 && regs[1].rm_eo>=0) {pbuf=buf+regs[1].rm_so; buf[regs[1].rm_eo]=0;}
            if (regs[2].rm_so>=0 && regs[2].rm_eo>=0) {tbuf=buf+regs[2].rm_so; buf[regs[2].rm_eo]=0;}

            /* jettison the pedantic regex */
            regfree(&regex);

            /* compile the regex pattern for a float */
            assert(!regcomp(&regex,FLOAT,REG_EXTENDED|REG_NEWLINE));

            /* protect against segfaults */
            assert(regex.re_nsub==1);

            /* check the pressure-field against a nonpedantic float regex pattern */
            if (pbuf && !regexec(&regex,pbuf,regex.re_nsub+1,regs,0)) 
            {
               /* extract pressure from the buffer */
               *p = atof(extract(pbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }

            /* check the temperature-field against a nonpedantic float regex pattern */
            if (tbuf && !regexec(&regex,tbuf,regex.re_nsub+1,regs,0))
            {
               /* extract temperature from the buffer */               
               *t = atof(extract(tbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }
         }
         
         else
         {
            /* create the message */
            static cc format[]="Violation of 2-fields regex: [%s\\r\\n]\n";

            /* make logentry */
            LogEntry(FuncName,format,buf);
            
            /* indicate that the response from the SBE41 violated even the nonpedantic regex */
            status = (errcode==REG_NOMATCH) ? Sbe41RegexFail : Sbe41RegExceptn; 
         }
         
         /* clean up the regex pattern buffer */
         regfree(&regex); 
      }
   }
   
   return status;
   
   #undef NSUB 
}

/*------------------------------------------------------------------------*/
/* function to get a full PTS sample from the SBE41                       */
/*------------------------------------------------------------------------*/
/**
   This function queries the SBE41 CTD for a full PTS measurement.

      \begin{verbatim}
      input:
         ctdio....A structure that contains pointers to machine dependent
                  primitive IO functions.  See the comment section of the
                  SerialPort structure for details.  The function checks to
                  be sure this pointer is not NULL.

      output:
      
         p........This is where the pressure will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).
      
         t........This is where the temperature will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).
      
         s........This is where the salinity will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).

         This function returns a positive number if the response received
         from the CTD serial port matched (for each of p, t, and s) a regex
         for a float.  Zero is returned if a response was received but it
         failed the regex match.  A negative return value indicates failure
         due to an exceptional condition.  Here are the possible return
         values of this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not match (for
                                each of p, t, and s) the regex pattern for a
                                float.
         Sbe41Ok................Response received that matched the pedantic
                                regex pattern.
         Sbe41PedanticFail......Response received that matched a float but
                                failed to match the pedantic regex pattern.
         Sbe41PedanticExceptn...Response received that matched a float but
                                the pedantic regex experienced an exception.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe41GetPts(float *p, float *t, float *s)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41GetPts()";

   int status=Sbe41NullArg;

   /* pet the watchdog timer */
   WatchDog();
      
   /* stack-check assertion */
   assert(StackOk());

   /* make sure that floats are 4 bytes in length */
   assert(sizeof(float)==4);
   
   /* validate the function argument */
   if (!p || !t || !s)
   {
      /* create the message */
      static cc msg[]="NULL function argument(s).\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* validate the CTD serial ports ioflush() function */
   else if (!(ctdio.ioflush))
   {
      /* create the message */
      static cc msg[]="NULL ioflush() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   else
   {
      int errcode;
      
      /* initialize the communications timeout period */
      const time_t timeout=5;

      /* reinitialize the return values */
      status=Sbe41Ok;

      /* initialize the return value of 'p', 't', and 's' to IEEE NaN */
      *p=NaN(); *t=NaN(); *s=NaN();
 
      /* activate the SBE41 with the hardware control lines */
      errcode=CtdPtsSample(buf,MAXLEN,timeout);

      /* log the data received from the SBE41 */
      if (debuglevel>=4)
      {
         /* create the message */
         static cc format[]="Received: [%s]\n";

         /* log the message */
         LogEntry(FuncName,format,buf);
      }
      
      /* check if an error was detected */
      if (errcode<=0)
      {
         /* create the message */
         static cc msg[]="No response from SBE41.\n";
         
         /* log the message */
         LogEntry(FuncName,msg);

         /* indicate failure */
         status=Sbe41NoResponse;
      }
      
      /* subject the SBE41 response to lexical analysis */
      else
      {
         /* define the number of subexpressions in the regex pattern */
         #define NSUB 3
          
         /* define objects needed for regex matching */
         regex_t regex; regmatch_t regs[NSUB+1]; int errcode;

         /* define the the nonpedantic pattern that will match a float */
         const char *pattern = "^" FIELD "," FIELD "," FIELD;
         
         /* compile the regex pattern */
         assert(!regcomp(&regex,pattern,REG_EXTENDED|REG_NEWLINE));

         /* protect against segfaults */
         assert(regex.re_nsub==NSUB);

         /* log the string received from the CTD */
         if (debuglevel>=4)
         {
            /* create the message */
            static cc format[]="[%s]\n";
         
            /* log the message */
            LogEntry(FuncName,format,buf);
         }
         
         /* check if the current line matches the regex */
         if (!(errcode=regexec(&regex,buf,regex.re_nsub+1,regs,0)))
         {
            /* initialize pointers to the P,T,S fields */
            const char *pbuf=0, *tbuf=0, *sbuf=0;

            /* define the pedantic form of the expected response */
            pattern = "^" P "," T "," S "$"; 

            /* jettison the nonpedantic regex */
            regfree(&regex);

            /* compile the pedantic regex pattern */
            assert(!regcomp(&regex,pattern,REG_NOSUB|REG_EXTENDED|REG_NEWLINE));

            /* check if the response matches exactly the expected form */
            if ((errcode=regexec(&regex,buf,0,0,0)))
            {
               /* create the message */
               static cc format[]="Violation of pedantic regex: [%s\\r\\n]\n";
               
               /* log the message */
               LogEntry(FuncName,format,buf);

               /* reinitialize the return value */
               status = (errcode==REG_NOMATCH) ? Sbe41PedanticFail : Sbe41PedanticExceptn;
            }

            /* segment the buffer into separate P,T, and S fields */
            if (regs[1].rm_so>=0 && regs[1].rm_eo>=0) {pbuf=buf+regs[1].rm_so; buf[regs[1].rm_eo]=0;}
            if (regs[2].rm_so>=0 && regs[2].rm_eo>=0) {tbuf=buf+regs[2].rm_so; buf[regs[2].rm_eo]=0;}
            if (regs[3].rm_so>=0 && regs[3].rm_eo>=0) {sbuf=buf+regs[3].rm_so; buf[regs[3].rm_eo]=0;}

            /* jettison the pedantic regex */
            regfree(&regex);

            /* compile the regex pattern for a float */
            assert(!regcomp(&regex,FLOAT,REG_EXTENDED|REG_NEWLINE));

            /* protect against segfaults */
            assert(regex.re_nsub==1);

            /* check the pressure-field against a nonpedantic float regex pattern */
            if (pbuf && !regexec(&regex,pbuf,regex.re_nsub+1,regs,0)) 
            {
               /* extract pressure from the buffer */
               *p = atof(extract(pbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }

            /* check the temperature-field against a nonpedantic float regex pattern */
            if (tbuf && !regexec(&regex,tbuf,regex.re_nsub+1,regs,0))
            {
               /* extract temperature from the buffer */               
               *t = atof(extract(tbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }
            
            /* check the salinity-field against a nonpedantic float regex pattern */
            if (sbuf && !regexec(&regex,sbuf,regex.re_nsub+1,regs,0))
            {
               /* extract salinity from the buffer */               
               *s = atof(extract(sbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }
         }
         
         else
         {
            /* create the message */
            static cc format[]="Violation of 3-fields regex: [%s\\r\\n]\n";

            /* make logentry */
            LogEntry(FuncName,format,buf);
            
            /* indicate that the response from the SBE41 violated even the nonpedantic regex */
            status = (errcode==REG_NOMATCH) ? Sbe41RegexFail : Sbe41RegExceptn; 
         }
         
         /* clean up the regex pattern buffer */
         regfree(&regex); 
      }
   }
   
   return status;
   
   #undef NSUB 
}

/*------------------------------------------------------------------------*/
/* function to get a full PTSO sample from the SBE41/43                   */
/*------------------------------------------------------------------------*/
/**
   This function queries the SBE41/43 CTD for a full PTSO measurement.

      \begin{verbatim}
      input:
         ctdio....A structure that contains pointers to machine dependent
                  primitive IO functions.  See the comment section of the
                  SerialPort structure for details.  The function checks to
                  be sure this pointer is not NULL.

      output:
      
         p........This is where the pressure will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).
      
         t........This is where the temperature will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).
      
         s........This is where the salinity will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).
      
         o........This is where the oxygen frequency will be stored when the
                  function returns.  If an error occurs, this is set to NaN
                  (according to IEEE floating point format).

         This function returns a positive number if the response received
         from the CTD serial port matched (for each of p, t, and s) a regex
         for a float.  Zero is returned if a response was received but it
         failed the regex match.  A negative return value indicates failure
         due to an exceptional condition.  Here are the possible return
         values of this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not match (for
                                each of p, t, and s) the regex pattern for a
                                float.
         Sbe41Ok................Response received that matched the pedantic
                                regex pattern.
         Sbe41PedanticFail......Response received that matched a float but
                                failed to match the pedantic regex pattern.
         Sbe41PedanticExceptn...Response received that matched a float but
                                the pedantic regex experienced an exception.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe41GetPtso(float *p, float *t, float *s, float *o)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41GetPtso()";

   int status=Sbe41NullArg; 

   /* pet the watchdog timer */
   WatchDog();
     
   /* stack-check assertion */
   assert(StackOk());

   /* make sure that floats are 4 bytes in length */
   assert(sizeof(float)==4);

   /* validate the function argument */
   if (!p || !t || !s || !o) 
   {
      /* create the message */
      static cc msg[]="NULL function argument(s).\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* validate the CTD serial ports ioflush() function */
   else if (!(ctdio.ioflush))
   {
      /* create the message */
      static cc msg[]="NULL ioflush() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   else
   {
      int errcode;
      
      /* initialize the communications timeout period */
      const time_t timeout=65;

      /* reinitialize the return values */
      status=Sbe41Ok;

      /* initialize the return value of 'p', 't', 's', and 'o' to IEEE NaN */
      *p=NaN(); *t=NaN(); *s=NaN(); *o=NaN();

      /* activate the SBE41 with the hardware control lines */
      errcode=CtdPtsSample(buf,MAXLEN,timeout);

      /* log the data received from the SBE41 */
      if (debuglevel>=4)
      {
         /* create the message */
         static cc format[]="Received: [%s]\n";
      
         /* log the message */
         LogEntry(FuncName,format,buf);
      }
      
      /* check if an error was detected */
      if (errcode<=0)
      {
         /* create the message */
         static cc msg[]="No response from SBE41.\n";

         /* make the logentry */
         LogEntry(FuncName,msg);

         /* indicate failure */
         status=Sbe41NoResponse;
      }
      
      /* subject the SBE41 response to lexical analysis */
      else
      {
         /* define the number of subexpressions in the regex pattern */
         #define NSUB 4
          
         /* define objects needed for regex matching */
         regex_t regex; regmatch_t regs[NSUB+1]; int errcode;

         /* define the the nonpedantic pattern that will match a float */
         const char *pattern = "^" FIELD "," FIELD "," FIELD "," FIELD;

         /* compile the regex pattern */
         assert(!regcomp(&regex,pattern,REG_EXTENDED|REG_NEWLINE));

         /* protect against segfaults */
         assert(regex.re_nsub==NSUB);
         
         /* check if the current line matches the regex */
         if (!(errcode=regexec(&regex,buf,regex.re_nsub+1,regs,0)))
         {
            const char *pbuf=0, *tbuf=0, *sbuf=0, *obuf=0;

            /* define the pedantic form of the expected response */
            pattern = "^" P "," T "," S "," O "$"; 

            /* jettison the nonpedantic regex */
            regfree(&regex);

            /* compile the regex pattern */
            assert(!regcomp(&regex,pattern,REG_NOSUB|REG_EXTENDED|REG_NEWLINE));

            /* check if the response matches exactly the expected form */
            if ((errcode=regexec(&regex,buf,0,0,0)))
            {
               /* create the message */
               static cc format[]="Violation of pedantic regex: [%s\\r\\n]\n";

               /* log the message */
               LogEntry(FuncName,format,buf);

               /* reinitialize the return value */
               status = (errcode==REG_NOMATCH) ? Sbe41PedanticFail : Sbe41PedanticExceptn;
            }

            /* segment the buffer into separate P,T, S, and O fields */
            if (regs[1].rm_so>=0 && regs[1].rm_eo>=0) {pbuf=buf+regs[1].rm_so; buf[regs[1].rm_eo]=0;}
            if (regs[2].rm_so>=0 && regs[2].rm_eo>=0) {tbuf=buf+regs[2].rm_so; buf[regs[2].rm_eo]=0;}
            if (regs[3].rm_so>=0 && regs[3].rm_eo>=0) {sbuf=buf+regs[3].rm_so; buf[regs[3].rm_eo]=0;}
            if (regs[4].rm_so>=0 && regs[4].rm_eo>=0) {obuf=buf+regs[4].rm_so; buf[regs[4].rm_eo]=0;}

            /* jettison the pedantic regex */
            regfree(&regex);

            /* compile the regex pattern for a float */
            assert(!regcomp(&regex,FLOAT,REG_EXTENDED|REG_NEWLINE));

            /* protect against segfaults */
            assert(regex.re_nsub==1);

            /* check the pressure-field against a nonpedantic float regex pattern */
            if (pbuf && !regexec(&regex,pbuf,regex.re_nsub+1,regs,0)) 
            {
               /* extract pressure from the buffer */
               *p = atof(extract(pbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }

            /* check the temperature-field against a nonpedantic float regex pattern */
            if (tbuf && !regexec(&regex,tbuf,regex.re_nsub+1,regs,0))
            {
               /* extract temperature from the buffer */               
               *t = atof(extract(tbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }

            /* check the salinity-field against a nonpedantic float regex pattern */
            if (sbuf && !regexec(&regex,sbuf,regex.re_nsub+1,regs,0))
            {
               /* extract salinity from the buffer */               
               *s = atof(extract(sbuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }

            /* check the oxygen-field against a nonpedantic float regex pattern */
            if (obuf && !regexec(&regex,obuf,regex.re_nsub+1,regs,0))
            {
               /* extract oxygen from the buffer */               
               *o = atof(extract(obuf,regs[1].rm_so+1,regs[1].rm_eo-regs[1].rm_so));
            }
         }
         
         else
         {
            /* create the message */
            static cc format[]="Violation of 4-fields regex: [%s\\r\\n]\n";

            /* make logentry */
            LogEntry(FuncName,format,buf);
            
            /* indicate that the response from the SBE41 violated even the nonpedantic regex */
            status = (errcode==REG_NOMATCH) ? Sbe41RegexFail : Sbe41RegExceptn; 
         }
         
         /* clean up the regex pattern buffer */
         regfree(&regex); 
      }
      
      /* disable communications via the CTD serial port */
      CtdDisableIo();
   }
   
   return status;
   
   #undef NSUB 
}

/*------------------------------------------------------------------------*/
/* function to log the SBE41's calibration coefficents                    */
/*------------------------------------------------------------------------*/
/**
   This function uses the SBE41 communications mode to log its calibration
   coefficients via the SBE41 'dc' command.
   
      \begin{verbatim}
      input:
         ctdio....A structure that contains pointers to machine dependent
                  primitive IO functions.  See the comment section of the
                  SerialPort structure for details.  The function checks to
                  be sure this pointer is not NULL.

      output:
      
         This function returns a positive number if a response was received
         from the CTD serial port.  Zero is returned if an unexpected
         response was received.  A negative return value indicates failure
         due to an exceptional condition.  Here are the possible return
         values of this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                     an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not match the
                                     regex pattern for the serial number.
         Sbe41Ok................Response received.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe41LogCal(void)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41LogCal()";

   /* initialize the return value */
   int status=Sbe41NullArg;
      
   /* pet the watchdog timer */
   WatchDog();
     
   /* stack-check assertion */
   assert(StackOk());

   /* enter SBE41 command mode */
   if ((status=Sbe41EnterCmdMode())>0)
   {
      /* define the timeout period for communications mode */
      const time_t To=time(NULL), timeout=2, TimeOut=60;

      /* reinitialize the return value */
      status = Sbe41Ok;

      /* flush the Rx queue of the ctd serial port */
      if (ctdio.iflush) ctdio.iflush();
      
      /* send the command to display calibration coefficients */
      pputs(&ctdio,"dc\r",timeout,"");

      /* wait for the data to be uploaded to the Ctd fifo */
      while (CtdActiveIo(timeout) && difftime(time(NULL),To)<TimeOut/2) {}

      /* enable console io */
      ConioEnable();
      
      /* read the SBE41 response from the serial port */
      while (pgets(&ctdio,buf,MAXLEN,timeout,"\r\n")>0 && difftime(time(NULL),To)<TimeOut)
      {
         /* look for end of calibration coefficient display */
         if (!strncmp("S>",buf,2)) break;
         
         /* ignore "dc" and log the current coefficients */
         if (strncmp("dc",buf,2))
         {
            /* create the message */
            static cc format[]="%s\n";

            /* make logentry */
            LogEntry(FuncName,format,buf);
         }
      }
   }

   /* put the SBE41 back to sleep */
   if (CtdEnableIo()>0 && chat(&ctdio,"\r","S>",2)>0) Sbe41ExitCmdMode();

   /* enable console IO */
   CtdDisableIo(); ConioEnable();

   if (status<=0)
   {
      /* create the message */
      static cc format[]="Attempt to enter command mode failed [errcode: %d] - aborting.\n";

      /* make the logentry */
      LogEntry(FuncName,format,status); 
   }
   
   return status;
}

/*------------------------------------------------------------------------*/
/* function to query an SBE41 for its serial number                       */
/*------------------------------------------------------------------------*/
/**
   This function queries the SBE41 for its serial number. 

      \begin{verbatim}
      input:
         ctdio....A structure that contains pointers to machine dependent
                  primitive IO functions.  See the comment section of the
                  SerialPort structure for details.  The function checks to
                  be sure this pointer is not NULL.

      output:
      
         This function returns a positive number if the response received
         from the CTD serial port matched (for each of p, t, and s) a regex
         for a 4-digit integer.  Zero is returned if a response was received
         but it failed the regex match.  A negative return value indicates
         failure due to an exceptional condition.  Here are the possible
         return values of this function:
         
         Sbe41NoResponse........No response received from SBE41.
         Sbe41RegExceptn........Response received but regexec() failed with
                                an exceptional error.
         Sbe41NullArg...........Null function argument.
         Sbe41RegexFail.........Response received but it did not matchthe
                                regex pattern for a 4-digit integer.
         Sbe41Ok................Response received that matched the pedantic
                                regex pattern.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe41SerialNumber(void)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe41SerialNumber()";

   /* initialize the return value */
   int status=Sbe41NullArg;

   /* pet the watchdog timer */
   WatchDog();
   
   /* stack-check assertion */
   assert(StackOk());

   /* SBE41 serial number is the return value of Sbe41EnterCmdMode() */
   if ((status=Sbe41EnterCmdMode())<=0)
   {
      /* create the message */
      static cc msg[]="Attempt to query SBE41 serial number failed.\n";

      /* make logentry */
      ConioEnable(); LogEntry(FuncName,msg);
   }
      
   /* exit the SBE41's command mode */
   Sbe41ExitCmdMode();

   return status;

   #undef NSUB
}

/*------------------------------------------------------------------------*/
/* function to configure the SBE41                                        */
/*------------------------------------------------------------------------*/
/**
   This function configures the SBE41 for 2-decibar bins over the whole
   water column.  It does this by entering the SBE41's command mode and
   executing commands that set the configuration parameters.

      \begin{verbatim}
      input:
         PCutOff...The continuous profile is automatically halted when the
                   pressure falls below this cut-off pressure.  In addition,
                   the SBE41 will automatically be powered down.
      
      output:

         This function returns a positive number if the configuration
         attempt was successful.  Zero is returned if a response was
         received but it failed the regex match.  A negative return value
         indicates failure due to an exceptional condition.  Here are the
         possible return values of this function:

         Sbe41RegexFail.........Response received, regex no-match
         Sbe41ChatFail..........Execution of configuration commands failed.
         Sbe41RegExceptn........Response received but regexec() failed with
                                    an exceptional error.
         Sbe41NoResponse........No response received from SBE41.
         Sbe41NullArg...........Null function argument.
         Sbe41Fail..............Post-configuration verification failed.
         Sbe41Ok................Configuration attempt was successful.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
      \end{verbatim}
*/
int Sbe43Config(int *Ido)
{
   /* define the logging signature */
   static cc FuncName[] = "Sbe43Config()";
   
   /* initialize the return value */
   int status=Sbe41NullArg;

   /* define the timeout period */
   const time_t TimeOut = 2;
   
   /* pet the watchdog timer */
   WatchDog();
   
   /* stack-check assertion */
   assert(StackOk());

   /* initialize the return value */
   if (Ido) *Ido=-1;
   
   /* SBE41 serial number is the return value of Sbe41EnterCmdMode() */
   if ((status=Sbe41EnterCmdMode())>0)
   {
      float Nf,Ns,Tau20;
         
      /* reinitialize the return value */
      status=Sbe41Ok;
     
      /* initialize the control parameters of the SBE41 */
      if (chat(&ctdio, "oxnf=2.0\r", "S>", TimeOut)<=0 ||
          chat(&ctdio, "oxns=0.0\r", "S>", TimeOut)<=0  )
      {
         /* create the message */
         static cc msg[]="chat() failed.\n";

         /* log the configuration failure */
         ConioEnable(); LogEntry(FuncName,msg);

         /* indicate failure */
         status=Sbe41ChatFail;
      }

      /* analyze the query response to verify expected configuration */
      else if ((status=Sbe43Status(&Ns,&Nf,&Tau20,Ido))>0)
      { 
         /* verify the configuration parameters */
         if (fabs(Nf-2.0)>0.1 || fabs(Ns)>0.1)
         {
            /* create the message */
            static cc msg[]="Configuration failed.\n";
            
               /* log the configuration failure */
            ConioEnable(); LogEntry(FuncName,msg);
            
            /* indicate failure */
            status=Sbe41Fail;
         }

         /* log the configuration success */
         else if (debuglevel>=2 || (debugbits&SBE41_H))
         {
            /* create the message */
            static cc msg[]="Configuration successful";
  
            /* log the configuration failure */
            ConioEnable(); LogEntry(FuncName,msg);

            /* write the Sbe43i serial number, if available */
            if (Ido && (*Ido)!=-1) LogAdd(" (SerNo: %04d).\n",*Ido);
            else LogAdd(".\n");
         }
      }
   }
  
   /* put the SBE41 back to sleep */
   if (CtdEnableIo()>0 && chat(&ctdio,"\r","S>",2)>0) Sbe41ExitCmdMode();

   /* enable console IO */
   CtdDisableIo(); ConioEnable();

   if (status<=0)
   {
      /* create the message */
      static cc msg[]="Attempt to configure the SBE43i failed.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   return status;
}

/*------------------------------------------------------------------------*/
/* function to query the SBE41 for configuration/status parameters        */
/*------------------------------------------------------------------------*/
/**
   This function executes an SBE41 status query and then extracts various
   configuration/status parameters from the response.
   
     \begin{verbatim}
     output:
        Nf........The number of
        Ns........The number of samples presently stored in the SBE41 nvram.
        Tau20....The number of bins created by the bin-averaging scheme.

     This function returns a positive return value on success and a zero or
     negative value on failure.  Here are the possible return values of this
         function:

         Sbe41RegExceptn........Response received but regexec() failed with
                                    an exceptional error.
         Sbe41NoResponse........No response received from SBE41.
         Sbe41NullArg...........Null function argument.
         Sbe41Fail..............Attempt to start profile failed.
         Sbe41Ok................Attempt to start profile was successful.
                                 
      On success, the normal return value for this function will be 'Sbe41Ok'.
   \end{verbatim}
*/
int Sbe43Status(float *Ns,float *Nf,float *Tau20,int *Ido)
{
   #define MaxBufLen 79
   char *p,buf[MaxBufLen+1];

   /* define the parameter tokens */
   const char *const token[]=
   {
      "Ns =",
      "Nf =",
      "TAU_20 =",
      "oxygen S/N ="
   };

   /* initialize the return value */
   int i,status=Sbe41NoResponse;

   /* define the timeout period */
   const time_t TimeOut = 2;

   /* pet the watchdog timer */
   WatchDog();
   
   /* stack-check assertion */
   assert(StackOk());

   /* initialize the function parameters */
   if (Ns) (*Ns) = NaN();
   if (Nf) (*Nf) = NaN();
   if (Tau20) (*Tau20) = NaN();
   if (Ido) (*Ido) = (int)(-1);

   for (i=0; i<3 && status!=Sbe41Ok; i++)
   {
      /* flush the IO queues */
      if (ctdio.ioflush) {Wait(10); ctdio.ioflush(); Wait(10);}

      if (chat(&ctdio,"\r","S>",2)>0)
      {
         /* query the SBE41 for its current configuration */
         pputs(&ctdio,"dc\r",TimeOut,"");

         /* analyze the query response to verify expected configuration */
         while (pgets(&ctdio,buf,MaxBufLen,TimeOut,"\r\n")>0)
         {
            if      (Ns    && (p=strstr(buf,token[ 0]))) {*Ns    = atof(p+strlen(token[ 0]));} 
            else if (Nf    && (p=strstr(buf,token[ 1]))) {*Nf    = atof(p+strlen(token[ 1]));}
            else if (Tau20 && (p=strstr(buf,token[ 2]))) {*Tau20 = atof(p+strlen(token[ 2]));}
            else if (Ido   && (p=strstr(buf,token[ 3]))) {*Ido   = atoi(p+strlen(token[ 3]));}
            
            if (strstr(buf,"Nf")) break;
            
            status=Sbe41Ok;
         }
      }
   }
   
   /* validate each requested parameter */
   if (status>0)
   {
      if (Ns    && isNaN(*Ns))    status=Sbe41Fail;
      if (Nf    && isNaN(*Nf))    status=Sbe41Fail;
      if (Tau20 && isNaN(*Tau20)) status=Sbe41Fail;
      if (Ido   && (*Ido)==-1)   status=Sbe41Fail;
   }

   /* flush the IO queues */
   if (ctdio.ioflush) ctdio.ioflush();
   
   return status;

   #undef MaxBufLen
}

/*------------------------------------------------------------------------*/
/* function to compute the total pumping time of the SBE41/43F            */
/*------------------------------------------------------------------------*/
/**
   This function computes the total pumping time for the adaptive pumping
   scheme that SeaBird uses for the SBE41/43F.  The response time scale of
   the SBE43F is pressure and temperature dependent  The documentation used to
   write this function was the C source code for the main() function of the
   SBE41/43F's firmware (provided by Norge).
  
      \begin{verbatim}
      input:
         p ....... The insitu pressure (dbars).
         t ....... The insitu temperture (C).
         Tau1P ... The timescale of the oxygen sensor at 20C and atmospheric
                   pressure.
         N ....... The number of timescale periods to allow the pump to run.

      output:
         This function returns the total pumping time (seconds) for the
         SBE41/43F for values Ns=5.5, and Tau1P=2.5sec.  The following table
         shows a few values.
         
         Pres |          T e m p e r a t u r e (C)
         dbar | -2   0   2   4   8  12  16  20  24  28  30  36
         -----+-----------------------------------------------
            0 | 35  32  29  27  22  18  15  12  10   9   8   8
          100 | 35  32  30  27  22  18  15  12  10   9   8   8
          200 | 36  33  30  27  23  19  15  12  10   9   8   8
          300 | 36  33  30  28  23  19  15  13  10   9   8   8
          400 | 37  34  31  28  23  19  16  13  11   9   9   8
          500 | 37  34  31  29  24  19  16  13  11   9   9   8
          600 | 38  35  32  29  24  20  16  13  11   9   9   8
          700 | 38  35  32  29  24  20  16  13  11   9   9   8
          800 | 39  36  33  30  25  20  17  14  11  10   9   8
          900 | 40  36  33  30  25  21  17  14  11  10   9   9
         1000 | 40  37  34  31  26  21  17  14  12  10   9   9
         1100 | 41  37  34  31  26  21  17  14  12  10   9   9
         1200 | 41  38  35  32  26  22  18  14  12  10  10   9
         1300 | 42  38  35  32  27  22  18  15  12  10  10   9
         1400 | 43  39  36  33  27  22  18  15  12  10  10   9
         1500 | 43  40  36  33  27  22  18  15  12  11  10   9
         1600 | 44  40  37  34  28  23  19  15  13  11  10  10
         1700 | 44  41  37  34  28  23  19  15  13  11  10  10
         1800 | 45  41  38  35  29  23  19  16  13  11  10  10
         1900 | 46  42  38  35  29  24  19  16  13  11  11  10
         2000 | 46  43  39  36  29  24  20  16  13  11  11  10
      \end{verbatim}
*/
time_t Sbe43PumpTime(float p, float t, float Tau1P, int N)
{
   float Tau;
   time_t PumpTime;

   /* define the temperature coefficients (ref: SWJ16:259, p289) */
   const float Tcor = -4.1776e-2;

   /* define the pressure coefficient (ref: SWJ16:259, p289) */
   const float Pcor = 1.964e-4;

   /* define the SBE43F response timescale at 20C, 0dbar */
   if (Tau1P<=1 || Tau1P>10) Tau1P = 2.75;

   /* make sure the input temperature is well conditioned */
   if (t<-2 || t>40) t=4;

   /* make sure the input pressure is well conditioned */
   if (p<0 || p>2500) p=2000;

   /* compute the SBE43F response timescale at t,p (ref: SWJ16:259, p289) */
   Tau = Tau1P*exp(Pcor*p)*exp(Tcor*(t-20));

   /* make sure the response timescale is well conditioned */
   if (Tau<2.0) Tau=2.0; else if (Tau>30) Tau=30;

   /* compute the total pumping time of the SBE41/SBE43F */
   PumpTime = (time_t)(N*Tau + 0.5);

   /* make sure the pump time stays well conditioned */
   if (PumpTime<7) PumpTime=7; else if (PumpTime>100) PumpTime=100;
   
   return PumpTime;
}

/*------------------------------------------------------------------------*/
/* function to negotiate commands                                         */
/*------------------------------------------------------------------------*/
/**
   This function transmits a command string to the serial port and verifies
   and expected response.  The command string should not include that
   termination character (\r), as this function transmits the termination
   character after the command string is transmitted.  The command string
   may include wait-characters (~) to make the processing pause as needed.
   The command string is processed each byte in turn.  Each time a
   wait-character is encountered, processing is halted for one wait-period
   (1 sec) and then processing is resumed.
   
      \begin{verbatim}
      input:

         port.......A structure that contains pointers to machine dependent
                    primitive IO functions.  See the comment section of the
                    SerialPort structure for details.  The function checks
                    to be sure this pointer is not NULL.

         cmd........The command string to transmit.

         expect.....The expected response to the command string.

         sec........The number of seconds this function will attempt to
                    match the prompt-string.

      output:

         This function returns a positive number if the exchange was
         successful.  Zero is returned if the exchange failed.  A negative
         number is returned if the function parameters were determined to be
         ill-defined. 
         
      \end{verbatim}

   written by Dana Swift
*/
static int chat(const struct SerialPort *port, const char *cmd,
                const char *expect, time_t sec)
{
   /* define the logging signature */
   static cc FuncName[] = "sbe41.c::chat()";

   int status = -1;

   ConioEnable();
   
   /* verify the serialport */
   if (!port)
   {
      /* create the message */
      static cc msg[]="Serial port not ready.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* verify the cmd */
   else if (!cmd)
   {
      /* create the message */
      static cc msg[]="NULL pointer to the command string.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* verify the expect string */
   else if (!expect) 
   {
      /* create the message */
      static cc msg[]="NULL pointer to the expect string.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* verify the serial port's putb() function */
   else if (!port->putb)
   {
      /* create the message */
      static cc msg[]="NULL putb() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* verify the serial port's getb() function */
   else if (!port->getb)
   {
      /* create the message */
      static cc msg[]="NULL getb() function for serial port.\n";

      /* log the message */
      LogEntry(FuncName,msg);
   }

   /* verify the timeout period */
   else if (sec<=0) 
   {
      /* create the message */
      static cc format[]="Invalid time-out period: %ld\n";

      /* log the message */
      LogEntry(FuncName,format,sec);
   }

   /* flush the IO buffers prior to sending the command string */
   else if (pflushio(port)<=0) 
   {
      /* create the message */
      static cc msg[]="Attempt to flush IO buffers failed.";

      /* log the message */
      LogEntry(FuncName,msg);
   }
  
   else
   {
      /* compute the length of the command string */
      int i,len=strlen(cmd);

      /* work around a time descretization problem */
      if (sec==1) sec=2;

      CtdEnableIo(); Wait(50);

      /* transmit the command to the serial port */
      for (status=0, i=0; i<len; i++)
      {
         if (port->putb(cmd[i])<=0)
         {
            /* create the message */
            static cc msg[]="Attempt to send command string (%s) failed.\n";

            /* log the message */
            ConioEnable(); LogEntry(FuncName,msg,cmd);

            goto Err;
         }
      }
      
      /* seek the expect string in the modem response */
      if (*expect)
      {
         unsigned char byte;

         /* get the reference time */
         time_t Tnow,To=time(NULL);
         
         /* compute the length of the prompt string */
         int len=strlen(expect);
      
         /* define the index of prompt string */
         i=0;
         
         do 
         {
            /* read the next byte from the serial port */
            if (pgetb(port,&byte,1)>0)
            {
               /* check if the current byte matches the expected byte from the prompt */
               if (byte==expect[i]) {i++;} else i=0;

               /* the expect-string has been found if the index (i) matches its length */
               if (i>=len) {status=1; break;}
            }
            
            /* get the current time */
            Tnow=time(NULL);
         }

         /* check the termination conditions */
         while (Tnow>=0 && To>=0 && difftime(Tnow,To)<sec);
         
         /* write the response string if the prompt was found */
         if (status<=0)
         {
            /* create the message */
            static cc format[]="Expected string [%s] not received.\n";

            /* make logentry */
            ConioEnable(); LogEntry(FuncName,format,expect);
         }
         
         /* report a successful chat session */
         else if (debuglevel>=3)
         {
           /* create the message */
            static cc format[]="Expected response [%s] received.\n";

            /* make logentry */
            ConioEnable(); LogEntry(FuncName,format,expect);
         }
      }
      else status=1;
   }

   Err: /* collection point for errors */

   CtdEnableIo();

   return status;
}
