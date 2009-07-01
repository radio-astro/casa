import math
import asap as sd
import casac
from numarray import *
import numpy as NP
import pyfits
import shutil
import re


def Class2ASAP(filenamein, filenameRoot, ExposureTime=10.0, ExposureLag=3.0, MetaFile=False):
    # Copy original
    shutil.copy(filenamein, filenameRoot+'.fits')
    # Open file with pyfits
    hdulist = pyfits.open(filenameRoot+'.fits', mode='update')
    # Extention header
    hdr = hdulist[1].header
    hdr['EXTNAME'] = 'SINGLE DISH'
    hdr.update('NMATRIX', 1, before='MAXIS')
    if not hdr.has_key('TFORM3'):
        hdr.update('TFORM3', hdr['TFORM003'], after='TTYPE3')
        del hdr['TFORM003']
    hdr['TTYPE3'] = 'DATA'
    hdr.update('TDIM3', '(%s,%s,%s,%s)' % (hdr['MAXIS1'],hdr['MAXIS2'],hdr['MAXIS3'],hdr['MAXIS4']), after='TFORM3')

    # Store original pointing
    RA = hdr['CRVAL2']
    DEC = hdr['CRVAL3']
    Nrows = hdr['NAXIS2']
    Data = hdulist[1].data
    DRA = Data.field(0)
    DDEC = Data.field(1)
    # Calculate actual pointing
    for i in range(Nrows):
        NewRA = DRA[i] + float(RA)
        NewDEC = DDEC[i] + float(DEC)
        Data[i].setfield(0, NewRA)
        Data[i].setfield(1, NewDEC)

    # Modify header keywords
    hdr['TTYPE1'] = 'CRVAL2'
    hdr['TTYPE2'] = 'CRVAL3'
    del hdr['CRVAL2']
    del hdr['CRVAL3']
    hdr.update('CDELT2', 0.0, before='CRPIX2')
    hdr.update('CDELT3', 0.0, before='CRPIX3')
    RestFreq = hdr['RESTFREQ']
    Telescope = hdr['TELESCOP']

    # Write FITS
    hdulist.flush()
    hdulist.close()
    

    # Convert format into ASAP
    s=sd.scantable(filenameRoot+'.fits', average=False)
    # if no unit has been set, Kelvin is assumed
    fluxunit = s.get_fluxunit()
    if fluxunit == '': s.set_fluxunit('K')
    
    s.save(name=filenameRoot+'.ASAP', format='ASAP', overwrite=True)
    del s


    azelExist = False

    # Modify Header on ASAP
    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()
    tb.open(filenameRoot+'.ASAP', nomodify=False)
    #fixed telescope name so that it is easily recongnized in later processing
    telName=tb.getkeyword('AntennaName')
    if re.match('^APEX',telName):
      telName = 'APEX'
    elif re.match('^HHT|^SMT',telName):
      telName = 'SMT'
    else:
      pass
    tb.putkeyword('AntennaName',telName)
 
    #check telescope position
    AntPosExist = True
    antpos = tb.getkeyword('AntennaPosition')
    zarr = NP.zeros(3)
    if all(antpos == zarr):AntPosExist = False 
 
    # renumerate SCANNO with rows
    scol = tb.getcol('SCANNO')
    zarr2 = NP.zeros(len(scol))
    if all(scol==zarr2): 
        newscol = NP.arange(len(scol))
    else:
        newscol = scol
    tb.putcol('SCANNO', newscol)

    InitialTime = tb.getcol('TIME')[0]
    if MetaFile == False:
        array1 = []
        array2 = []
        for i in xrange(Nrows):
            array1.append(float(ExposureTime))
            array2.append(i * float(ExposureTime + ExposureLag)/3600./24. + InitialTime)
    else:
        azExist = False
        elExist = False
        #keys = ['ROW', 'INTEGRATION', 'TIME', 'TSYS']
        keysNoAzEl = ['ROWNUMBER', 'INTEGRATIONTIME', 'UNIXTIME', 'TSYS']
        #
        # with Az El - may need to be revised as actual meta data with Az/El
        # become available.  
        keysAzEl = ['ROWNUMBER', 'INTEGRATIONTIME', 'UNIXTIME', 'TSYS', 'AZIMUTH', 'ELEVATION']
        Meta = open(MetaFile, 'r')
        while 1:
            line = Meta.readline()
            if not line: break
            elif line[0] == '#': continue
            elif line[0] == '!':
                # Keywork should be 'row, Integration, Time, Tsys'
                key = line[1:-1].upper().replace(' ', '').split(',')
                for x in key:
                    if re.match('AZIMUTH', x):
                        azExist = True
                        if re.search('DEG', x): azConv = 3.141592653/180.
                        else: azConv = 1.0
                    if re.match('ELEVATION', x):
                        elExist = True
                        if re.search('DEG', x): elConv = 3.141592653/180.
                        else: elConv = 1.0
                if azExist == True and elExist == True:
                    azelExist = True
                    keys = keysAzEl
                else:
                    azelExist = False
                    keys = keysNoAzEl

                modline = re.sub('\[\S+\]','',line[1:-1])
                key = modline.upper().replace(' ', '').split(',')
                nkey = len(key)

                keyindex = []
                plotdata = []
                n = 0
                try:
                    for x in keys:
                        keyindex.append((n, key.index(x)))
                        plotdata.append([])
                        if re.match('TSYS', x): plotdata[n].append([])
                        #if re.match('AZIMUTH', x): azelExist = True
                        n = n + 1
                except ValueError:
                    print 'Key %s could not find in the header' % (x)
                    return
            else:
                valuelist = line[:-1].replace(' ', '').split(',')
                if len(valuelist) != nkey: continue
                for (n, i) in keyindex: 
                    if n == 3:
                        plotdata[n][0].append(float(valuelist[i]))
                    else:
                        plotdata[n].append(float(valuelist[i]))
        Meta.close()
        array1 = plotdata[1]
        array2 = plotdata[2]
        array3 = plotdata[3]
        # for Az/El data
        if azelExist:
            array4 = NP.array(plotdata[4]) * azConv
            array5 = NP.array(plotdata[5]) * elConv
        t0 = array2[0]
        for i in range(len(array2)): array2[i] = (array2[i] - t0)/3600./24. + InitialTime
     

    if len(array1) == Nrows:
        tb.putcol('TIME', NP.array(array2))
        tb.putcol('INTERVAL', NP.array(array1))
        if MetaFile != False:
            tb.putcol('TSYS', NP.array(array3))
            # add here to put Az El from meta data
            if azelExist:
                tb.putcol('AZIMUTH', NP.array(array4))
                tb.putcol('ELEVATION', NP.array(array5))
        tb.ok()
        tb.close()
    else:
        tb.close()
        print 'Number of Spectrum and Meta data length mismatch!'
        return

    # if MetaFile = True and az/el data are available in the meta data
    # it tries to use those values, 
    # otherwise try to calculate it from telescope position 
    #
    # Calculate Az El if necessary
    #if not MetaFile:
    if not azelExist:
        print "No az/el data from the meta data, try to calculate them using the telescope position..."
        if AntPosExist:
            s=sd.scantable(filenameRoot+'.ASAP')
            s.recalc_azel()
            s.save(name=filenameRoot+'.ASAP', format='ASAP', overwrite=True)
            del s
        else:
            print "WARNING: Missing antenna position. az/el cannot be calculated.\n"

    print 'Beam radius (degree) is: %s / Antenna Diameter (m)' % (2.9979E+8/RestFreq*180.0/3.141592653)
    print 'Telescope is: %s' % Telescope




def AddTimeGap(filename, ROWS=[], Gap=100.0):
    # add time gap (seconds) after each ROWS
    # Modify Header on ASAP
    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()
    tb.open(filename, nomodify=False)
    Time = tb.getcol('TIME')
    DayGap = Gap / 3600. / 24.

    # Calculate time
    for row in ROWS:
        for x in range(row, len(Time)):
            Time[x] += DayGap
    tb.putcol('TIME', Time)

    # Save Changes
    tb.ok()
    tb.close()
    

def RemoveRows(filename, ROWS=[]):
    # Remove specified rows
    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()
    tb.open(filename, nomodify=False)

    tb.removerows(ROWS)

    # Save Changes
    tb.ok()
    tb.close()
    
