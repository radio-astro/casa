import time
import os
import sys
from taskinit import *
im,cb,ms,tb,fg,af,me,ia,po,sm,cl,cs,rg,dc,vp=gentools()

debug = False
mode = ''
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
             summary = None,
             minrel = None,
             maxrel = None,
             minabs = None,
             maxabs = None,
             unflag = None):

    casalog.origin('flagdata2')

    fg.done()
    fg.clearflagselection(0)
    autocorr = False
    try: 
        if ((type(vis)==str) & (os.path.exists(vis))):
            fg.open(vis)
        else:
            raise Exception, 'Visibility data set not found - please verify the name'

        ## FIXME: autcorr
        # The elif should be replaced by if
        # Do we need the selectdata params for subsequent mode runs?
        # Is the flagged MS saved between modes?
        if clip:
            mode == 'clip'
                # In manualflag and quack modes,
                # filter out the parameters which are not used
            casalog.post('Start flagging using clip mode')
            clipquack(selectdata, flagbackup,
                         autocorr=autocorr,
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
            casalog.post("End flagging in clip mode")
        if quack:
            mode == 'quack'
            casalog.post('Start flagging using quack mode')
            clipquack(selectdata, flagbackup,
                         autocorr=autocorr,
                         clipminmax=[], clipoutside=False,
                         clipcolumn="",channelavg=False,
                         quackinterval=quackinterval,   # quack only
                         quackmode=quackmode,           # quack only
                         quackincrement=quackincrement, # quack only
                         spw=spw,
                         field=field,
                         antenna=antenna,
                         timerange=timerange,
                         correlation=correlation,
                         scan=scan,
                         feed=feed,
                         array=array,
                         uvrange=uvrange)
            casalog.post('End flagging in quack mode')
        if shadow:
            mode == 'shadow'
            casalog.post('Start flagging using shadow mode')
            fg.setdata()
            fg.setshadowflags( \
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

            if flagbackup:
                backup_flags(mode)
#            fg.run()
            casalog.post('End flagging in shadow mode')
            
        if autoflag:
            mode == 'autoflag'
            casalog.post('Start flagging using autoflag mode')
            fg.setdata(field = field, \
                           spw = spw, \
                           array = array, \
                           feed = feed, \
                           scan = scan, \
                           baseline = antenna, \
                           uvrange = uvrange, \
                           time = timerange, \
                           correlation = correlation)
            rec = fg.getautoflagparams(algorithm=algorithm)
            rec['expr'] = expr
            rec['thr'] = thr
            #rec['rowthr'] = rowthr;
            rec['hw'] = window
            #rec['rowhw'] = rowhw;
            #if( algorithm == 'uvbin' ):
            #     rec['nbins'] = nbins;
            #     rec['minpop'] = minpop;
            rec['column'] = column
            fg.setautoflag(algorithm = algorithm, parameters = rec)
                
            if flagbackup:
                backup_flags(mode)
#            fg.run()
            casalog.post('End flagging in autoflag mode')

        if rfi:
            mode == 'rfi'
            casalog.post('Start flagging using rfi mode')
            fg.setdata(field = field, \
                           spw = spw, \
                           array = array, \
                           feed = feed, \
                           scan = scan, \
                           baseline = antenna, \
                           uvrange = uvrange, \
                           time = timerange, \
                           correlation = correlation)

            # Get the detault parameters for a particular algorithm,
            # then modify them
                
            par = fg.getautoflagparams(algorithm='tfcrop')
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
            fg.setautoflag(algorithm='tfcrop', parameters=par)

            if flagbackup:
                backup_flags(mode)

#            fg.run()
            casalog.post('End flagging in rfi mode')

        if unflag:
            casalog.post('Start flagging using unflag mode')
            clipquack(selectdata, flagbackup,
                         autocorr=autocorr,
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
            casalog.post('End flagging in unflag mode')

        if summary:
            mode == 'summary'
            casalog.post('Start flagging using summary mode')
            fg.setdata()
            fg.setflagsummary(field=field, \
                                  spw=spw, \
                                  array=array, \
                                  feed=feed, \
                                  scan=scan, \
                                  baseline=antenna, \
                                  uvrange=uvrange, \
                                  time=timerange, \
                                  correlation=correlation)
                
            casalog.post('End flagging in summary mode')
            # do not backup existing flags

#        elif query:
#            mode == 'query'
#            print "Sorry - not yet implemented !"
#            fg.done()
#            return False
        
#        elif extend:
#            mode == 'extend'
#            print "Sorry - not yet implemented !"
#            fg.done()
#            return False
            
        # Finallly, run the flagging
        stats = fg.run()
        fg.done()

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
        
        # Add unflag mode here. Remove it from modes above
        

    except Exception, instance:
        fg.done()
        print '*** Error ***', instance
        #raise
    fg.done()

    
    #write history
    ms.open(vis,nomodify=False)
    ms.writehistory(message='taskname = flagdata', origin='flagdata')
    ms.writehistory(message='vis      = "' + str(vis) + '"', origin='flagdata')
    ms.writehistory(message='mode     = "' + str(mode) + '"', origin='flagdata')
    ms.close()

    return

#
# Handle clip=True mode
#
def clipquack(selectdata, flagbackup, **params):
    if debug: print params

    if not selectdata:
        params['antenna'] = params['timerange'] = params['correlation'] = params['scan'] = params['feed'] = params['array'] = params['uvrange'] = ''
    
#    print params.keys()
#    for x in params.keys():
#        print 'x=%s'%x

#    casalog.post('Start flagging using clip mode')

    fg.setdata()
    rename_params(params)
    fg.setmanualflags(**params)

    if flagbackup:
        if params.keys().__contains__('quackmode'):
            backup_flags('quack')
        else:
            backup_flags('clip')
        
#    fg.run()

#
# Handle mode = 'manualflag' and mode = 'quack'
#
def manualflag_quack(mode, selectdata, flagbackup, **params):
        if debug: print params

        if not selectdata:
                params['antenna'] = params['timerange'] = params['correlation'] = params['scan'] = params['feed'] = params['array'] = params['uvrange'] = ''
        
        vector_mode = False         # Are we in vector mode?
        vector_length = -1          # length of all vectors
        vector_var = ''             # reference parameter
        is_vector_spec = {}         # is a variable a vector specification?
        for x in params.keys():
                is_vector_spec[x] = False
                #print x, params[x], type(params[x])
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
                fg.setdata()
                rename_params(params)
                fg.setmanualflags(**params)
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

                fg.setdata()
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
                        fg.setmanualflags(**param_i)

        if flagbackup:
                backup_flags(mode)
        fg.run()

# rename some parameters,
# in order to match the interface of fg.tool
#
# validate parameters for clip quack and unflag modes
def rename_params(params):
                
    if params.has_key('quackmode') and \
      not params['quackmode'] in ['beg', 'endb', 'end', 'tail']:
        raise Exception, "Illegal value '%s' of parameter quackmode, must be either 'beg', 'endb', 'end' or 'tail'" % (params['quackmode'])
    
    params['baseline']        = params['antenna']     ; del params['antenna']
    params['time']            = params['timerange']   ; del params['timerange']
    params['autocorrelation'] = params['autocorr']    ; del params['autocorr']
    if params.has_key('clipminmax'):
        params['cliprange']       = params['clipminmax']  ; del params['clipminmax']
    if params.has_key('clipoutside'):
        params['outside']         = params['clipoutside'] ; del params['clipoutside']

def backup_flags(mode):

    # Create names like this:
    # before_manualflag_1,
    # before_manualflag_2,
    # before_manualflag_3,
    # etc
    #
    # Generally  before_<mode>_<i>, where i is the smallest
    # integer giving a name, which does not already exist
       
    existing = fg.getflagversionlist(printflags=False)

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

        fg.saveflagversion(versionname=versionname,
                           comment='flagdata autosave before ' + mode + ' on ' + time_string,
                           merge='replace')

