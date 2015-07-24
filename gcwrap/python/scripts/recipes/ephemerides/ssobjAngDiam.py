# Function to return angular diameter of an SS object
#
# Use MeasComent functions in Measures
# Note: alternate way to get the diameter is via
# solar_system_setjy.py but may return slightly different
# value even with the same ephemeris table
 
import glob
import os
import taskinit

def ssobjangdiam(srcName, epoch, ephemdata="",unit=""):
    """
    srcName: solar system object name (case insensitive)
    epoch: in string format (e.g. "2015/09/01:12:00:00")
    ephemdata: ephemeris data (default = "", look up the standard ~/alma/JPL_Horizons in the data repo
    unit: unit for the output ang. diam.
    """
  
    defaultDataPath = os.getenv("CASAPATH").split()[0] + "/data/ephemerides/JPL-Horizons/"
    me = taskinit.metool()
    qa = taskinit.qatool()
    if ephemdata!="":
       datapath = ephemdata
    else:
       mjd = me.epoch("utc",epoch)['m0']['value'] 
       datapath = findEphemTable(defaultDataPath, srcName, mjd)

    me.framecomet(datapath)
    me.doframe(me.epoch("utc", epoch))
    me.doframe(me.observatory("ALMA"))
   
    angdiamrad= me.cometangdiam()
    if unit=="" or unit=="rad":
        angdiam = angdiamrad
    else:
        angdiam = qa.convert(qa.quantity(angdiamrad), unit)

    return angdiam


def findEphemTable(datapath,srcName, mjd):

    if not os.path.exists(datapath):
       raise IOError, "%s does not exist"

    thetable = ""
    tblist = glob.glob(datapath+"*_J2000.tab")
    for t in tblist:
        tbname = os.path.basename(t)
        (obj, mjdrangestr,therest) = tbname.split('_')
        mjdrange = mjdrangestr.strip('dUTC')
        if obj.upper() == srcName.upper():
            startmjd = float(mjdrange.split('-')[0]) 
            endmjd = float(mjdrange.split('-')[1]) 
            if startmjd <= mjd and endmjd > mjd:
               thetable = t 
               break
      
    if thetable == "":
        raise IOError, "Input epoch(mjd) = %s is out of the ranges of available ephemeris tables" % mjd
    return thetable
                   
