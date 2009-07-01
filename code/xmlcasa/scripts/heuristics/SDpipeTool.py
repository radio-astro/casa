"""
Tools for Single-dish Heuristic Pipeline
         George KOSUGI    2005/08
"""
def SDpipeTool_GetMJD(sdate, stime):
    """
    Calculate Modified Julian Day
    sdate: yyyy/mm/ss format string
    stime: hh:mm:ss.ss format string

    return: Modified Julian Day (float)
    """
    year = int(sdate[:4])
    month = int(sdate[5:7])
    day = int(sdate[8:10])

    if month <= 2: correct = -1
    else: correct = 0
    imjd = int(((1461 * (year + 4800 + correct)) / 4 + (367 * (month - 2
          - correct * 12)) / 12 - (3 * ((year + 4900 + correct) / 100)) / 4)
          + day - 2432076)
    return float(imjd) + SDpipeTool_Time2Sec(stime)/86400.0


def SDpipeTool_Time2Sec(stime):
    """
    Convert Time string 'hh:mm:ss.ss' format to second
    Accept 'm:ss.ss' or 'h:m:ss', etc. several variaties

    return: seconds (float)
    """
    ltime = stime.split(':')
    n = len(ltime)
    sec = 0.0
    for x in ltime:
        if n == 3:
            sec += float(x) * 3600.0
            n -= 1
            continue
        elif n == 2:
            sec += float(x) * 60.0
            n -= 1
            continue
        elif n == 1:
            sec += float(x)
            return sec
        else:
            print "Time format Error"
            return False


def SDpipeTool_RA2Degree(sRA):
    """
    Convert RA string 'HH:MM:SS.SSS' format to degree

    return: degree (float)
    """
    hour = float(sRA[:2])
    minute = float(sRA[3:5])
    second = float(sRA[6:])
    return (hour + minute / 60.0 + second / 3600.0) * 15.0
    

def SDpipeTool_DEC2Degree(sDEC):
    """
    Convert DEC string '-DD:MM:SS.SSS' format to degree

    return: degree (float)
    """
    if sDEC[0] == '-': sign = -1.0
    else: sign = 1.0
    degree = float(sDEC[1:3])
    minute = float(sDEC[4:6])
    second = float(sDEC[7:])
    return (degree + minute / 60.0 + second / 3600.0) * sign
    

def SDpipeTool_Plot1DArray(y):
    """
    Plotter for simple array
    Sample: 
        import numarray as NA
        SDpipeTool_Plot1DArray(NA.arange(len(array)), array)
    """
    import pylab as PL
    PL.clf()
    PL.plot(y)
    PL.show()

