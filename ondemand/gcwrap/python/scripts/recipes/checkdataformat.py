# guess what kind (format) of the input data is
import commands
import os
from  casac import *

tb = casac.table()

# main function 
def dataformat(dataname):
    """
    find(guess) the data format of the input data
    by quick checks on the data without opening or loading
    fullly.
    Currently check if it is MS, ALMA/EVLA SDM with its version,
    ASAP scantable with its version, CASA image, some kind of
    FITS, or ASCII text. 
    """    
    isMS = False
    isASDM = False
    isASAP = False
    isCASAimage = False
    dataformat = 'unknown'


    # directory?
    if(commands.getoutput('file '+dataname).count('directory')):
        # check for MS, ASDM, scantable..., CASA image, ... 
        # try to ms.open
        try:
            checkms(dataname)
            isMS = True
        except:
            isMS = False
            #check for ASDM
            (isASDM,isEVLA,ver)=checkasdm(dataname)
            #if not isASDM
            if not isASDM:
                # is this ASAP scantable (for Single dish)
                try:
                    (isASAP,ver)=checkscantable(dataname)
                except:
                    # try if it is a CASA image
                    if(os.path.exists(dataname+'/table.dat')):
                         tb.open(dataname)
                         if tb.keywordnames().count('imageinfo')>0:
                             isCASAimage=True
                             dataformat = "CASA image"
                         elif tb.colnames()=='map' and \
                           any([k=='coords' for k in tb.keywordnames()]):
                             isCASAimage=True
                             dataformat ="CASA image" 

                         # todo: check for component?
    
                         tb.close()
        finally:
            if isMS:
                dataformat="MeasurementSet"
            if isASDM:
                if isEVLA:
                    dataformat="EVLA SDM"+ver
                else:
                    dataformat="ALMA SDM"+ver
            if isASAP:
                dataformat="ASAP Scantable ver."+ver

            print "%s is appeared to be %s " % (dataname, dataformat)
    elif(commands.getoutput('file '+dataname).count('text')):
         dataformat='ASCII'
    elif(commands.getoutput('file '+dataname).count('FITS')):
         print "Probably some kind of FITS (e.g. image fits,  uvfits, etc)" 
         dataformat='FITS'

    return dataformat


def checkms(dname):
    """
    check if the input data is an MS
    """
    isMS = False
    mstables= set(["table.dat",
                   "ANTENNA/table.dat",
                   "DATA_DESCRIPTION/table.dat",
                   "FEED/table.dat",
                   "FIELD/table.dat",
                   "FLAG_CMD/table.dat",
                   "HISTORY/table.dat",
                   "OBSERVATION/table.dat",
                   "POINTING/table.dat",
                   "POLARIZATION/table.dat",
                   "PROCESSOR/table.dat",
                   "SPECTRAL_WINDOW/table.dat",
                   "STATE/table.dat"
                   ])
    for dat in mstables:
        if not os.path.exists(dname+'/'+dat):
            isMS=False
            raise Exception
        else:
            isMS=True
    return 
    

def checkasdm(dname):
    """
    check if input data is ALMA/EVLA SDM
    """
    isASDM=False
    isEVLA=False
    version =''
    if(os.path.exists(dname+'/ASDM.xml')):
        from xml.etree.ElementTree import ElementTree
        rt = ElementTree(file=dname+'/ASDM.xml')
        iter = rt.getiterator()
        for k, n in iter[1].items():
            if n =='ASDM':
                isASDM=True
            if k=='schemaVersion':
                #if int(n) == 1:
                #    ver='2'
                #else:
                #    ver = str(n)
                ver = str(n)
                version='v1.'+ver
            elif k=='entityId':
                if n.count('evla'):
                    isEVLA=True
    else:
        isASDM=False
        isEVLA=False

    return (isASDM,isEVLA,version)     


def checkscantable(dname):
    """
    check if the input data is ASAP Scantable 
    """
    scantables = set(["table.dat",
                      "FREQUENCIES/table.dat",
                      "WEATHER/table.dat",
                      "FOCUS/table.dat",
                      "TCAL/table.dat",
                      "MOLECULES/table.dat",
                      "HISTORY/table.dat",
                      "FIT/table.dat"
                      ]) 
    for dat in scantables:
        if not os.path.exists(dname+'/'+dat):
            raise Exception
    tb.open(dname)
    version=tb.getkeyword('VERSION')
    tb.close()
    return (True, str(version)) 
