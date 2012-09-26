from casac import *
import os
import commands
import math
import pdb
import shutil
import string
import time
import ast
from taskinit import casalog,qa,ms

'''
A set of helper functions for the tasks tflagdata and flagcmd.
I/O functions:
    readFile
    readXML
    makeDict
    readAntennaList
    writeAntennaList
    writeFlagCmd
    writeRflagThresholdFile
    
Parameter handling
    compressSelectionList
    fixType
    getLinePars
    getNumPar
    getReason
    getSelectionPars
    getUnion
    purgeEmptyPars
    purgeParameter
    setupAgent
    
Others
    backupFlags
    convertDictToString
    convertStringToDict
    extractAntennaInfo
    extractRflagOutputFromSummary
    
'''
###some helper tools
tb=casac.table()


debug = False

#######################################################
#
#     Reading functions
#
#######################################################
def readFile(inputfile):
    '''Read in the lines from an input file
    inputfile -->  file in disk with a list of strings per line
    
    Returns a list. Blank lines are skipped.
    '''        
    
    flagfile = inputfile

    if (type(flagfile) == str) & os.path.exists(flagfile):
        try:
            ff = open(flagfile, 'r')
        except:
            raise Exception, 'Error opening file ' + flagfile
    else:
        raise Exception, \
            'ASCII file not found - please verify the name'
            
    #
    # Parse file
    try:
        cmdlist = []
        for line in ff:
            cmd = line.rstrip()
            cmdlist.append(cmd)

    except:
        raise Exception, 'Error reading lines from file ' \
            + flagtable
            
    ff.close()
    
    return cmdlist

# jagonzal (CAS-4119): Use absolute paths for input files to ensure that the engines find them
def addAbsPath(input_file):
    '''Read in the lines from an input file
    inputfile -->  file in disk with a list of strings per line
    
    Re-writes the file changing relative file names to absolute file names
    '''        

    output_file = input_file + ".tmp"
    if (type(input_file) == str) & os.path.exists(input_file):
        try:
            input_file_id = open(input_file, 'r')
        except:
            raise Exception, 'Error opening file ' + input_file
        
        try:
            output_file_id = open(output_file, 'w')
        except:
            output_file_id.close()
            raise Exception, 'Error opening file ' + output_file
    else:
        raise Exception, \
            'ASCII file not found - please verify the name'
            
    #
    # Parse file
    try:
        for line in input_file_id:
            cmd = line.rstrip()
            cmd_pars = cmd.split(" ")
            for par in cmd_pars:
                if ((par.count("inpfile") > 0) or 
                    (par.count("outfile") > 0) or 
                    (par.count("addantenna") > 0) or 
                    (par.count("timedev") > 0) or 
                    (par.count("freqdev") > 0)):
                    par = par.split("=")
                    file_local = par[1]
                    if file_local.count("'")>0:
                        file_local = file_local.split("'")
                        file_local = file_local[1]
                    elif file_local.count('"')>0:
                        file_local = file_local.split('"')
                        file_local = file_local[1]
                    else:
                        continue
                    file_abs = os.path.abspath(file_local)
                    cmd = cmd.replace(file_local,file_abs)
                    
            output_file_id.write(cmd+"\n")

    except:
        input_file_id.close()
        output_file_id.close()
        raise Exception, 'Error reading lines from file ' \
            + flagtable
            
    input_file_id.close()
    output_file_id.close()
    os.rename(output_file, input_file)

def makeDict(cmdlist, myreason='any'):
    '''Make a dictionary compatible with a FLAG_CMD table structure
       and select by reason if any is given
    
       cmdlist --> list of parameters to go into the COMMAND column
       myreason --> reason to select from
       
       Returns a dictionary with the the selected rows with the following 
       structure and default values:
       
        flagd['row']      =
        flagd['applied']  = False
        flagd['command']  = ''
        flagd['interval'] = 0.0
        flagd['level']    = 0
        flagd['reason']   = ''
        flagd['severity'] = 0
        flagd['time']     = 0.0
        flagd['type']     = 'FLAG'

    '''
    
    if cmdlist.__len__() == 0:
        raise Exception, 'Empty list of commands'

    # select by these reasons
    myreaslist = []
    if type(myreason) == str:
        if myreason != 'any':
            myreaslist.append(myreason)
    elif type(myreason) == list:
        myreaslist = myreason
    else:
        casalog.post('Cannot read reason; it contains unknown variable types',
                     'ERROR')
        return
    
    if debug:
        print "reason in selection"
        print myreaslist

    # List of reasons in input file
    nrows = cmdlist.__len__()
    
    # If any selection was requested
    reaslist = []
    for i in range(nrows):            
        command = cmdlist[i]
        if command.__contains__('reason'):
            reaslist.append(getReason(command))
        else:
            # so that cmdlist and reaslist have the same sizes
            reaslist.append('NoReasonToMatch')
        
    if debug:
        print "reason in input"
        print reaslist
            
    
    # Defaults for columns
    applied = False
    interval = 0.0
    level = 0
    severity = 0
    coltime = 0.0
    coltype = 'FLAG'
    
    myflagd = {}
    ncmds = 0
    rowlist = range(nrows)

    # If select by reason is requested
    if myreaslist.__len__() > 0:
        rowl = []

        # If selection matches what is in input line
        for j in range(reaslist.__len__()):
            if myreaslist.count(reaslist[j]) > 0:
                rowl.append(j)
        rowlist = rowl

    try:
        for i in rowlist:            
            flagd = {}
            reason = ''

            command = cmdlist[i]
            if reaslist[i] != 'NoReasonToMatch':
                reason = reaslist[i]
                                
            # Skip comment lines
            if command.startswith('#'):
                continue
            if command == '':
                casalog.post('Ignoring empty command line', 'WARN')
                continue
            if command.__contains__('summary'):
                casalog.post('Mode summary is not allowed in list operation', 'WARN')
                continue

            # If shadow, remove the addantenna dictionary
            if command.__contains__('shadow') and command.__contains__('addantenna'):
                i0 = command.rfind('addantenna')
                if command[i0+11] == '{':
                    # It is a dictionary. Remove it from line
                    i1 = command.rfind('}')
                    antpar = command[i0+11:i1+1]
                    temp = command[i0:i1+1]
                    newcmd = command.replace(temp,'')
                    antpardict = convertStringToDict(antpar)
                    flagd['addantenna'] = antpardict
                    command = newcmd                                
                        
            flagd['row'] = str(i)
            flagd['applied'] = applied
            flagd['reason'] = reason 
            
            # Remove reason from command line   
            command = command.rstrip()
            newline = purgeParameter(command, 'reason')
            
            # Remove any empty parameter (CAS-4015)
            command = purgeEmptyPars(newline)
            command = command.strip()
            
            flagd['command'] = command
            flagd['interval'] = interval
            flagd['level'] = level
            flagd['severity'] = severity
            flagd['time'] = coltime
            flagd['type'] = coltype
            # Insert into main dictionary
            myflagd[ncmds] = flagd
            ncmds += 1
                
    except:
        raise Exception, 'Cannot create dictionary'
    
    casalog.post(':makeDict::myflagd=%s'%myflagd,'DEBUG')
    
    return myflagd



def readXML(sdmfile, mytbuff):
    '''
#   readflagxml: reads Antenna.xml and Flag.xml SDM tables and parses
#                into returned dictionary as flag command strings
#      sdmfile (string)  path to SDM containing Antenna.xml and Flag.xml
#      mytbuff (float)   time interval (start and end) padding (seconds)
#
#   Usage: myflags = readflagxml(sdmfile,tbuff)
#
#   Dictionary structure:
#   fid : 'id' (string)
#         'mode' (string)         flag mode ('online')
#         'antenna' (string)
#         'timerange' (string)
#         'reason' (string)
#         'time' (float)          in mjd seconds
#         'interval' (float)      in mjd seconds
#         'command' (string)          string (for COMMAND col in FLAG_CMD)
#         'type' (string)         'FLAG' / 'UNFLAG'
#         'applied' (bool)        set to True here on read-in
#         'level' (int)           set to 0 here on read-in
#         'severity' (int)        set to 0 here on read-in
#
#   Updated STM 2011-11-02 handle new SDM Flag.xml format from ALMA
#   Updated STM 2012-02-14 handle spectral window indices, names, IDs
#   Updated STM 2012-02-21 handle polarization types
#
#   Mode to use for spectral window selection in commands:
#   spwmode =  0 none (flag all spw)
#   spwmode =  1 use name
#   spwmode = -1 use index (counting rows in SpectralWindow.xml)
#
#   Mode to use for polarization selection in commands:
#   polmode =  0 none (flag all pols/corrs)
#   polmode =  1 use polarization type
#
#   CURRENT DEFAULT: Use spw names, flag pols
    '''

    spwmode = 1
    polmode = 1

#
    try:
        from xml.dom import minidom
    except ImportError, e:
        print 'failed to load xml.dom.minidom:\n', e
        exit(1)

    if type(mytbuff) != float:
        casalog.post('Found incorrect type for tbuff','SEVERE')
        exit(1)
#        mytbuff = 1.0

    # make sure Flag.xml and Antenna.xml are available (SpectralWindow.xml depends)
    flagexist = os.access(sdmfile + '/Flag.xml', os.F_OK)
    antexist = os.access(sdmfile + '/Antenna.xml', os.F_OK)
    spwexist = os.access(sdmfile + '/SpectralWindow.xml', os.F_OK)
    if not flagexist:
        casalog.post('Cannot open ' + sdmfile + '/Flag.xml', 'SEVERE')
        exit(1)
    if not antexist:
        casalog.post('Cannot open ' + sdmfile + '/Antenna.xml', 'SEVERE'
                     )
        exit(1)
    if not spwexist:
        casalog.post('Cannot open ' + sdmfile + '/SpectralWindow.xml',
                     'WARN')

    # construct look-up dictionary of name vs. id from Antenna.xml
    xmlants = minidom.parse(sdmfile + '/Antenna.xml')
    antdict = {}
    rowlist = xmlants.getElementsByTagName('row')
    for rownode in rowlist:
        rowname = rownode.getElementsByTagName('name')
        ant = str(rowname[0].childNodes[0].nodeValue)
        rowid = rownode.getElementsByTagName('antennaId')
        # CAS-4532: remove spaces between content and tags
        antid = str(rowid[0].childNodes[0].nodeValue).strip()
        antdict[antid] = ant
    casalog.post('Found ' + str(rowlist.length)
                 + ' antennas in Antenna.xml')

    # construct look-up dictionary of name vs. id from SpectralWindow.xml
    if spwexist:
        xmlspws = minidom.parse(sdmfile + '/SpectralWindow.xml')
        spwdict = {}
        rowlist = xmlspws.getElementsByTagName('row')
        ispw = 0
        for rownode in rowlist:
            rowid = rownode.getElementsByTagName('spectralWindowId')
            # CAS-4532: remove spaces between content and tags
            spwid = str(rowid[0].childNodes[0].nodeValue).strip()
            spwdict[spwid] = {}
            spwdict[spwid]['index'] = ispw
            # SMC: 6/3/2012 ALMA SDM does not have name
            if rownode.getElementsByTagName('name'):
                rowname = rownode.getElementsByTagName('name')
                spw = str(rowname[0].childNodes[0].nodeValue)
                spwdict[spwid]['name'] = spw
            else:
                spwmode = -1
                
#            rowid = rownode.getElementsByTagName('spectralWindowId')
#            spwid = str(rowid[0].childNodes[0].nodeValue)
#            spwdict[spwid] = {}
#            spwdict[spwid]['index'] = ispw
            ispw += 1
        casalog.post('Found ' + str(rowlist.length)
                     + ' spw in SpectralWindow.xml')

    # report chosen spw and pol modes
    if spwmode > 0:
        casalog.post('Will construct spw flags using names')
    elif spwmode < 0:
        casalog.post('Will construct spw flags using table indices')
    else:
        casalog.post('')
    #
    if polmode == 0:
        casalog.post('Will not set polarization dependent flags (flag all corrs)'
                     )
    else:
        casalog.post('Will construct polarization flags using polarizationType'
                     )

    # now read Flag.xml into dictionary row by row
    xmlflags = minidom.parse(sdmfile + '/Flag.xml')
    flagdict = {}
    rowlist = xmlflags.getElementsByTagName('row')
    nrows = rowlist.length
    newsdm = -1
    newspw = -1
    newpol = -1
    for fid in range(nrows):
        rownode = rowlist[fid]
        rowfid = rownode.getElementsByTagName('flagId')
        fidstr = str(rowfid[0].childNodes[0].nodeValue)
        flagdict[fid] = {}
        flagdict[fid]['id'] = fidstr
        rowid = rownode.getElementsByTagName('antennaId')
        antid = rowid[0].childNodes[0].nodeValue
    # check if there is a numAntenna specified (new format)
        rownant = rownode.getElementsByTagName('numAntenna')
        antname = ''
        if rownant.__len__() > 0:
            xid = antid.split()
            nant = int(rownant[0].childNodes[0].nodeValue)
            if newsdm < 0:
                casalog.post('Found numAntenna=' + str(nant)
                             + ' must be a new style SDM')
            newsdm = 1
            if nant > 0:
                for ia in range(nant):
                    aid = xid[2 + ia]
                    ana = antdict[aid]
                    if antname == '':
                        antname = ana
                    else:
                        antname += ',' + ana
            else:
            # numAntenna = 0 means flag all antennas
                antname = ''
        else:
            if newsdm < 0:
                casalog.post('No numAntenna entry found, must be a old style SDM'
                             )
            newsdm = 0
            nant = 1
            aid = antid
            ana = antdict[aid]
            antname = ana
        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName('startTime')
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjds = float(start) * 1.0E-9 - mytbuff
        t = qa.quantity(startmjds, 's')
        starttime = qa.time(t, form='ymd', prec=9)[0]
        rowend = rownode.getElementsByTagName('endTime')
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjds = float(end) * 1.0E-9 + mytbuff
        t = qa.quantity(endmjds, 's')
        endtime = qa.time(t, form='ymd', prec=9)[0]
    # time and interval for FLAG_CMD use
        times = 0.5 * (startmjds + endmjds)
        intervs = endmjds - startmjds
        flagdict[fid]['time'] = times
        flagdict[fid]['interval'] = intervs
        # reasons
        rowreason = rownode.getElementsByTagName('reason')
        reas = str(rowreason[0].childNodes[0].nodeValue)
        # Replace any white space with underscores
        reason = reas.replace(' ','_')
    # NEW SDM ADDITIONS 2011-11-01
        rownspw = rownode.getElementsByTagName('numSpectralWindow')
        spwstring = ''
        if spwmode != 0 and rownspw.__len__() > 0:
            nspw = int(rownspw[0].childNodes[0].nodeValue)
        # has a new-style spw specification
            if newspw < 0:
                if not spwexist:
                    casalog.post('Cannot open ' + sdmfile
                                 + '/SpectralWindow.xml', 'SEVERE')
                    exit(1)
                casalog.post('Found SpectralWindow=' + str(nspw)
                             + ' must be a new style SDM')
            newspw = 1
            if nspw > 0:
                rowspwid = \
                    rownode.getElementsByTagName('spectralWindowId')
                spwids = rowspwid[0].childNodes[0].nodeValue
                xspw = spwids.split()
                for isp in range(nspw):
                    spid = str(xspw[2 + isp])
                    if spwmode > 0:
                        spstr = spwdict[spid]['name']
                    else:
                        spstr = str(spwdict[spid]['index'])
                    if spwstring == '':
                        spwstring = spstr
                    else:
                        spwstring += ',' + spstr
        polstring = ''
        rownpol = rownode.getElementsByTagName('numPolarizationType')
        if polmode != 0 and rownpol.__len__() > 0:
            npol = int(rownpol[0].childNodes[0].nodeValue)
        # has a new-style pol specification
            if newpol < 0:
                casalog.post('Found numPolarizationType=' + str(npol)
                             + ' must be a new style SDM')
            newpol = 1
            if npol > 0:
                rowpolid = \
                    rownode.getElementsByTagName('polarizationType')
                polids = rowpolid[0].childNodes[0].nodeValue
                xpol = polids.split()
                for ipol in range(npol):
                    polid = str(xpol[2 + ipol])
                    if polstring == '':
                        polstring = polid
                    else:
                        polstring += ',' + polid
    #
        # Construct antenna name and timerange and reason strings
        flagdict[fid]['antenna'] = antname
        timestr = starttime + '~' + endtime
        flagdict[fid]['timerange'] = timestr
        flagdict[fid]['reason'] = reason
        # Construct command strings (per input flag)
        cmd = "antenna='" + antname + "' timerange='" + timestr + "'"
        if spwstring != '':
            cmd += " spw='" + spwstring + "'"
            flagdict[fid]['spw'] = spwstring
#        if polstring != '':
#            cmd += " poln='" + polstring + "'"
#            flagdict[fid]['poln'] = polstring
        if polstring != '':
            # Write the poln translation in correlation
            if polstring.count('R')>0:
                if polstring.count('L')>0:
                    corr = 'RR,RL,LR,LL'
                else:
                    corr = 'RR,RL,LR'
            elif polstring.count('L')>0:
                corr = 'LL,LR,RL'
            elif polstring.count('X')>0:
                if polstring.count('Y')>0:
                    corr = 'XX,XY,YX,YY'
                else:
                    corr = 'XX,XY,YX'
            elif polstring.count('Y')>0:
                corr = 'YY,YX,XY'

            cmd += " correlation='" + corr + "'"
#            flagdict[fid]['poln'] = polstring
        flagdict[fid]['command'] = cmd
    #
        flagdict[fid]['type'] = 'FLAG'
        flagdict[fid]['applied'] = False
        flagdict[fid]['level'] = 0
        flagdict[fid]['severity'] = 0
        flagdict[fid]['mode'] = 'online'

    flags = {}
    if rowlist.length > 0:
        flags = flagdict
        casalog.post('Found ' + str(rowlist.length)
                     + ' flags in Flag.xml')
    else:
        casalog.post('No valid flags found in Flag.xml')

    # return the dictionary for later use
    return flags



#def getUnion(mslocal, vis, cmddict):
def getUnion(vis, cmddict):
    '''Get a dictionary of a union of all selection parameters from a list of lines:
       vis --> MS
       cmddict --> dictionary of parameters and values (par=val) such as the one
                   returned by makeDict()
    '''
    
    # Dictionary of parameters to return
    dicpars = {
    'field':'',
    'scan':'',
    'antenna':'',
    'spw':'',
    'timerange':'',
    'correlation':'',
    'intent':'',
    'feed':'',
    'array':'',
    'uvrange':'',
    'observation':''
    }

    # Strings for each parameter
    scans = ''
    fields = ''
    ants = ''
    times = ''
    corrs = ''
    ints = ''
    feeds = ''
    arrays = ''
    uvs = ''
    spws = ''
    obs = ''

    nrows = cmddict.keys().__len__()       
#    nrows = cmdlist.__len__()

#    for i in range(nrows):
    for k in cmddict.keys():
#        cmdline = cmdlist[i]
        cmdline = cmddict[k]['command']
        
        # Skip if it is a comment line
        if cmdline.startswith('#'):
            break
        
        # split by white space
        keyvlist = cmdline.split()
        if keyvlist.__len__() > 0:  

            # Split by '='
            for keyv in keyvlist:
                   
                (xkey,xval) = keyv.split('=')

                # Remove quotes
                if type(xval) == str:
                    if xval.count("'") > 0:
                        xval = xval.strip("'")
                    if xval.count('"') > 0:
                        xval = xval.strip('"')

                # Check which parameter
                if xkey == "scan":
                    scans += xval + ','

                elif xkey == "field":
                    fields += xval + ','

                elif xkey == "antenna":
                    ants += xval + ';'

                elif xkey == "timerange":
                    times += xval + ','

                elif xkey == "correlation":
                    corrs += xval + ','

                elif xkey == "intent":
                    ints += xval + ','

                elif xkey == "feed":
                    feeds += xval + ','

                elif xkey == "array":
                    arrays += xval + ','

                elif xkey == "uvrange":
                    uvs += xval + ','

                elif xkey == "spw":
                    spws += xval + ','

                elif xkey == "observation":
                    obs += xval + ','

                        
    # Strip out the extra comma at the end
    scans = scans.rstrip(',')
    fields = fields.rstrip(',')
    ants = ants.rstrip(';')
    times = times.rstrip(',')
    corrs = corrs.rstrip(',')
    ints = ints.rstrip(',')
    feeds = feeds.rstrip(',')
    arrays = arrays.rstrip(',')
    uvs = uvs.rstrip(',')
    spws = spws.rstrip(',')
    obs = obs.rstrip(',')

    dicpars['scan'] = scans
    dicpars['field'] = fields
    # Antennas are handled better within the framework.
    dicpars['antenna'] = ''
    # Times are handled better within the framework.
    dicpars['timerange'] = '' 
    # Correlations should be handled only by the agents
    dicpars['correlation'] = ''
    dicpars['intent'] = ints
    dicpars['feed'] = feeds
    dicpars['array'] = arrays
    dicpars['uvrange'] = uvs
    dicpars['spw'] = spws
    dicpars['observation'] = obs
    

    # Compress the selection list to reduce MSSelection parsing time.
    # 'field','spw','antenna' strings in dicpars will be modified in-place.
    compressSelectionList(vis,dicpars);

    # Real number of input lines
    # Get the number of occurrences of each parameter
    npars = getNumPar(cmddict)
    nlines = nrows - npars['comment']
        
    # Make the union. 
    for k,v in npars.iteritems():
        if k != 'comment':
            if v < nlines:
                dicpars[k] = ''


    uniondic = dicpars.copy()
    # Remove empty parameters from the dictionary
    for k,v in dicpars.iteritems():
        if v == '':
            uniondic.pop(k)
    
    return uniondic


def getNumPar(cmddict):
    '''Get the number of occurrences of all parameter keys
       cmdlist --> list of strings with parameters and values
    '''

#    nrows = cmdlist.__len__()
            
    # Dictionary of number of occurrences to return
    npars = {
    'field':0,
    'scan':0,
    'antenna':0,
    'spw':0,
    'timerange':0,
    'correlation':0,
    'intent':0,
    'feed':0,
    'array':0,
    'uvrange':0,
    'comment':0,
    'observation':0
    }

    ci = 0  # count the number of lines with comments (starting with a #)
    si = 0  # count the number of lines with scan
    fi = 0  # count the number of lines with field
    ai = 0  # count the number of lines with antenna
    ti = 0  # count the number of lines with timerange
    coi = 0  # count the number of lines with correlation
    ii = 0  # count the number of lines with intent
    fei = 0  # count the number of lines with feed
    ari = 0  # count the number of lines with array
    ui = 0  # count the number of lines with uvrange
    pi = 0  # count the number of lines with spw
    oi = 0  # count the number of lines with observation
    
#    for i in range(nrows):
    for k in cmddict.keys():
#        cmdline = cmdlist[i]
        cmdline = cmddict[k]['command']

        if cmdline.startswith('#'):
            ci += 1
            npars['comment'] = ci
            continue

        # split by white space
        keyvlist = cmdline.split()
        if keyvlist.__len__() > 0:  

            # Split by '='
            for keyv in keyvlist:

                # Skip if it is a comment character #
                if keyv.count('#') > 0:
                    break

                (xkey,xval) = keyv.split('=')

                # Remove quotes
                if type(xval) == str:
                    if xval.count("'") > 0:
                        xval = xval.strip("'")
                    if xval.count('"') > 0:
                        xval = xval.strip('"')
                    # Remove blanks
                    xval=xval.replace(' ','');

                # Check which parameter, if not empty
                if xval != "":
                    if xkey == "scan":
                        si += 1
                        npars['scan'] = si
                        
                    elif xkey == "field":
                        fi += 1
                        npars['field'] = fi
                        
                    elif xkey == "antenna":
                        ai += 1
                        npars['antenna'] = ai
                        
                    elif xkey == "timerange":
                        ti += 1
                        npars['timerange'] = ti
                        
                    elif xkey == "correlation":
                        coi += 1
                        npars['correlation'] = coi
                        
                    elif xkey == "intent":
                        ii += 1
                        npars['intent'] = ii
                        
                    elif xkey == "feed":
                        fei += 1
                        npars['feed'] = fei
                        
                    elif xkey == "array":
                        ari += 1
                        npars['array'] = ari
                        arrays += xval + ','
                        
                    elif xkey == "uvrange":
                        ui += 1
                        npars['uvrange'] = ui
                        
                    elif xkey == "spw":
                        pi += 1
                        npars['spw'] = pi
                        
                    elif xkey == "observation":
                        oi += 1
                        npars['observation'] = oi
                        
                        
    return npars


#def compressSelectionList(mslocal=None, vis='',dicpars={}):
def compressSelectionList(vis='',dicpars={}):
    """
    - Find a loose union of data-selection parameters, to reduce the MSSelection parsing load.
    - This compressed selection list is only meant to be used with tf.selectdata(), because
      further selections are handled internally.
    - Use MSSelection in its 'onlyparse=True' mode to gather a list of fields, spws, antennas
      touched by the selection list. These are the only keys for which MSSelection does not 
      need to parse the MS, and will cover the expensive cases where complicated antenna
      and spw expressions can slow MSSelection down.   
    """
    from numpy import unique;
    
#    mslocal = casac.ms()
    ms.open(vis, nomodify=False)
    try:
        indices = ms.msseltoindex(vis=vis,field=dicpars['field'], spw=dicpars['spw'],baseline=dicpars['antenna']);
    finally:
        ms.close()
        
    c_field = str(list(unique(indices['field']))).strip('[]');
    c_spw = str(list(unique(indices['spw']))).strip('[]');
    c_antenna = str(list( unique( list(indices['antenna1']) + list(indices['antenna2']) ) ) ).strip('[]');

    dicpars['field'] = c_field;
    dicpars['spw'] = c_spw;
    dicpars['antenna'] = c_antenna;

    # Programmer note : Other selection parameters that can be compressed accurately
    # from MS subtable information alone (no need to parse the main table) are 
    # 'array', 'observationid', 'state(or intent)'. They are currently un-available 
    # via ms.msseltoindex() and therefore not used here yet.

    return;



def writeFlagCmd(msfile, myflags, vrows, applied, add_reason, outfile):
    '''
    Writes the flag commands to FLAG_CMD or to an ASCII file
    
    msfile  -->   MS
    myflags -->  dictionary of commands read from inputfile (from readFromTable, etc.)
    vrows   -->  list of valid rows from myflags dictionary to save
    applied -->  value to update APPLIED column of FLAG_CMD
    add_reason --> reason to add to output (replace input reason, if any)
    outfile -->  if not empty, save to it
        Returns the number of commands written to output
    '''
    
    nadd = 0
    try:
        import pylab as pl
    except ImportError, e:
        print 'failed to load pylab:\n', e
        exit(1)
    
    # append to a file   
    if outfile != '':                          
        ffout = open(outfile, 'a')
        
        try:                            
            
            for key in myflags.keys():
                # Remove leading and trailing white spaces
                cmdline = myflags[key]['command'].strip()
                
                # Add addantenna parameter back
                if myflags[key].__contains__('addantenna'):
                    addantenna = myflags[key]['addantenna']
                    cmdline = cmdline + ' addantenna=' + str(addantenna)
                                                        
                reason = myflags[key]['reason']
                
                # There is no reason in input
                if reason == '':
                    # Add new reason to output
                    if add_reason != '':
                        print >> ffout, '%s reason=\'%s\'' %(cmdline, add_reason)
                    else:
                        print >> ffout, '%s' %cmdline
                    
                # There is reason in input    
                else:
                    # Output reason is empty
                    if add_reason == '':
                        print >> ffout, '%s reason=\'%s\'' %(cmdline, reason)
                        
                    else:
                        # Replace input reason with new reason
                        print >> ffout, '%s reason=\'%s\'' %(cmdline, add_reason)
                                
        except:
            raise Exception, 'Error writing lines to file ' \
                + outfile
        ffout.close()
        return
    
    # Append new commands to existing table
    if vrows.__len__() > 0:
        # Extract flags from dictionary into list
        tim_list = []
        intv_list = []
        cmd_list = []
        reas_list = []
        typ_list = []
        sev_list = []
        lev_list = []
        app_list = []
        
        # Only write valid rows that have been applied to MS

        for key in vrows:
            # do not write line with summary mode
            command = myflags[key]['command']
            if command.__contains__('summary'):
                continue
            
            # Add addantenna back
            if myflags[key].__contains__('addantenna'):
                addantenna = myflags[key]['addantenna']
                command = command + ' addantenna=' + str(addantenna)

            cmd_list.append(command)
            tim_list.append(myflags[key]['time'])
            intv_list.append(myflags[key]['interval'])
            if add_reason != '':
                reas_list.append(add_reason)
            else:
                reas_list.append(myflags[key]['reason'])
            typ_list.append(myflags[key]['type'])
            sev_list.append(myflags[key]['severity'])
            lev_list.append(myflags[key]['level'])
            app_list.append(applied)
    
            
        # Save to FLAG_CMD table
        nadd = cmd_list.__len__()

        mstable = msfile + '/FLAG_CMD'
        try:
            tb.open(mstable, nomodify=False)
        except:
            raise Exception, 'Error opening FLAG_CMD table ' + mstable
        nrows = int(tb.nrows())
        casalog.post('There are ' + str(nrows)
                     + ' rows already in FLAG_CMD', 'DEBUG')
        # add blank rows
        tb.addrows(nadd)
        # now fill them in
        tb.putcol('TIME', pl.array(tim_list), startrow=nrows, nrow=nadd)
        tb.putcol('INTERVAL', pl.array(intv_list), startrow=nrows,
                  nrow=nadd)
        tb.putcol('REASON', pl.array(reas_list), startrow=nrows,
                  nrow=nadd)
        tb.putcol('COMMAND', pl.array(cmd_list), startrow=nrows,
                  nrow=nadd)
        # Other columns
        tb.putcol('TYPE', pl.array(typ_list), startrow=nrows, nrow=nadd)
        tb.putcol('SEVERITY', pl.array(sev_list), startrow=nrows,
                  nrow=nadd)
        tb.putcol('LEVEL', pl.array(lev_list), startrow=nrows,
                  nrow=nadd)
        tb.putcol('APPLIED', pl.array(app_list), startrow=nrows,
                  nrow=nadd)
        tb.close()
    
        casalog.post('Saved ' + str(nadd) + ' rows to FLAG_CMD')
        
    else:
        casalog.post('Saved zero rows to FLAG_CMD; no flags found')

    return nadd


def getReason(cmdline):
    '''Get the reason values from a line with strings
       cmdline --> a string with parameters
          returns a string with reason values.
    '''
            
    reason = ''

    # Skip comment lines
    if cmdline.startswith('#'):
        return reason    
    
    # Split by white space
    keyvlist = cmdline.split()
    if keyvlist.__len__() > 0:  
        
        for keyv in keyvlist:            

            # Split by '='
            (xkey,xval) = keyv.split('=')

            # Remove quotes
            if type(xval) == str:
                if xval.count("'") > 0:
                    xval = xval.strip("'")
                if xval.count('"') > 0:
                    xval = xval.strip('"')

            # Check if reason is in
            if xkey == "reason":
                reason = xval
                break;
    
                                    
    return reason


def getLinePars(cmdline, mlist=[]):
    '''Get a dictionary of all selection parameters from a line:
       cmdline --> a string with parameters
       mlist --> a list of mode's parameters to add to the output dictionary
          
        Returns a  dictionary.
    '''
            
    # Dictionary of parameters to return
    dicpars = {}
        
    # Skip comment lines
    if cmdline.startswith('#'):
        return dicpars    
    
    # split by white space
    keyvlist = cmdline.split()
    if keyvlist.__len__() > 0:  
        
        # Split by '='
        for keyv in keyvlist:

            (xkey,xval) = keyv.split('=')

            # Remove quotes
            if type(xval) == str:
                if xval.count("'") > 0:
                    xval = xval.strip("'")
                if xval.count('"') > 0:
                    xval = xval.strip('"')

            # Check which parameter
            if xkey == "scan":
                dicpars['scan'] = xval

            elif xkey == "field":
                dicpars['field'] = xval

            elif xkey == "antenna":
                dicpars['antenna'] = xval

            elif xkey == "timerange":
                dicpars['timerange'] = xval

            elif xkey == "correlation":
                dicpars['correlation'] = xval.upper()

            elif xkey == "intent":
                dicpars['intent'] = xval

            elif xkey == "feed":
                dicpars['feed'] = xval

            elif xkey == "array":
                dicpars['array'] = xval

            elif xkey == "uvrange":
                dicpars['uvrange'] = xval

            elif xkey == "spw":
                dicpars['spw'] = xval
                
            elif xkey == "observation":
                dicpars['observation'] = xval

            elif xkey == "mode":
                if xval == 'manualflag':
                    xval = 'manual'
                dicpars['mode'] = xval

            elif mlist != []:
                # Any parameters requested for this mode?
                for m in mlist:
                    if xkey == m:
                        dicpars[m] = xval
                        
    casalog.post(':getLinePars::dicpars=%s'%dicpars, 'DEBUG')         
            
    return dicpars

def getSelectionPars(cmdline):
    '''Get a dictionary of all selection parameters from a line:
       cmdline --> a string with parameters
    '''
            
    # Dictionary of parameters to return
    dicpars = {}
        
    # Skip comment lines
    if cmdline.startswith('#'):
        return dicpars    
    
    # split by white space
    keyvlist = cmdline.split()
    if keyvlist.__len__() > 0:  
        
        # Split by '='
        for keyv in keyvlist:

            (xkey,xval) = keyv.split('=')

            # Remove quotes
            if type(xval) == str:
                if xval.count("'") > 0:
                    xval = xval.strip("'")
                if xval.count('"') > 0:
                    xval = xval.strip('"')

            # Check which parameter
            if xkey == "scan":
                dicpars['scan'] = xval

            elif xkey == "field":
                dicpars['field'] = xval

            elif xkey == "antenna":
                dicpars['antenna'] = xval

            elif xkey == "timerange":
                dicpars['timerange'] = xval
                
            # Correlation will be handled by the agent
            elif xkey == "correlation":
                dicpars['correlation'] = ''

            elif xkey == "intent":
                dicpars['intent'] = xval

            elif xkey == "feed":
                dicpars['feed'] = xval

            elif xkey == "array":
                dicpars['array'] = xval

            elif xkey == "uvrange":
                dicpars['uvrange'] = xval

            elif xkey == "spw":
                dicpars['spw'] = xval
                
            elif xkey == "observation":
                dicpars['observation'] = xval
                        
            
    return dicpars


def readNtime(params):
    '''Check the value and units of ntime
       params --> dictionary of agent's parameters '''

    newtime = 0.0
    
    if params.has_key('ntime'):
        ntime = params['ntime']

        # Verify the ntime value
        if type(ntime) == float or type(ntime) == int:
            if ntime <= 0:
                raise Exception, 'Parameter ntime cannot be < = 0'
            else:
                # units are seconds
                newtime = float(ntime)
        
        elif type(ntime) == str:
            if ntime == 'scan':
                # iteration time step is a scan
                newtime = 0.0
            else:
                # read the units from the string
                qtime = qa.quantity(ntime)
                
                if qtime['unit'] == 'min':
                    # convert to seconds
                    qtime = qa.convert(qtime, 's')
                elif qtime['unit'] == '':
                    qtime['unit'] = 's'
                    
                # check units
                if qtime['unit'] == 's':
                    newtime = qtime['value']
                else:
                    casalog.post('Cannot convert units of ntime. Will use default 0.0s', 'WARN')
          
    params['ntime'] = float(newtime)


def fixType(params):
    '''Give correct types to non-string parameters
       The types are defined in the XML file of the task tflagdata'''

    # manual parameter
    if params.has_key('autocorr'):
        params['autocorr'] = eval(params['autocorr'].capitalize())
        
    # quack parameters
    if params.has_key('quackmode') and not params['quackmode'] in ['beg'
            , 'endb', 'end', 'tail']:
        raise Exception, \
            "Illegal value '%s' of parameter quackmode, must be either 'beg', 'endb', 'end' or 'tail'" \
            % params['quackmode']
    if params.has_key('quackinterval'):
        params['quackinterval'] = float(params['quackinterval'])        
    if params.has_key('quackincrement'):
        if type(params['quackincrement']) == str:
            params['quackincrement'] = eval(params['quackincrement'].capitalize())

    # clip parameters
    if params.has_key('clipminmax'):
        value01 = params['clipminmax']
        # turn string into [min,max] range
        value0 = value01.lstrip('[')
        value = value0.rstrip(']')
        r = value.split(',')
        rmin = float(r[0])
        rmax = float(r[1])
        params['clipminmax'] = [rmin, rmax]        
    if params.has_key('clipoutside'):
        if type(params['clipoutside']) == str:
            params['clipoutside'] = eval(params['clipoutside'].capitalize())
        else:
            params['clipoutside'] = params['clipoutside']
    if params.has_key('channelavg'):
        params['channelavg'] = eval(params['channelavg'].capitalize())
    if params.has_key('clipzeros'):
        params['clipzeros'] = eval(params['clipzeros'].capitalize())
            
            
    # shadow parameter
    if params.has_key('tolerance'):
        params['tolerance'] = float(params['tolerance'])
           
    # elevation parameters
    if params.has_key('lowerlimit'):
        params['lowerlimit'] = float(params['lowerlimit'])        
    if params.has_key('upperlimit'):
        params['upperlimit'] = float(params['upperlimit'])
        
    # extend parameters
    if params.has_key('extendpols'):        
        params['extendpols'] = eval(params['extendpols'].capitalize())
    if params.has_key('growtime'):
        params['growtime'] = float(params['growtime'])
    if params.has_key('growfreq'):
        params['growfreq'] = float(params['growfreq'])
    if params.has_key('growaround'):
        params['growaround'] = eval(params['growaround'].capitalize())
    if params.has_key('flagneartime'):
        params['flagneartime'] = eval(params['flagneartime'].capitalize())
    if params.has_key('flagnearfreq'):
        params['flagnearfreq'] = eval(params['flagnearfreq'].capitalize())

    # tfcrop parameters
    if params.has_key('combinescans'):
        params['combinescans'] = eval(params['combinescans'].capitalize())        
    if params.has_key('timecutoff'):
        params['timecutoff'] = float(params['timecutoff'])       
    if params.has_key('freqcutoff'):
        params['freqcutoff'] = float(params['freqcutoff'])        
    if params.has_key('maxnpieces'):
        params['maxnpieces'] = int(params['maxnpieces'])        
    if params.has_key('halfwin'):
        params['halfwin'] = int(params['halfwin'])
        
    # rflag parameters
    if params.has_key('winsize'):
        params['winsize'] = int(params['winsize']);
    if params.has_key('timedev'):
        timepar = params['timedev']
        try:
            timepar = eval(timepar)
        except Exception:
            timepar = readRFlagThresholdFile(params['timedev'],'timedev');
        params['timedev'] = timepar
    if params.has_key('freqdev'):
        freqpar = params['freqdev']
        try:
            freqpar = eval(freqpar)
        except Exception:
            freqpar = readRFlagThresholdFile(params['freqdev'],'freqdev');
        params['freqdev'] = freqpar
    if params.has_key('timedevscale'):
        params['timedevscale'] = float(params['timedevscale']);
    if params.has_key('freqdevscale'):
        params['freqdevscale'] = float(params['freqdevscale']);
    if params.has_key('spectralmin'):
        params['spectralmin'] = float(params['spectralmin']);
    if params.has_key('spectralmax'):
        params['spectralmax'] = float(params['spectralmax']);
    


def purgeEmptyPars(cmdline):
    '''Remove empty parameters from a string:
       cmdline --> a string with parameters
       
       returns a string containing only parameters with values
    '''
    newstr = ''
    
    # split by white space
    keyvlist = cmdline.split()
    if keyvlist.__len__() > 0:  
        
        # Split by '='
        for keyv in keyvlist:

            (xkey,xval) = keyv.split('=')

            # Remove quotes
            if type(xval) == str:
                if xval.count("'") > 0:
                    xval = xval.strip("'")
                if xval.count('"') > 0:
                    xval = xval.strip('"')
            
            # Write only parameters with values
            if xval == '':
                continue
            else:
                newstr = newstr+xkey+'='+xval+' '
            
    else:
        casalog.post('String of parameters is empty','WARN')   
         
    return newstr


def purgeParameter(cmdline, par):
    '''Remove parameter from a string:
       cmdline --> a string with a parameter to be removed
       par --> the parameter to be removed from the string
       
       returns a string containing the remaining parameters
    '''
               
    newstr = ''
    
    # split by white space
    keyvlist = cmdline.split()
    if keyvlist.__len__() > 0:  
        
        # Split by '='
        for keyv in keyvlist:

            (xkey,xval) = keyv.split('=')

            # Remove quotes
            if type(xval) == str:
                if xval.count("'") > 0:
                    xval = xval.strip("'")
                if xval.count('"') > 0:
                    xval = xval.strip('"')
            
            # Write only parameters with values
            if xkey == par:
                continue
            else:
                newstr = newstr+xkey+'=' +  xval+  ' '
            
    else:
        casalog.post('String of parameters is empty','WARN')   
         
    return newstr

def setupAgent(tflocal, myflagcmd, myrows, apply, writeflags, display=''):
    ''' Setup the parameters of each agent and call the testflagger tool
    
        myflagcmd --> it is a dictionary coming from readFromTable, readFile, etc.
        myrows --> selected rows to apply/unapply flags
        apply --> it's a boolean to control whether to apply or unapply the flags
        writeflags --> used by mode=rflag only
        display --> used by mode='rflag only'''


    if not myflagcmd.__len__() >0:
        casalog.post('There are no flag cmds in list', 'SEVERE')
        return
    
    # Parameters for each mode
    manualpars = ['autocorr']
    unflagpars = []
    clippars = ['clipminmax', 'clipoutside','datacolumn', 'channelavg', 'clipzeros']
    quackpars = ['quackinterval','quackmode','quackincrement']
    shadowpars = ['tolerance', 'addantenna']
    elevationpars = ['lowerlimit','upperlimit'] 
    tfcroppars = ['ntime','combinescans','datacolumn','timecutoff','freqcutoff',
                  'timefit','freqfit','maxnpieces','flagdimension','usewindowstats','halfwin']
    extendpars = ['ntime','combinescans','extendpols','growtime','growfreq','growaround',
                  'flagneartime','flagnearfreq']
    rflagpars = ['winsize','timedev','freqdev','timedevscale','freqdevscale','spectralmax','spectralmin']
    
        
    # dictionary of successful command lines to save to outfile
    savelist = {}

    # Setup the agent for each input line    
    for key in myflagcmd.keys():
        cmdline = myflagcmd[key]['command']
        applied = myflagcmd[key]['applied']
        interval = myflagcmd[key]['interval']
        level = myflagcmd[key]['level']
        reason = myflagcmd[key]['reason']
        severity = myflagcmd[key]['severity']
        coltime = myflagcmd[key]['time']
        coltype = myflagcmd[key]['type']
        if debug:
            print 'cmdline for key%s'%key
            print '%s'%cmdline
            print 'applied is %s'%applied
        
        if cmdline.startswith('#'):
            continue
    
        modepars = {}
        parslist = {}
        mode = ''    
        valid = True
        addantenna = {}
                
        # Get the specific parameters for the mode
        if cmdline.__contains__('mode'):                 
            if cmdline.__contains__('manual'): 
                mode = 'manual'
                modepars = getLinePars(cmdline,manualpars)   
            elif cmdline.__contains__('clip'):
                mode = 'clip'
                modepars = getLinePars(cmdline,clippars)
            elif cmdline.__contains__('quack'):
                mode = 'quack'
                modepars = getLinePars(cmdline,quackpars)
            elif cmdline.__contains__('shadow'):
                mode = 'shadow'
                modepars = getLinePars(cmdline,shadowpars)
                
                # Get addantenna dictionary
                if myflagcmd[key].__contains__('addantenna'):
                    addantenna = myflagcmd[key]['addantenna']
                    modepars['addantenna'] = addantenna                    
                else:                                    
                    # Get antenna filename
                    if (modepars.__contains__('addantenna')):
                        ant_par = modepars['addantenna']
                        
                        # It must be a string
                        if (type(ant_par) == str and ant_par != ''):
                            antennafile = modepars['addantenna']
                            addantenna = readAntennaList(antennafile)
                            modepars['addantenna'] = addantenna
                                                           
            elif cmdline.__contains__('elevation'):
                mode = 'elevation'
                modepars = getLinePars(cmdline,elevationpars)
            elif cmdline.__contains__('tfcrop'):
                mode = 'tfcrop'
                modepars = getLinePars(cmdline,tfcroppars)
            elif cmdline.__contains__('extend'):
                mode = 'extend'
                modepars = getLinePars(cmdline,extendpars)
            elif cmdline.__contains__('unflag'):
                mode = 'unflag'
                modepars = getLinePars(cmdline,unflagpars)
            elif cmdline.__contains__('rflag'):
                mode = 'rflag'
                modepars = getLinePars(cmdline,rflagpars)

                ##### 
                ### According to 'shadow' file handling, this code should be here...
                ###  but, it's already done inside fixType. 
                #####
                #if( type(modepars['timedev']) == str and writeflags == True):
                #    timedev = readRFlagThresholdFile(modepars['timedev'],'timedev')
                #    modepars['timedev'] = timedev
                #if( type(modepars['freqdev']) == str and writeflags == True):
                #    freqdev = readRFlagThresholdFile(modepars['freqdev'],'freqdev')
                #    modepars['freqdev'] = freqdev

                # Add the writeflags and display parameters
                modepars['writeflags'] = writeflags
                modepars['display'] = display
            else:
                # Unknown mode, ignore it
                casalog.post('Ignoring unknown mode', 'WARN')
                valid = False

        else:
            # No mode means manual
            mode = 'manual'
            cmdline = cmdline+' mode=manual'
            modepars = getLinePars(cmdline,manualpars)   
                
                
        # Read ntime
        readNtime(modepars)
        
        # Cast the correct type to non-string parameters
        fixType(modepars)

        # Add the apply/unapply parameter to dictionary            
        modepars['apply'] = apply
        
        # Unapply selected rows only and re-apply the other rows with APPLIED=True
        if not apply and myrows.__len__() > 0:
            if key in myrows:
                modepars['apply'] = False
            elif not applied:
                casalog.post("Skipping this %s"%modepars,"DEBUG")
                continue
            elif applied:
                modepars['apply'] = True
                valid = False
        
        # Keep only cmds that overlap with the unapply cmds
        # TODO later
        
        # Hold the name of the agent and the cmd row number
        agent_name = mode.capitalize()+'_'+str(key)
        modepars['name'] = agent_name
        
        # Remove the data selection parameters if there is only one agent for performance reasons.
        # Explanation: if only one agent exists and the data selection parameters are parsed to it, 
        # it will have to go through the entire MS and check if each data selection given to the agent
        # matches what the user asked in the selected data.

        # Only correlation, antenna and timerange will go to the agent
        # CAS-3959 Handle channel selection at the FlagAgent level, leave spw in here too
        if myflagcmd.__len__() == 1:
            sellist=['scan','field','intent','feed','array','uvrange','observation']
            for k in sellist:
                if modepars.has_key(k):
                    modepars.pop(k)

        casalog.post('Parsing parameters of mode %s in row %s'%(mode,key), 'DEBUG')
        casalog.post('%s'%modepars, 'DEBUG')
        if debug:
            print 'Parsing parameters of mode %s in row %s'%(mode,key)
            print modepars

        # Parse the dictionary of parameters to the tool
        if (not tflocal.parseagentparameters(modepars)):
            casalog.post('Failed to parse parameters of mode %s in row %s' %(mode,key), 'WARN')
            continue
                            
        # Save the dictionary of valid agents
        if valid:               
            # add this command line to list to save in outfile
            parslist['row'] = key
            parslist['command'] = cmdline
            parslist['applied'] = applied
            parslist['interval'] = interval
            parslist['level'] = level
            parslist['reason'] = reason
            parslist['severity'] = severity
            parslist['time'] = coltime
            parslist['type'] = coltype
            if addantenna != {}:
                parslist['addantenna'] = addantenna
            savelist[key] = parslist
        
    if debug:
        casalog.post('Dictionary of valid commands to save')
        casalog.post('%s'%savelist)
    
    return savelist


def backupFlags(msfile, prename):
    '''
         Create names like this:
         flags.flagcmd_1,
         flags.tflagdata_1,
        
        Generally  <task>_<i>, where i is the smallest
        integer giving a name, which does not already exist'''
        
    prefix = prename
    tftool = casac.testflagger()
    tftool.open(msfile)
    try:
        existing = tftool.getflagversionlist(printflags=False)
    
        # remove comments from strings
        existing = [x[0:x.find(' : ')] for x in existing]
        i = 1
        while True:
            versionname = prefix + '_' + str(i)
    
            if not versionname in existing:
                break
            else:
                i = i + 1
    
        time_string = str(time.strftime('%Y-%m-%d %H:%M:%S'))
    
        casalog.post('Saving current flags to ' + versionname, 'DEBUG')
    
        tftool.saveflagversion(versionname=versionname,
                                comment='Flags autosave on ' + time_string, merge='replace')
    finally:
        tftool.done()
        
    return


####
####   Set of functions to handle antenna information for shadowing.
####
####   -   extractAntennaInfo : Extract info into a returned dictionary (and text file)
####   -   antListWrite : Write the dictionary to a text file
####   -   antListRead : Read the text file and return a dictionary
####
####   Example : 
####    alist = extractAntennaInfo(msname='../Data/shadowtest.ms',
####                                                  antnamelist=['VLA1','VLA15'], outfile='ttt.txt')
####
####    alist = antListRead('ttt.txt');
####
###################################
####    Example output text file ( 'ttt.txt' )
###################################
#
#name = VLA1
#diameter = 25.0
#position = [-1601144.961466915, -5041998.0197185818, 3554864.76811967]
#name = VLA15
#diameter = 25.0
#position = [-1601556.245351332, -5041990.7252590274, 3554684.6464035073]
#
###################################
####    Example output dictionary ( alist )
###################################
#
#{'0': {'diameter ': 25.0,
#       'name ': 'VLA1',
#       'position ': [-1601144.961466915,
#                     -5041998.0197185818,
#                     3554864.7681196001]},
# '1': {'diameter ': 25.0,
#       'name ': 'VLA15',
#       'position ': [-1601556.245351332,
#                     -5041990.7252590274,
#                     3554684.6464035069]}}
#
##################################

def extractAntennaInfo(msname='', antnamelist=[], outfile=''):
    """
    Function to extract antenna names, positions, and diameters
    for a specified subset of the ANTENNA subtable of the specified MS.
    - It writes a text file, which can be sent as input for shadow in the task
    - It also returns a dictionary, which can be sent as input for shadow in the tool.
      This dictionary can also be given as input in the task.
    
    msname : name of MS
    antennalist : list of strings (antenna names). Names must match exactly. Case insensitive.
    outfile : name of text file. Will be overwritten if exists. If outfile='', no output file is written
    
    Always returns a dictionary containing the same info as in the file
    
    Example : 
    antinfo =  extractAntennaInfo(msname='xxx.ms',antnamelist=['vla1','vla2'],outfile='out.txt');
    """
    ## Check that the MS exists
    if(not os.path.exists(msname)):
          print "Cannot find MS : ", msname;
          return False;
    
    ## If outfile exists, delete it
    if(os.path.exists(outfile)):
          print "Replacing existing file : ", outfile;
          rmcmd = "rm -rf "+outfile;
          os.system(rmcmd);
    
    ## Convert input antenna names to upper-case
    newants=[];
    for ants in antnamelist:
        newants.append( ants.upper() );
    antnamelist = newants;
    
    ## Read antenna subtable of input MS
    tb.open(msname+'/ANTENNA');
    a_position = (tb.getcol('POSITION')).transpose();
    a_dish_diameter = tb.getcol('DISH_DIAMETER');
    a_name = tb.getcol('NAME');
    tb.close();
    
    ## Pick out only selected antennas from this list, and make a dictionary
    antlist = {};
    counter=0;
    for antid in range(0, len(a_name)):
          if (a_name[antid]).upper() in antnamelist:
                antlist[str(counter)] = { 'name':a_name[antid] , 'position': list(a_position[antid]) , 'diameter': a_dish_diameter[antid]  }  ;
                counter=counter+1;
    
    ## Open a new file and write this info into it, if requested
    if(outfile != ''):
          print "Making new file : ", outfile;
          writeAntennaList(outfile, antlist);
    ## always return the dictionary anyway.
    return antlist;

##############################################
def writeAntennaList(outfile='', antlist={}):
    """
    Save the antlist dictionary as a text file
    """
    ofile = file(outfile, 'w');
    for apid in sorted(antlist):
          apars = antlist[apid];
          ofile.write("name=" + str(apars['name']) + '\n');
          ofile.write("diameter=" + str(apars['diameter'])+'\n');
          ofile.write("position=" + str((apars['position']))+'\n');
    ofile.close();

##############################################
def readAntennaList(infile=''):
    """
    Read the antlist text file and return a dictionary
    
    A return value of empty {} indicates an error (or, empty file).
    
    The file needs to have 3 entries per antenna, on separate lines.
    The diameter and position are in units of meters, with positions in ITRF.
    Multiple antennas can be specified by repeating these three lines.
    Blank lines are allowed.
    Lines can be commented with '#' as the first character.
    
    Example : 
          name = ea05
          diameter = 25.0
          position = [-1601144.96146691, -5041998.01971858, 3554864.76811967]
    
    """

    if (type(infile) == str) & os.path.exists(infile):
        try:
            ifile = file(infile,'r');
        except:
            raise Exception, 'Error opening file ' + infile
        
        thelist = ifile.readlines();
        ifile.close();
    else:
        raise Exception, \
            'File %s not found - please verify the name'%infile
    
    cleanlist=[];
    for aline in thelist:
          if(len(aline)>5 and aline[0] != '#'):
               cleanlist.append(aline.rstrip());
    
    #print 'Found ' + str(len(cleanlist)) + ' valid lines out of ' + str(len(thelist));
    
    if( len(cleanlist) > 0 and len(cleanlist) % 3 != 0 ):
          print "\nThe file needs to have 3 entries per antenna, on separate lines. For example :"
          print "name=ea05"
          print "diameter=25.0";
          print "position=[-1601144.96146691, -5041998.01971858,  3554864.76811967]";
          print "\n";
          print "The diameter and position are in units of meters, with positions in ITRF";
          return False;
    
    antlist={};
    counter=0;
    for aline in range(0,len(cleanlist),3):
          antdict = {};
          for row in range(0,3):
               pars = cleanlist[aline+row].split("=");
               #print aline, row, pars
               if(len(pars) != 2):
                    print 'Error in parsing : ', cleanlist[aline+row];
                    return {};
               else:
                    if(pars[0].count('name') > 0 ):
                           antdict[pars[0].rstrip()] = str(pars[1].rsplit()[0]);
                    if(pars[0].count('diameter') > 0 ):
                           antdict[pars[0].rstrip()] = float(pars[1]);
                    if(pars[0].count('position') > 0 ):
                           plist = pars[1][1:-2].replace('[','').split(',');
                           if(len(plist) != 3):
                                 print 'Error in parsing : ', cleanlist[aline+row]
                                 return {};
                           else:
                                 qlist=[];
                                 for ind in range(0,3):
                                     qlist.append(float(plist[ind]));
                           antdict[pars[0].rstrip()] = qlist;
          antlist[str(counter)] = antdict;
          counter = counter+1;
    
    return antlist;

################################################
#
#  Function to pull out RFLAG thresholds from the returned report dictionary.
#
def writeRFlagThresholdFile(rflag_thresholds={},timedevfile='', freqdevfile='',agent_id=0):
    """
    Extract the RFLAG output thresholds from the threshold dictionary
    Return them as arrays, and optionally, write them into a file.
    """
    # Decide the output file name.
    if( type(timedevfile) == str and timedevfile != '' and timedevfile.count('[')==0 and (not timedevfile.replace('.','').isdigit() ) ):
        toutfile = timedevfile
    else:
        toutfile = 'rflag_output_thresholds_timedev'+str(agent_id)+'.txt'

    # Decide the output file name.
    if( type(freqdevfile) == str and freqdevfile != ''  and freqdevfile.count('[')==0 and (not freqdevfile.replace('.','').isdigit() ) ):
        foutfile = freqdevfile
    else:
        foutfile = 'rflag_output_thresholds_freqdev'+str(agent_id)+'.txt'

    # save rflag output in file, and print them everywhere.
    casalog.post("Saving RFlag_"+str(agent_id)+" output in : " + toutfile + " and " + foutfile, 'INFO')

    ofiletime = file(toutfile, 'w');
    ofilefreq = file(foutfile, 'w');
    # Construct dictionary from what needs to be stored.
    timedict = {'name':rflag_thresholds['name'] , 'timedev': (rflag_thresholds['timedev']).tolist()}
    freqdict = {'name':rflag_thresholds['name'] , 'freqdev': (rflag_thresholds['freqdev']).tolist()}
    timestr = convertDictToString(timedict)
    freqstr = convertDictToString(freqdict)
    # Write to file
    ofiletime.write(timestr + '\n');
    ofilefreq.write(freqstr + '\n');
    # Send to logger
    casalog.post("RFlag_"+str(agent_id)+" output timedev written to " + toutfile + " : " + timestr, 'INFO');
    casalog.post("RFlag_"+str(agent_id)+" output freqdev written to " + foutfile + " : " + freqstr, 'INFO');
    # End filed
    ofiletime.write('\n');
    ofiletime.close();
    ofilefreq.write('\n');
    ofilefreq.close();
    # Returne timedev, freqdev contents. This is for savepars later.
    return timedict['timedev'], freqdict['freqdev']

##############################################
def readRFlagThresholdFile(infile='',inkey=''):
    """
    Read the input RFlag threshold file, and return dictionaries.
    """
    if(infile==''):
        return [];

    if ( not os.path.exists(infile) ):
        print 'Cannot find file : ', infile
        return [];

    ifile = file(infile,'r');
    thelist = ifile.readlines();
    ifile.close();

    cleanlist=[];
    for aline in thelist:
        if(len(aline)>5 and aline[0] != '#'):
            cleanlist.append(aline.rstrip().rstrip('\n'))
              
    threshlist={};
    for aline in range(0,len(cleanlist)):
        threshlist[str(aline)] = convertStringToDict(cleanlist[aline]);
        if threshlist[str(aline)].has_key(inkey):
            devthreshold = threshlist[str(aline)][inkey]

    # return only the last one. There should be only one anyway.
    return devthreshold

##############################################
## Note - replace all arrays by lists before coming here.
def convertDictToString(indict={}):
    '''Convert dictionary to string'''
    thestr = str(indict);
    # Remove newlines and spaces from this string.
    thestr = thestr.replace('\n','');
    thestr = thestr.replace(' ','');
    return thestr;
##############################################
def convertStringToDict(instr=''):
    '''Convert string to dictionary'''
    instr = instr.replace('\n','');
    try:
        thedict = ast.literal_eval(instr)
    except Exception, instance:
        casalog.post("*** Error converting string %s to dictionary : \'%s\'" % (instr,instance),'ERROR');
    return thedict;
##############################################

def extractRFlagOutputFromSummary(mode,summary_stats_list, flagcmd):
    """
    Function to pull out 'rflag' output from the long dictionary, and 
    (1) write the output files with thresholds. If the filename is specified, use it.
          If filename is not specified, make one up.
    (2) modify entries in 'cmdline' so that it is ready for savepars. 
          This is to ensure that 'savepars' saves the contents of the threshold-files
          and not just the file-names. It has to save it in the form that tflagdata 
          accepts inline : e.g.  timedev=[[1,10,0.1],[1,11,0.07]] . This way, the user
          need not keep track of threshold text files if they use 'savepars' with action='apply'.
    """
    if type(summary_stats_list) is dict:
        nreps = summary_stats_list['nreport']
        for rep in range(0,nreps):
            repname = 'report'+str(rep)
            if summary_stats_list[repname]['type'] == "rflag":
                # Pull out the rflag threshold dictionary. This has a 'name' in it.
                rflag_thresholds = summary_stats_list[repname]
                ##print rflag_thresholds
                # Get the rflag id, to later construct a 'name' from to match the above.
                rflagid = 0
                if mode=='list':
                    rflagid = int( rflag_thresholds['name'].replace('Rflag_','') )
                # Go through the flagcmd list, to find the 'rflags'.....
                for key in flagcmd.keys():
                    cmdline = flagcmd[key]['command'];
                    if cmdline.__contains__('rflag'):
                        # Check for match between input flagcmd and output threshold, via the rflag id
                        if(key==rflagid):  
                            # If timedev,freqdev are missing from cmdline, add empty ones.
                            if( not cmdline.__contains__('timedev=') ):  # aah. don't confuse it with timedevscale
                                cmdline = cmdline + " timedev=[] ";
                            if( not cmdline.__contains__('freqdev=') ):
                                cmdline = cmdline + " freqdev=[] ";
                            # Pull out timedev, freqdev strings from flagcmd
                            rflagpars = getLinePars(cmdline , ['timedev','freqdev','writeflags']);
                            ##print "cmdline : ", cmdline
                            ##print "rflagpars : ", rflagpars
                            # Write RFlag thresholds to these file names. 
                            newtimedev,newfreqdev = writeRFlagThresholdFile(rflag_thresholds, rflagpars['timedev'], rflagpars['freqdev'], rflagid)
                            ## Modify the flagcmd string, so that savepars sees the contents of the file
                            if( rflagpars['timedev'].__contains__('[') ):
                                oldstring = 'timedev='+str(rflagpars['timedev'])
                                newstring = 'timedev='+str(newtimedev).replace(' ','')
                                ##print "time : replacing " , oldstring , newstring
                                cmdline = cmdline.replace( oldstring, newstring );
                            if( rflagpars['freqdev'].__contains__('[') ):
                                oldstring = 'freqdev='+str(rflagpars['freqdev'])
                                newstring = 'freqdev='+str(newfreqdev).replace(' ','')
                                ##print "freq : replacing " , oldstring , newstring
                                cmdline = cmdline.replace( oldstring, newstring );
                            # Remove writeflags from the cmd to prevent it from going into savepars
                            oldstring = 'writeflags='+str(rflagpars['writeflags'])
                            cmdline = cmdline.replace( oldstring, "" );
                            
                            flagcmd[key]['command'] = cmdline;



##############################################
