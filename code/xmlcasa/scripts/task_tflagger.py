from taskinit import *
import time
import os
import sys

debug = True


def tflagger(vis=None,
             ntime=None, # taken only once per session
             mode=None,
             selectdata=None,
             spw=None, # data selection parameters
             field=None,
             antenna=None,
             uvrange=None,
             timerange=None,
             correlation=None,
             scan=None,
             intent=None,
             feed=None,
             array=None,
             observation=None,
             expression=None, # mode=clip parameters
             clipminmax=None,
             datacolumn=None,
             clipoutside=None,
             channelavg=None,
             quackinterval=None, # mode=quack parameters
             quackmode=None,
             quackincrement=None,
             diameter=None, # mode=shadow parameter
             lowerlimit=None, # mode=elevation parameters
             upperlimit=None,
             timecutoff=None, # mode=tfcrop parameters
             freqcutoff=None,
             timefit=None,
             freqfit=None,
             maxnpieces=None,
             flagdimension=None,
             usewindowstats=None,
             halfwin=None,
             extendpols=None, # mode=extendflags parameters
             growtime=None,
             growfreq=None,
             growaround=None,
             flagneartime=None,
             flagnearfreq=None,
             minrel=None, # mode=summary parameters
             maxrel=None,
             minabs=None,
             maxabs=None,
             spwchan=None,
             spwcorr=None,
             extend=None, # extend the private flags of any agent
             datadisplay=None,
             writeflags=None,
             flagbackup=None,
             async=None):

    # Things to consider:
    #
    # * Default mode is manualflag
    # * Extendflags can be applied to every mode
    # * Parameter ntime can be applied only once per session
    # 
    # Schema
    # Configure the TestFlagger tool -> ::open()
    # Parse the union to the data selection -> ::selectdata()
    # Read the mode and specific parameters.
    # Parse the agent's parameters -> setupAgent() -> ::parseAgentParameters()
    # Initialize the FlagDataHandler and the agents -> ::init()
    # Run the tool
    # Delete the tool
                        
    # TODO:
    # * what to do about flagbackup
    
    
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
        # Open the MS and attach it to the tool
        if ((type(vis) == str) & (os.path.exists(vis))):
            tflocal.open(vis, ntime)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'

        
        # Select the data 
        selection = []
        tflocal.selectdata(selection, field, spw, array, feed, scan, antenna,\
                           uvrange, timerange, correlation, intent, observation)   
        
                 
        if mode == 'manualflag':
            # Get specific + global parameters
            setupAgent(tflocal, mode)
            
        elif mode == "clip":
            # Get clip parameters
            setupAgent(tflocal, mode, expression=expression, datacolumn=datacolumn,\
                       clipminmax=clipminmax, clipoutside=clipoutside, channelavg=channelavg)
            
#            elif mode == 'quack':#            elif (mode == 'shadow'):
                

#            elif (mode == 'summary'):

            # filter out baselines/antennas/fields/spws/...
            # which do not fall within limits
#                if type(stats) is dict:
#                    for x in stats.keys():
#                        if type(stats[x]) is dict:
#                            for xx in stats[x].keys():
#                                flagged = stats[x][xx]
#                                assert type(flagged) is dict
#                                assert flagged.has_key('flagged')
#                                assert flagged.has_key('total')
#                                if flagged['flagged'] < minabs or \
#                                   (flagged['flagged'] > maxabs and maxabs >= 0) or \
#                                   flagged['flagged'] * 1.0 / flagged['total'] < minrel or \
#                                   flagged['flagged'] * 1.0 / flagged['total'] > maxrel:
#                                        del stats[x][xx]
#                
#                return stats

    except Exception, instance:
            print '*** Error ***', instance
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

def setupAgent(tflocal,mode,**pars):
    ''' Setup agent with its own parameters '''
        
    # TODO: correlation is to be considered here
    
    if debug:
        print pars
        
    # Create a dictionary of the parameters
    params = {}
    
    # Common global parameters for every mode
    params['mode'] = mode
    params['extend'] = ''
    params['writeflags'] = ''
    params['datadisplay'] = ''
    
    if mode == "manualflag":
        for x in pars.keys():
            if x == "mode":
                params['mode'] = pars[x]
    if mode == "clip":
        for x in pars.keys():
            if x == "expression":
                params['expression'] = pars[x]
                
    print params
    
    
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
    
    # command list of successful agents to save to outfile
    savelist = []

    # Parse the dictionary of agents
    # It should take the mode and mode-specific parameters
#    if (not tflocal.parseAgentParameters(modepars)):
#        casalog.post('Failed to parse parameters of agent %s' %mode, 'WARN')
                            
        # add this command line to list to save in outfile
#        savelist[i] = cmdline
        
        # FIXME: Backup the flags
#        if (flagbackup):
#            backup_cmdflags(tflocal, 'tflagger_' + mode)
    
    return savelist
        
