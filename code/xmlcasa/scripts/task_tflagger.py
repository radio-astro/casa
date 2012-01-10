from taskinit import *
import time
import os
import sys

debug = False


def tflagger(vis,
             mode,
             selectdata,
             spw, # data selection parameters
             field,
             antenna,
             uvrange,
             timerange,
             correlation,
             scan,
             intent,
             feed,
             array,
             observation,
             expression, # mode=clip parameters
             clipminmax,
             datacolumn,
             clipoutside,
             channelavg,
             quackinterval, # mode=quack parameters
             quackmode,
             quackincrement,
             diameter, # mode=shadow parameter
             lowerlimit, # mode=elevation parameters
             upperlimit,
             timecutoff, # mode=tfcrop parameters
             freqcutoff,
             timefit,
             freqfit,
             maxnpieces,
             flagdimension,
             usewindowstats,
             halfwin,
             extendpols,
             growtime,
             growfreq,
             growaround,
             flagneartime,
             flagnearfreq,
             minrel,        # mode summary
             maxrel,
             minabs,
             maxabs,
             spwchan,
             spwcorr,
             ntime, # taken only once per session
             combinescans,
             display,
             format,
             writeflags,
             savepars,    # save the current parameters to FLAG_CMD 
             outfile,   # output file to save flag commands
             flagbackup):

    # Global parameters
    # vis, ntime, savepars, flagbackup, datadisplay, writeflags, summarydisplay

    # 
    # Schema
    # Configure the TestFlagger tool -> ::open()
    # Parse the union to the data selection -> ::selectdata()
    # Read the mode and specific parameters.
    # Parse the agent's parameters -> ::parseAgentParameters()
    # Initialize the FlagDataHandler and the agents -> ::init()
    # Run the tool
    # Delete the tool
                        
    # TODO:
    # Implement the new summary schema
    # enable combinescans sub-parameter
    
    
    #
    # Task tflagger
    #    Flags data based on data selection in various ways

    if pCASA.is_mms(vis):
        pCASA.execute("tflagger", locals())
        return

    casalog.origin('tflagger')

    tflocal = casac.homefinder.find_home_by_name('testflaggerHome').create()
    mslocal = casac.homefinder.find_home_by_name('msHome').create()

    # MS HISTORY
    mslocal.open(vis, nomodify=False)
    mslocal.writehistory(message='taskname = tflagger', origin='tflagger')
    mslocal.open(vis, nomodify=False)


    try: 
        # Verify the ntime value
        newtime = 0.0
        if type(ntime) == float or type(ntime) == int:
            if ntime == 0:
                raise Exception, 'Parameter ntime cannot be 0.0'
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
                                    
        if debug:
            casalog.post("new ntime is of type %s and value %s"%(type(newtime),newtime))
        
        
        # Open the MS and attach it to the tool
        if ((type(vis) == str) & (os.path.exists(vis))):
            tflocal.open(vis, newtime)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'


        # Select the data 
        if (debug):
#            print 'selection is: %s %s %s %s %s %s %s %s %s %s %s'%(type(field),type(spw),type(array),
#                        type(feed),type(scan),type(antenna),type(uvrange),
#                        type(timerange),type(correlation),type(intent),type(observation))

            print 'selection is: field=%s spw=%s array=%s feed=%s scan=%s antenna=%s uvrange=%s' \
                        'timerange=%s correlation=%s intent=%s observation=%s'%(field,spw,array,
                        feed,scan,antenna,uvrange,
                        timerange,correlation,intent,observation)

        # Correlation should not go in here            
        tflocal.selectdata(field=field, spw=spw, array=array, feed=feed, scan=scan, \
                           baseline=antenna, uvrange=uvrange, time=timerange, \
                           intent=intent, observation=str(observation))   

        # Set constraints to some parameters
        if mode == '':
            mode = 'manualflag'

        if (writeflags == True and mode == 'summary'):
            # It was probably a mistake of the user, reset writeflags
            casalog.post('Parameter writeflags will be reset to False to run together with mode=%s'%mode, 'WARN')
            writeflags = False
                        
        # Create a list of the selection parameters to save later
        sel_pars = ''
        sel_pars = 'mode='+mode+' field='+field+' spw='+spw+' array='+array+' feed='+feed+\
                    ' scan='+scan+' antenna='+antenna+' uvrange='+uvrange+' timerange='+timerange+\
                    ' correlation='+correlation+' intent='+intent+' observation='+str(observation)+\
                    ' ntime='+str(newtime)+' combinescans='+str(combinescans)

        # Setup global parameters
        agent_pars = {}

        # Add the global parameters to the dictionary of agent's parameters            
        agent_pars['mode'] = mode
        
        # Set up agent's parameters based on mode
        if mode == 'manualflag':
            casalog.post('Manualflag mode is active')
            
        elif mode == 'clip':
            agent_pars['expression'] = expression
            agent_pars['datacolumn'] = datacolumn
            agent_pars['clipoutside'] = clipoutside
            agent_pars['channelavg'] = channelavg
            
            # If clipminmax = [], do not write it in the dictionary.
            # It will be handled by the framework
            if clipminmax != []:      
                # Cast to float to avoid the missing decimal point                 
                clipmin = float(clipminmax[0])
                clipmax = float(clipminmax[1])
                clipminmax = []
                clipminmax.append(clipmin)
                clipminmax.append(clipmax)     
                agent_pars['clipminmax'] = clipminmax
                
            casalog.post('Clip mode is active')
            
            # Replace the white spaces
            expr = delspace(expression, '_')            
            cliprange = delspace(str(clipminmax), '')
            
            sel_pars = sel_pars+' expression='+str(expr)+' datacolumn='+datacolumn+\
                       ' clipminmax='+str(cliprange)+' clipoutside='+str(clipoutside)+\
                       ' channelavg='+str(channelavg)
            
        elif mode == 'shadow':
            agent_pars['diameter'] = diameter
            casalog.post('Shadow mode is active')
            
            sel_pars = sel_pars+' diameter='+str(diameter)

        elif mode == 'quack':
            agent_pars['quackmode'] = quackmode
            agent_pars['quackinterval'] = quackinterval
            agent_pars['quackincrement'] = quackincrement
            casalog.post('Quack mode is active')
            
            sel_pars = sel_pars+' quackmode='+str(quackmode)+' quackinterval='+str(quackinterval)+\
                       ' quackincrement='+str(quackincrement)

        elif mode == 'elevation':
            agent_pars['lowerlimit'] = lowerlimit
            agent_pars['upperlimit'] = upperlimit
            casalog.post('Elevation mode is active')
            
            sel_pars = sel_pars+' lowerlimit='+str(lowerlimit)+' upperlimit='+str(upperlimit)

        elif mode == 'tfcrop':
            agent_pars['expression'] = expression
            agent_pars['datacolumn'] = datacolumn
            agent_pars['timecutoff'] = timecutoff
            agent_pars['freqcutoff'] = freqcutoff
            agent_pars['timefit'] = timefit
            agent_pars['freqfit'] = freqfit
            agent_pars['maxnpieces'] = str(maxnpieces)
            agent_pars['flagdimension'] = flagdimension
            agent_pars['usewindowstats'] = usewindowstats
            agent_pars['halfwin'] = str(halfwin)
            casalog.post('Time and Frequency (tfcrop) mode is active')

            expr = delspace(expression, '_')
            
            sel_pars = sel_pars+' expression=\"'+str(expr)+'\" datacolumn='+datacolumn+\
                      ' timecutoff='+str(timecutoff)+' freqcutoff='+str(freqcutoff)+\
                      ' timefit='+str(timefit)+' freqfit='+str(freqfit)+' maxnpieces='+str(maxnpieces)+\
                      ' flagdimension='+str(flagdimension)+' usewindowstats='+str(usewindowstats)+\
                      ' halfwin='+str(halfwin)

        elif mode == 'extend':
            agent_pars['extendpols'] = extendpols
            agent_pars['growtime'] = growtime
            agent_pars['growfreq'] = growfreq
            agent_pars['growaround'] = growaround
            agent_pars['flagneartime'] = flagneartime
            agent_pars['flagnearfreq'] = flagnearfreq
            casalog.post('Extend mode is active')
            
            sel_pars = sel_pars+' extendpols='+str(extendpols)+' growtime='+str(growtime)+' growfreq='+\
                       str(growfreq)+' growaround='+str(growaround)+' flagneartime='+str(flagneartime)+\
                       ' flagnearfreq='+str(flagnearfreq)
            
        elif mode == 'unflag':                      
            casalog.post('Unflag mode is active')                
            
        elif mode == 'summary':
            agent_pars['spwchan'] = spwchan
            agent_pars['spwcorr'] = spwcorr
            casalog.post('Summary mode is active')
            
            # Do not save flag command for this mode
            savepars = False
            

        # Now Parse the agent's parameters
        casalog.post('Parsing the parameters for the %s agent'%mode)
        # Due to the way the MS Selection works, the correlation
        # selection needs to be done in here instead of in the
        # selectdata()
        agent_pars['correlation'] = correlation
        if debug:
            print agent_pars
            
        if (not tflocal.parseAgentParameters(agent_pars)):
            casalog.post('Failed to parse parameters of agent %s' %mode, 'ERROR')
        
        # Do display if requested
        # TODO: uncomment when FlagAgentDisplay is implemented!
        if mode != 'summary' and display != '':
            
            agent_pars = {}
            casalog.post('Parsing the display parameters')
                
            # need to create different parameters for both, data and report.
#            if display == 'both':
#                agent_pars['mode'] = 'datadisplay'
#                tflocal.parseAgentParameters(agent_pars)
#                agent_pars['mode'] = 'reportdisplay'
#                agent_pars['format'] = format
#                tflocal.parseAgentParameters(agent_pars)
#            
#            elif display == 'data':
#                agent_pars['mode'] = 'datadisplay'
#                tflocal.parseAgentParameters(agent_pars)
#            
#            elif display == 'report':
#                agent_pars['mode'] = 'reportdisplay'
#                agent_pars['format'] = format
#                tflocal.parseAgentParameters(agent_pars)
                

        # Initialize the agent
        casalog.post('Initializing the agent')
        tflocal.init()

        # Purge the empty parameters from the selection string
        flagcmd = getLinePars(sel_pars) 

        # Backup the existing flags before applying new ones
        if flagbackup and writeflags:
            casalog.post('Backup original flags before applying new flags')
            backupFlags(tflocal, mode, flagcmd)

        
        # Run the tool
        casalog.post('Running the tflagger tool')
        summary_stats = tflocal.run(writeflags)


        # Write the current parameters as flag commands to output
        if savepars:         
            ncmd = writeCMD(vis, flagcmd, outfile)
            
            
        # Destroy the tool
        tflocal.done()

        if mode == 'summary':
            #filter out baselines/antennas/fields/spws/...
            #which do not fall within limits
            if type(summary_stats) is dict:
                for x in summary_stats.keys():
                    if type(summary_stats[x]) is dict:
                        for xx in summary_stats[x].keys():
                            flagged = summary_stats[x][xx]
                            assert type(flagged) is dict
                            assert flagged.has_key('flagged')
                            assert flagged.has_key('total')
                            if flagged['flagged'] < minabs or \
                               (flagged['flagged'] > maxabs and maxabs >= 0) or \
                               flagged['flagged'] * 1.0 / flagged['total'] < minrel or \
                               flagged['flagged'] * 1.0 / flagged['total'] > maxrel:
                                    del summary_stats[x][xx]
        
        return summary_stats
    
    except Exception, instance:
        casalog.post('%s'%instance,'ERROR')
        raise
        
        #write history
#        try:
#                param_names = flagdata.func_code.co_varnames[:flagdata.func_code.co_argcount]
#                param_vals = [eval(p) for p in param_names]
#                retval &= write_history(mslocal, vis, 'flagdata', param_names,
#                                        param_vals, casalog)
#        except Exception, instance:
#                casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
#                             'WARN')
        
    return


#def writeCMD(vis, myflags, tag=''):
def writeCMD(msfile, flagcmd, outfile):
    # Reads a list of parameters and save it on the FLAG_CMD table or on a text file    
    # When saving in the FLAG_CMD table it will also update the APPLIED column to True
    # Returns the number of flag commands written (it's always one!!!)
    #
    
    
    nadd = 0
    try:
        import pylab as pl
    except ImportError, e:
        print 'failed to load pylab:\n', e
        exit(1)

    # Create lists for each column in the FLAG_CMD table
    # TODO: How about the TIME column? How to calculate it?
    tim_list = [0]
    intv_list = [0]
    reas_list = ['']        
    typ_list = ['FLAG']
    sev_list = [0]
    lev_list = [0]
    app_list = [True]
       
    
    if debug:
        casalog.post("Flag command to save is %s"%flagcmd)


    # Save to a text file
    if outfile != '':
        try:
            ffout = open(outfile, 'w')
        except:
            raise Exception, 'Error opening output file ' \
                + outfile
        try:
            casalog.post('Will save the current parameters to '+outfile)
            print >> ffout, '%s' % flagcmd
        except:
            raise Exception, 'Error writing parameters to file ' \
                + outfile
        ffout.close()
        return
    
    # Save to the FLAG_CMD table    
    cmdline = []
    cmdline.append(flagcmd)
    nadd = cmdline.__len__()
    mstable = msfile + '/FLAG_CMD'
    try:
        tb.open(mstable, nomodify=False)
    except:
        raise Exception, 'Error opening FLAG_CMD table ' + mstable
    
    nrows = int(tb.nrows())
    casalog.post('There are ' + str(nrows) + ' rows already in the FLAG_CMD table')
    
    # add blank rows
    if (debug):
        print pl.array(cmdline)
        
    tb.addrows(nadd)
    # now fill them in
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
#
    casalog.post('Wrote ' + str(nadd) + ' rows to FLAG_CMD')

    return nadd


def getLinePars(cmdline):
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

def backupFlags(tflocal, mode, flagcmd):
    ''' Backup the flags before applying new ones'''
    
    # Create names like this:
    # before_manualflag_1,
    # before_manualflag_2,
    # before_manualflag_3,
    # etc
    #
    # Generally  before_<mode>_<i>, where i is the smallest
    # integer giving a name, which does not already exist
    
    # Get the existing flags from the FLAG_VERSION_LIST file
    # in the MS directory
    existing = tflocal.getflagversionlist(printflags=False)

    # Remove the comments from strings
    existing = [x[0:x.find(' : ')] for x in existing]

    i = 1
    while True:
        versionname = mode +"_" + str(i)
#        versionname = mode + str(i)

        if not versionname in existing:
            break
        else:
            i = i + 1

    time_string = str(time.strftime('%Y-%m-%d %H:%M:%S'))

    if debug:
        casalog.post("Saving current flags to " + versionname + " before applying new flags")

                      
#    tflocal.saveflagversion(versionname=versionname,
#                           comment='flagdata autosave before ' + mode + ' on ' + time_string,
#                           merge='replace')
    tflocal.saveflagversion(versionname=versionname,
                           comment='backup before applying\"'+flagcmd+'\"on ' + time_string,
                           merge='replace')

    # Save flagcmd to flagbackup
    # Need to consider flagmanager when writing this
    
    # We already have the flagbackup in disk, now append the FLAG_CMD
    # sub-table to it
#    mstable = msfile + '/FLAG_CMD'
#    try:
#        tb.open(mstable, nomodify=False)
#    except:
#        raise Exception, 'Error opening FLAG_CMD table ' + mstable
    
#    tb.copy(newtablename=versionname + '/FLAG_CMD')
#    tb.done()
    

    return

def createstr(selpars):
    '''Get a string par=value. If value is empty, return
       accumulated string with only par that has non-empty value'''
    
    
    # walk through string and get the value after =
    # if value is empty, get next
    
    return strpar

def delspace(word, replace):
    '''Replace the white space of a string'''
    
    newword = word
    if word.count(' ') > 0:
        newword = word.replace(' ', replace)
    
    return newword
    
