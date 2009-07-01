# modified by TT list->array
#
import asap as sd
import casac
import numarray as NA
import numpy as NP

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
    # return: dict: DataTable[ID] = [Row, Scan, IF, ..., TargetName, 
    #     LowFreqRMS, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, 
    #     flag1, flag2, flag3, flag4, Nmask]
    # "LowFreqRMS, NewRMS, ..." fields were added only into the dictionary.

    s = sd.scantable(filename, average=False)
    s.summary('tmpfile')
    infile = open('tmpfile', 'r')
    outfile = open(SummaryOut, 'w')
    print >> outfile, '<html><body>\n<code>'
    while 1:
        line = infile.readline()
        if not line: break
        print >> outfile, '%s<br>' % line[:-1]
    infile.close()
    print >> outfile, '</code>'
    print >> outfile, '<img src="./RADEC.png">'
    print >> outfile, '<img src="./AzEl.png">'
    print >> outfile, '</body></html>'
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

    SpStorage = NA.zeros((3, nrow, nchan), type=NA.Float32)
    for x in range(nrow):
        SpStorage[0][x] = NA.array(s._getspectrum(x))

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
    outfile = open(TableOut, 'w')
    outfile.write("!ID,Row,Scan,IF,Pol,Beam,Date,MJD,ElapsedTime,ExpTime,RA,Dec,Az,El,Nchan,Tsys,TargetName\n")

    ID = 0
    ROWs = []
    IDs = []
    for x in range(nrow):
        [sRA, sDEC] = s.get_direction(x).split(' ')
        RA = sRA.split(':')
        DEC = sDEC.split('.')
        RAdeg = float(RA[0]) * 15.0 + float(RA[1]) / 4.0 + float(RA[2]) / 240.0
        DECsign = (DEC[0][0] == '-') * (-2.0) + 1.0
        DECdeg = DECsign * (float(DEC[0]) * DECsign + float(DEC[1]) / 60.0 + float(DEC[2]) / 3600.0 + float(DEC[3]) / 36000.0)
        Ttime = s.get_time(x)
        sDate = ("%4d-%02d-%02d" % (Ttime.year, Ttime.month, Ttime.day))
        # Calculate MJD
        mo = int(-(14 - Ttime.month) / 12)
        pp = int((1461 * (Ttime.year + 4800 + mo)) / 4) \
              + (int(367 * (Ttime.month - 2 - mo * 12)) / 12)
        MJD = pp - int(3 * int((Ttime.year + 4900 + mo) / 100) / 4) - 32075.5 \
              + Ttime.day + Ttime.hour / 24.0 + Ttime.minute / 1440. \
              + Ttime.second / 86400. - 2400000.5
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
                     int(s.nchan(s.getif(x))), Tsys[x], sName, -1.0, -1.0, \
                     -1.0, -1.0, -1.0, 0, 0, 0, 0, 0]
            ROWs.append(int(x))
            IDs.append(int(ID))
            ID += 1
    outfile.close()
    listall = [IDs, ROWs, list(s.getscannos()), list(s.getifnos()), list(s.getpolnos()), list(s.getbeamnos())]
    del s
    return (DataTable, listall, SpStorage)

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
    if len(scnos) < nr :
	tbtool = casac.homefinder.find_home_by_name('tableHome')
	tb = tbtool.create()
    	tb.open(tabname, nomodify=False)
    	scol = tb.getcol('SCANNO')
        newscol = NP.arange(len(scol))
        ok = tb.putcol('SCANNO',newscol)
        tmpout = sd.scantable(tabname,False)
    else:
        tmpout = s.copy()
    del s    
    return(tmpout)

def WriteMSData(filename, MSout, SpStorage, DataTable, LogLevel=2, LogFile=False):
    # Write result in MS format
    #
    # filename: reference file from which header and tables are copied
    # MSout: output file with MS format
    # SpStorage: Numarray of processed spectra.  SpStorage[3][nrows]
    #             Overwrite SpStorage[1] on copied MS
    # DataTable: Dictionary includes flag info.  They will be stored
    #             as MS sub-table in the future.
    #            See comments of "ReadData" for the reference.

    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  WriteMSData  ' + '>'*15)
    s = sd.scantable(filename, average=False)
    nrow = s.nrow()

    for row in range(nrow):
        s._setspectrum(SpStorage[1][row], row)

    s.save(MSout, format='ASAP', overwrite=True)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Write Baselined Spectra to Disk as %s (ASAP format)' % MSout)
#    s.save(MSout, format='MS2', overwrite=False)
#    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Write Baselined Spectra to Disk as %s (MS format)' % MSout)

    del s

# Modified by TT; use of lists-> (numpy) arrays 
def WriteNewMSData(ReferenceFile, MSout, SpStorage, Table, LogLevel=2, LogFile=False):
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

    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  WriteNewMSData  ' + '>'*15)
    OutRows = len(Table)
    s = sd.scantable(ReferenceFile, average=False)
    nrow = s.nrow()
    s.save(MSout, format='ASAP', overwrite=True)
    del s

    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()
    tb.open(MSout, nomodify=False)
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
    if OutRows < nrow:
        tb.removerows(range(OutRows, nrow))
    elif OutRows > nrow:
        tb.addrows(OutRows - nrow)
    timeCol = NP.array([Ref_time]*OutRows, dtype='float64')
    beamnoCol = NP.zeros([1,OutRows], dtype='int32')
    scannoCol = NP.zeros([1,OutRows], dtype='int32') 
    tsysCol = NP.array([[Ref_tsys]*OutRows])
    directionCol = tb.getcol('DIRECTION')
    ifnoCol = tb.getcol('IFNO')
    polnoCol = tb.getcol('POLNO')
    intervalCol = tb.getcol('INTERVAL')
    ra = directionCol[0] 
    dec = directionCol[1] 
  
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
        intervalCol[row] = Ref_interval * Table[row][6]
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
    tb.ok()
    tb.close()
    #del L_time, L_direction, L_ifno, L_polno, L_beamno, L_scanno, L_tsys, L_interval
    del timeCol, directionCol, ifnoCol, polnoCol, beamnoCol, scannoCol, tsysCol, intervalCol

    s = sd.scantable(MSout, average=False)
    for row in range(OutRows):
        s._setspectrum(SpStorage[row], row)
    s.recalc_azel()

    s.save(MSout, format='ASAP', overwrite=True)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Write Re-Gridded Spectra to Disk as %s (ASAP format)' % MSout)
#    s.save(MSout, format='MS2', overwrite=True)
#    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Write Re-Gridded Spectra to Disk as %s (MS format)' % MSout)

    del s

