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
        print 'ran __init__'

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
    
    def glider(self, dn):
        busyCount = 0
        noCarrierCount = 0
        noDialToneCount = 0
        totalCount = 0
        success = 0
        check = self.cmd('',cmdTimeout)
        if check and check.find('>') >= 0:
            print '>'
            time.sleep(1)
            m_hello = self.cmd('m_hello', cmdTimeout)
            print 'm_hello'
            time.sleep(1)
            if m_hello and m_hello.find('>') >= 0:
                print '>'
                time.sleep(1)
                twreng = self.cmd('twreng', cmdTimeout)
                print 'twreng'
                time.sleep(1)
                if twreng and twreng.find('Password:') >= 0:
                    print 'Password:'
                    time.sleep(1)
                    password = self.cmd('pikabo', cmdTimeout)
                    time.sleep(1)
                    print 'eng>'
                    time.sleep(1)
                    m_hello2 = self.cmd('m_hello', cmdTimeout)
                    print 'm_hello'
                    time.sleep(1)
                    if m_hello2 and m_hello2.find('eng>') >= 0:
                        print 'eng>'
                        time.sleep(1)
                        buoy_stop = self.cmd('buoy_stop', cmdTimeout)
                        print 'buoy_stop'
                        time.sleep(1)
                        if buoy_stop and buoy_stop.find('eng>') >= 0:
                            print 'eng>'
                            time.sleep(1)
                            m_production = self.cmd('m_production', cmdTimeout)
                            print 'm_production'
                            time.sleep(1)
                            if m_production and m_production.find('eng>') >= 0:
                                print 'eng>'
                                time.sleep(1)
                                m_state = self.cmd('m_state', 0)
                                print 'm_state'
                                time.sleep(1)
                                if m_state and m_state.find('Mission State: PRODUCTION') >= 0:
                                    print 'Mission State: PRODUCTION'
                                    time.sleep(1)
                                    chat = self.cmd('sys_chat COM2 B9600', cmdTimeout)
                                    print 'sys_chat COM2 B9600'
                                    time.sleep(1)
                                    if chat and chat.find('Connected!') >= 0:
                                        print 'Connected!'
                                        time.sleep(3)
                                        idOn = self.cmd('id', cmdTimeout)
                                        idOn2 = self.cmd('id', cmdTimeout)
                                        print 'id'
                                        time.sleep(1)
                                        if idOn2 and idOn2.find('ice detect mode on') >= 0:
                                            print 'ice detect mode on'
                                            time.sleep(1)
                                            return
                        
                        
        elif check and check.find('eng>') >= 0:
            print 'eng>'

##            
##                self.send('\nVehicle Name: test\r\n\r\n')
##                time.sleep(5)
##                zModem = os.system(' '.join(['sz','-w','2048','-b','/home/localuser/test.txt','</dev/tty_dgrp_pk_3 >/dev/tty_dgrp_pk_3']))
##                if str(zModem) == '0':
##                    print 'Test passed'
##                else:
##                    print 'Test failed'
##            else:
##                print 'Test failed'
           
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
