from taskinit import *
import time
import os
import sys
import flaghelper as fh
from parallel.parallel_task_helper import ParallelTaskHelper

debug = False


def tflagdata(vis,
             mode,
             autocorr,      # mode manual parameter
             inpfile,       # mode list parameters
             reason,
             spw,           # data selection parameters
             field,
             antenna,
             uvrange,
             timerange,
             correlation,
             scan,
             intent,
             array,
             observation,
             feed,
             clipminmax,    # mode clip parameters
             datacolumn,
             clipoutside,
             channelavg,
             clipzeros,
             quackinterval, # mode quack parameters
             quackmode,
             quackincrement,
             tolerance,      # mode shadow parameter
             addantenna,
             lowerlimit,    # mode elevation parameters
             upperlimit,
             ntime,         # mode tfcrop
             combinescans,
             timecutoff,    
             freqcutoff,
             timefit,
             freqfit,
             maxnpieces,
             flagdimension,
             usewindowstats,
             halfwin,
             winsize,    # rflag parameters
             timedev,
             freqdev,
             timedevscale,
             freqdevscale,
             spectralmax,
             spectralmin,
             extendpols,    # mode extend
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
             basecnt,
             action,           # run or not the tool
             display,
             flagbackup,
             savepars,      # save the current parameters to FLAG_CMD  or to a file
             cmdreason,     # reason to save to flag cmd
             outfile):      # output file to save flag commands

    # Global parameters
    # vis, mode, action, savepars                      
    
    #
    # Task tflagdata
    #    Flags data based on data selection in various ways
    #    
    # This is a replacement task to flagdata. It takes different parameters and
    # different default values. This task uses a new tool and framework underneath.
    
    # jagonzal (CAS-4119): Use absolute paths for input files to ensure that the engines find them
    if (inpfile != ""):
        inpfile = os.path.abspath(inpfile)
        fh.addAbsPath(inpfile)
        
    if (outfile != ""):
        outfile = os.path.abspath(outfile)        
        
    if (isinstance(addantenna,str) and addantenna != ""):
        addantenna = os.path.abspath(addantenna)
        
    if (isinstance(timedev,str) and timedev != ""):
        timedev = os.path.abspath(timedev)        
        
    if (isinstance(freqdev,str) and freqdev != ""):
        freqdev = os.path.abspath(freqdev)        
        
    # SMC: moved the flagbackup to before initializing the cluster.
    # Note that with this change, a flag backup will be created even if
    # an error happens that prevents the flagger tool from running.    
    if (mode != 'summary' and flagbackup):
        casalog.post('Backup original flags before applying new flags')
        fh.backupFlags(vis, 'tflagdata')    

    if pCASA.is_mms(vis):
        # Set flagbackup to False because only the controller
        # should create a backup
        flagbackup = False
        pCASA.execute("tflagdata", locals())
        return

    casalog.origin('tflagdata')

    # Take care of the trivial parallelization
    if ParallelTaskHelper.isParallelMS(vis):
        # Set flagbackup to False because only the controller
        # should create a backup
        flagbackup = False
        # To be safe convert file names to absolute paths.
        helper = ParallelTaskHelper('tflagdata', locals())
        retVar = helper.go()
        return retVar
    
    # Create local tools
    tflocal = casac.testflagger()
    mslocal = casac.ms()

    try: 
        # Verify the ntime value
        newtime = 0.0
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
                                    
        casalog.post("New ntime is of type %s and value %s"%(type(newtime),newtime), 'DEBUG')
                
        # Open the MS and attach it to the tool
        if ((type(vis) == str) & (os.path.exists(vis))):
            tflocal.open(vis, newtime)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'


        # Get the parameters for the mode
        agent_pars = {}

        
        # By default, write flags to the MS
        writeflags = True
        
        # Only the apply action writes to the MS
        # action=apply     --> write to the MS
        # action=calculate --> do not write to the MS
        # action=''        --> do not run the tool and do not write to the MS
        if action != 'apply':
            writeflags = False
                                         
        # Default mode
        if mode == '' or mode == 'manualflag':
            mode = 'manual'
        
        # Read in the list of commands
        if mode == 'list':
            casalog.post('List mode is active')
            # Parse the input file
            try:
                if inpfile == '':
                     casalog.post('Input file is empty', 'ERROR')
                     
                flaglist = fh.readFile(inpfile)
                casalog.post('%s'%flaglist,'DEBUG')
                
                # Make a FLAG_CMD compatible dictionary. Select by reason if requested
                flagcmd = fh.makeDict(flaglist, reason)
                casalog.post('%s'%flagcmd,'DEBUG')
                
                # Update the list of commands with the selection
                flaglist = []
                for k in flagcmd.keys():
                    cmdline = flagcmd[k]['command']
                    flaglist.append(cmdline)
                                    
                # List of command keys in dictionary
                vrows = flagcmd.keys()
                

            except:
                raise Exception, 'Error reading the input file '+inpfile
            
            casalog.post('Read ' + str(vrows.__len__())
                         + ' lines from file ' + inpfile)
                             
        elif mode == 'manual':
            agent_pars['autocorr'] = autocorr
            casalog.post('Manual mode is active')

            
        elif mode == 'clip':

            if correlation == '':
                # default
                correlation = "ABS_ALL"
                
            agent_pars['datacolumn'] = datacolumn.upper()
            agent_pars['clipoutside'] = clipoutside
            agent_pars['channelavg'] = channelavg
            agent_pars['clipzeros'] = clipzeros
            

            if type(clipminmax) != list:
                casalog.post('Error : clipminmax must be a list : [min,max]', 'ERROR')
            # If clipminmax = [], do not write it in the dictionary.
            # It will be handled by the framework to flag NaNs only
            if clipminmax.__len__() == 2:      
                # Cast to float to avoid the missing decimal point                 
                clipmin = float(clipminmax[0])
                clipmax = float(clipminmax[1])
                clipminmax = []
                clipminmax.append(clipmin)
                clipminmax.append(clipmax)     
                agent_pars['clipminmax'] = clipminmax
                
            casalog.post('Clip mode is active')
                        
        elif mode == 'shadow':
            agent_pars['tolerance'] = tolerance
            
            if type(addantenna) == str:
                if addantenna != '':
                    # it's a filename, create a dictionary
                    antdict = fh.readAntennaList(addantenna)
                    agent_pars['addantenna'] = antdict
                    
            elif type(addantenna) == dict:
                if addantenna != {}:
                    agent_pars['addantenna'] = addantenna
                                                               
            casalog.post('Shadow mode is active')
            
        elif mode == 'quack':
            agent_pars['quackmode'] = quackmode
            agent_pars['quackinterval'] = quackinterval
            agent_pars['quackincrement'] = quackincrement
            casalog.post('Quack mode is active')
            

        elif mode == 'elevation':
            agent_pars['lowerlimit'] = lowerlimit
            agent_pars['upperlimit'] = upperlimit
            casalog.post('Elevation mode is active')
            

        elif mode == 'tfcrop':
            if correlation == '':
                # default
                correlation = "ABS_ALL"
                
            agent_pars['ntime'] = newtime
            agent_pars['combinescans'] = combinescans            
            agent_pars['datacolumn'] = datacolumn.upper()
            agent_pars['timecutoff'] = timecutoff
            agent_pars['freqcutoff'] = freqcutoff
            agent_pars['timefit'] = timefit
            agent_pars['freqfit'] = freqfit
            agent_pars['maxnpieces'] = maxnpieces
            agent_pars['flagdimension'] = flagdimension
            agent_pars['usewindowstats'] = usewindowstats
            agent_pars['halfwin'] = halfwin
            casalog.post('Time and Frequency (tfcrop) mode is active')

                      
        elif mode == 'rflag':
            if correlation == '':
                # default
                correlation = "ABS_ALL"

            agent_pars['ntime'] = newtime
            agent_pars['combinescans'] = combinescans   
            agent_pars['datacolumn'] = datacolumn.upper()
            agent_pars['winsize'] = winsize
            agent_pars['timedevscale'] = timedevscale
            agent_pars['freqdevscale'] = freqdevscale
            agent_pars['spectralmax'] = spectralmax
            agent_pars['spectralmin'] = spectralmin

            # These can be double, doubleArray, or string.
            # writeflags=False : calculate and return thresholds.
            # writeflags=True : use given thresholds for this run.
            if( type(timedev) == str and writeflags == True):
                timedev = fh.readRFlagThresholdFile(timedev,'timedev')
            if( type(freqdev) == str and writeflags == True):
                freqdev = fh.readRFlagThresholdFile(freqdev,'freqdev')

            agent_pars['timedev'] = timedev
            agent_pars['freqdev'] = freqdev
            
            agent_pars['writeflags'] = writeflags
            agent_pars['display'] = display

            casalog.post('Rflag mode is active')

        elif mode == 'extend':
            agent_pars['ntime'] = newtime
            agent_pars['combinescans'] = combinescans                            
            agent_pars['extendpols'] = extendpols
            agent_pars['growtime'] = growtime
            agent_pars['growfreq'] = growfreq
            agent_pars['growaround'] = growaround
            agent_pars['flagneartime'] = flagneartime
            agent_pars['flagnearfreq'] = flagnearfreq
            casalog.post('Extend mode is active')
            
            
        elif mode == 'unflag':      
            casalog.post('Unflag mode is active')                
            
        elif mode == 'summary':
            agent_pars['spwchan'] = spwchan
            agent_pars['spwcorr'] = spwcorr
            agent_pars['basecnt'] = basecnt
            
            # Disable writeflags and savepars
            writeflags = False
            savepars = False
            casalog.post('Summary mode is active')


        # Create a flagcmd dictionary of the interface parameters to save
        # when savepars = True
        if mode != 'list' and mode != 'summary':

            # CAS-4063: remove any white space in the values of the
            # selection parameters before creating the string.
            
            # Create a dictionary of the selection parameters
            seldic = {}
            seldic['field'] = field
            seldic['spw'] = spw
            seldic['array'] = array
            seldic['feed'] = feed
            seldic['scan'] = scan
            seldic['antenna'] = antenna
            seldic['uvrange'] = uvrange
            seldic['timerange'] = timerange
            seldic['intent'] = intent
            seldic['observation'] = str(observation)
            
            # String to hold the selection parameters
            sel_pars = []
            sel_pars = ' mode='+mode
            if correlation != '':
                # Replace an empty space, in case there is one
                expr = delspace(correlation, '_')
                sel_pars = sel_pars +' correlation=' + expr
            
            # Include only parameters with values in the string
            # Remove the white spaces from the values
            for k in seldic.keys():
                if seldic[k] != '':
                    # Delete any space in the value
                    val = delspace(seldic[k], '')
                    sel_pars = sel_pars +' ' + k + '=' + val
                               
            # Add the agent's parameters to the same string 
            for k in agent_pars.keys():
                if agent_pars[k] != '':
                    # Remove any white space from the string value
                    nospace = delspace(str(agent_pars[k]),'')
                    sel_pars = sel_pars + ' ' + k + '=' + nospace
                
            
            # Create a dictionary according to the FLAG_CMD structure
            flagcmd = fh.makeDict([sel_pars])
                        
            # Number of commands in dictionary
            vrows = flagcmd.keys()
            casalog.post('There are %s cmds in dictionary of mode %s'%(vrows.__len__(),mode),'DEBUG')


        # Setup global parameters in the agent's dictionary
        apply = True
                    
        # Correlation does not go in selectdata, but in the agent's parameters
        if correlation != '':
            agent_pars['correlation'] = correlation.upper()
        
        
        # Hold the name of the agent
        agent_name = mode.capitalize()
        agent_pars['name'] = agent_name
        agent_pars['mode'] = mode
        agent_pars['apply'] = apply      
                          
        ##########  Only save the parameters and exit; action = ''     
        if (action == '' or action == 'none') and savepars == False:
            casalog.post('Parameter action=\'\' is only meaningful with savepars=True.', 'WARN')
            return 0
        
        if (action == '' or action == 'none') and savepars == True:
            fh.writeFlagCmd(vis, flagcmd, vrows, False, cmdreason, outfile)  
            if outfile == '':
                casalog.post('Saving parameters to FLAG_CMD')
            else:
                casalog.post('Saving parameters to '+outfile)                            
            return 0

        
        ######### From now on it is assumed that action = apply or calculate
        
        # Select the data and parse the agent's parameters
        if mode != 'list':
            tflocal.selectdata(field=field, spw=spw, array=array, feed=feed, scan=scan, \
                               antenna=antenna, uvrange=uvrange, time=timerange, \
                               intent=intent, observation=str(observation))   

            # CAS-3959 Handle channel selection at the FlagAgent level
            agent_pars['spw'] = spw
            casalog.post('Parsing the parameters for the %s mode'%mode)
            if (not tflocal.parseagentparameters(agent_pars)):
                casalog.post('Failed to parse parameters for mode %s' %mode, 'ERROR')
                
            casalog.post('%s'%agent_pars, 'DEBUG')
       
        else:        
            # Select a loose union of the data selection from the list
            # The loose union will be calculated for field and spw only
            # antenna, correlation and timerange should be handled by the agent
            if vrows.__len__() == 0:
                raise Exception, 'There are no valid commands in list'
            
            unionpars = {}
            if vrows.__len__() > 1:
               unionpars = fh.getUnion(vis, flagcmd)
               
               if( len( unionpars.keys() ) > 0 ):
                    casalog.post('Pre-selecting a subset of the MS : ');
                    casalog.post('%s'%unionpars)
                    
               else:
                    casalog.post('Iterating through the entire MS');
                    
#               mslocal.close()
                    
            # Get all the selection parameters, but set correlation to ''
            elif vrows.__len__() == 1:
                cmd0 = flagcmd[vrows[0]]['command']
                unionpars = fh.getSelectionPars(cmd0)
                casalog.post('The selected subset of the MS will be: ');
                casalog.post('%s'%unionpars);
                
            tflocal.selectdata(unionpars);

            # Parse the parameters for each agent in the list
            list2save = fh.setupAgent(tflocal, flagcmd, [], apply, writeflags, display)

        # Do display if requested
        if display != '':
            
            agent_pars = {}
            casalog.post('Parsing the display parameters')
                
            agent_pars['mode'] = 'display'
            # need to create different parameters for both, data and report.
            if display == 'both':
                agent_pars['datadisplay'] = True
                agent_pars['reportdisplay'] = True
            
            elif display == 'data':
                agent_pars['datadisplay'] = True
            
            elif display == 'report':
                agent_pars['reportdisplay'] = True
                
            # jagonzal: CAS-3966 Add datacolumn to display agent parameters
            agent_pars['datacolumn'] = datacolumn.upper()
            tflocal.parseagentparameters(agent_pars)
            
            # Disable saving the parameters to avoid inconsistencies
            if savepars:
                casalog.post('Disabling savepars for the display', 'WARN')
                savepars = False
                    
        # Initialize the agents
        casalog.post('Initializing the agents')
        tflocal.init()

        # HPC work: moved the flagbackup to before the cluster
        # initialization.
        # Backup the existing flags before applying new ones
#        if flagbackup and writeflags:
#            casalog.post('Backup original flags before applying new flags')
#            fh.backupFlags(tflocal, 'tflagdata')
        
        # Run the tool
        casalog.post('Running the testflagger tool')
        summary_stats_list = tflocal.run(writeflags, True)
        
        # Inform the user that end of MS summary was not written to the MS
        if not writeflags:
            casalog.post("Flags are not written to the MS. (action=\'calculate\')")


        # Now, deal with all the modes that return output.
        # Summary : Currently, only one is allowed in the task
        # Rflag : There can be many 'rflags' in the list mode.

        ## Pull out RFlag outputs. There will be outputs only if writeflags=False
        if (mode == 'rflag' or mode== 'list') and (writeflags==False):  
            fh.extractRFlagOutputFromSummary(mode,summary_stats_list, flagcmd)

        # Save the current parameters/list to FLAG_CMD or to output
        if savepars:  

            if outfile == '':
                casalog.post('Saving parameters to FLAG_CMD')        
            else:
                casalog.post('Saving parameters to '+outfile)
                                      
            if mode != 'list':     
                fh.writeFlagCmd(vis, flagcmd, vrows, writeflags, cmdreason, outfile)  
            else:
                valid_rows = list2save.keys()
                fh.writeFlagCmd(vis, list2save, valid_rows, writeflags, cmdreason, outfile)        
            
        # Destroy the tool
        tflocal.done()

        # Pull out the 'summary' part of summary_stats_list.
        # (This is the task, and there will be only one such dictionary.)
        # After this step, the only thing left in summary_stats_list are the
        # list of reports/views, if any.  Return it, if the user wants it.
        summary_stats={};
        if mode == 'summary':
           if type(summary_stats_list) is dict:
               nreps = summary_stats_list['nreport'];
               for rep in range(0,nreps):
                    repname = "report"+str(rep);
                    if summary_stats_list[repname]['type'] == "summary":
                          summary_stats = summary_stats_list.pop(repname);
                          summary_stats_list[repname] = {'type':'none','name':'none'};
                          break;  # pull out only one summary.
        
           # Filter out baselines/antennas/fields/spws/... from summary_stats
           # which do not fall within limits
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
        
        # if (need to return the reports/views as well as summary_stats) :
        #      return summary_stats , summary_stats_list;
        # else :
        #      return summary_stats;
        tb.showcache()
        return summary_stats;
    
    except Exception, instance:
        tflocal.done()
        tb.showcache()
        casalog.post('%s'%instance,'ERROR')
        raise
        
    # Write history to the MS
    try:
        mslocal.open(vis, nomodify=False)
        mslocal.writehistory(message='taskname = tflagdata', origin='tflagdata')
        param_names = tflagdata.func_code.co_varnames[:tflagdata.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        retval &= write_history(mslocal, vis, 'tflagdata', param_names,
                                param_vals, casalog)
        
        mslocal.close()
    except Exception, instance:
        mslocal.close()
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                         'WARN')
        
    tb.showcache()
    return



def delspace(word, replace):
    '''Replace the white space of a string with another character'''
    
    newword = word
    if word.count(' ') > 0:
        newword = word.replace(' ', replace)
    
    return newword
    
