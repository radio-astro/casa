import pylab as PL
import sys
import time
import math
    

def LogMessage(Category='Title', LogLevel=2, FileOut=False, ConsoleOut=True, Msg='...'):
    """
    Category: Title, Error, Warning, Timer, Progress, Info, Debug, Other
    FileOut: False: no output, 'FileName': output to file
    ConsoleOut: True: print on console, False: not print on console
    LogLevel: Show message if the level higher than LogLevel
       LogLevel=0: Title, Error
       LogLevel=1: Warning, Timer, Progress
       LogLevel=2: Info
       LogLevel=3: Debug
       LogLevel=4: Other      
    Msg: 'Message'
    """
    CATEGORY = Category.upper()
    if CATEGORY == 'TITLE':
        Level = 0
        #Heading = '\nTITLE: '
        Heading = ''
    elif CATEGORY == 'ERROR':
        Level = 0
        Heading = '\nERROR: '
    elif CATEGORY == 'WARNING':
        Level = 1
        Heading = 'WARNING: '
    elif CATEGORY == 'TIMER':
        Level = 1
        Heading = ''
    elif CATEGORY == 'PROGRESS':
        Level = 1
        Heading = 'PROGRESS: '
    elif CATEGORY == 'INFO':
        Level = 2
        Heading = 'INFO: '
    elif CATEGORY == 'DEBUG':
        Level = 3
        Heading = 'DEBUG: '
    else:
        Level = 4
        Heading = 'OTHER: '

    if Level > LogLevel: return

    if ConsoleOut:
        print '%s%s' % (Heading, Msg)
        sys.stdout.flush()
    if FileOut != False:
        print >> FileOut, '%s%s' %(Heading, Msg)
        FileOut.flush()

    return


def FreqWindow2Channel(Abcissa, SpectrumWindow):
    """
    Convert Frequency Window (GHz) to Channel Window
    Input:
      Abcissa: [0]:channel [1]:Frequency(GHz) [2]:Velocity(Km/s) [3]:Wavelength(mm)
      SpectrumWindow: [[CenterFreq(GHz), minVel(Km/s), maxVel(Km/s)],[CF,,],,,]
          if one velocity is specified (e.g., [CF, Vel]), read as [CenterFreq, -Vel, +Vel]
    Output:
      SpWin: [[CenterChannel, FullWidthChannel, True],,,]
      SpWinRange: [[channel0, channel1],,,,]
    """
    SpWin = []
    SpWinRange = []
    for n in range(len(SpectrumWindow)):
        # Check if the specified Freq. is within the range
        if SpectrumWindow[n][0] < max(Abcissa[1][0], Abcissa[1][-1]) and \
           SpectrumWindow[n][0] > min(Abcissa[1][0], Abcissa[1][-1]):
            # Convert Velocity to Freq.
            if len(SpectrumWindow[n]) == 2:
               MaxVel = abs(SpectrumWindow[n][1])
               MinVel = abs(SpectrumWindow[n][1]) * -1.0
            else:
               MaxVel = SpectrumWindow[n][2]
               MinVel = SpectrumWindow[n][1]
            MinFreq = SpectrumWindow[n][0] * (1.0 - MinVel / 299792.458)
            MaxFreq = SpectrumWindow[n][0] * (1.0 - MaxVel / 299792.458)
            # Convert Freq. to Channel
            DiffMinFreq = MinFreq
            DiffMaxFreq = MaxFreq
            MinChan = Abcissa[0][0]
            MaxChan = Abcissa[0][0]
            for i in range(len(Abcissa[0])):
                if abs(Abcissa[1][i] - MinFreq) < DiffMinFreq:
                    DiffMinFreq = abs(Abcissa[1][i] - MinFreq)
                    MinChan = i
                if abs(Abcissa[1][i] - MaxFreq) < DiffMaxFreq:
                    DiffMaxFreq = abs(Abcissa[1][i] - MaxFreq)
                    MaxChan = i
            if MinChan > MaxChan:
                tmpChan = MaxChan
                MaxChan = MinChan
                MinChan = tmpChan
            if MinChan > Abcissa[0][0]: MinChan -= 1
            if MaxChan < Abcissa[0][-1]: MaxChan += 1
            SpWinRange.append([int(MinChan), int(MaxChan)])
            SpWin.append([int((MinChan + MaxChan)/2.0 + 0.5), int(MaxChan - MinChan + 1), True])
    return(SpWin, SpWinRange)


def PlotCheck(Flag, CurrentIteration, TotalIteration):
    """
    Flag: True, False, or 'Last'
    """
    if Flag != False:
        if Flag != 'Last' or CurrentIteration == TotalIteration: return True
    return False



