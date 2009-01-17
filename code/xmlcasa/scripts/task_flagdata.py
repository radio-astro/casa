import os
from taskinit import *

def flagdata(vis=None,mode=None,antenna=None,spw=None,correlation=None,field=None,uvrange=None,timerange=None,scan=None,feed=None,array=None,clipexpr=None,clipminmax=None,clipcolumn=None,clipoutside=None,quackinterval=None,autocorr=None,unflag=None,algorithm=None,column=None,expr=None,thr=None,window=None):
        """ All purpose flagging task based on selections:

        The task will select a subset of data explicitly for flagging,
        quacking, clipping, and autocorrelation flagging. Unflagging is
        also available.

	In a dual polarization data set, each polarization can be flagged
	separately.  However, at present the calibration and imaging will
	only use data with both parallel hands unflagged.

        Keyword arguments:
        vis -- Name of input visibility file
                default: none example: vis='ngc5921.ms'
        antenna -- Select data based on baseline
                default: '' (all); example: antenna='5&6' baseline 5-6
                antenna='5&6;7&8' #baseline 5-6 and 7-8
                antenna='5' # all baselines with antenna 5
                antenna='5,6' # all baselines with antennas 5 and 6
        spw -- Select data based on spectral window and channels
                default: '' (all); example: spw='1'
                spw='<2' #spectral windows less than 2
                spw='>1' #spectral windows greater than 1
                spw='0:0~10' # first 10 channels from spw 0
                spw='0:0~5;56~60' # multiple separated channel chunks.
        correlation -- Correlation types
                default: '' (all);
                example: correlation='RR,LL'
        field -- Select data based on field id(s) or name(s)
                default: '' (all); example: field='1'
                field='0~2' # field ids inclusive from 0 to 2
                field='3C*' # all field names starting with 3C
        uvrange -- Select data within uvrange (default units meters)
                default: '' (all); example:
                uvrange='0~1000kl'; uvrange from 0-1000 kilo-lamgda
                uvrange='>4kl';uvranges greater than 4 kilo-lambda
                uvrange='0~1000km'; uvrange in kilometers
        timerange  -- Select data based on time range:
                default = '' (all); example,
                timerange = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
                Note: YYYY/MM/DD can be dropped as needed:
                timerange='09:14:0~09:54:0' # this time range
                timerange='09:44:00' # data within one integration of time
                timerange='>10:24:00' # data after this time
                timerange='09:44:00+00:13:00' #data 13 minutes after time
        scan -- Select data based on scan number
                default: '' (all); example: scan='>3'
        feed -- Selection based on the feed - NOT IMPLEMENTED YET
        array -- Selection based on the antenna array
        mode -- Mode of operation.
                options: 'manualflag','autoflag','summary','quack'

        --- MANUALFLAG option does data-selected flagging, autocorrelation
	      flagging and/or clipping.

	   Direct flagging of data will occur if
	             autocorr = F; unflag = F; clipminmax = []
		     
	   Other flagging options can be used with:
        autocorr -- Flag autocorrelations 
                default: False
                options: True,False
        unflag -- Unflag the data
                default: False (i.e. flag); example: unflag=True
        clipexpr -- Clip using the following:
                default: 'ABS RR'; example: clipexpr='RE XX'
                Options: 'ABS','ARG','RE','IM','NORM' + ' ' +
		          'I','XX','YY','RR','LL'
        clipminmax -- Range of data (Jy) that will NOT be flagged
                default: [] means do not use clip option
                example: [0.0,1.5]
        clipcolumn -- Column to use for clipping.
                default: 'DATA'
                options: 'DATA','CORRECTED','MODEL'
        clipoutside -- Clip OUTSIDE the range ?
                default: True
                example: False -> flag data WITHIN the range.

        --- QUACK option removes specified part of scan beginning
        quackinterval -- Time in seconds from scan beginning to flag
                Make time slightly small than desired time
        unflag -- Unflag the data
                default => False (quack as indicated)
                           True (unquack)
        autocorr -- Flag autocorrelations (independent option)
                default => False

        -- SUMMARY option lists number of rows and data points flagged

        No subparameters

        -- AUTOFLAG option runs an automatic program for removing outliers.
	            It is still under development and not recommended.
        algorithm -- autoflag algorithm name
                default: 'timemed'
                options: 'timemed','freqmed'
        column -- the column on which to operate (DATA, CORRECTED, MODEL)
        expr -- expression to use for flagging option
                default: 'ABS RR'; example: expr='RE XX'
                Options: 'ABS','ARG','RE','IM','NORM' + ' ' +
		     'I','XX','YY','RR','LL'

        thr -- flagging threshold as a multiple of standard-deviation ( n sigma )
        window -- half width for sliding window median filters.  The size should be
	          about the number of integration in a scan, or the number of
		  spectral channels.

	"""

	#Python script

	fg.clearflagselection(0)
	
        if( antenna == [-1] ):antenna='';

	try: 
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fg.open(vis);
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
                
                torun=True;
                if( mode == 'manualflag' or mode == 'quack' ):
                        fg.setdata();
                        # for a default of expr = 'ABS RR LL'
                        # need to call setmanualflags multiple times here.
                        fg.setmanualflags(field=field,spw=spw,array=array,feed=feed,scan=scan,baseline=antenna,uvrange=uvrange,time=timerange,correlation=correlation,autocorrelation=autocorr,unflag=unflag,clipexpr=clipexpr,cliprange=clipminmax,clipcolumn=clipcolumn,outside=clipoutside,quackinterval=quackinterval);
                        fg.setflagsummary();
                if( mode == 'autoflag' ):
                        fg.setdata(field=field,spw=spw,array=array,feed=feed,scan=scan,baseline=antenna,uvrange=uvrange,time=timerange,correlation=correlation);
                        rec = fg.getautoflagparams(algorithm=algorithm);
                        rec['expr'] = expr;
                        rec['thr'] = thr;
                        #rec['rowthr'] = rowthr;
                        rec['hw'] = window;
                        #rec['rowhw'] = rowhw;
                        #if( algorithm == 'uvbin' ):
                        #     rec['nbins'] = nbins;
                        #     rec['minpop'] = minpop;
                        fg.setautoflag(algorithm=algorithm,parameters=rec);
                        fg.setflagsummary();
                if( mode == 'summary' ):
                        fg.setdata();
                        fg.setflagsummary(field=field,spw=spw,array=array,feed=feed,scan=scan,baseline=antenna,uvrange=uvrange,time=timerange,correlation=correlation);
                if( mode == 'query' ):
                        print "Sorry - not yet implemented !"
			torun = False;
                if( mode == 'extend' ):
                        print "Sorry - not yet implemented !"
			torun = False;
                if( torun == True ) :
                        fg.run();
                        fg.done();
                else :
                        return False;

        	#write history
        	ms.open(vis,nomodify=False)
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
		print '*** Error ***',instance
