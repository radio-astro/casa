import time
import os
import sys
from taskinit import *
im,cb,ms,tb,fg,af,me,ia,po,sm,cl,cs,rg,dc,vp=gentools()

debug = False
def flagdata2(vis = None,
             flagbackup = None,
             selectdata = None,
             spw = None, 
             field = None,
             antenna = None,
             uvrange = None,
             timerange = None,
             correlation = None,
             scan = None,
             feed = None, 
             array = None,
             manualflag = None,
             mf_spw = None, 
             mf_field = None,
             mf_antenna = None,
             mf_uvrange = None,
             mf_timerange = None,
             mf_correlation = None,
             mf_scan = None,
             mf_feed = None, 
             mf_array = None,
             clip = None,
             clipexpr = None, 
             clipminmax = None,
             clipcolumn = None, 
             clipoutside = None, 
             channelavg = None,
             shadow = None,
             diameter = None,
             quack = None,
             quackinterval = None, 
             quackmode = None, 
             quackincrement = None,
             autoflag = None,
             algorithm = None,
             column = None, 
             expr = None,
             thr = None, 
             window = None,
             rfi = None,
             rfi_clipexpr = None, 
             rfi_clipcolumn = None, 
             rfi_time_amp_cutoff = None,
             rfi_freq_amp_cutoff = None,
             rfi_freqlinefit = None,
             rfi_auto_cross = None,
             rfi_num_time = None,
             rfi_start_chan = None,
             rfi_end_chan = None,
             rfi_bs_cutoff = None,
             rfi_ant_cutoff = None,
             rfi_flag_level = None,
             unflag = None,
             summary = None,
             minrel = None,
             maxrel = None,
             minabs = None,
             maxabs = None):

    casalog.origin('flagdata2')
    fglocal = casac.homefinder.find_home_by_name('flaggerHome').create()
    mslocal = casac.homefinder.find_home_by_name('msHome').create()

#    fg.done()
#    fg.clearflagselection(0)
#    autocorr = False
    try: 
        if ((type(vis)==str) & (os.path.exists(vis))):
            fglocal.open(vis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'
        
        if (not manualflag and not clip and not quack and not shadow and not rfi and 
            not autoflag and not unflag and not summary):
#            casalog.post('No flagging mode was selected', 'WARN')
#            casalog.post('Please, set at least one mode to run this task', 'WARN')
            msg = 'No flagging mode was selected. Please, set at least one mode to run this task.'
            casalog.post(msg, 'SEVERE')
            raise Exception
        
        if (unflag and (manualflag or clip or quack or shadow or rfi or autoflag)):
            casalog.post('Cannot run unflag simultaneously with any other mode', 'SEVERE')
            casalog.post('Please, verify your parameters.', 'SEVERE')
            raise Exception
        
        ## FIXME: autocorr
        # Select the data
        casalog.post('Flagging selection')
        fglocal.setdata()
#        if selectdata:
#            fglocal.setdata(field = field, 
#                           spw = spw, 
#                           array = array, 
#                           feed = feed, 
#                           scan = scan, 
#                           baseline = antenna, 
#                           uvrange = uvrange, 
#                           time = timerange, 
#                           correlation = correlation)
#        else:
#            field = spw = array = feed = scan = baseline = uvrange = time = correlation = ""
#            fglocal.setdata()
        
        # In manualflag, clip and quack modes,
        # filter out the parameters which are not used                         
        if clip:
            mode = 'clip'
                # In manualflag and quack modes,
                # filter out the parameters which are not used
            casalog.post('Flagging in clip mode')
            manual_clip_quack(fglocal, mode, selectdata, flagbackup,
#                         autocorr=autocorr,
                         clipexpr=clipexpr,       
                         clipminmax=clipminmax,   
                         clipcolumn=clipcolumn,   
                         clipoutside=clipoutside, 
                         channelavg=channelavg,   
                         spw=spw,
                         field=field,
                         antenna=antenna,
                         timerange=timerange,
                         correlation=correlation,
                         scan=scan,
                         feed=feed,
                         array=array,
                         uvrange=uvrange)
        if quack:
            mode = 'quack'
            casalog.post('Flagging in quack mode')
            manual_clip_quack(fglocal, mode, selectdata, flagbackup,
#                         autocorr=autocorr,
                         clipminmax=[], clipoutside=False,
                         clipcolumn="",channelavg=False,
                         quackinterval=quackinterval,
                         quackmode=quackmode,           
                         quackincrement=quackincrement, 
                         spw=spw,
                         field=field,
                         antenna=antenna,
                         timerange=timerange,
                         correlation=correlation,
                         scan=scan,
                         feed=feed,
                         array=array,
                         uvrange=uvrange)
        if shadow:
            mode = 'shadow'
            casalog.post('Flagging in shadow mode')
#            fglocal.setdata()
            fglocal.setshadowflags( \
                        field = field, \
                        spw = spw, \
                        array = array, \
                        feed = feed, \
                        scan = scan, \
                        baseline = antenna, \
                        uvrange = uvrange, \
                        time = timerange, \
                        correlation = correlation, \
                       diameter = diameter)
#            fglocal.setshadowflags(diameter = diameter)

            if flagbackup:
                backup_flags(fglocal,mode)
            
        if autoflag:
            mode = 'autoflag'
            casalog.post('Flagging in autoflag mode')
#            fglocal.setdata(field = field, \
#                           spw = spw, \
#                           array = array, \
#                           feed = feed, \
#                           scan = scan, \
#                           baseline = antenna, \
#                           uvrange = uvrange, \
#                           time = timerange, \
#                           correlation = correlation)
            rec = fglocal.getautoflagparams(algorithm=algorithm)
            rec['expr'] = expr
            rec['thr'] = thr
            #rec['rowthr'] = rowthr;
            rec['hw'] = window
            #rec['rowhw'] = rowhw;
            #if( algorithm == 'uvbin' ):
            #     rec['nbins'] = nbins;
            #     rec['minpop'] = minpop;
            rec['column'] = column
            fglocal.setautoflag(algorithm = algorithm, parameters = rec)
                
            if flagbackup:
                backup_flags(fglocal,mode)

        if rfi:
            mode = 'rfi'
            casalog.post('Flagging in rfi mode')
#            fglocal.setdata(field = field, \
#                           spw = spw, \
#                           array = array, \
#                           feed = feed, \
#                           scan = scan, \
#                           baseline = antenna, \
#                           uvrange = uvrange, \
#                           time = timerange, \
#                           correlation = correlation)

            # Get the detault parameters for a particular algorithm,
            # then modify them
                
            par = fglocal.getautoflagparams(algorithm='tfcrop')
            #print "par =", par
                
            ## True : Show plots of each time-freq chunk.
            ## Needs 'gnuplot'
            ## Needs "ds9 &" running in the background (before starting casapy)
            ## Needs xpaset, xpaget, etc.. accessible in the path (for ds9)
            par['showplots']=False
    ## jmlarsen: Do not show plots. There's no way for the user to interrupt
    ## a lengthy sequence of plots (CAS-1655)
                
            ## channel range (1 based)
            par['start_chan']=rfi_start_chan 
            par['end_chan']=rfi_end_chan
                
            ## number of time-steps in each chunk
            par['num_time']=rfi_num_time

            ## flag on cross-correlations and auto-correlations. (0 : only autocorrelations)
            par['auto_cross']= rfi_auto_cross   
                
            ## Flag Level :
            ## 0: flag only what is found. 
            ## 1: extend flags one timestep before and after
            ## 2: 1 and extend flags one channel before/after.
            par['flag_level']=rfi_flag_level

            ## data expression on which to flag.
            par['expr']=rfi_clipexpr

            ## data column to use.
            par['column']=rfi_clipcolumn

            ## False : Fit the bandpass with a piecewise polynomial
            ## True : Fit the bandpass with a straight line.
            par['freqlinefit']=rfi_freqlinefit

            ## Flagging thresholds ( N sigma ), where 'sigma' is the stdev of the "fit".
            #par['freq_amp_cutoff']=3
            #par['time_amp_cutoff']=4
            par['freq_amp_cutoff']=rfi_freq_amp_cutoff
            par['time_amp_cutoff']=rfi_time_amp_cutoff
                
            # Tell the 'fg' tool which algorithm to use, and set the parameters.
            # Note : Can set multiple instances of this (will be done one after the other)
            #
            fglocal.setautoflag(algorithm='tfcrop', parameters=par)

            if flagbackup:
                backup_flags(fglocal,mode)

        if manualflag:
            # it needs to have all clip parameters set to their defaults
            mode = 'manualflag'
            casalog.post('Flagging in manualflag mode')
            # call it on the general selection parameters
            manual_clip_quack(fglocal, mode, selectdata, flagbackup,
#                         autocorr=autocorr,
                         clipminmax=[],   
                         clipcolumn="",   
                         clipoutside=False, 
                         channelavg=False,   
                         spw=spw,
                         field=field,
                         antenna=antenna,
                         timerange=timerange,
                         correlation=correlation,
                         scan=scan,
                         feed=feed,
                         array=array,
                         uvrange=uvrange)

            # call it with specific manualflag selection. Do we need the flagbackup???
#            manual(fglocal, mode, flagbackup,
#                         autocorr=autocorr,
#                         clipminmax=[],   
#                         clipcolumn="",   
#                         clipoutside=False, 
#                         channelavg=False,   
#                         mf_spw=mf_spw,
#                         mf_field=mf_field,
#                         mf_antenna=mf_antenna,
#                         mf_timerange=mf_timerange,
#                         mf_correlation=mf_correlation,
#                         mf_scan=mf_scan,
#                         mf_feed=mf_feed,
#                         mf_array=mf_array,
#                         mf_uvrange=mf_uvrange)

        
        
        if unflag:
            casalog.post('Flagging in unflag mode')
            # pretends to be manualflag mode, but it will only unflag
            mode = "unflag"
            manual_clip_quack(fglocal, mode, selectdata, flagbackup,
#                         autocorr=autocorr,
                         unflag=unflag,
                         spw=spw,
                         field=field,
                         antenna=antenna,
                         timerange=timerange,
                         correlation=correlation,
                         scan=scan,
                         feed=feed,
                         array=array,
                         uvrange=uvrange)

        if summary:
            mode = 'summary'
            casalog.post('Flagging in summary mode')
#            fglocal.setdata()
            fglocal.setflagsummary(field=field, \
                                  spw=spw, \
                                  array=array, \
                                  feed=feed, \
                                  scan=scan, \
                                  baseline=antenna, \
                                  uvrange=uvrange, \
                                  time=timerange, \
                                  correlation=correlation)
                
            # do not backup existing flags
            
        # Finallly, run the flagging for all modes
        stats = fglocal.run()
        fglocal.done()

        # filter out baselines/antennas/fields/spws/...
        # which do not fall within limits
        if type(stats) is dict:
            for x in stats.keys():
                if type(stats[x]) is dict:
                    for xx in stats[x].keys():
                        flagged = stats[x][xx]
                        assert type(flagged) is dict
                        assert flagged.has_key('flagged')
                        assert flagged.has_key('total')
                        if flagged['flagged'] < minabs or \
                                (flagged['flagged'] > maxabs and maxabs >= 0) or \
                                flagged['flagged']*1.0/flagged['total'] < minrel or \
                                flagged['flagged']*1.0/flagged['total'] > maxrel:
                            del stats[x][xx]
        if summary==True:               
            return stats        
                

    except Exception, instance:
#        fglocal.done()
        print '*** Error ***', instance

#    fglocal.done()

    
    # FIXME: write history
    mslocal.open(vis,nomodify=False)
    mslocal.writehistory(message='taskname = flagdata', origin='flagdata')
    mslocal.writehistory(message='vis      = "' + str(vis) + '"', origin='flagdata')
    mslocal.writehistory(message='mode     = "' + str(mode) + '"', origin='flagdata')
    mslocal.close()

    return

def manual_clip_quack(fglocal, mode, selectdata, flagbackup, **params):
    if debug: print params

    if not selectdata:
        params['antenna'] = params['timerange'] = params['correlation'] = params['scan'] = params['feed'] = params['array'] = params['uvrange'] = ''

    # NOTE: if mode=manualflag, there might be further selections, therefore it
    # is necessary to call fg.setdata twice (?????). Will check this now
            
    vector_mode = False         # Are we in vector mode?
    vector_length = -1          # length of all vectors
    vector_var = ''             # reference parameter
    is_vector_spec = {}         # is a variable a vector specification?
    for x in params.keys():
        is_vector_spec[x] = False
#        print x, params[x], type(params[x])
        if x != 'clipminmax':
            if type(params[x]) == list:
                is_vector_spec[x] = True

        else:
            # clipminmax is a special case
            if type(params[x]) == list and \
                    len(params[x]) > 0 and \
                    type(params[x][0]) == list:
                is_vector_spec[x] = True

        if is_vector_spec[x]:
            vector_mode = True
            vector_length = len(params[x])
            vector_var = x
            if debug: print x, "is a vector => vector mode, length", vector_length
        else:
            if debug: print x, "is not a vector"

    if not vector_mode:            
#        fglocal.setdata()
        rename_params(params)
        fglocal.setmanualflags(**params)

    else:
        # Vector mode
        plural_s = ''
        if vector_length > 1:
            plural_s = 's'
            
        casalog.post('In parallel mode, will apply the following ' + str(vector_length) + \
                     ' flagging specification' + plural_s)
        
        # Check that parameters are consistent,
        # i.e. if they are vectors, they must have the same length
        for x in params.keys():
            if is_vector_spec[x]:
                l = len(params[x])

                if debug: print x, "has length", l
                if l != vector_length:
                    raise Exception(str(x) + ' has length ' + str(l) + \
                                    ', but ' + str(vector_var) + ' has length ' + str(vector_length))
            else:
                # vectorize this parameter (e.g.  '7' -> ['7', '7', '7']
                params[x] = [params[x]] * vector_length

        if debug: print params
        
        # Input validation done.
        # Now call setmanualflags for every specification

#        fglocal.setdata()
        for i in range(vector_length):
            param_i = {}
            param_list = ''
            for e in params.keys():
                param_i[e] = params[e][i]
                if param_i[e] != '':
                    if param_list != '':
                        param_list += '; '
                            
                    param_list = param_list + e + ' = ' + str(param_i[e])
                    if(debug): print param_list

            casalog.post(param_list)
            rename_params(param_i)
            if debug: print param_i
            
            fglocal.setmanualflags(**param_i)

    if flagbackup:
        backup_flags(fglocal, mode)
        
def manual(fglocal, mode, flagbackup, **params):
    # NOTE: if mode=manualflag, there might be further selections, therefore it
    # is necessary to call fg.setdata twice (?????). Will check this now
    print 'INSIDE manual'
    if debug: print params
    
#    if selectdata:
#        params['antenna'] = params['timerange'] = params['correlation'] = params['scan'] = params['feed'] = params['array'] = params['uvrange'] = ''
    
    params['spw'] = params['mf_spw']; del params['mf_spw']
    params['field'] = params['mf_field']; del params['mf_field']
    params['antenna'] = params['mf_antenna']; del params['mf_antenna']
    params['timerange'] = params['mf_timerange']; del params['mf_timerange']
    params['correlation'] = params['mf_correlation']; del params['mf_correlation']
    params['scan'] = params['mf_scan']; del params['mf_scan']
    params['feed'] = params['mf_feed']; del params['mf_feed']
    params['array'] = params['mf_array']; del params['mf_array']
    params['uvrange'] = params['mf_uvrange']; del params['mf_uvrange']


            
    vector_mode = False         # Are we in vector mode?
    vector_length = -1          # length of all vectors
    vector_var = ''             # reference parameter
    is_vector_spec = {}         # is a variable a vector specification?
    for x in params.keys():
        is_vector_spec[x] = False
        print x, params[x], type(params[x])
        if x != 'clipminmax':
            if type(params[x]) == list:
                is_vector_spec[x] = True

#        else:
#            # clipminmax is a special case
#            if type(params[x]) == list and \
#                    len(params[x]) > 0 and \
#                    type(params[x][0]) == list:
#                is_vector_spec[x] = True

        if is_vector_spec[x]:
            vector_mode = True
            vector_length = len(params[x])
            vector_var = x
            if debug: print x, "is a vector => vector mode, length", vector_length
        else:
            if debug: print x, "is not a vector"

    if not vector_mode:            
#        fglocal.setdata()
        print "HERRREEE"
        rename_params(params)
        fglocal.setmanualflags(**params)

    else:
        # Vector mode
        plural_s = ''
        if vector_length > 1:
            plural_s = 's'
            
        casalog.post('In parallel mode, will apply the following ' + str(vector_length) + \
                     ' flagging specification' + plural_s)
        
        # Check that parameters are consistent,
        # i.e. if they are vectors, they must have the same length
        for x in params.keys():
            if is_vector_spec[x]:
                l = len(params[x])

                if debug: print x, "has length", l
                if l != vector_length:
                    raise Exception(str(x) + ' has length ' + str(l) + \
                                    ', but ' + str(vector_var) + ' has length ' + str(vector_length))
            else:
                # vectorize this parameter (e.g.  '7' -> ['7', '7', '7']
                params[x] = [params[x]] * vector_length

        if debug: print params
        
        # Input validation done.
        # Now call setmanualflags for every specification

#        fglocal.setdata()
        for i in range(vector_length):
            param_i = {}
            param_list = ''
            for e in params.keys():
                param_i[e] = params[e][i]
                if param_i[e] != '':
                    if param_list != '':
                        param_list += '; '
                            
                    param_list = param_list + e + ' = ' + str(param_i[e])

            casalog.post(param_list)
            rename_params(param_i)
            if debug: print param_i
            
            fglocal.setmanualflags(**param_i)

    if flagbackup:
        backup_flags(fglocal, mode)

# rename some parameters,
# in order to match the interface of fg.tool
#
# validate parameters for manualflag, clip quack and unflag modes
def rename_params(params):
                
    if params.has_key('quackmode') and \
      not params['quackmode'] in ['beg', 'endb', 'end', 'tail']:
        raise Exception, "Illegal value '%s' of parameter quackmode, must be either 'beg', 'endb', 'end' or 'tail'" % (params['quackmode'])
    
    params['baseline']        = params['antenna']     ; del params['antenna']
    params['time']            = params['timerange']   ; del params['timerange']
#    params['autocorrelation'] = params['autocorr']    ; del params['autocorr']
    if params.has_key('clipminmax'):
        params['cliprange']       = params['clipminmax']  ; del params['clipminmax']
    if params.has_key('clipoutside'):
        params['outside']         = params['clipoutside'] ; del params['clipoutside']

def backup_flags(fglocal, mode):

    # Create names like this:
    # before_manualflag_1,
    # before_manualflag_2,
    # before_manualflag_3,
    # etc
    #
    # Generally  before_<mode>_<i>, where i is the smallest
    # integer giving a name, which does not already exist
    existing = fglocal.getflagversionlist(printflags=False)

    # remove comments from strings
    existing = [x[0:x.find(' : ')] for x in existing]
    i = 1
    while True:
        versionname = mode +"_" + str(i)
        if not versionname in existing:
            break
        else:
            i = i + 1

        time_string = str(time.strftime('%Y-%m-%d %H:%M:%S'))

        casalog.post("Saving current flags to " + versionname + " before applying new flags")

        fglocal.saveflagversion(versionname=versionname,
                           comment='flagdata autosave before ' + mode + ' on ' + time_string,
                           merge='replace')

