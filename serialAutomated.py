import serial
import socket

#the baudrate
br = 9600

errors = ['cd']
errorIndex = -1

class test(object):

    #Initialize the serial port with the given name and baudrate
    def __init__ (self, portname):
        self.port = serial.Serial(port = portname, baudrate = br)

    #Empty input buffer, transmit a message
    def transmit(self, s):
        self.port.flushInput()
        self.port.write(s)

    #Get the response
    def recieve(self, timeout):
        self.port.setTimeout(timeout)
        return self.port.readline()

    #Allow for a back and forth test that involves sending a message,
    #waiting for a response, and verifying that the response is correct.

    def communicate(port):
        while(passed):
            result0 = self.transmit('twreng')
            if result0 and result0.find('password:') >= 0:
                errorIndex = 0
                passed = 1
            else:
                errorIndex = 0
                passed = -1
            result1 = self.transmit('pikabo')
            if result1 and result1.find('') >= 0:
                errorIndex++
                passed = 1
            else:
                errorIndex++
                passed = -1
            result2 = self.transmit('')
            if result2 and result2.find('root>') >= 0:
                errorIndex++
                passed = 1
            else:
                errorIndex++
                passed = -1
            result3 = self.transmit('?')
            if result3 and result3.find('') >= 0:
                errorIndex++
                passed = 1
            else:
                errorIndex++
                passed = -1
        if(errorIndex == -1):
            print 'All tests passed'
        else:
            print errors[errorIndex]
            print ' command failed'

if __name__ == '__main__':
    communicate(port)
