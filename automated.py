###########################################################################
#                                                                         #
#                                 root_bang                               #
#                                 #########                               #
#                                                                         #
# parameters: readFile, file that contains test commands and responses    #
#             writeFile, file that conatins the results of the tests      #
#                                                                         #
# returns: none                                                           #
#                                                                         #
# Opens the readFile then checks each line while incrementing lineCount.  #
# If the ? command is found, it checks each subsequent line for the       #
# the correct list of possible commands. It ensures that the list is in   #
# the correct order by creating a string that is compared to an expected  #
# string at the end of the test. The prupose of the function is to test   #
# the command that lists all of the other commands.                       #
#                                                                         #
###########################################################################

def root_bang(readFile, writeFile):
    count = ""
    root_bang = -1
    lineCount = 0
    with open(readFile) as f:
        for line in f:
            lineCount = lineCount + 1
            if root_bang > 0:
                if 'fs_cat' in line:
                    count += '0'
                elif 'fs_cd' in line:
                    count += '1'
                elif 'fs_cp' in line:
                    count += '2'
                elif 'fs_date' in line:
                    count += '3'
                elif 'fs_df' in line:
                    count += '4'
                elif 'fs_ls' in line:
                    count += '5'
                elif 'fs_mv' in line:
                    count += '6'
                elif 'fs_od' in line:
                    count += '7'
                elif 'fs_pwd' in line:
                    count += '8'
                elif 'fs_rm' in line:
                    count += '9'
                elif 'fs_touch' in line:
                    count += '10'
                elif 'fs_wc' in line:
                    count += '11'
                elif 'term_fc' in line:
                    count += '12'
                elif 'sh_help' in line:
                    count += '13'
                elif 'ib_reset' in line:
                    count += '14'
                elif 'ib_print' in line:
                    count += '15'
                elif 'pt_start' in line:
                    count += '16'
                elif 'pt_stop' in line:
                    count += '17'
                elif 'pt_status' in line:
                    count += '18'
                elif 'vt_bladder' in line:
                    count += '19'
                elif 'vt_pump' in line:
                    count += '20'
                elif 'vt_battery' in line:
                    count += '21'
                elif 'vt_ctd' in line:
                    count += '22'
                elif 'vt_exp' in line:
                    count += '23'
                elif 'vt_humidity' in line:
                    count += '24'
                elif 'vt_leak' in line:
                    count += '25'
                elif 'vt_rf' in line:
                    count += '26'
                elif 'vt_vacuum' in line:
                    count += '27'
                elif 'mission_read' in line:
                    count += '28'
                elif 'mission_print' in line:
                    count += '29'
                elif 'mission_save' in line:
                    count += '30'
                elif 'mission_set' in line:
                    count += '31'
                elif 'mission_get' in line:
                    count += '32'
                elif count == '01234567891011121314151617181920212223242526272829303132' and  'air_inflate' in line:
                    count += '33'
                elif 'air_inflate_once' in line:
                    count += '34'
                elif 'air_stop_inflate' in line:
                    count += '35'
                elif 'air_deflate' in line:
                    count += '36'
                elif 'buoy_goto' in line:
                    count += '37'
                elif 'buoy_stop' in line:
                    count += '38'
                elif 'coul_amphr' in line:
                    count += '39'
                elif 'coul_current' in line:
                    count += '40'
                elif 'coul_raw_amphr' in line:
                    count += '41'
                elif 'coul_raw_current' in line:
                    count += '42'
                elif 'coul_clear' in line:
                    count += '43'
                elif 'pot_stream' in line:
                    count += '44'
                elif 'pot_read' in line:
                    count += '45'
                elif 'm_idle' in line:
                    count += '46'
                elif 'm_deploy' in line:
                    count += '47'
                elif 'm_hello' in line:
                    count += '48'
                elif 'm_bye' in line:
                    count += '49'
                elif 'm_state' in line:
                    count += '50'
                elif 'm_production' in line:
                    count += '51'
                elif 'gps_update' in line:
                    count += '52'
                elif 'irad_sample' in line:
                    count += '53'
                elif 'irad_config' in line:
                    count += '54'
                elif count == '0123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354' and  'rad_sample' in line:
                    count += '55'
                elif count == '012345678910111213141516171819202122232425262728293031323334353637383940414243444546474849505152535455' and  'rad_config' in line:
                    count += '56'
                elif 'optode_sample' in line:
                    count += '57'
                elif 'optode_config' in line:
                    count += '58'
                elif 'flbb_sample' in line:
                    count += '59'
                elif 'flbb_config' in line:
                    count += '60'
                elif 'tnt_heading' in line:
                    count += '61'
                elif 'tnt_config' in line:
                    count += '62'
                elif count == '01234567891011121314151617181920212223242526272829303132333435363738394041424344454647484950515253545556575859606162' and  'seabird_get_p' in line:
                    count += '63'
                elif count == '0123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263' and  'seabird_get_pt' in line:
                    count += '64'
                elif count == '012345678910111213141516171819202122232425262728293031323334353637383940414243444546474849505152535455565758596061626364' and  'seabird_get_pts' in line:
                    count += '65'
                elif 'log_up' in line:
                    count += '66'
                elif 'modem_serial' in line:
                    count += '67'
                elif 'modem_transfer' in line:
                    count += '68'
                elif 'modem_csq' in line:
                    count += '69'
                elif 'sys_chat' in line:
                    count += '70'
                elif 'sys_grep' in line:
                    count += '71'
                elif 'sys_date' in line:
                    count += '72'
                elif 'sys_clk_set' in line:
                    count += '73'
                elif 'sys_save_file' in line:
                    count += '74'
                elif 'sys_emerg_clr' in line:
                    count += '75'
                elif 'sys_capture' in line:
                    count += '76'
                elif 'sys_ver' in line:
                    count += '77'
                elif 'sys_show' in line:
                    count += '78'
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    if count == '0123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778':
                        f.write('at line ')
                        f.write(temp)
                        f.write(' root> ?: SUCCESS\n')
                        f.write('\n')
                        count = ""
                        root_bang = -1
                else:
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    f.write('at line ')
                    f.write(temp)
                    f.write(' root> ?: FAILED ......')
                    f.write('(error in line ')
                    f.write(str(lineCount))
                    f.write(')\n\n')
                    count = ""
                    root_bang = -1
                    
            if 'root> ?' in line:
                root_bang = 1
                commandLine = lineCount

            
###########################################################################
#                                                                         #
#                              mission_print                              #
#                              #############                              #
#                                                                         #
# parameters: readFile, file that contains test commands and responses    #
#             writeFile, file that conatins the results of the tests      #
#                                                                         #
# returns: none                                                           #
#                                                                         #
# Opens the readFile then checks each line while incrementing lineCount.  #
# If the mission_print command is found, it checks each subsequent lines  #
# for the correct list of parameters. There is also an option to use the  #
# values that have been parsed from the response. It ensures that the     #
# correct parameters are there by comparing a string created throughout   #
# the test like in the root_bang function.                                #
#                                                                         #
###########################################################################
                          
def mission_print(readFile, writeFile):
    count = ""
    mission_print = -1
    lineCount = 0
    with open(readFile) as f:
        for line in f:
            lineCount = lineCount + 1
            if mission_print > 0:
                if 'mission_save' in line:
                    mission_print = -1
                elif 'ActivateRecoveryMode' in line:
                    values0 = line.split()
                    ActivateRecoveryMode = values0[1]
                    ##print ActivateRecoveryMode
                    count += '0'
                elif 'AscentRate' in line:
                    values1= line.split()
                    AscentRate = float(values1[1])
                    ##print AscentRate
                    count += '1'
                elif 'AscentTimeout' in line:
                    values2 = line.split()
                    AscentTimeout = int(values2[1])
                    ##print AscentTimeout
                    count += '2'
                elif 'AscentTimerInterval' in line:
                    values3 = line.split()
                    AscentTimerInterval = values3[1]
                    ##print AscentTimerInterval
                    count += '3'
                elif  count == '0123' and 'BuoyancyNudge' in line:
                    values4 = line.split()
                    BuoyancyNudge = int(values4[1])
                    ##print BuoyancyNudge
                    count += '4'
                elif 'DeepDescentCount' in line:
                    values5 = line.split()
                    DeepDescentCount = int(values5[1])
                    ##print DeepDescentCount
                    count += '5'
                elif 'DeepDescentPressure' in line:
                    values6 = line.split()
                    DeepDescentPressure = float(values6[1])
                    ##print DeepDescentPressure
                    count += '6'
                elif 'DeepDescentTimeout' in line:
                    values7 = line.split()
                    DeepDescentTimeout = int(values7[1])
                    ##print DeepDescentTimeout
                    count += '7'
                elif 'DeepDescentTimerInterval' in line:
                    values8 = line.split()
                    DeepDescentTimerInterval = int(values8[1])
                    ##print DeepDescentTimerInterval
                    count += '8'
                elif 'DeepProfileFirst' in line:
                    values9 = line.split()
                    DeepProfileFirst = values9[1]
                    ##print DeepProfileFirst
                    count += '9'
                elif 'DownTime' in line:
                    values10 = line.split()
                    DownTime = values10[1]
                    ##print DownTime
                    count += '10'
                elif 'EmergencyTimerInterval' in line:
                    values11 = line.split()
                    EmergencyTimerInterval = int(values11[1])
                    ##print EmergencyTimerInterval
                    count += '11'
                elif 'IdleTimerInterval' in line:
                    values12 = line.split()
                    IdleTimerInterval = int(values12[1])
                    ##print IdleTimerInterval
                    count += '12'
                elif 'InitialBuoyancyNudge' in line:
                    values13 = line.split()
                    InitialBuoyancyNudge = int(values13[1])
                    ##print InitialBuoyancyNudge
                    count += '13'
                elif 'LeakDetect' in line:
                    values14 = line.split()
                    LeakDetect = values14[1]
                    ##print LeakDetect
                    count += '14'
                elif 'MActivationCount' in line:
                    values15 = line.split()
                    MActivationCount = int(values15[1])
                    ##print MActivationCount
                    count += '15'
                elif 'MActivationPressure' in line:
                    values16 = line.split()
                    MActivationPressure = float(values16[1])
                    ##print MActivationPressure
                    count += '16'
                elif 'MinVacuum' in line:
                    values17 = line.split()
                    MinVacuum = float(values17[1])
                    ##print MinVacuum
                    count += '17'
                elif 'ParkBuoyancyNudge' in line:
                    values18 = line.split()
                    ParkBuoyancyNudge = int(values18[1])
                    ##print ParkBuoyancyNudge
                    count += '18'
                elif 'ParkDeadBand' in line:
                    values19 = line.split()
                    ParkDeadBand = float(values19[1])
                    ##print ParkDeadBand
                    count += '19'
                elif 'ParkDescentCount' in line:
                    values20 = line.split()
                    ParkDescentCount = int(values20[1])
                    ##print ParkDescentCount
                    count += '20'
                elif 'ParkDescentTimeout' in line:
                    values21 = line.split()
                    ParkDescentTimeout = int(values21[1])
                    ##print ParkDescentTimeout
                    count += '21'
                elif 'ParkDescentTimerInterval' in line:
                    values22 = line.split()
                    ParkDescentTimerInterval = int(values22[1])
                    ##print ParkDescentTimerInterval
                    count += '22'
                elif 'ParkPressure' in line:
                    values23 = line.split()
                    ParkPressure = float(values23[1])
                    ##print ParkPressure
                    count += '23'
                elif 'ParkTimerInterval' in line:
                    values24 = line.split()
                    ParkTimerInterval = int(values24[1])
                    ##print ParkTimerInterval
                    count += '24'
                elif 'PnPCycleLen' in line:
                    values25 = line.split()
                    PnPCycleLen = int(values25[1])
                    ##print PnPCycleLen
                    count += '25'
                elif 'SurfacePressure' in line:
                    values26 = line.split()
                    SurfacePressure = float(values26[1])
                    ##print SurfacePressure
                    count += '26'
                elif 'TelemetryRetryInterval' in line:
                    values27 = line.split()
                    TelemetryRetryInterval = int(values27[1])
                    ##print TelemetryRetryInterval
                    count += '27'
                elif 'UpTime' in line:
                    values28 = line.split()
                    UpTime = int(values28[1])
                    ##print UpTime
                    count += '28'
                elif 'CheckSum' in line:
                    values29 = line.split()
                    CheckSum = int(values29[1])
                    ##print CheckSum
                    count += '29'
                    ##print count
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    if count == '01234567891011121314151617181920212223242526272829':
                        f.write('at line ')
                        f.write(temp)
                        f.write(' mission_print: SUCCESS\n')
                        f.write('\n')
                        count = ""
                        mission_print = -1
                else:
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    f.write('at line ')
                    f.write(temp)
                    f.write(' mission_print: FAILED ......')
                    f.write('(error in line ')
                    f.write(str(lineCount))
                    f.write(')\n\n')
                    count = ""
                    mission_print = -1
            if 'mission_print' in line:
                mission_print = 1
                commandLine = lineCount

        
###########################################################################
#                                                                         #
#                                 m_state                                 #
#                                 #######                                 #
#                                                                         #
# parameters: readFile, file that contains test commands and responses    #
#             writeFile, file that conatins the results of the tests      #
#                                                                         #
# returns: none                                                           #
#                                                                         #
# Opens the readFile then checks each line while incrementing lineCount.  #
# If the m_state command is found, it checks each subsequent lines for    #
# the correct list of parameters. There is also an option to use the      #
# values that have been parsed from the response. It ensures that the     #
# correct parameters are there by comparing a string created throughout   #
# the test like in the previous functions.                                #
#                                                                         #
###########################################################################

def m_state(readFile, writeFile):
    count = ""
    m_state = -1
    lineCount = 0
    with open(readFile) as f:
        for line in f:
            lineCount = lineCount +1
            if m_state > 0:
                if 'm_production' in line:
                    m_state = -1
                elif 'Mission State:' in line:
                    values0 = line.split()
                    Mission_State = values0[2]
                    ##print Mission_State
                    count += '0'
                elif 'Pressure Activation Depth:' in line:
                    values1 = line.split()
                    Pressure_Activation_Depth = values1[3]
                    ##print Pressure_Activation_Depth
                    count += '1'
                elif 'Standby Mode:' in line:
                    values2 = line.split()
                    Standby_Mode = values2[2]
                    ##print Standby_Mode
                    count +='2'
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    if count == '012':
                        f.write('at line ')
                        f.write(temp)
                        f.write(' m_state: SUCCESS\n')
                        f.write('\n')
                        count = ""
                        m_state = -1
                else:
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    f.write('at line ')
                    f.write(temp)
                    f.write(' m_state: FAILED ......')
                    f.write('(error in line ')
                    f.write(str(lineCount))
                    f.write(')\n\n')
                    count = ""
                    m_state = -1
            if 'm_state' in line:
                m_state = 1
                commandLine = lineCount

###########################################################################
#                                                                         #
#                                 m_deploy                                #
#                                 ########                                #
#                                                                         #
# parameters: readFile, file that contains test commands and responses    #
#             writeFile, file that conatins the results of the tests      #
#                                                                         #
# returns: none                                                           #
#                                                                         #
# Opens the readFile then checks each line while incrementing lineCount.  #
# If the m_deploy command is found, it checks each subsequent lines for   #
# the m_state command. Next, it will confirm that all of the mission      #
# parameters are correct for the current state after being deployed. It   #
# ensures that the correct parameters are there by comparing a string     #
# created throughout the test like in the previous functions.             #
#                                                                         #
###########################################################################

def m_deploy(readFile, writeFile):
    count = ""
    commandLine = -2
    m_deploy = -1
    lineCount = 0
    error = 0
    with open(readFile) as f:
        for line in f:
            lineCount = lineCount +1
            if m_deploy > 0:
                if 'm_hello' in line:
                    m_deploy = -1
                elif lineCount == commandLine + 1:
                    count += '0'
                elif 'root> m_state' in line:
                    count += '1'
                elif 'Mission State:' in line:
                    values0 = line.split()
                    Mission_State = values0[2]
                    ##print Mission_State
                    count += '2'
                    if Mission_State == 'PRELUDE':
                        count += '3'
                    else:
                        error = 1
                elif 'Pressure Activation Depth:' in line:
                    values1 = line.split()
                    Pressure_Activation_Depth = values1[3]
                    ##print Pressure_Activation_Depth
                    count += '4'
                elif 'Standby Mode:' in line:
                    values2 = line.split()
                    Standby_Mode = values2[2]
                    ##print Standby_Mode
                    count +='5'
                    if Standby_Mode == 'off':
                        count += '6'
                    else:
                        error = 2
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    if count == '0123456':
                        f.write('at line ')
                        f.write(temp)
                        f.write(' m_deploy: SUCCESS\n')
                        f.write('\n')
                        count = ""
                        m_deploy = -1
                else:
                    f = open(writeFile, 'a')
                    temp = str(commandLine)
                    f.write('at line ')
                    f.write(temp)
                    f.write(' m_deploy: FAILED ......')
                    f.write('(error in line ')
                    f.write(str(lineCount))
                    if error == 0:
                        f.write(')\n\n')
                    if error == 1:
                        f.write(': wrong Mission State)\n\n')
                    if error == 2:
                        f.write(': wrong Standby Mode)\n\n')
                    error = 0
                    count = ""
                    m_deploy = -1
            if 'm_deploy' in line:
                m_deploy = 1
                commandLine = lineCount

                

###########################################################################
#                                                                         #
#                                   test                                  #
#                                   ####                                  #
#                                                                         #
# parameters: readFile, file that contains test commands and responses    #
#             writeFile, file that conatins the results of the tests      #
#                                                                         #
# returns: none                                                           #
#                                                                         #
# Calls all of the functions to test the file readFile, it outputs the    #
# results to the file writeFile. Note, it appends the output to the end   #
# of the file rather than overwriting it.                                 #
#                                                                         #
###########################################################################

def test(readFile, writeFile):
    root_bang(readFile, writeFile)
    mission_print(readFile, writeFile)
    m_state(readFile, writeFile)
    m_deploy(readFile, writeFile)



test('logs/DeepApex0009041714.txt', 'logs/testfile.txt')

