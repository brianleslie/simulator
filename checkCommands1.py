# dialtest.py:
# Call a dockserver, pretend to be a glider, verify that it answers

import os
import serial
import socket
import subprocess
import sys
import time

echoTimeout = 1
huTimeout = 5
connTimeout = 40
cmdTimeout = 2


#reserved for testing
modemPort = 'COM8'

# Connect to dockserver over dialup.  Must match rudics class, below
class dialup(object):
    def __init__(self, portname):
        self.port = serial.Serial(port = portname, baudrate = 115200)
        print 'serial port configured...'

    # Empty input buffer, Send a string to the modem
    def send(self, s):
        self.port.flushInput()
        self.port.write(s)
    
    # Get response from the modem
    def getline(self, timeout):
        self.port.setTimeout(timeout)
        return self.port.readline()
    
    # Do a modem command: send string, wait for response line
    def cmd(self, s, timeout):
        done = False
        self.send(s + '\r')
        echo = self.getline(echoTimeout)
        if echo.strip() != s:
            print 'Echoed: "%s"' % echo
            return None
        while not done:
            response = self.getline(timeout)
            if response[0] != '\r':
                done = True
        return response.strip()
    
    def connect(self, dn):
        busyCount = 0
        noCarrierCount = 0
        totalCount = 0
        for n in xrange(5):
            result = self.cmd('atd ' + dn, connTimeout)
            print result
            if result and result.find('CONNECT 2400/NONE') >= 0:
                print 'Connected after' + totalCount + 'tries'
                break
            elif result and result.find('BUSY') >= 0:
                totalCount = totalCount + 1
                busyCount = busyCount + 1
                print 'BUSY on try' + busyCount + 'out of' + totalCount
            elif result and result.find('NO CARRIER') >= 0:
                totalCount = totalCount + 1
                noCarrierCount = noCarrierCount + 1
                print 'No carrier on try' + noCarrierCount + 'out of' + totalCount
        else:
            return False
        return True

    def runCmd(self, command, expect, commandTimeout):
        passed = self.cmd(command, commandTimeout)
        print command
        time.sleep(1)
        if passed and passed.find(expect) >= 0:
            print expect
            time.sleep(1)
            return 1
        else:
            return -1
            
    
    def glider(self, dn):
        busyCount = 0
        noCarrierCount = 0
        noDialToneCount = 0
        totalCount = 0
        success = 0
        if self.runCmd('m_hello','>', cmdTimeout) >= 0:
            if self.runCmd('twreng','Password:', cmdTimeout) >= 0:
                password = self.cmd('pikabo', cmdTimeout)
                time.sleep(1)
                print 'eng>'
                time.sleep(1)
                if self.runCmd('m_hello','eng>', cmdTimeout) >= 0:
                    if self.runCmd('buoy_stop','eng>', cmdTimeout) >= 0:
                        if self.runCmd('m_production','eng>', cmdTimeout) >= 0:
                            if self.runCmd('m_state','Mission State: PRODUCTION', 0) >= 0:
                                return
##                                if self.runCmd('sys_chat COM2 B9600','Connected!', cmdTimeout) >= 0:
##                                    time.sleep(2)
##                                    self.send('\r')
##                                    time.sleep(1)
##                                    self.send('\r')
##                                    print self.getline(1)
##                                    if self.getline(1) == 'S>':
##                                        ##print 'S>'
##                                        time.sleep(1)
##                                        if self.runCmd('id','ice detect mode on', cmdTimeout) >= 0:
##                                            print 'passed'
##                                        else:
##                                            print 'failed'
##                                            return
##                        
                        
        elif self.runCmd('m_hello','eng>', cmdTimeout) >= 0:
            if self.runCmd('buoy_stop','eng>', cmdTimeout) >= 0:
                if self.runCmd('m_production','eng>', cmdTimeout) >= 0:
                    if self.runCmd('m_state','Mission State: PRODUCTION', 0) >= 0:
                        if self.runCmd('sys_chat COM2 B9600','Connected!', cmdTimeout) >= 0:
                            time.sleep(2)
                            self.send('\r')
                            time.sleep(1)
                            self.send('\r')
                            self.getline(1)
                            print self.getline(1)
                            if self.getline(1) == 'S>':
                                ##print 'S>'
                                time.sleep(1)
                                if self.runCmd('id','ice detect mode on', cmdTimeout) >= 0:
                                    print 'passed'
                                else:
                                    print 'failed'
                                    return
        else:
            time.sleep(2)
            self.send('\r')
            time.sleep(1)
            self.send('\r')
            self.getline(1)
            s = self.getline(1)
            if s.find('S>') >= 0:
                print 'S>'
                time.sleep(1)
                if self.runCmd('id','ice detect mode on', cmdTimeout) >= 0:
                    print 'passed'
                else:
                    print 'failed'
                    return
            
           
    # Hang up the modem, this is special since the command does not echo
    def hu(self):
        for n in xrange(5):       
            self.send('+++')
            self.getline(huTimeout)
            response = self.getline(huTimeout)
            if response.strip() == 'NO CARRIER':
                return True
        return False   
  
# Connect to dockserver over rudics.  Must match dialup class, above
class rudics(object):
    def connect(self, server):
        (ip, port) = server.split(':')
        self.conn = socket.create_connection((ip, int(port)))
        return True
        
    def send(self, s):
        return self.conn.send(s)
    
    def getline(self, timeout):
        self.conn.settimeout(timeout)
        result = ''
        try:
            while True:
                c = self.conn.recv(1)
                if c == '\r' or c == '\n':
                    break
                result += c
        except socket.timeout:
            pass
        return result

    def glider(self, dn):
        self.glider(dn)
    
    def hu(self):
        self.conn.close()



    
    
# Complete comm test for a given host.  Host is either:
#   7818711051 (a phone #),
#   twr-dockserver.webbresearch.com:6565 (rudics w/ domain name), or
#   69.84.158.158:6565 (rudics w/ dotted quad )
#   We use the ":" to determine dialup vs rudics
def commtest():             
    transport = dialup(modemPort)                                    
    testPassed = transport.glider(modemPort)
    return



if __name__ == '__main__':
    commtest()
