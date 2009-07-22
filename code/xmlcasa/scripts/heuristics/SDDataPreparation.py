# started to modified for numarray -> numpy
# added/fixed necessary info for imaging 08-02-15 TT
# modified by TT list->array
#
import os
import asap as sd
import casac
#import numarray as NA
import numpy as NP
import re
import sys

import SDTool as SDT

Rad2Deg = 180. / 3.141592653

def ReadData(filename, TableOut, SummaryOut):
    # Read header table to make new table file for SDpipeline
    # Table format:
    #  ID,  Row, Scan,  IF, Pol, Beam, Date,  Time, ElapsedTime, Exptime,
    #  int, int,  int, int, int,  int,  str, float,       float,   float,
    #                                         sec,         sec,     sec,
    #
    #    RA,   DEC,    Az,    El, nchan,  Tsys, TargetName
    # float, float, float, float,   int, float, str
    #   deg,   deg,   deg,   deg
    #
    # TableOut: name of the output ascii table file
    # SummaryOut: name of the output summary text
    # return: dict: DataTable[ID]
    #   DataTable(old): [Row, Scan, IF, Pol, Beam, Date, Time, ElapsedTime,
    #                 0     1   2    3     4     5     6            7
    #               Exptime, RA, DEC, Az, El, nchan, Tsys, TargetName, 
    #                     8   9   10  11  12     13    14          15
    #               LowFreqRMS, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, 
    #                       16      17      18          19          20
    #               flag1, flag2, flag3, flag4, flagTsys, Nmask, MaskList, NoChange]
    #                  21     22     23     24        25     26        27        28
    #   DataTable: [Row, Scan, IF, Pol, Beam, Date, Time, ElapsedTime,
    #                 0     1   2    3     4     5     6            7
    #               Exptime, RA, DEC, Az, El, nchan, Tsys, TargetName, 
    #                     8   9   10  11  12     13    14          15
    #               Statistics, Flags, PermanentFlags, SummaryFlag, Nmask, MaskList, NoChange]
    #                       16,    17,             18,          19,    20,       21,       22
    #   Statistics: DataTable[ID][16] =
    #               [LowFreqRMS, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, ExpectedRMS, ExpectedRMS]
    #                         0       1       2           3           4            5            6
    #   Flags: DataTable[ID][17] =
    #               [LowFrRMSFlag, PostFitRMSFlag, PreFitRMSFlag, PostFitRMSdiff, PreFitRMSdiff, PostFitExpRMSFlag, PreFitExpRMSFlag]
    #                           0               1              2               3              4                  5                 6
    #   PermanentFlags: DataTable[ID][18] =
    #               [WeatherFlag, TsysFlag, UserFlag]
    #                          0         1         2
    #  flag = 1: valid data,  flag = 0: flag out
    # "LowFreqRMS, NewRMS, ..." fields were added only into the dictionary.
    # OutPut:
    #  SpStorage: Array where all raw and reduced spectra are stored
    #    Raw data, Result, Averaged data
    #  Abcissa: is array storing channel information
    #    Abcissa[0]: row number
    #    Abcissa[1]: Frequency (GHz)
    #    Abcissa[2]: LSRK velocity (km/s)
    #    Abcissa[3]: Wavelength (mm)

    # ASDM data
    import sys
    p=sys.path
    olderversion=False
    for ip in p:
        if ( ip.find('20.0.5654') != -1 or ip.find('23.1.6826') ):
            olderversion=True
            break
    if ( os.path.isdir(filename) and os.path.exists(filename+'/ASDM.xml') ):
        tasks.importasdm(asdm=filename,corr_mode='all',srt='all',time_sampling='all',ocorr_mode='ao',async=False)
        if olderversion:
            s=sd.scantable(filename+'.ms',average=False)
        else:
            s=sd.scantable(filename+'.ms',average=False,getpt=True)
        s.set_freqframe( 'TOPO' )
        #os.system('\\rm -rf '+filename+'.ms')
        filename=filename+'.ms'
        os.system('\\rm -rf '+filename+'.ms.flagversions')
    else:
        if olderversion:
            s = sd.scantable(filename, average=False)
        else:
            s = sd.scantable(filename, average=False, getpt=True)
    #s = sd.scantable(filename, average=False)
    outfile = open(SummaryOut, 'w')
    print >> outfile, '<html><body>'
    print >> outfile, '<h3>Input data file: %s</h3>' % filename
    print >> outfile, '<img src="./RADEC.png">'
    print >> outfile, '<p>Figure 1: The figure shows individual telescope pointings (blue dots) and telescope slew (green line) along with the first pointing position with a beam pattern (red circle) and last pointing position (red dot).</p>'
    print >> outfile, '<HR>'
    print >> outfile, '<img src="./AzEl.png">'
    #print >> outfile, '<p>Figure 2: Upper Panel: Elevation is plotted against MJD (Modified Julian DaY). Vertical cyan lines (if any) represent larger gaps of observing time.  Lower Panel: Azimuth position is plotted against MJD. Vertical green lines (if any) show the time when the pointing changed largely.</p>'
    print >> outfile, '<p>Figure 2: Upper Panel: Elevation is plotted against  Time (UT). Vertical cyan lines (if any) represent larger gaps of observing time.  Lower Panel: Azimuth position is plotted against Time (UT). Vertical green lines (if any) show the time when the pointing changed largely.</p>'
    print >> outfile, '<HR>'
    print >> outfile, '<code>'
    tmp = s._summary()
#    s.summary('tmpfile')
#    infile = open('tmpfile', 'r')
#    while 1:
#        line = infile.readline()
#        if not line: break
#        print >> outfile, '%s<br>' % line[:-1]
#    infile.close()
    print >> outfile, tmp.replace('\n', '<br>\n')
    print >> outfile, '</code>'
    print >> outfile, '</body></html>'
    del tmp
    outfile.close()

    # If the format is either sdfits or rpf, save it to asap format
    if filename[-5:].upper().find('FIT') != -1 or filename[-5:].upper().find('RPF') != -1:
        tmpfilename = filename+'.tmpms'
        s.save(tmpfilename, format='asap')
        filename = tmpfilename

    nrow = s.nrow()
    npol = s.npol()
    nbeam = s.nbeam()
    nif = s.nif()
    nchan = s.nchan()
    Tsys = s.get_tsys()

    ##SpStorage = NA.zeros((3, nrow, nchan), type=NA.Float32)
    SpStorage = NP.zeros((3, nrow, nchan), dtype=NP.float32)
    # Raw data, Result data, Averaged data
    ##Abcissa = NA.zeros((4, nchan), type=NA.Float32)
    Abcissa = NP.zeros((4, nchan), dtype=NP.float32)
    # row, Frequency(GHz), LSRK velocity(km/s), Wavelength(mm)

    for x in range(nrow):
        ##SpStorage[0][x] = NA.array(s._getspectrum(x))
        SpStorage[0][x] = NP.array(s._getspectrum(x))

#    if format.upper() == 'MS':
    if s.get_azimuth()[0] == 0: s.recalc_azel()
    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()
    tb.open(filename)
    Texpt = tb.getcol('INTERVAL')
#    Ttime = tb.getcol('TIME_CENTROID')
        # ASAP doesn't know the rows for cal are included in s.nrow()
        # nrow = len(Ttime)
    tb.close()
#    Texpt = 5.0
#    tb.open(filename+'/POINTING')
#    [[RA], [DEC]] = tb.getcol('DIRECTION')
#    tb.close()
        
    DataTable = {}
    # Save file name to be able to load the special setup needed for the
    # flagging based on the expected RMS.
    DataTable['FileName'] = filename
    outfile = open(TableOut, 'w')
    outfile.write("!ID,Row,Scan,IF,Pol,Beam,Date,MJD,ElapsedTime,ExpTime,RA,Dec,Az,El,Nchan,Tsys,TargetName\n")

    ID = 0
    ROWs = []
    IDs = []
    qatool = casac.homefinder.find_home_by_name('quantaHome')
    qa = qatool.create()
    for x in range(nrow):
        [sRA, sDEC] = s.get_direction(x).split(' ')
        RA = sRA.split(':')
        DEC = sDEC.split('.')
        RAdeg = float(RA[0]) * 15.0 + float(RA[1]) / 4.0 + float(RA[2]) / 240.0
        DECsign = (DEC[0][0] == '-') * (-2.0) + 1.0
        DECdeg = DECsign * (float(DEC[0]) * DECsign + float(DEC[1]) / 60.0 + float(DEC[2]) / 3600.0 + float(DEC[3]) / 36000.0)
        if ( sd.__version__ == '2.1.1' ):
            Ttime = s.get_time(x)
        else:
            Ttime = s.get_time(x,True)
        sDate = ("%4d-%02d-%02d" % (Ttime.year, Ttime.month, Ttime.day))
        # Calculate MJD
        sTime = ("%4d/%02d/%02d/%02d:%02d:%.1f" % (Ttime.year, Ttime.month, Ttime.day, Ttime.hour, Ttime.minute, Ttime.second))
        qTime = qa.quantity(sTime)
        MJD = qTime['value']
        
        #mo = int(-(14 - Ttime.month) / 12)
        #pp = int((1461 * (Ttime.year + 4800 + mo)) / 4) \
        #      + (int(367 * (Ttime.month - 2 - mo * 12)) / 12)
        #MJD = pp - int(3 * int((Ttime.year + 4900 + mo) / 100) / 4) - 32075.5 \
        #      + Ttime.day + Ttime.hour / 24.0 + Ttime.minute / 1440. \
        #      + Ttime.second / 86400. - 2400000.5
        if x == 0: MJD0 = MJD
        sName = s.get_sourcename(x)
        # If it is not a cal data
        if sName.find('_calon') < 0:
            outfile.write("%d,%d,%d,%d,%d,%d,%s,%.8f,%.3f,%.3f,%.8f,%.8f,%.3f,%.3f,%d,%f,%s\n" % \
                    (ID, x, s.getscan(x), s.getif(x), s.getpol(x), s.getbeam(x), \
                     sDate, MJD, (MJD - MJD0) * 86400., Texpt[ID], \
                     RAdeg, DECdeg, \
                     s.get_azimuth(x)*Rad2Deg, s.get_elevation(x)*Rad2Deg, \
                     s.nchan(s.getif(x)), Tsys[x], sName))
            DataTable[ID] = [int(x), int(s.getscan(x)), int(s.getif(x)), int(s.getpol(x)), int(s.getbeam(x)), \
                     sDate, MJD, (MJD - MJD0) * 86400., Texpt[ID], \
                     RAdeg, DECdeg, \
                     s.get_azimuth(x)*Rad2Deg, s.get_elevation(x)*Rad2Deg, \
                     int(s.nchan(s.getif(x))), Tsys[x], sName, \
                     [-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0], \
                     [1, 1, 1, 1, 1, 1, 1], \
                     [1, 1, 1], \
                     1, 0, [], False]
            ROWs.append(int(x))
            IDs.append(int(ID))
            ID += 1
    outfile.close()
    listall = [IDs, ROWs, list(s.getscannos()), list(s.getifnos()), list(s.getpolnos()), list(s.getbeamnos())]
    # Get telescope name
    TelName=s.get_antennaname()

    # Get Abcissa info (Frequency) for every IF
    # Abcissa[IF] [0]:channel [1]:Frequency(GHz) [2]:Velocity(Km/s) [3]:Wavelength(mm)
    Abcissa = {}
    for IF in listall[3]:
        #print 'IF=', IF
        # Select one IF
        sel = s.get_selection()
        sel.set_ifs(IF)
        s.set_selection(sel)
        Abcissa[IF] = []
        # Set Abcissa info
        s.set_unit('channel')
        ##Abcissa[IF].append(NA.array(s.get_abcissa()[0]))
        Abcissa[IF].append(NP.array(s.get_abcissa()[0]))
        #s.set_unit('Hz')
        s.set_unit('GHz')
        #RestFreq = s.get_restfreqs()
        #temporary fix
        #print s.get_restfreqs()
        if type(s.get_restfreqs())==dict:
            RestFreq = s.get_restfreqs().values()[0][0]/1.0e9
            #RestFreq = s.get_restfreqs()[IF][0]/1.0e9
            #RestFreq = s.get_restfreqs()[0][0]/1.0e9
        else:
            #RestFreq = s.get_restfreqs()[IF]/1.0e9
            RestFreq = s.get_restfreqs()[0]/1.0e9
        #if re.match('^AP', TelName) or re.match('^SMT', TelName) or re.match('^HHT', TelName):
        if re.match('APEX$', TelName) or re.match('^SMT', TelName) or re.match('^HHT', TelName):
            ##Abcissa[IF].append(NA.array(s.get_abcissa()[0]) + RestFreq)
            Abcissa[IF].append(NP.array(s.get_abcissa()[0]) + RestFreq)
        else:
            ##Abcissa[IF].append(NA.array(s.get_abcissa()[0]))
            Abcissa[IF].append(NP.array(s.get_abcissa()[0]))
        #is this correct?
        #original
        Abcissa[IF].append(((Abcissa[IF][1] / RestFreq) - 1.0) * 299792.458)
        #modified version by TT
        ##Abcissa[IF].append((1.0 - (Abcissa[IF][1] / RestFreq)) * 299792.458)
        #
        #Abcissa[3] = 299792.458e+6 / Abcissa[1]
        Abcissa[IF].append(299.792458 / Abcissa[IF][1])
        #Abcissa[1] = Abcissa[1] / 1.0e+9

    #debug (TT)
    #firstif=listall[3][0]
    #for x in range(nchan):
    #    tempoutfile.write("%d,%.8f,%.8f\n" % (Abcissa[firstif][0][x],Abcissa[firstif][1][x],Abcissa[firstif][2][x]))
    #tempoutfile.close()
    s.set_unit('channel')
    del s
    return (DataTable, listall, SpStorage, Abcissa)


# Added by TT 
# renumerate SCANNO so that different scan number for each row
def RenumASAPData(filename):
    # renumerate ASAP scantable
    import os.path
    filename = os.path.expandvars(filename)
    filename = os.path.expanduser(filename)
    if os.path.isdir(filename) \
                    and not os.path.exists(filename+'/table.f1'):
    	# crude check if asap table
	if os.path.exists(filename+'/table.info'):
            isasapf = True
            tabname = filename
        else:
            isasapf = False
            tabname = filename+'.ASAP'
    s = sd.scantable(filename, average=False)
    nr = s.nrow()
    scnos = s.getscannos()
    if not isasapf:
        s.save(tabname)
##  26/06/09 TN
##  Renumbering SCANNO is always enabled.
##     if len(scnos) < nr :
## 	tbtool = casac.homefinder.find_home_by_name('tableHome')
## 	tb = tbtool.create()
##     	tb.open(tabname, nomodify=False)
##     	scol = tb.getcol('SCANNO')
##         #newscol = NP.arange(len(scol))
##         newscol = NP.arange(len(scol)).tolist()
##         ok = tb.putcol('SCANNO',newscol)
##         tb.flush()
##         tb.close()
##         tmpout = sd.scantable(tabname,False)
##     else:
##         tmpout = s.copy()
    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()
    tb.open(tabname, nomodify=False)
    scol = tb.getcol('SCANNO')
    #newscol = NP.arange(len(scol))
    newscol = NP.arange(len(scol)).tolist()
    ok = tb.putcol('SCANNO',newscol)
    tb.flush()
    tb.close()
    tmpout = sd.scantable(tabname,False)

    del s    
    return(tmpout)

def WriteMSData(filename, MSout, SpStorage, DataTable, outform='ASAP', LogLevel=2, LogFile=False):
    # Write result in MS format
    #
    # filename: reference file from which header and tables are copied
    # MSout: output file with MS format
    # SpStorage: Numarray of processed spectra.  SpStorage[3][nrows]
    #             Overwrite SpStorage[1] on copied MS
    # DataTable: Dictionary includes flag info.  They will be stored
    #             as MS sub-table in the future.
    #            See comments of "ReadData" for the reference.

    import SDpipelineControl as SDC
    reload(SDC)
    #clip = SDC.SDParam['Gridding']['Clipping']
    rms_weight = SDC.SDParam['Gridding']['WeightRMS']
    # newrms = DataTable[row][16][1]
    # Weight = 1/(RMS**2)
    tsys_weight = SDC.SDParam['Gridding']['WeightTsysExptime']
    # tsys = DataTable[row][14]
    # exptime = DataTable[row][8]
    # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))

    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  WriteMSData  ' + '>'*15)
    s = sd.scantable(filename, average=False)

    #if fluxunit is unset, set the default unit, K
    funit=s.get_fluxunit()
    if funit == '':
        s.set_fluxunit('K')

    #get rest frequency
    #temporary fix
    if type(s.get_restfreqs())==dict:
        restf = s.get_restfreqs()[0][0]
    else:
        restf = s.get_restfreqs()[0]
    TelName=s.get_antennaname()

    # Store baselined data
    #sp = list(s._getspectrum(0))
    #for row in range(s.nrow()):
    #    for i in range(len(sp)):
    #        sp[i] = float(SpStorage[1][row][i])
    #    s._setspectrum(sp, row)
    
    ## TT modification 2008-09-24
    #for row in range(s.nrow()):
    #    s._setspectrum(NP.array(SpStorage[1][row],dtype=NP.float64), row)
    ## GK modification 2009-02-11
    for row in range(s.nrow()):
        nChan = len(s._getspectrum(row))
        s._setspectrum(NP.array(SpStorage[1][row][0:nChan],dtype=NP.float64), row)

    # create a temporary file
    s.save(MSout, format='ASAP', overwrite=True)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Write Baselined Spectra to Disk as %s (ASAP format)' % MSout)

    # now need to fix rel freqs to abs freqs for APEX, HHT
    if re.match('^GBT', TelName) or re.match('^DV', TelName) or re.match('^DA', TelName):
        pass # do nothing
    else:
        # fix and fill in some missing info
        # assume frequencies are  relative as in APEX and SMT data
        tbtool = casac.homefinder.find_home_by_name('tableHome')
        tb = tbtool.create()
        tb.open(MSout+'/FREQUENCIES', nomodify=False)
        refval = NP.array(tb.getcol('REFVAL')) + restf
        tb.putcol('REFVAL',refval)
        freqframe = tb.getkeyword('FRAME')
        tb.flush()
        tb.close()
        # insert frequency reference info in main table
        tb.open(MSout, nomodify=False)
        tb.putkeyword('FreqRefFrame',freqframe)
        tb.putkeyword('FreqRefVal',refval[0])
        flagcol = tb.getcol('FLAGTRA')
        (nchan, nr)=flagcol.shape
        flagged = NP.ones(nchan,int)
        for row in range(tb.nrows()):
            #if sum(DataTable[row][21:23]) > 1.5 and DataTable[row][25] > 0.5:
            #if sum(DataTable[row][17][1:3]) > 1.5 and DataTable[row][17][5] > 0.5:
            # Check Summary Flag 2008/6/5 DataTable[row][19] == 1: data is valid
            if DataTable[row][19] == 1:
            # set flag = 0 (no flag?)
                pass
            else:
            # set to flag
                flagcol[:,row] = flagged
                SDT.LogMessage('INFO', LogLevel, LogFile, Msg='set flag col to 1 for row= %s' % row)

        tb.putcol('FLAGTRA',flagcol)
        tb.close()
    temps=sd.scantable(MSout)
    temps.save(MSout+'.temp')
    del temps
    if outform=='MS2':
        s = sd.scantable(MSout+'.temp', average=False)
        s.save(MSout, format='MS2', overwrite=True)
        # 2008/9/24 weight added
        #tb.open(MSout, nomodify=False)
        #IW = tb.getcol('IMAGING_WEIGHT')
        #sIW = IW.swapaxes(0,1)
        #if rms_weight == True:
        #    for row in range(tb.nrows()):
        #        sIW[row] = 1.0 / (DataTable[row][16][1] ** 2.0)
        #elif tsys_weight == True:
        #    for row in range(tb.nrows()):
        #        if DataTable[row][14] == 0.0:
        #            sIW[row] = 0.0
        #        else:
        #            sIW[row] = DataTable[row][8] / (DataTable[row][14] ** 2.0)
        #IW = sIW.swapaxes(0,1)
        #tb.putcol('IMAGING_WEIGHT', IW)
        #tb.flush()
        #tb.close()

        # for APEX data (2009/4/24)
        tb.open(MSout+'/OBSERVATION',nomodify=False)
        telname=tb.getcol('TELESCOPE_NAME')
        for i in range(len(telname)):
            if ( telname[i].find('APEX-12m') != -1 ):
                telname[i] = 'ALMA'
        tb.putcol('TELESCOPE_NAME',telname)
        tb.flush()
        tb.close()
        #
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Write Baselined Spectra to Disk as %s (MS format)' % MSout)
    else:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Write Baselined Spectra to Disk as %s (ASAP format)' % MSout)
    del s
    os.system('rm -rf '+MSout+'.temp')


# Modified by TT; use of lists-> (numpy) arrays 
def WriteNewMSData(ReferenceFile, MSout, SpStorage, Table, outform='ASAP', LogLevel=2, LogFile=False):
    # Write result in MS format
    #
    # ReferenceFile: reference file from which header and tables are copied
    # MSout: output file with MS format
    # SpStorage: Numarray of processed spectra.  SpStorage[nrows][Spectrum]
    # Table format:
    #    [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
    #     [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
    #             ......
    #     [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]]
    #      0   1    2  3  4   5    6                  7

    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  WriteNewMSData  ' + '>'*15)
    OutRows = len(Table)
    s = sd.scantable(ReferenceFile, average=False)
    nrow = s.nrow()
    #if fluxunit is unset, set the default unit, K
    funit=s.get_fluxunit()
    if funit == '':
        s.set_fluxunit('K')
    #get rest frequency
    #temporary fix
    if type(s.get_restfreqs())==dict:
        restf = s.get_restfreqs()[0][0]
    else:
        restf = s.get_restfreqs()[0]
    TelName=s.get_antennaname()
    #use temporary file
    #s.save(MSout, format='ASAP', overwrite=True)
    s.save(MSout.rstrip('/')+'.tmp', format='ASAP', overwrite=True)
    #s.save(MSout, format=outform, overwrite=True)
    del s
    #n = int((OutRows - 1) / nrow) + 1
    #if n <= 1:
    #    s.save(MSout, format='ASAP', overwrite=True)
    #    del s
    #else:
    #    t = sd.merge([s]*n)
    #    nrow *= n
    #    t.save(MSout, format='ASAP', overwrite=True)
    #    del s, t

    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()

    # fix and fill in some missing info
    # assume frequencies are  relative as in APEX and SMT data
    if re.match('^GBT', TelName) or re.match('^DV', TelName) or re.match('DA', TelName):
        pass # do nothing
    else:
        #use temporary file
        #tb.open(MSout+'/FREQUENCIES', nomodify=False)
        tb.open(MSout.rstrip('/')+'.tmp/FREQUENCIES', nomodify=False)
        refval = NP.array(tb.getcol('REFVAL')) + restf
        tb.putcol('REFVAL',refval)
        freqframe = tb.getkeyword('FRAME')
        tb.flush()
        tb.close()
        # insert frequency reference info in main table
        #use temporary file
        #tb.open(MSout, nomodify=False)
        tb.open(MSout.rstrip('/')+'.tmp', nomodify=False)
        tb.putkeyword('FreqRefFrame',freqframe)
        tb.putkeyword('FreqRefVal',refval[0])
        tb.flush()
        tb.close()

    #use temporary file
    #tb.open(MSout, nomodify=False)
    tb.open(MSout.rstrip('/')+'.tmp', nomodify=False)
    #L_time = []
    #L_direction = {}
    L_dir1 = []
    L_dir2 = []
    L_ifno = []
    L_polno = []
    #L_beamno = []
    #L_scanno = []
    #L_tsys = []
    L_interval = []
    Ref_time = tb.getcol('TIME')[0]
    #Ref_tsys = tb.getcol('TSYS')[0]
    Ref_tsys = tb.getcol('TSYS')[0][0]
    Ref_interval = tb.getcol('INTERVAL')[0]
    Src_name = tb.getcol('SRCNAME')[0]
    Fld_name = tb.getcol('FIELDNAME')[0]
    print 'OutRows=', OutRows, 'nrow=', nrow
    if OutRows < nrow:
        tb.removerows(range(OutRows, nrow))
    elif OutRows > nrow:
    #    print 'ERROR'
    #    sys.exit()
        tb.addrows(OutRows - nrow)
    print 'tb.nrows()=', tb.nrows()
    timeCol = NP.array([Ref_time]*OutRows, dtype='float64')
    # Time should be identical for each row
    # Otherwise the POINTING TABLE is created with a single row
    for row in range(OutRows): timeCol[row] += float(row/86400.)
    beamnoCol = NP.zeros([1,OutRows], dtype='int32')
    #scannoCol = NP.zeros([1,OutRows], dtype='int32') 
    scannoCol = NP.arange(OutRows, dtype='int32') 
    tsysCol = NP.array([[Ref_tsys]*OutRows])
    directionCol = tb.getcol('DIRECTION')
    ifnoCol = tb.getcol('IFNO')
    polnoCol = tb.getcol('POLNO')
    intervalCol = tb.getcol('INTERVAL')
    srcCol = NP.array([[Src_name]*OutRows])
    fldCol = NP.array([[Fld_name]*OutRows])
    ra = directionCol[0] 
    dec = directionCol[1] 
    # assume no flag, propermotion, srcdirection, and scanrate info.
    # and just use row 0 values for these
    flag = tb.getcell('FLAGTRA', 0)
    srcpm = tb.getcell('SRCPROPERMOTION', 0)
    srcdir = tb.getcell('SRCDIRECTION', 0)
    scanrate = tb.getcell('SCANRATE', 0)

  
    for row in range(OutRows):
        #L_time.append(Ref_time)
        #L_direction['r%s' % row] = [(Table[row][4] / Rad2Deg,), (Table[row][5] / Rad2Deg,)]
        #L_dir1.append(Table[row][4] / Rad2Deg,)
        #L_dir2.append(Table[row][5] / Rad2Deg,)
        ra[row] = Table[row][4] / Rad2Deg
        dec[row] = Table[row][5] /Rad2Deg
        #L_ifno.append(Table[row][0])
        ifnoCol[row] = Table[row][0]
        #L_polno.append(Table[row][1])
        polnoCol[row] = Table[row][1]
        #L_beamno.append(0)
        #L_scanno.append(0)
        #L_tsys.append(Ref_tsys)
        #L_interval.append(Ref_interval * Table[row][6])
        #intervalCol[row] = Ref_interval * Table[row][6]
        intervalCol[row] = 0.1
#    dir1 = NA.array(L_dir1,dtype='float64')
#    dir2 = NA.array(L_dir2,dtype='float64')
    #NewDirectionCol = NA.array([dir1, dir2])
    directionCol[0]=ra
    directionCol[1]=dec
    #origDir = tb.getcol('DIRECTION')
    tb.putcol('TIME', timeCol)
    #tb.putvarcol('DIRECTION', L_direction)
    #tb.putvarcol('DIRECTION', directionCol)
    tb.putcol('DIRECTION', directionCol)
    #tb.putcol('IFNO', L_ifno)
    tb.putcol('IFNO', ifnoCol)
    #tb.putcol('POLNO', L_polno)
    tb.putcol('POLNO', polnoCol)
    #tb.putcol('BEAMNO', L_beamno)
    tb.putcol('BEAMNO', beamnoCol)
    #tb.putcol('SCANNO', L_scanno)
    tb.putcol('SCANNO', scannoCol)
    #tb.putcol('TSYS', L_tsys)
    tb.putcol('TSYS', tsysCol)
    #tb.putcol('INTERVAL', L_interval)
    tb.putcol('INTERVAL', intervalCol) 
    tb.putcol('SRCNAME', srcCol)
    tb.putcol('FIELDNAME',fldCol)
    for row in range(OutRows):
        tb.putcell('SPECTRA', row, NP.array(SpStorage[row]))
        tb.putcell('FLAGTRA', row, NP.array(flag))
        tb.putcell('SRCPROPERMOTION', row, NP.array(srcpm))
        tb.putcell('SRCDIRECTION', row, NP.array(srcdir))
        tb.putcell('SCANRATE', row, NP.array(scanrate))

    tb.putkeyword('FreqRefFrame', 'LSRK')

    flagcol = tb.getcol('FLAGTRA')
    (nchan, nr)=flagcol.shape
    clearflag = NP.zeros(nchan,int)
    for row in range(tb.nrows()):
        flagcol[:,row] = clearflag
    tb.ok()
    tb.close()
    #del L_time, L_direction, L_ifno, L_polno, L_beamno, L_scanno, L_tsys, L_interval
    del timeCol, directionCol, ifnoCol, polnoCol, beamnoCol, scannoCol, tsysCol, intervalCol

    #use temporary file
    #s = sd.scantable(MSout, average=False)
    s = sd.scantable(MSout.rstrip('/')+'.tmp', average=False)
    #for row in range(OutRows):
    #    s._setspectrum(SpStorage[row], row)
    #s.recalc_azel()

    if outform=='MS2':
        savemsg='Write Re-Gridded Spectra to Disk as %s (MS format)'
    elif outform=='ASAP':
        savemsg='Write Re-Gridded Spectra to Disk as %s (ASAP format)'
    else:
        outform='ASAP'

    SDT.LogMessage('INFO', LogLevel, LogFile, Msg=savemsg % MSout)
    s.save(MSout, format=outform, overwrite=True)
    # for APEX data (2009/4/24)
    if ( outform=='MS2' ):
        tb.open(MSout+'/OBSERVATION',nomodify=False)
        telname=tb.getcol('TELESCOPE_NAME')
        for i in range(len(telname)):
            if ( telname[i].find('APEX-12m') != -1 ):
                telname[i] = 'ALMA'
        tb.putcol('TELESCOPE_NAME',telname)
        tb.flush()
        tb.close()
    #
    #delete temporary file
    del s
    os.system('rm -rf '+MSout+'.tmp')
