import casac
import os
import commands
import math
import pdb
import numpy
import shutil
import string
from numpy import unique
from odict import odict

###some helper tools
mslocal = casac.homefinder.find_home_by_name('msHome').create()
tbtool = casac.homefinder.find_home_by_name('tableHome')
tb=tbtool.create()

debug = True

class flaghelper:
    def __init__(self, tflocal, casalog=None):
        '''Class with helper functions for tflagdata and tflagcmd'''
        
        if not casalog:  # Not good!
            loghome =  casac.homefinder.find_home_by_name('logsinkHome')
            casalog = loghome.create()
            #casalog.setglobal(True)
        self._casalog = casalog
        self.tf = tflocal
        

    def readList(self, inputfile):
        '''Read in the lines from an input file'''
        
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

    def makeDict(self, flaglist):
        '''Make a dictionary compatible with a FLAG_CMD table structure
        
           cmdlist --> list of parameters
           returns a dictionary
        '''

        if flaglist.__len__() == 0:
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
        nrows = flaglist.__len__()
        try:
            for i in range(nrows):            
                command = flaglist[i]
                                    
                if command == '':
                    self._casalog.post('Ignoring empty command line', 'WARN')
                    continue
                if command.__contains__('summary'):
                    self._casalog.post('Mode summary is not allowed in list operation', 'WARN')
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
    
   
    def getUnion(self, cmdlist):
        '''Get a dictionary of a union of all selection parameters from a list of lines:
           -> cmdlist is a list of strings with parameters and values
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
        npars = self.getNumPar(cmdlist)
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
    

    def getNumPar(self, cmdlist):
        '''Get the number of occurrences of all parameter keys
           -> cmdlist is a list of strings with parameters and values
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

    
    def writeCMD(self, msfile, flagcmd, writeflags, outfile):
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
        
        self._casalog.post("Flag command to save is %s"%flagcmd, 'DEBUG')    
    
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
    

    def writeFlagCmd(self, msfile, myflags, vrows, applied, outfile):
        '''
        Writes the flag commands to FLAG_CMD or to an ASCII file
        msfile    MS
        myflags   dictionary of commands read from inputfile (from readFromTable, etc.)
        vrows     list of valid rows from myflags dictionary to save
        applied   value to update APPLIED column of FLAG_CMD
        outfile   if not empty, save to it
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
            self._casalog.post('There are ' + str(nrows)
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
        
            self._casalog.post('Saved ' + str(nadd) + ' rows to FLAG_CMD', 'DEBUG')
            
        else:
            self._casalog.post('Saved zero rows to FLAG_CMD; no flags found', 'DEBUG')
    
        return nadd

    
    def setupAgent(self, myflagcmd, myrows, apply):
        ''' Setup the parameters of each agent and call the testflagger tool
        
            myflagcmd --> it is a dictionary with the following structure:
            {0: {'cmd': " mode='clip' clipmin=[0,4]",'id': '0'}
            myrows --> selected rows to apply/unapply flags
            apply --> it's a boolean to control whether to apply or unapply the flags
            Returns a dictionary '''
    
    
        if not myflagcmd.__len__() >0:
            self._casalog.post('There are no flag commands in list', 'SEVERE')
            return
        
        # Parameters for each mode
        manualpars = []
        clippars = ['clipminmax', 'expression', 'clipoutside','datacolumn', 'channelavg', 'clipzeros']
        quackpars = ['quackinterval','quackmode','quackincrement']
        shadowpars = ['diameter']
        elevationpars = ['lowerlimit','upperlimit'] 
        tfcroppars = ['ntime','combinescans','expression','datacolumn','timecutoff','freqcutoff',
                      'timefit','freqfit','maxnpieces','flagdimension','usewindowstats','halfwin']
        extendpars = ['ntime','combinescans','extendpols','growtime','growfreq','growaround',
                      'flagneartime','flagnearfreq']
        
            
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
            time = myflagcmd[key]['time']
            type = myflagcmd[key]['type']

            self._casalog.post('cmdline for key%s'%key, 'DEBUG')
            self._casalog.post('%s'%cmdline, 'DEBUG')
            self._casalog.post('applied is %s'%applied, 'DEBUG')
            
            if cmdline.startswith('#'):
                continue
        
            modepars = {}
            parslist = {}
            mode = ''    
            valid = True
                    
            # Get the specific parameters for the mode
            if cmdline.__contains__('mode'):                 
                if cmdline.__contains__('manual'): 
                    mode = 'manual'
                    modepars = self.getLinePars(cmdline,manualpars)   
                elif cmdline.__contains__('clip'):
                    mode = 'clip'
                    modepars = self.getLinePars(cmdline,clippars)
                elif cmdline.__contains__('quack'):
                    mode = 'quack'
                    modepars = self.getLinePars(cmdline,quackpars)
                elif cmdline.__contains__('shadow'):
                    mode = 'shadow'
                    modepars = self.getLinePars(cmdline,shadowpars)
                elif cmdline.__contains__('elevation'):
                    mode = 'elevation'
                    modepars = self.getLinePars(cmdline,elevationpars)
                elif cmdline.__contains__('tfcrop'):
                    mode = 'tfcrop'
                    modepars = self.getLinePars(cmdline,tfcroppars)
                elif cmdline.__contains__('extend'):
                    mode = 'extend'
                    modepars = self.getLinePars(cmdline,extendpars)
                elif cmdline.__contains__('unflag'):
                    mode = 'unflag'
                    modepars = self.getLinePars(cmdline,manualpars)
                elif cmdline.__contains__('rflag'):
                    mode = 'rflag'
                    modepars = self.getLinePars(cmdline,rflagpars)
                else:
                    # Unknown mode, ignore it
                    self._casalog.post('Ignoring unknown mode', 'WARN')
                    valid = False
    
            else:
                # No mode means manual
                mode = 'manual'
                cmdline = cmdline+' mode=manual'
                modepars = self.getLinePars(cmdline,manualpars)   
                    
                    
            # CHECK if modepars gets modified in task
            # Read ntime
            self.readNtime(modepars)
            
            # Cast the correct type to non-string parameters
            self.fixType(modepars)
            
            # Add the apply/unapply parameter to dictionary            
            modepars['apply'] = apply
            
            # Unapply selected rows only and re-apply the other rows with APPLIED=True
            if not apply and myrows.__len__() > 0:
                if key in myrows:
                    modepars['apply'] = False
                elif not applied:
                    self._casalog.post("Skipping this %s"%modepars,"DEBUG")
                    continue
                elif applied:
                    modepars['apply'] = True
                    valid = False
            
            # Keep only cmds that overlap with the unapply cmds
            # TODO later
            
            # Hold the name of the agent and the cmd row number
            agent_name = mode.capitalize()+'_'+str(key)
            modepars['name'] = agent_name
            
            # Remove the data selection parameters if there is only one agent,
            # for performance reasons
            if myflagcmd.__len__() == 1:
                sellist=['scan','field','antenna','timerange','intent','feed','array','uvrange',
                         'spw','observation']
                for k in sellist:
                    if modepars.has_key(k):
                        modepars.pop(k)
    
            self._casalog.post('Parsing parameters of mode %s in row %s'%(mode,key), 'DEBUG')
            self._casalog.post('%s'%modepars, 'DEBUG')
    
            # Parse the dictionary of parameters to the tool
            if (not self.tf.parseagentparameters(modepars)):
                self._casalog.post('Failed to parse parameters of mode %s in row %s' %(mode,key), 'WARN')
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
                parslist['time'] = time
                parslist['type'] = type
                savelist[key] = parslist
                    
        self._casalog.post('Dictionary of valid commands to save','DEBUG')
        self._casalog.post('%s'%savelist, 'DEBUG')
    
        return savelist

    def getLinePars(self, cmdline, mlist=[]):
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



    def readNtime(self, params):
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
                        self._casalog.post('Cannot convert units of ntime. Will use default 0.0s', 'WARN')
              
        params['ntime'] = float(newtime)


    def fixType(self, params):
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
        if params.has_key('diameter'):
            params['diameter'] = float(params['diameter'])
            
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


    def purgeEmptyPars(self, cmdline):
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
            self._casalog.post('String of parameters is empty','WARN')   
             
        return newstr


