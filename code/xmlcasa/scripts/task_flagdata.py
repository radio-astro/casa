import os
import sys
from taskinit import *

pathname = os.environ.get('CASAPATH').split()[0]
arch     = os.environ.get('CASAPATH').split()[1]
if pathname.find('lib') >= 0:
        filepath = pathname+'/lib/python2.5/heuristics/'
else:
   if pathname.find('Contents') >= 0 :
        filepath = pathname+'/Resources/python/heuristics/'
   else :
        filepath = pathname+'/'+arch+'/python/2.5/heuristics/'

sys.path.append(filepath)

import sfiReducer
import baseFlagger
import htmlLogger

debug = False
def flagdata(vis = None, mode = None,
             spw = None, field = None,
             selectdata = None,
             antenna = None,
             uvrange = None,
             timerange = None,
             correlation = None,
             scan = None,
             feed = None, array = None,
             clipexpr = None, clipminmax = None,
             clipcolumn = None, clipoutside = None,
             quackinterval = None, quackmode = None, quackincrement = None,
             autocorr = None,
             unflag = None, algorithm = None,
             recipe = None,
             column = None, expr = None,
             thr = None, window = None,
             diameter = None,
             source = None,
             flux = None,
             bpass = None,
             gain = None,
             time_amp_cutoff = None,
             freq_amp_cutoff = None,
             freqlinefit = None,
             auto_cross = None,
             num_time = None,
             start_chan = None,
             end_chan = None,
             bs_cutoff = None,
             ant_cutoff = None,
             showplots = None,
             flag_level = None):

        casalog.origin('flagdata')

        if mode == 'alma':
                #import inspect

                #print "now at ", inspect.currentframe().f_lineno, inspect.currentframe().f_code.co_filename

                sfir = sfiReducer.SFIReducer(vis, recipe=recipe)
                casalog.post('Invoking ' + str(recipe))
                casalog.post('Source field(s)        = ' + str(source))
                casalog.post('Flux     calibrator(s) = ' + str(flux))
                casalog.post('Gain     calibrator(s) = ' + str(gain))
                casalog.post('Bandpass calibrator(s) = ' + str(bpass))
                #casalog.post('A log of the run is available in ./html/AAAROOT.html')
                #print "now at ", inspect.currentframe().f_lineno, inspect.currentframe().f_code.co_filename
                
                sfir.reduce(source=source, flux=flux, gain=gain, bandpass=bpass)
                
                casalog.post(str(recipe) + ' done')
                casalog.post('A log of the run is available in ./html/AAAROOT.html')
                
                return False

        fg.done()
        fg.clearflagselection(0)
        
        try: 
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fg.open(vis)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'


                if mode == 'manualflag':
                        # In manualflag and quack modes,
                        # filter out the parameters which are not used

                        manualflag_quack(mode, selectdata,
                                         autocorr=autocorr,
                                         unflag=unflag,
                                         clipexpr=clipexpr,       # manualflag only
                                         clipminmax=clipminmax,   # manualflag only
                                         clipcolumn=clipcolumn,   # manualflag only
                                         clipoutside=clipoutside, # manualflag only
                                         spw=spw,
                                         field=field,
                                         antenna=antenna,
                                         timerange=timerange,
                                         correlation=correlation,
                                         scan=scan,
                                         feed=feed,
                                         array=array,
                                         uvrange=uvrange)
                elif mode == 'quack':
                        manualflag_quack(mode, selectdata,
                                         autocorr=autocorr,
                                         unflag=unflag,
                                         clipminmax=[], clipoutside=False,
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
                elif ( mode == 'shadow' ):
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
                        fg.run()
                elif ( mode == 'autoflag' ):
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
                        fg.setautoflag(algorithm = algorithm,
                                       parameters = rec)
                        fg.run()

                elif mode == 'rfi':
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
                        par['showplots']=showplots
                        
                        ## channel range (1 based)
                        par['start_chan']=start_chan 
                        par['end_chan']=end_chan
                        
                        ## number of time-steps in each chunk
                        par['num_time']=num_time

                        ## flag on cross-correlations and auto-correlations. (0 : only autocorrelations)
                        par['auto_cross']= auto_cross   
                        
                        ## Flag Level :
                        ## 0: flag only what is found. 
                        ## 1: extend flags one timestep before and after
                        ## 2: 1 and extend flags one channel before/after.
                        par['flag_level']=flag_level

                        ## data expression on which to flag.
                        par['expr']=clipexpr

                        ## data column to use.
                        par['column']=clipcolumn

                        ## False : Fit the bandpass with a piecewise polynomial
                        ## True : Fit the bandpass with a straight line.
                        par['freqlinefit']=freqlinefit

                        ## Flagging thresholds ( N sigma ), where 'sigma' is the stdev of the "fit".
                        #par['freq_amp_cutoff']=3
                        #par['time_amp_cutoff']=4
                        par['freq_amp_cutoff']=freq_amp_cutoff
                        par['time_amp_cutoff']=time_amp_cutoff
                        
                        # Tell the 'fg' tool which algorithm to use, and set the parameters.
                        # Note : Can set multiple instances of this (will be done one after the other)
                        #
                        fg.setautoflag(algorithm='tfcrop', parameters=par)

                        fg.run()

                elif ( mode == 'summary' ):
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
                        #tosave = False
                        stats = fg.run()
                        fg.done()
                        return stats
                elif ( mode == 'query' ):
                        print "Sorry - not yet implemented !"
                        fg.done()
                        return False
                elif ( mode == 'extend' ):
                        print "Sorry - not yet implemented !"
                        fg.done()
                        return False
                #if tosave:
                #fg.saveflagversion(versionname=versionname,comment='flagdata autosave',merge='replace')

                #write history
                ms.open(vis, nomodify=False)
                ms.writehistory(message='taskname = flagdata',origin='flagdata')
                ms.writehistory(message='vis         = "'+str(vis)+'"',origin='flagdata')
                ms.writehistory(message='mode         = "'+str(mode)+'"',origin='flagdata')
                ms.writehistory(message='antenna    = '+str(antenna),origin='flagdata')
                ms.writehistory(message='spw    = '+str(spw),origin='flagdata')
                ms.writehistory(message='correlation  = '+str(correlation),origin='flagdata')
                ms.writehistory(message='field    = '+str(field),origin='flagdata')
                ms.writehistory(message='uvrange    = '+str(uvrange),origin='flagdata')
                ms.writehistory(message='timerange    = '+str(timerange),origin='flagdata')
                ms.writehistory(message='scan    = '+str(scan),origin='flagdata')
                ms.writehistory(message='feed    = '+str(feed),origin='flagdata')
                ms.writehistory(message='array    = '+str(array),origin='flagdata')
                ms.writehistory(message='clipexpr = "'+str(clipexpr)+'"',origin='flagdata')
                ms.writehistory(message='clipminmax = "'+str(clipminmax)+'"',origin='flagdata')
                ms.writehistory(message='clipcolumn = "'+str(clipcolumn)+'"',origin='flagdata')
                ms.writehistory(message='clipoutside = "'+str(clipoutside)+'"',origin='flagdata')
                ms.writehistory(message='quackinterval  = '+str(quackinterval),origin='flagdata')
                ms.writehistory(message='autocorr  = '+str(autocorr),origin='flagdata')
                ms.writehistory(message='unflag      = '+str(unflag),origin='flagdata')
                ms.writehistory(message='algorithm      = '+str(algorithm),origin='flagdata')
                ms.writehistory(message='column      = '+str(column),origin='flagdata')
                ms.writehistory(message='expr      = '+str(expr),origin='flagdata')
                ms.writehistory(message='thr      = '+str(thr),origin='flagdata')
                ms.writehistory(message='window      = '+str(window),origin='flagdata')
                ms.close()
        
        except Exception, instance:
                fg.done()
                print '*** Error ***', instance
                #raise
        fg.done()


#
# Handle mode = 'manualflag' and mode = 'quack'
#
def manualflag_quack(mode, selectdata, **params):
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
#                                 fg.setmanualflags(field=params['field'][i], \
#                                                   spw=params['spw'][i], \
#                                                   array=params['array'][i], \
#                                                   feed=params['feed'][i], \
#                                                   scan=params['scan'][i], \
#                                                   baseline=params['antenna'][i], \
#                                                   uvrange=params['uvrange'][i], \
#                                                   time=params['timerange'][i], \
#                                                   correlation=params['correlation'][i], \
#                                                   autocorrelation=params['autocorr'][i], \
#                                                   unflag=params['unflag'][i], \
#                                                   clipexpr=params['clipexpr'][i], \
#                                                   cliprange=params['clipminmax'][i], \
#                                                   clipcolumn=params['clipcolumn'][i], \
#                                                   outside=params['clipoutside'][i])
#                                                   quackinterval=quackinterval[i])
        fg.run()



# rename some parameters,
# in order to match the interface of fg.tool
#
# validate parameter quackmode

def rename_params(params):
        if params.has_key('quackmode') and \
          not params['quackmode'] in ['beg', 'endb', 'end', 'tail']:
                raise Exception, "Illegal value '%s' of parameter quackmode, must be either 'beg', 'endb', 'end' or 'tail'" % (params['quackmode'])
        
        params['baseline']        = params['antenna']     ; del params['antenna']
        params['time']            = params['timerange']   ; del params['timerange']
        params['autocorrelation'] = params['autocorr']    ; del params['autocorr']
        params['cliprange']       = params['clipminmax']  ; del params['clipminmax']
        params['outside']         = params['clipoutside'] ; del params['clipoutside']
