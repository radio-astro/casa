from taskinit import *
import time
import os
import sys
import copy
import pprint
import flaghelper as fh
from parallel.parallel_task_helper import ParallelTaskHelper
# this should be replaced when CASA really moves to Python 2.7
from OrderedDictionary import OrderedDict

debug = False

# Decorator function to print the arguments of a function
def dump_args(func):
    "This decorator dumps out the arguments passed to a function before calling it"
    argnames = func.func_code.co_varnames[:func.func_code.co_argcount]
    fname = func.func_name
   
    def echo_func(*args,**kwargs):
        print fname, ":", ', '.join('%s=%r' % entry for entry in zip(argnames,args) + kwargs.items())
        return func(*args, **kwargs)
   
    return echo_func

# Helper class
class FlagHelper(ParallelTaskHelper):
    def __init__(self, args={}):
        self.__args = args
    
#    @dump_args
    def setupInputFile(self, parname):
        '''Create a temporary input file with
           absolute pathnames for other input files
           such as addantenna, timedev, freqdev.
           The temporary input file will have the same
           name with an extension .tmp'''
        
        newpar = None
        if isinstance(parname, str) and parname != '':
            newpar = fh.addAbsolutePath(parname)
        elif isinstance(parname, list) and os.path.isfile(parname[0]):
            newpar = []
            for i in range(len(parname)):
                newpar.append(fh.addAbsolutePath(parname[i]))
        
        return newpar    
        
#    @dump_args
    def setupCluster(self, thistask=''):
        '''Get a simple_cluster to execute this task'''
        if thistask == '':
            thistask = 'flagdata'
            
        ParallelTaskHelper.__init__(self, task_name=thistask, args=self.__args)
        
#    @dump_args 
    def setupRflag(self, devpar):
        '''cast rflag's list parameters from numpy types to Python types
        devpar --> list of numeric parameters or list of list
                   such as timedev or freqdev'''
        
        nt = copy.deepcopy(devpar)
        for i in range(len(nt)):
            if (isinstance(nt[i],list)):
                nnt = nt[i]
                for j in range(len(nnt)):
                    elem = fh.evaluateNumpyType(nnt[j])
                    # write the casted element back  
                    devpar[i][j] = elem
        
        
# The flagdata task
def flagdata(vis,
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
             extendflags,
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
             name,
             action,           # run or not the tool
             display,
             flagbackup,
             savepars,      # save the current parameters to FLAG_CMD  or to a file
             cmdreason,     # reason to save to flag cmd
             outfile):      # output file to save flag commands

    
    #
    # Task flagdata
    #    Flags data from an MS or calibration table based on data selection in various ways
    
    casalog.origin('flagdata')
                    
    if (action == 'none' or action=='' or action=='calculate'):
        flagbackup = False
        
    # SMC: moved the flagbackup to before initializing the cluster.
    # Note that with this change, a flag backup will be created even if
    # an error happens that prevents the flagger tool from running.    
    if (mode != 'summary' and flagbackup):
        casalog.post('Backup original flags before applying new flags')
        fh.backupFlags(aflocal=None, msfile=vis, prename='flagdata')
        # Set flagbackup to False because only the controller
        # should create a backup
        flagbackup = False

    # Initialize the helper class
    orig_locals = locals()
    FHelper = FlagHelper()

    # Check if vis is a MS, MMS or cal table:
    # typevis = 1 --> cal table
    # typevis = 0 --> MS
    # typevis = 2 --> MMS
    iscal = False
    typevis = fh.isCalTable(vis)
    if typevis == 1:
        iscal = True


    # ***************** Input is MMS -- Parallel Processing ***********************   
         
    if typevis == 2 and action != '' and action != 'none':
                            
        # Create a temporary input file with .tmp extension.
        # Use this file for all the processing from now on.
        if (isinstance(inpfile,str) and inpfile != '') or \
           (isinstance(inpfile, list) and os.path.isfile(inpfile[0])):
            inpfile = FHelper.setupInputFile(inpfile)
            if inpfile != None:
                orig_locals['inpfile'] = inpfile
        
        if outfile != '':
            outfile = os.path.abspath(outfile)
            orig_locals['outfile'] = outfile
        if isinstance(addantenna, str) and addantenna != '':
            addantenna = os.path.abspath(addantenna)
            orig_locals['addantenna'] = addantenna
        if isinstance(timedev, str) and timedev != '':
            timedev = os.path.abspath(timedev)
            orig_locals['timedev'] = timedev
        if isinstance(freqdev, str) and freqdev != '':
            freqdev = os.path.abspath(freqdev)
            orig_locals['freqdev'] = freqdev    
    
        FHelper.__init__(orig_locals)
        
        # For tests only
#        FHelper.bypassParallelProcessing(1)

        FHelper.setupCluster('flagdata')
        # (CAS-4119): Override summary minabs,maxabs,minrel,maxrel 
        # so that it is done after consolidating the summaries
        
        # By-pass options to filter summary
        filterSummary = False
        if ((mode == 'summary') and ((minrel != 0.0) or (maxrel != 1.0) or (minabs != 0) or (maxabs != -1))):
            filterSummary = True
            
            myms = mstool()
            myms.open(vis)
            subMS_list = myms.getreferencedtables()
            myms.close()
            
            if (minrel != 0.0):
                minreal_dict = create_arg_dict(subMS_list,0.0)
                FHelper.override_arg('minrel',minreal_dict)
            if (maxrel != 1.0):
                maxrel_dict = create_arg_dict(subMS_list,1.0)
                FHelper.override_arg('maxrel',maxrel_dict)
            if (minabs != 0):
                minabs_dict = create_arg_dict(subMS_list,0)
                FHelper.override_arg('minabs',minabs_dict)
            if (maxabs != -1):
                maxabs_dict = create_arg_dict(subMS_list,-1)
                FHelper.override_arg('maxabs',maxabs_dict)
                
        # By-pass options to filter summary
        if savepars:  
            
            myms = mstool()
            myms.open(vis)
            subMS_list = myms.getreferencedtables()
            myms.close()
            
            savepars_dict = create_arg_dict(subMS_list,False)
            FHelper.override_arg('savepars',savepars_dict)
            
        # Execute the parallel engines
        retVar = FHelper.go()
        
        # Filter summary at MMS level
        if (mode == 'summary'):
            if filterSummary:
                retVar = filter_summary(retVar,minrel,maxrel,minabs,maxabs)
            return retVar
        # Save parameters at MMS level
        elif savepars:
            action = 'none'
        else:
            return retVar
    
    summary_stats={};
    
#    if pCASA.is_mms(vis):
#        pCASA.execute("flagdata", orig_locals)
#        return
    
    
    # ***************** Input is a normal MS/cal table ****************
    
    # Create local tools
    aflocal = casac.agentflagger()
    mslocal = mstool()

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
            aflocal.open(vis, newtime)
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
        # Make a dictionary of the input commands. Select by reason if requested
        flagcmd = {}
        
        if mode == 'list':
            casalog.post('List mode is active')
            # Parse the input file
            try:            
                # Input commands are given in a list
                if isinstance(inpfile, list):
                    
                    # It is a list of input files
                    if os.path.isfile(inpfile[0]):
                        flaglist = []
                        for ifile in inpfile:
                            casalog.post('Will read commands from the file '+ifile)                    
                            flaglist = flaglist + fh.readFile(ifile)
                        
                        flagcmd = fh.parseDictionary(flaglist, reason)
                    
                    # It is a list of strings with flag commands
                    else:
                        casalog.post('Will read commands from a Python list')
                        flagcmd = fh.parseDictionary(inpfile, reason)
                    
                # Input commands are given in a file
                elif isinstance(inpfile, str):
                    
                    if inpfile == '':
                         casalog.post('Input file is empty', 'ERROR')
                         
                    casalog.post('Will read commands from the file '+inpfile)
                    flaglist = fh.readFile(inpfile)
                    casalog.post('%s'%flaglist,'DEBUG')
                    
                    flagcmd = fh.parseDictionary(flaglist, reason)
                
                else:
                    casalog.post('Input type is not supported', 'ERROR')
                    
                casalog.post('%s'%flagcmd,'DEBUG1')
                                                                    
                # List of flag commands in dictionary
                vrows = flagcmd.keys()
                
            except Exception, instance:
                casalog.post('%s'%instance,'ERROR')
                raise Exception, 'Error reading the input list '
            
            casalog.post('Read ' + str(vrows.__len__())
                         + ' lines from input list ')
                             
        elif mode == 'manual':
            agent_pars['autocorr'] = autocorr
            casalog.post('Manual mode is active')

            
        elif mode == 'clip':
                
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
            agent_pars['extendflags'] = bool(extendflags)
            casalog.post('Time and Frequency (tfcrop) mode is active')

                      
        elif mode == 'rflag':
            if newtime != 0.0:
                # this means ntime='scan', the default
                agent_pars['ntime'] = newtime
                
            agent_pars['combinescans'] = combinescans   
            agent_pars['datacolumn'] = datacolumn.upper()
            agent_pars['winsize'] = winsize
            agent_pars['timedevscale'] = timedevscale
            agent_pars['freqdevscale'] = freqdevscale
            agent_pars['spectralmax'] = spectralmax
            agent_pars['spectralmin'] = spectralmin
            agent_pars['extendflags'] = bool(extendflags)

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
            agent_pars['name'] = name
            
            # Disable writeflags and savepars
            writeflags = False
            savepars = False
            casalog.post('Summary mode is active')


        # Add the mode to the agent's parameters
        agent_pars['mode'] = mode

        # Correlation does not go in selectdata, but in the agent's parameters
        if correlation != '':
            agent_pars['correlation'] = correlation.upper()

        # Create a flagcmd dictionary of the interface parameters for saving
        if mode != 'list' and mode != 'summary':
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
            
            # Add the agent's parameters
            seldic.update(agent_pars)
            
            tempdict = copy.deepcopy(seldic)
            # Remove the empty parameters
            for k,v in seldic.iteritems():
                if v == '':
                    tempdict.pop(k)
            
            cmddict = {'command':tempdict}
            cmddict['reason'] = ''
            cmddict['applied'] = False
            flagcmd[0] = cmddict
            
            # Number of commands in dictionary
            vrows = flagcmd.keys()
            casalog.post('There are %s cmds in dictionary of mode %s'%(vrows.__len__(),mode),'DEBUG1')
            
        modified_flagcmd = flagcmd

        # Setup global parameters in the agent's dictionary
        apply = True        
        
        # Hold the name of the agent
        agent_name = mode.capitalize()
        agent_pars['agentname'] = agent_name
        agent_pars['apply'] = apply      
                          
        ##########  Only save the parameters and exit; action = ''     
        if action == '' or action == 'none':
            if savepars == False:
                casalog.post('Parameter action=\'\' is only meaningful with savepars=True.', 'WARN')
                aflocal.done()
                return summary_stats
            
            else:
                if iscal and outfile == '':
                    casalog.post('Saving to FLAG_CMD is not supported for cal tables', 'WARN')

                else:                                 
                    fh.writeFlagCommands(vis, flagcmd, writeflags, cmdreason, outfile, True) 
                     
            aflocal.done()
            return summary_stats

        
        ######### From now on it is assumed that action = apply or calculate
        
        # Select the data and parse the agent's parameters
        if mode != 'list':
            aflocal.selectdata(field=field, spw=spw, array=array, feed=feed, scan=scan, \
                               antenna=antenna, uvrange=uvrange, timerange=timerange, \
                               intent=intent, observation=str(observation))   

            # CAS-3959 Handle channel selection at the FlagAgent level
            agent_pars['spw'] = spw
            casalog.post('Parsing the parameters for the %s mode'%mode, 'DEBUG1')
            if (not aflocal.parseagentparameters(agent_pars)):
#                casalog.post('Failed to parse parameters for mode %s' %mode, 'ERROR')
                raise ValueError, 'Failed to parse parameters for mode %s' %mode
                
            casalog.post('%s'%agent_pars, 'DEBUG')
       
        else:        
            # Select a loose union of the data selection from the list
            # The loose union will be calculated for field and spw only;
            # antenna, correlation and timerange should be handled by the agent
            if vrows.__len__() == 0:
                raise Exception, 'There are no valid commands in list'
            
            unionpars = {}
            # Do not create union for a cal table
            if iscal:
                if vrows.__len__() == 1:
                    unionpars = fh.parseSelectionPars(flagcmd[0]['command'])
                    casalog.post('The selected subset of the cal table will be: ');
                    casalog.post('%s'%unionpars);
                    
            else:
                if vrows.__len__() > 1:
                   unionpars = fh.parseUnion(vis, flagcmd)
                   
                   if( len( unionpars.keys() ) > 0 ):
                        casalog.post('Pre-selecting a subset of the MS : ');
                        casalog.post('%s'%unionpars)
                        
                   else:
                        casalog.post('Iterating through the entire MS');
                                                
                # Get all the selection parameters, but set correlation to ''
                elif vrows.__len__() == 1:
                    unionpars = fh.parseSelectionPars(flagcmd[0]['command'])
                    casalog.post('The selected subset of the MS will be: ');
                    casalog.post('%s'%unionpars);
                
            aflocal.selectdata(unionpars);

            # Parse the parameters for each agent in the list
            # Get the returned modified dictionary of flag commands which
            # is needed by the rflag agent, when present in a list
            modified_flagcmd = fh.parseAgents(aflocal, flagcmd, [], apply, writeflags, display)

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
                
            # CAS-3966 Add datacolumn to display agent parameters
            agent_pars['datacolumn'] = datacolumn.upper()
            aflocal.parseagentparameters(agent_pars)
            
            # Disable saving the parameters to avoid inconsistencies
            if savepars:
                casalog.post('Disabling savepars for the display', 'WARN')
                savepars = False
                    
        # Initialize the agents
        casalog.post('Initializing the agents')
        aflocal.init()
        
        # Run the tool
        casalog.post('Running the agentflagger tool')
        summary_stats_list = aflocal.run(writeflags, True)
        
        # Inform the user when flags are not written to the MS
        if not writeflags:
            casalog.post("Flags are not written to the MS. (action=\'calculate\')")


        # Now, deal with all the modes that return output.
        # Rflag : There can be many 'rflags' in list mode.

        ## Pull out RFlag outputs. There will be outputs only if writeflags=False
        if (mode == 'rflag' or mode== 'list') and (writeflags==False):  
            pprint.pprint(summary_stats_list)
            fh.parseRFlagOutputFromSummary(mode,summary_stats_list, modified_flagcmd)

        # Save the current parameters/list to FLAG_CMD or to output
        if savepars:  
            # Cal table type
            if iscal:
                if outfile == '':
                    casalog.post('Saving to FLAG_CMD is not supported for cal tables', 'WARN')
                else:
                    casalog.post('Saving parameters to '+outfile)
                    fh.writeFlagCommands(vis, flagcmd, writeflags, cmdreason, outfile, True)  
                    
            # MS type
            else:                
                if outfile == '':
                    casalog.post('Saving parameters to FLAG_CMD')        
                else:
                    casalog.post('Saving parameters to '+outfile)                                          
                
                fh.writeFlagCommands(vis, flagcmd, writeflags, cmdreason, outfile, True)
            
        # Destroy the tool
        aflocal.done()

        retval = True
        # Write history to the MS. Only for modes that write to the MS
        if not iscal:
            if mode != 'summary' and action == 'apply':
                try:
                    param_names = flagdata.func_code.co_varnames[:flagdata.func_code.co_argcount]
                    param_vals = [eval(p) for p in param_names]
                    retval &= write_history(mslocal, vis, 'flagdata', param_names,
                                            param_vals, casalog)
                    
                except Exception, instance:
                    casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                                 'WARN')
        
        # Pull out the 'summary' reports of summary_stats_list.
        if mode == 'summary' or mode == 'list':
           ordered_summary_list = OrderedDict(summary_stats_list)
           nreps = ordered_summary_list['nreport']
           if nreps > 0:                   
               for rep in range(0,nreps):
                   repname = "report"+str(rep)
                   if ordered_summary_list[repname]['type'] != "summary":
                       ordered_summary_list.pop(repname)
                   else:
                       # apply requested filtering
                       summary_stats = ordered_summary_list.pop(repname);
                       summary_stats = filter_summary(summary_stats,minrel,maxrel,minabs,maxabs)
                       # add filtered dictionary back
                       ordered_summary_list[repname] = summary_stats

               # Clean up the dictionary before returning it
               ordered_summary_list.pop('type')
               ordered_summary_list.pop('nreport')
               
               if len(ordered_summary_list) == 1:
                   repkey = ordered_summary_list.keys()
                   summary_stats_list = ordered_summary_list.pop(repkey[0])
               else:                       
                   # rename the keys of the dictionary according to
                   # the number of reports left in dictionary
                   counter = 0
                   summary_reports = OrderedDict()
                   for k in ordered_summary_list.iterkeys():
                       repname = "report"+str(counter)
                       summary_reports[repname] = ordered_summary_list[k]
                       counter += 1
                       
                   summary_stats_list = dict(summary_reports)
               
           # for when there is no summary mode in a list
           else:
               summary_stats_list = {}  
               
        else:
             summary_stats_list = {} 
                   
        return summary_stats_list
    
    except Exception, instance:
        aflocal.done()
        casalog.post('%s'%instance,'ERROR')
        return summary_stats


# Helper functions
def delspace(word, replace):
    '''Replace the white space of a string with another character'''
    
    newword = word
    if word.count(' ') > 0:
        newword = word.replace(' ', replace)
    
    return newword

def filter_summary(summary_stats,minrel,maxrel,minabs,maxabs):
    
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

def create_arg_dict(subMS_list,value):
    
    ret_dict = []
    for subMS in subMS_list:
        ret_dict.append(value)
        
    return ret_dict
    
   
    
    
    
    
    