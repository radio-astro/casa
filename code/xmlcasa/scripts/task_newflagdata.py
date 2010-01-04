import os
from taskinit import *

def newflagdata(vis=None,setdata=None,mode=None,baseline=None,spw=None,correlation=None,field=None,uvrange=None,selecttime=None,scan=None,feed=None,array=None,setclip=None,setquack=None,autocorr=None,unflag=None,flagcorr=None,flagfield=None,flaguvrange=None,flagtime=None,flagscan=None,flagfeed=None,flagarray=None):
	""" Flag/Clip data based on selections:
	The task will select a subset of data explicitly for
	flagging (unflagging) or for clipping.

	Keyword arguments:
	vis -- Name of input visibility file (MS)
		default: none example: vis='ngc5921.ms'
        setdata -- Create a subset of the MS to operate on.
                default: 'False' : operate on the entire MS
        mode -- Mode of operation.
                options: 'manualflag','autoflag','summary','query','extend','run'
	baseline -- Select data based on baseline
		default: '' (all); example: baseline='5&6' baseline 5-6
		baseline='5&6;7&8' #baseline 5-6 and 7-8
		baseline='5' # all baselines with antenna 5
		baseline='5,6' # all baselines with antennas 5 and 6
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
	uvrange -- Select data based on uvrange(s)
		default: '' (all); example:
		uvrange='0~1000' # uvranges from 0-1000 meters
		uvrange='>4kl' #uvranges greater than 4 kilo lambda
	selecttime  -- Select data based on time range:
	        default = '' (all); example,
		selecttime = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
	 	Note: YYYY/MM/DD can be dropped as needed:
		selecttime='09:14:0~09:54:0' # this time range
		selecttime='09:44:00' # data within one integration of time
		selecttime='>10:24:00' # data after this time
		selecttime='09:44:00+00:13:00' #data 13 minutes after time
	scan -- Select data based on scan number - New, under developement
		default: '' (all); example: scan='>3'
	feed -- Selection based on the feed - NOT IMPLEMENTED YET
        array -- Selection based on the antenna array - NOT IMPLEMENTED YET

        setclip -- Setup Clip operation. Flag OUTSIDE this range
                [clipexpr, clipminmax,outside]
                default: ['ABS I',[],True]
	        clipexpr -- Clip using the following: 
	        	default: 'ABS I'; example: clipexpr='RE XX'
	        	Options: 'ABS','ARG','RE','IM','NORM' + ' ' +  'I','XX','YY','RR','LL'
	        clipminmax -- Range of data (Jy) that will NOT be flagged
	        	default: [] means do not use clip option
	        	example: [0.0,1.5]
                outside -- Clip OUTSIDE the range ?
                        default: True
                        example: False -> flag data WITHIN the range.
        setquack -- [Scan Interval, Quack length] for VLA quack-flagging
                     default: [] means do not use quack option
                     example: [15,2]
        autocorr -- Flag autocorrelations ?
                default: False
                options: True,False
	unflag -- Unflag the data 
		default: False (i.e. flag); example: unflag=True

	"""

	#Python script
        if( baseline == [-1] ):baseline='';

	try: 
                if ((type(vis)==str) & (os.path.exists(vis))):
                        fg.open(vis);
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'
                if ( setdata == True ):
                        fg.setdata(field=field,spw=spw,array=array,feed=feed,scan=scan,baseline=baseline,uvrange=uvrange,time=selecttime,correlation=correlation);
                else:
                        fg.setdata();
                
                if( mode == 'manualflag' ):
                        fg.setmanualflags(field=flagfield,spw=flagspw,array=flagarray,feed=flagfeed,scan=flagscan,baseline=flagbaseline,uvrange=flaguvrange,time=flagtime,correlation=flagcorr,autocorrelation=autocorr,unflag=unflag,clipexpr=setclip[0],cliprange=setclip[1],outside=setclip[2],quackparams=setquack);
                if( mode == 'autoflag' ):
                        print "Sorry - not yet implemented !"
                        #fg.setautoflag(algo=algo,........)
                if( mode == 'summary' ):
                        print "Sorry - not yet implemented !"
                if( mode == 'query' ):
                        print "Sorry - not yet implemented !"
                if( mode == 'extend' ):
                        print "Sorry - not yet implemented !"
                
                fg.run();
                fg.clearmanualflagselection();
                fg.done();

        	#write history
        	ms.open(vis,nomodify=False)
        	ms.writehistory(message='taskname = newflagdata',origin='newflagdata')
        	ms.writehistory(message='vis         = "'+str(vis)+'"',origin='newflagdata')
        	ms.writehistory(message='setdata      = "'+str(setdata)+'"',origin='newflagdata')
        	ms.writehistory(message='mode         = "'+str(mode)+'"',origin='newflagdata')
        	ms.writehistory(message='baseline    = '+str(baseline),origin='newflagdata')
        	ms.writehistory(message='spw    = '+str(spw),origin='newflagdata')
        	ms.writehistory(message='correlation  = '+str(correlation),origin='newflagdata')
        	ms.writehistory(message='field    = '+str(field),origin='newflagdata')
        	ms.writehistory(message='uvrange    = '+str(uvrange),origin='newflagdata')
        	ms.writehistory(message='selecttime    = '+str(selecttime),origin='newflagdata')
        	ms.writehistory(message='scan    = '+str(scan),origin='newflagdata')
        	ms.writehistory(message='feed    = '+str(feed),origin='newflagdata')
        	ms.writehistory(message='array    = '+str(array),origin='newflagdata')
        	ms.writehistory(message='setclip = "'+str(setclip)+'"',origin='newflagdata')
        	ms.writehistory(message='setquack  = '+str(setquack),origin='newflagdata')
        	ms.writehistory(message='autocorr  = '+str(autocorr),origin='newflagdata')
        	ms.writehistory(message='unflag      = '+str(unflag),origin='newflagdata')
        	ms.writehistory(message='flagbaseline    = '+str(flagbaseline),origin='newflagdata')
        	ms.writehistory(message='flagspw    = '+str(flagspw),origin='newflagdata')
        	ms.writehistory(message='flagcorr    = '+str(flagcorr),origin='newflagdata')
        	ms.writehistory(message='flagfield    = '+str(flagfield),origin='newflagdata')
        	ms.writehistory(message='flaguvrange    = '+str(flaguvrange),origin='newflagdata')
        	ms.writehistory(message='flagtime    = '+str(flagtime),origin='newflagdata')
        	ms.writehistory(message='flagscan    = '+str(flagscan),origin='newflagdata')
        	ms.writehistory(message='flagfeed    = '+str(flagfeed),origin='newflagdata')
        	ms.writehistory(message='flagarray    = '+str(flagarray),origin='newflagdata')
        	ms.close()
	
	except Exception, instance:
		print '*** Error ***',instance
