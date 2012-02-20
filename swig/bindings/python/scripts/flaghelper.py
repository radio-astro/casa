from casac import *
import os
import commands
import math
import pdb
import shutil
import string
from taskinit import *

###some helper tools
#mslocal = casac.homefinder.find_home_by_name('msHome').create()
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

def makeDict(cmdlist):
    '''Make a dictionary compatible with a FLAG_CMD table structure
    
       cmdlist --> list of parameters to go into the COMMAND column
       
       Returns a dictionary with the following structure and default values
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
    
    # Defaults for columns
    applied = False
    interval = 0.0
    level = 0
    reason = ''
    severity = 0
    time = 0.0
    type = 'FLAG'
    
    myflagd = {}
    ncmds = 0
    nrows = cmdlist.__len__()
    try:
        for i in range(nrows):            
            command = cmdlist[i]
                                
            if command == '':
                casalog.post('Ignoring empty command line', 'WARN')
                continue
            if command.__contains__('summary'):
                casalog.post('Mode summary is not allowed in list operation', 'WARN')
                continue
                
            flagd = {}
            flagd['row'] = str(i)
            flagd['applied'] = applied
            flagd['command'] = command
            flagd['interval'] = interval
            flagd['level'] = level
            flagd['reason'] = reason
            flagd['severity'] = severity
            flagd['time'] = time
            flagd['type'] = type
            # Insert into main dictionary
            myflagd[ncmds] = flagd
            ncmds += 1
                
    except:
        raise Exception, 'Cannot create dictionary'
    
    return myflagd


def readXML(sdmfile, mytbuff=1.0):    
    '''Reads Antenna.xml and Flag.xml SDM tables and parses
       into returned dictionary as flag command strings.
       sdmfile (string) -->  path to SDM containing Antenna.xml and Flag.xml
       mytbuff (float)  -->  time interval (start and end) padding (seconds)
       
       Returns a dictionary with a FLAG_CMD structure'''
#
#
#   Dictionary structure:
#   fid : 'id' (string)
#         'mode' (string)         flag mode ('online')
#         'antenna' (string)
#         'timerange' (string)
#         'reason' (string)
#         'time' (float)          in mjd seconds
#         'interval' (float)      in mjd seconds
#         'cmd' (string)          string (for COMMAND col in FLAG_CMD)
#         'type' (string)         'FLAG' / 'UNFLAG'
#         'applied' (bool)        set to True here on read-in
#         'level' (int)           set to 0 here on read-in
#         'severity' (int)        set to 0 here on read-in
#

    try:
        from xml.dom import minidom
    except ImportError, e:
        print 'failed to load xml.dom.minidom:\n', e
        exit(1)

    if type(mytbuff) != float:
        casalog.post('Incorrect type for tbuff, found "'
                     + str(mytbuff) + '", setting to 1.0', 'WARN')
        mytbuff = 1.0

    # construct look-up dictionary of name vs. id from Antenna.xml
    xmlants = minidom.parse(sdmfile + '/Antenna.xml')
    antdict = {}
    rowlist = xmlants.getElementsByTagName('row')
    for rownode in rowlist:
        rowname = rownode.getElementsByTagName('name')
        ant = str(rowname[0].childNodes[0].nodeValue)
        rowid = rownode.getElementsByTagName('antennaId')
        antid = str(rowid[0].childNodes[0].nodeValue)
        antdict[antid] = ant
    casalog.post('Found ' + str(rowlist.length)
                 + ' antennas in Antenna.xml', 'DEBUG')

    # now read Flag.xml into dictionary row by row
    xmlflags = minidom.parse(sdmfile + '/Flag.xml')
    flagdict = {}
    rowlist = xmlflags.getElementsByTagName('row')
    nrows = rowlist.length
    for fid in range(nrows):
        rownode = rowlist[fid]
        rowfid = rownode.getElementsByTagName('flagId')
        fidstr = str(rowfid[0].childNodes[0].nodeValue)
        flagdict[fid] = {}
        flagdict[fid]['id'] = fidstr
        rowid = rownode.getElementsByTagName('antennaId')
        antid = str(rowid[0].childNodes[0].nodeValue)
        antname = antdict[antid]
        # start and end times in mjd ns
        rowstart = rownode.getElementsByTagName('startTime')
        start = int(rowstart[0].childNodes[0].nodeValue)
        startmjds = float(start) * 1.0E-9 - mytbuff
        t = qa.quantity(startmjds, 's')
        starttime = qa.time(t, form='ymd', prec=9)
        rowend = rownode.getElementsByTagName('endTime')
        end = int(rowend[0].childNodes[0].nodeValue)
        endmjds = float(end) * 1.0E-9 + mytbuff
        t = qa.quantity(endmjds, 's')
        endtime = qa.time(t, form='ymd', prec=9)
    # time and interval for FLAG_CMD use
        times = 0.5 * (startmjds + endmjds)
        intervs = endmjds - startmjds
        flagdict[fid]['time'] = times
        flagdict[fid]['interval'] = intervs
        # reasons
        rowreason = rownode.getElementsByTagName('reason')
        reas = str(rowreason[0].childNodes[0].nodeValue)
        # Construct antenna name and timerange and reason strings
        flagdict[fid]['antenna'] = antname
        timestr = starttime + '~' + endtime
        flagdict[fid]['timerange'] = timestr
        flagdict[fid]['reason'] = reas
        # Construct command strings (per input flag)
        cmd = "antenna='" + antname + "' timerange='" + timestr + "'"
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
                     + ' flags in Flag.xml', 'DEBUG')
    else:
        casalog.post('No valid flags found in Flag.xml', 'WARN')

    # return the dictionary for later use
    return flags


def getUnion(cmdlist):
    '''Get a dictionary of a union of all selection parameters from a list of lines:
       cmdlist --> list of strings with parameters and values (par=val)
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
        
    nrows = cmdlist.__len__()

    for i in range(nrows):
        cmdline = cmdlist[i]
        
        # split by white space
        keyvlist = cmdline.split()
        if keyvlist.__len__() > 0:  

            # Split by '='
            for keyv in keyvlist:

                # Skip if it is a comment character #
#                if keyv.count('#') > 0:
                if keyv.startswith('#'):
                    break
                    
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
                    ants += xval + ','

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
    ants = ants.rstrip(',')
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
    dicpars['antenna'] = ants
    dicpars['timerange'] = times
    # Correlations should be handled only by the agents
    dicpars['correlation'] = ''
    dicpars['intent'] = ints
    dicpars['feed'] = feeds
    dicpars['array'] = arrays
    dicpars['uvrange'] = uvs
    dicpars['spw'] = spws
    dicpars['observation'] = obs

    # Real number of input lines
    # Get the number of occurrences of each parameter
    npars = getNumPar(cmdlist)
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


def getNumPar(cmdlist):
    '''Get the number of occurrences of all parameter keys
       cmdlist --> list of strings with parameters and values
    '''

    nrows = cmdlist.__len__()
            
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
    'comment':0
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
    ui = 0  # count the number of lines with yvrange
    pi = 0  # count the number of lines with spw
    
    for i in range(nrows):
        cmdline = cmdlist[i]

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

                # Check which parameter
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

    return npars


def writeCMD(msfile, flagcmd, writeflags, outfile):
    ''' Reads a list of parameters and save it to the FLAG_CMD table or to a text file.
        When saving in the FLAG_CMD table, it will also update the APPLIED column with the
        writeflags value.
        Returns the number of flag commands written'''
    
    
    nadd = 0
    try:
        import pylab as pl
    except ImportError, e:
        print 'failed to load pylab:\n', e
        return -1

    # Create lists for each column in the FLAG_CMD table
    # TODO: How about the TIME column? How to calculate it?
    tim_list = [0.0]
    intv_list = [0.0]
    reas_list = ['']        
    typ_list = ['FLAG']
    sev_list = [0]
    lev_list = [0]
    app_list = [writeflags]           
    
    casalog.post("Flag command to save is %s"%flagcmd, 'DEBUG')    

    if outfile != '':
        
        # Append to a text file                
        ffout = open(outfile, 'a')

        try:
            for line in flagcmd:
                print >> ffout, '%s' % line
        except:
            nadd = -1
            raise Exception, 'Error writing parameters to file ' \
                + outfile
        ffout.close()
        return nadd
    
    # Save to the FLAG_CMD table    
    cmdline = []
    cmdline.append(flagcmd)
    nadd = cmdline.__len__()
    mstable = msfile + '/FLAG_CMD'
    try:
        tb.open(mstable, nomodify=False)
    except:
        tb.close()
        raise Exception, 'Error opening FLAG_CMD table ' + mstable        
    
    nrows = int(tb.nrows())
    
    # Add blank rows
    if (debug):
        print pl.array(cmdline)
        
    tb.addrows(nadd)
    
    # Now fill them in
    tb.putcol('TIME', pl.array(tim_list), startrow=nrows, nrow=nadd)
    tb.putcol('INTERVAL', pl.array(intv_list), startrow=nrows,
              nrow=nadd)
    tb.putcol('REASON', pl.array(reas_list), startrow=nrows,
              nrow=nadd)
    tb.putcol('COMMAND', pl.array(cmdline), startrow=nrows,
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


    return nadd


def writeFlagCmd(msfile, myflags, vrows, applied, outfile):
    '''
    Writes the flag commands to FLAG_CMD or to an ASCII file
    msfile  -->   MS
    myflags -->  dictionary of commands read from inputfile (from readFromTable, etc.)
    vrows   -->  list of valid rows from myflags dictionary to save
    applied -->  value to update APPLIED column of FLAG_CMD
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
                cmdline = myflags[key]['command']
                print >> ffout, '%s' % cmdline
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
            if not command.__contains__('summary'):                   
                cmd_list.append(myflags[key]['command'])
                tim_list.append(myflags[key]['time'])
                intv_list.append(myflags[key]['interval'])
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
    
        casalog.post('Saved ' + str(nadd) + ' rows to FLAG_CMD', 'DEBUG')
        
    else:
        casalog.post('Saved zero rows to FLAG_CMD; no flags found', 'DEBUG')

    return nadd




def getLinePars(cmdline, mlist=[]):
    '''Get a dictionary of all selection parameters from a line:
       -> cmdline is a string with parameters
       -> mlist is a list of the mode parameters to add to the
          returned dictionary.
    '''
            
    # Dictionary of parameters to return
    dicpars = {}
        
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
                dicpars['correlation'] = xval

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
                dicpars['mode'] = xval

            elif mlist != []:
                # Any parameters requested for this mode?
                for m in mlist:
                    if xkey == m:
                        dicpars[m] = xval
                        
            
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
    '''Give correct types to non-string parameters'''

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
    if params.has_key('recalcuvw'):
        params['recalcuvw'] = eval(params['recalcuvw'].capitalize())
           
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


def purgeEmptyPars(cmdline):
    '''Remove empty parameters from a string:
       -> cmdline is a string with parameters
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
                newstr = newstr+' '+xkey+'='+xval+' '
            
    else:
        casalog.post('String of parameters is empty','WARN')   
         
    return newstr

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
          antListWrite(outfile, antlist);
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

    try:
        if(os.path.exists(infile)):
            ifile = file(infile,'r');
            thelist = ifile.readlines();
            ifile.close();
            
    except:
            raise Exception, 'Error opening file ' + infile
    
    
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
