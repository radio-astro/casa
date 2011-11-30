from taskinit import *
import time
import os
import sys

debug = True


def tflagger(vis = None,
             inputtype = None,          # file,table,xml,cmd
             inputfile = None,          # table parameters
             tablerows = None,          
             useapplied = None,
             tbuff = None,              # xml parameters
             ants = None,
             command = None,            # cmd parameter
             reason = None,             # applied to all input types
             ntime = None,              # taken only once per session
             extend = None,             # extend the private flags of any agent
             mode = None,
             selectdata = None,
             spw = None,                # data selection parameters
             field = None,
             antenna = None,
             uvrange = None,
             timerange = None,
             correlation = None,
             scan = None,
             intent = None,
             feed = None, 
             array = None,
             observation = None,
             expression = None,           # mode=clip parameters
             clipminmax = None,
             datacolumn = None, 
             clipoutside = None, 
             channelavg = None,
             quackinterval = None,      # mode=quack parameters
             quackmode = None, 
             quackincrement = None,
             diameter = None,           # mode=shadow parameter
             lowerlimit = None,         # mode=elevation parameters
             upperlimit = None,
             timecutoff = None,         # mode=tfcrop parameters
             freqcutoff = None,
             timefit = None, 
             freqfit = None, 
             maxnpieces = None, 
             flagdimension = None, 
             usewindowstats = None, 
             halfwin = None,
             extendpols = None,         # mode=extendflags parameters
             growtime = None, 
             growfreq = None, 
             growaround = None, 
             flagneartime = None, 
             flagnearfreq = None,
             minrel = None,             # mode=summary parameters
             maxrel = None,
             minabs = None,
             maxabs = None,
             spwchan = None,
             spwcorr = None,
             datadisplay = None,
             writeflags = None,
             flagbackup = None,
             async = None):

    # Things to consider:
    #
    # * Default inputtype is ''
    # * Default mode is manualflag
    # * Reason selecting for all input types
    # * Extendflags can be applied to every mode
    # * Parameter ntime can be applied only once per session
    # * Ability to read flags and edit them......
    # * It is already possible to read flags from the FLAG_CMD of an MS and 
    #   apply them to another MS. Do as follows
    #   flagcmd(vis='target.ms',flagmode='table',flagfile='source.ms',optype='apply')
    # 
    # Schema
    # Read input commands from input file
    # Select based on REASON
    # Configure the TestFlagger tool -> ::open()
    # Create a union of the data selection ranges -> getUnion()
    # Parse the union to the data selection -> ::selectdata()
    # Read the mode and specific parameters.
    # Parse the agent's parameters -> setupAgent() -> ::parseAgentParameters()
    # Initialize the FlagDataHandler and the agents -> ::init()
    # Run the tool
    # Delete the tool
                        
    # TODO:
    # * what to do about flagbackup
    # * add reason selection in other inputtypes
    # * what about those timeranges calculated in flagcmd?
    # * add other global parameters to agent's parameters
    # * generate flagcmds from summary views
    # * enable editing/listing/saving of flag commands
    
    #
    # Task tflagger
    #    Reads flag commands and applies to MS


    if pCASA.is_mms(vis):
        pCASA.execute("tflagger", locals())
        return

    try:
        from xml.dom import minidom
    except:
        raise Exception, 'Failed to load xml.dom.minidom into python'

    casalog.origin('tflagger')

    tflocal = casac.homefinder.find_home_by_name('testflaggerHome').create()
    mslocal = casac.homefinder.find_home_by_name('msHome').create()

    try:
        if not os.path.exists(vis):
            raise Exception, \
                'Visibility data set not found - please verify the name'


        # MS HISTORY
        mslocal.open(vis,nomodify=False)
        mslocal.writehistory(message='taskname = tflagger', origin='tflagger')
        mslocal.open(vis,nomodify=False)
            
        # TBD
        # Get overall MS time range for later use (if needed)

        # INPUT TYPE
        # TABLE
        myflagcmd = {}
        cmdlist = []

        if (inputtype == 'table'):
            casalog.post('Reading commands from FLAG_CMD table')
            if inputfile == '':
                input = vis
            else:
                input = inputfile
                
            # Read the flag commands from the MS table
            myflagcmd = readFromTable(input, myflagrows=tablerows,
                        useapplied=useapplied,
                        myreason=reason)

        
        # FILE
        elif (inputtype == 'file'):
            casalog.post('Reading commands from file %'+inputfile)
            if inputfile == '':
                input = vis + '/FlagCMD.txt'
            else:
                input = inputfile

            if (type(input) == str) & os.path.exists(input):
                try:
                    ff = open(input, 'r')
                except:
                    raise Exception, 'Error opening file ' + input
            else:
                raise Exception, \
                    'Input command file not found - please verify the name'
                    
            # readFromFile(input)

        # XML
        elif (inputtype == 'xml'):
            casalog.post('Reading commands from the XML file %s'+vis+'/Flag.xml')
            if inputfile == '':
                input = vis
            else:
                input = inputfile
                
            # readFromXML(input)

        # TBD: Does it make any sense to have this if we can already
        # run the task without any input?????
        
        # COMMAND LIST
        elif (inputtype == 'cmd'):
            casalog.post('Reading commands from a list')
            
        # NO INPUT
        #run the task with the provided parameters        
        elif (inputtype == ''):
            input = vis
            
            if (not selectdata):
                field = antenna = timerange = correlation = scan = intent = feed = array = \
                uvrange = observation = ''

            # Create a string of the required parameters
            cmdlist = readFromCMD(mode=mode,observation=observation,array=array,scan=scan,\
                                    field=field,antenna=antenna,spw=spw,timerange=timerange,\
                                    uvrange=uvrange,correlation=correlation,intent=intent,\
                                    feed=feed,extend=extend,datadisplay=datadisplay,\
                                    writeflags=writeflags)

        # Finished with reading the input
        # Get the list of parameters
        if myflagcmd.__len__() > 0:
            for i in range(myflagcmd.__len__()):
                cmdline = myflagcmd[i]['cmd']
                cmdlist.append(cmdline)


        # Open the MS and attach it to the tool
        if (not tflocal.open(vis, ntime)):
            raise Exception, "Cannot create tflagger tool " + msname
            
        # Get the union of all data selection parameters
        unionpars = {}
        unionpars = getUnion(cmdlist)
            
        if (debug):
            casalog.post('The union of all parameters is %s' %(unionpars))
        
        # Parse the data selection
        try:
            tflocal.selectdata(unionpars)
        except:
            casalog.post('Error in parsing the data selection', ERROR)
            raise Exception

        # Parse the agents parameters
        list2save = setupAgent(tflocal,cmdlist,flagbackup)
        
        # Initialized the agents
        tflocal.init()
        
        # Run the tool
        stats = tflocal.run()

        # Delete the tool
        tflocal.done()
        
    except Exception, instance:
        print '*** Error ***', instance
                # raise

    # write history
    try:
        mslocal.open(vis, nomodify=False)
        mslocal.close()
    except:
        casalog.post('Cannot open MS for history, ignoring', 'WARN')

    return

            
def readFromTable(
    msfile,
    myflagrows=[],
    useapplied=True,
    myreason='Any',
    ):
    #
    # Read flag commands from rows of the FLAG_CMD table of msfile
    # If useapplied=False then include only rows with APPLIED=False
    # If myreason is anything other than 'Any', then select on that
    #
    # Return flagcmd structure:
    #
    # The flagcmd key is the integer row number from FLAG_CMD
    #
    #   Dictionary structure:
    #   key : 'id' (string)
    #         'mode' (string)         flag mode '','clip','shadow','quack'
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
    #
    # Open and read columns from FLAG_CMD
    mstable = msfile + '/FLAG_CMD'

    # Note, tb.getcol doesn't allow random row access, read all
    try:
        tb.open(mstable)
        f_time = tb.getcol('TIME')
        f_interval = tb.getcol('INTERVAL')
        f_type = tb.getcol('TYPE')
        f_reas = tb.getcol('REASON')
        f_level = tb.getcol('LEVEL')
        f_severity = tb.getcol('SEVERITY')
        f_applied = tb.getcol('APPLIED')
        f_cmd = tb.getcol('COMMAND')
        tb.close()
    except:
        raise Exception, 'Error reading table ' + mstable
    nrows = f_time.__len__()

    myreaslist = []
    # Parse myreason
    if type(myreason) == str:
        if myreason != 'Any':
            myreaslist.append(myreason)
    elif type(myreason) == list:
        myreaslist = myreason
    else:
        casalog.post('ERROR: reason contains unknow variable types',
                     'SEVERE')
        return

    myflagcmd = {}
    if nrows > 0:
        nflagd = 0
        if myflagrows.__len__() > 0:
            rowlist = myflagrows
        else:
            rowlist = range(nrows)
        # Prune rows if needed
        if not useapplied:
            rowl = []
            for i in rowlist:
                if not f_applied[i]:
                    rowl.append(i)
            rowlist = rowl
        if myreaslist.__len__() > 0:
            rowl = []
            for i in rowlist:
                if myreaslist.count(f_reas[i]) > 0:
                    rowl.append(i)
            rowlist = rowl

        for i in rowlist:
            flagd = {}
            flagd['id'] = str(i)
            flagd['antenna'] = ''
            flagd['mode'] = ''
            flagd['time'] = f_time[i]
            flagd['interval'] = f_interval[i]
            flagd['type'] = f_type[i]
            flagd['reason'] = f_reas[i]
            flagd['level'] = f_level[i]
            flagd['severity'] = f_severity[i]
            flagd['applied'] = f_applied[i]
            cmd = f_cmd[i]
            flagd['cmd'] = cmd
            # Extract antenna and timerange strings from cmd
            antstr = ''
            timstr = ''
            keyvlist = cmd.split()
            if keyvlist.__len__() > 0:
                for keyv in keyvlist:
                    try:
                        (xkey, val) = keyv.split('=')
                    except:
                        print 'Error: not key=value pair for ' + keyv
                        break
                    xval = val
                # strip quotes from value
                    if xval.count("'") > 0:
                        xval = xval.strip("'")
                    if xval.count('"') > 0:
                        xval = xval.strip('"')
                    if xkey == 'timerange':
                        timstr = xval
                    elif xkey == 'antenna':
                        flagd['antenna'] = xval
                    elif xkey == 'id':
                        flagd['id'] = xval
                    elif xkey == 'mode':
                        flagd['mode'] = xval
            # STM 2010-12-08 Do not put timerange if not in command
            # if timstr=='':
            # ....    # Construct timerange from time,interval
            # ....    centertime = f_time[i]
            # ....    interval = f_interval[i]
            # ....    startmjds = centertime - 0.5*interval
            # ....    t = qa.quantity(startmjds,'s')
            # ....    starttime = qa.time(t,form="ymd",prec=9)
            # ....    endmjds = centertime + 0.5*interval
            # ....    t = qa.quantity(endmjds,'s')
            # ....    endtime = qa.time(t,form="ymd",prec=9)
            # ....    timstr = starttime+'~'+endtime
            flagd['timerange'] = timstr
            # Keep original key index, might need this later
            myflagcmd[i] = flagd
            nflagd += 1
        casalog.post('Read ' + str(nflagd)+ ' rows from FLAG_CMD table in MS')
    else:
        casalog.post('FLAG_CMD table is empty, no flags extracted'
                     , 'WARN')

    return myflagcmd


#def readFromCMD(observation,array,scan,field,antenna,spw,timerange,uvrange,
#                correlation,intent,feed,mode):
def readFromCMD(**params):
    '''Get all the parameters set in the task and write them into a list of
        string of key=value
       '''
    
    cmdlist = []
    cmdstr = ''
    
    # Parse the data selection parameters
    for k in params.keys():
        v = str(params[k])
        cmdstr = cmdstr+k+'='+v+' '

    cmdlist = [cmdstr]
    
    # Parse any other global parameter
    
#    flagcmd = {}
#    flagcmd['observation'] = observation
#    flagcmd['array'] = array
#    flagcmd['scan'] = scan
#    flagcmd['field'] = field
#    flagcmd['antenna'] = antenna
#    flagcmd['spw'] = spw
#    flagcmd['timerange']= timerange
#    flagcmd['uvrange'] = uvrange
#    flagcmd['correlation'] = correlation
#    flagcmd['intent'] = intent
#    flagcmd['feed'] = feed
#    
#    if (not selectdata):
#        field = antenna = timerange = correlation = scan = intent = feed = array = uvrange = observation = ''
#        
#        flagcmd = 'mode='+mode+' '
#        # Get parameters based on mode            
#        if (mode == 'summary'):
#            flagcmd = 'minrel='+minrel+' maxrel='+maxrel+' minabs='+minabs+' maxabs='+maxabs+\
#                      ' spwchan='+spwchan+' spwcorr='+spwcorr
#            
    
    if (debug):
        casalog.post('Selection from task is %s'%cmdlist)
        
    return cmdlist


def getUnion(cmdlist):
    '''Get a dictionary of a union of all selection parameters from a list of lines:
       -> cmdlist is a list of parameters and values
    '''
    
    # TODO: remove correlation from the union. Because it's an in-row selection,
    # it is not handled by MS Selection. It should go as part of the agent specific
    # data selection parameters
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
    if(debug):
        casalog.post("The list to create a union contains %s rows"%nrows)

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
    # The union of the correlations is all of them always.
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


    return dicpars

def getNumPar(cmdlist):
    '''Get the number of occurrences of all parameter keys
       -> cmdlist is a list of string with parameters
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
    'observation':0,
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
    ui = 0  # count the number of lines with uvrange
    pi = 0  # count the number of lines with spw
    oi = 0 # count the number of lines with observation
    
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


def setupAgent(tflocal,cmdlist,flagbackup):
    ''' for optype = apply '''
        
    # TODO: correlation is to be considered here
    
    # First remove the blank lines from the list (if any)
    blanks = cmdlist.count('\n')
    if blanks > 0:
        for i in range(blanks):
            cmdlist.remove('\n')

    
    # Parameters for each mode
    manualpars = []
    quackpars = ['quackinterval','quackmode','quackincrement']
    shadowpars = ['diameter']
    clippars = ['clipminmax', 'expression', 'clipoutside','datacolumn', 'channelavg']
    elevationpars = ['lowerlimit', 'upperlimit']
    tfcroppars = ['datacolumn','expression','timecutoff','freqcutoff','timefit','freqfit','maxnpieces',\
              'flagdimentsion','usewindowstats','halfwin']
    extendflagspars = ['extendpols','growtime','growfreq','growaround','flagneartime','flagnearfreq']
    summarypars = ['minrel', 'maxrel', 'minabs', 'maxabs', 'spwchan', 'spwcorr']
    
    nrows = cmdlist.__len__()
    
    if(debug):
        print "There are %s rows"%(nrows)
        
    # command list of successful agents to save to outfile
    savelist = []
    
    # Setup the agent for each input line
    for i in range(nrows):
        cmdline = cmdlist[i]
        if cmdline.startswith('#'):
            continue

        modepars = {}
        
        if (debug):
            print cmdline
        
        # Clear the agents before
        # CHECK this call. We don't want to clear the selection of the union
#        tflocal.clearflagselection(-1)
                            
        # Get the specific parameters for the mode and call the agents
        if cmdline.__contains__('mode'): 
            if cmdline.__contains__('manualflag'): 
                mode = 'manualflag'
                modepars = getLinePars(cmdline,manualpars)
            elif cmdline.__contains__('quack'):
                mode = 'quack'
                modepars = getLinePars(cmdline,quackpars)
            elif cmdline.__contains__('shadow'):
                mode = 'shadow'
                modepars = getLinePars(cmdline,shadowpars)
            elif cmdline.__contains__('clip'):
                mode = 'clip'
                modepars = getLinePars(cmdline,clippars)
            elif cmdline.__contains__('elevation'):
                mode = 'elevation'
                modepars = getLinePars(cmdline,elevationpars)
            elif cmdline.__contains__('tfcrop'):
                mode = 'tfcrop'
                modepars = getLinePars(cmdline,tfcroppars)
            elif cmdline.__contains__('extendflags'):
                mode = 'extendflags'
                modepars = getLinePars(cmdline,extendflagspars)
            elif cmdline.__contains__('unflag'):
                print cmdline
                mode = 'unflag'
                modepars = getLinePars(cmdline,manualpars)
            elif cmdline.__contains__('summary'):
                mode = 'summary'
                modepars = getLinePars(cmdline,summarypars)
        else:
            #no mode means manualflag
            casalog.post("No mode was set. Will use manualflag")
            mode = 'manualflag'
            # Add it to the cmdline
            
            modepars = getLinePars(cmdline,manualpars) 
        
        if (debug):
            casalog.post('Parameters of mode=%s are %s' %(mode, modepars))

        # Parse the dictionary of agents
        # It should take the data selection, mode and mode-specific parameters
        if (not tflocal.parseAgentParameters(modepars)):
            casalog.post('Failed to parse parameters of agent %s' %mode, 'WARN')
                            
        # add this command line to list to save in outfile
#        savelist[i] = cmdline
        
        # FIXME: Backup the flags
#        if (flagbackup):
#            backup_cmdflags(tflocal, 'tflagger_' + mode)
    
    return savelist


def getLinePars(cmdline, mlist=[]):
    '''Get a dictionary of all selection parameters from a line:
       -> cmdline is a string with parameters
       -> mlist is a list of the mode parameters to add to the
          returned dictionary.
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
#    'extend':'',
#    'datadisplay':'',
#    'writeflags':''
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
    modes = ''
    exts = ''
    ddisp = ''
    wrf = ''
    
    # split by white space
    keyvlist = cmdline.split()
    if keyvlist.__len__() > 0:  
        
        # Split by '='
        for keyv in keyvlist:
            if (debug):
                print "keyv %s"%keyv

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

            elif xkey == "mode":
                # write mode in dictionary
                modes += xval + ','
                
            elif xkey == "extend":
                # write mode in dictionary
                exts += xval + ','
                
            elif xkey == "datadisplay":
                # write mode in dictionary
                ddisp += xval + ','
                
            elif xkey == "writeflags":
                # write mode in dictionary
                wrf += xval + ','

            else:
                # Any mode parameter to add?
                if mlist:
                    for m in mlist:
                        if xkey == m:
                            dicpars[m] = xval
                            
#                else:
                    # unknown parameter; ignore it
#                    break

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
    modes = modes.rstrip(',')
    exts = exts.rstrip(',')
    ddisp = ddisp.rstrip(',')
    wrf = wrf.rstrip(',')

    dicpars['mode'] = modes
    dicpars['scan'] = scans
    dicpars['field'] = fields
    dicpars['antenna'] = ants
    dicpars['timerange'] = times
    dicpars['correlation'] = corrs
    dicpars['intent'] = ints
    dicpars['feed'] = feeds
    dicpars['array'] = arrays
    dicpars['uvrange'] = uvs
    dicpars['spw'] = spws
    dicpars['observation'] = obs
#    dicpars['extend'] = exts
#    dicpars['datadisplay'] = ddisp
#    dicpars['writeflags'] = wrf
    
    if (debug):
        casalog.post("Dictionary of agent's parameters is %s"%dicpars)
    
    return dicpars
            
            
