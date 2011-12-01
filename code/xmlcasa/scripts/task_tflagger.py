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
    # * Check extendpols, because it is always FALSE
    # * move correlation to agent's parameters
    # * implement saving parameters in FLAG_CMD
    
    
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
        tflocal.selectdata(field=field, spw=spw, array=array, feed=feed, scan=scan, \
                           baseline=antenna, uvrange=uvrange, time=timerange, \
                           correlation=correlation, intent=intent, observation=str(observation))   

        
        # Setup global parameters
        agent_pars = {}
        if mode == '':
            mode = 'manualflag'
            
        agent_pars['mode'] = mode
        agent_pars['extend'] = extend
        agent_pars['datadisplay'] = datadisplay
        agent_pars['writeflags'] = writeflags

        # Get extend sub parameters
        if (extend == True and mode != 'unflag' and mode != 'summary' and mode != 'extendflags'):
            
            agent_pars['extendpols'] = bool(extendpols)
            agent_pars['growtime'] = growtime
            agent_pars['growfreq'] = growfreq
            agent_pars['growaround'] = growaround
            agent_pars['flagneartime'] = flagneartime
            agent_pars['flagnearfreq'] = flagnearfreq
        
        
        # Set agent's parameters
        if mode == 'manualflag':
            casalog.post('Manualflag mode is active')
            
        elif mode == 'clip':
            agent_pars['expression'] = expression
            agent_pars['datacolumn'] = datacolumn
            agent_pars['clipminmax'] = clipminmax
            agent_pars['clipoutside'] = clipoutside
            agent_pars['channelavg'] = channelavg
            casalog.post('Clip mode is active')
            
        elif mode == 'shadow':
            agent_pars['diameter'] = diameter
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
            agent_pars['timecutoff'] = timecutoff
            agent_pars['freqcutoff'] = freqcutoff
            agent_pars['timefit'] = timefit
            agent_pars['freqfit'] = freqfit
            agent_pars['maxnpieces'] = maxnpieces
            agent_pars['flagdimension'] = flagdimension
            agent_pars['usewindowstats'] = usewindowstas
            agent_pars['halfwin'] = halfwin
            casalog.post('Time and Frequency (tfcrop) mode is active')

        elif mode == 'extendflags':
            agent_pars['extendpols'] = bool(extendpols)
            agent_pars['growtime'] = growtime
            agent_pars['growfreq'] = growfreq
            agent_pars['growaround'] = growaround
            agent_pars['flagneartime'] = flagneartime
            agent_pars['flagnearfreq'] = flagnearfreq
            casalog.post('Extendflags mode is active')
            
        elif mode == 'unflag':
            # Remove extend if set
            if extend:
                agent_pars['extend'] = False

            casalog.post('Unflag mode is active')
                
            
        elif mode == 'summary':
            # Remove extend if set
            if extend:
                agent_pars['extend'] = False
            if writeflags:
                agent_pars['writeflags'] = False

            agent_pars['minrel'] = minrel
            agent_pars['maxrel'] = maxrel
            agent_pars['minabs'] = minabs
            agent_pars['maxabs'] = maxabs
            agent_pars['spwchan'] = spwchan
            agent_pars['spwcorr'] = spwcorr
            casalog.post('Summary mode is active')
        
        # Now Parse the agent's parameters
        casalog.post('Parsing the agent\'s parameters')
        if debug:
            print agent_pars
            
        if (not tflocal.parseAgentParameters(agent_pars)):
            casalog.post('Failed to parse parameters of agent %s' %mode, 'ERROR')
        

        # Initialize the agent
        casalog.post('Initializing the agent')
        tflocal.init()
        
        # Run the tool
        casalog.post('Running the tflagger tool')
        summary_stats = tflocal.run()

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
                
        
        # Destroy the tool
        tflocal.done()
        
        return summary_stats
    
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

