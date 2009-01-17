import os
import time
from taskinit import *

def plotxy(vis=None,xaxis=None,yaxis=None,datacolumn=None,iteration=None,
           selectdata=None,antenna=None,spw=None,field=None,uvrange=None,
           timerange=None,correlation=None,scan=None,feed=None,array=None,
           averagemode=None,width=None,timebin=None,crossscans=None,
           crossbls=None,crossarrays=None,stackspw=None,
           restfreq=None,frame=None,doppler=None,
           extendflag=None,extendcorr=None,extendchan=None,extendspw=None,
           extendant=None,extendtime=None,
           subplot=None,plotsymbol=None,plotcolor=None,
           markersize=None,linewidth=None,
           plotrange=None,multicolor=None,
           selectplot=None,overplot=None,newplot=None,clearpanel=None,
           skipnrows=None,title=None,xlabels=None,ylabels=None,
           fontsize=None,windowsize=None,
           showflags=None,interactive=None,figfile=None):
	"""An X-Y plotter/interactive flagger for visibility data. 

             Selected data can be flexibly plotted with a wide variety of
	     axes options and averaging options.  Data may be listed, flagged or
	     unflagged interactively.  The plots can be zoomed and sent to
	     af file.  Data can be iterated through antennas and fields

             It is advisable to run flagmanager before using plotxy in
	     order to save the current flag state before altering:
             flagmanager(vis='vis_name',mode='save',versionname='before_plotxy',
                   comment=versionname)
	     
     Keyword arguments:
     vis -- Name of input visibility file
             default: none.  example: vis='ngc5921.ms'
     xaxis -- Visibility file data to plot along the x-axis
             default: 'time'

             Options for BOTH x-axis and y-axis
	        'time','uvdist','chan','corr','freq','velo', 
                'u','v','w',
	        'azimuth','elevation','baseline','hourangle','parallacticangle'
                'amp','phase','real','imag','weight'.  'x' gives antenna array
	     
     yaxis -- Visibility data to plot along the y-axis
             default: 'amp'
             Options: same as on xaxis:

     >>> yaxis expandable parameter
         datacolumn -- Visibility file data column
             Note: this parameter is automatically shown whenever the
             chosen yaxis is a data quantity (i.e. amp)
             default: 'data'; example: datacolumn='model'
             Options: 'data' (raw),'corrected','model','residual'(corrected-model)

      --- Data Selection (see help par.selectdata for more detailed information)

     field -- Select field using field id(s) or field name(s).
              ['go listobs' to obtain the list id's or names]
            default: ''=all fields
            If field string is a non-negative integer, it is assumed to
               be a field index otherwise, it is assumed to be a field name
            field='0~2'; field ids 0,1,2
            field='0,4,5~7'; field ids 0,4,5,6,7
            field='3C286,3C295'; field named 3C286 and 3C295
            field = '3,4C*'; field id 3, all names starting with 4C
     spw -- Select spectral window/channels 
             spw='0~2,4'; spectral windows 0,1,2,4 (all channels)
             spw='&lt;2';  spectral windows less than 2 (i.e. 0,1)
             spw='0:5~61'; spw 0, channels 5 to 61, INCLUSIVE
             spw='*:5~61'; all spw with channels 5 to 61
             spw='0,10,3:3~45'; spw 0,10 all channels, spw 3, channels 3 to 45.
             spw='0~2:2~6'; spw 0,1,2 with channels 2 through 6 in each.
             spw='0:0~10;15~60'; spectral window 0 with channels 0-10,15-60
                       NOTE ';' to separate channel selections
             spw='0:0~10^2,1:20~30^5'; spw 0, channels 0,2,4,6,8,10,
                   spw 1, channels 20,25,30
     selectdata -- Other data selection parameters
            default: True
     >>> selectdata expandable parameters
         timerange  -- Select data based on time range:
            default = '' (all); examples,
            timerange = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
            Note: if YYYY/MM/DD is missing date defaults to first day in data set
            timerange='09:14:0~09:54:0' picks 40 min on first day
            timerange= '25:00:00~27:30:00' picks 1 hr to 3 hr 30min on next day
            timerange='09:44:00' data within one integration of time
            timerange='>10:24:00' data after this time
         antenna -- Select data based on antenna/baseline
            default: ''=all
            If antenna string is a non-negative integer, it is assumed
              to be an antenna index, otherwise it is assumed to be an
              antenna name
            antenna='5&6'; baseline between antenna index 5 and index 6.
            antenna='VA05&VA06'; baseline between VLA antenna 5 and 6.
            antenna='5&6;7&8'; baseline 5-6 and 7-8
            antenna='5'; all baselines with antenna index 5
	    antenna= '05'; all baselines with antenna named 05
            antenna='5,6,10'; all baselines with antennas 5,6 and 10
         correlation -- Select correlations:
            default: '' = all correlations. Other options are any combination
	                  of values from one of the valid lists:
			  'RR','LL','RL','LR','RR LL','RL LR',
			  'XX','YY','XY','YX','XX YY','XY YX'
         uvrange -- Select data within uvrange (default units meters)
            default: ''=all; example:
            uvrange='0~1000klambda'; uvrange from 0-1000 kilo-lambda
            uvrange='>4klambda';uvranges greater than 4 kilo-lambda
            uvrange='0~1000km'; uvrange in kilometers
         scan -- Scan number range
         array -- Array number range - under development
         feed -- Multi-feed number range - under development
	 
     averagemode -- Select average mode
            default: 'vector' 
            Options: 'vector' = average over real and image part separately.
                     'scalar' = scalar average of amplitude, vector
                                    average of the phase.
	             ''       = no averaging
     >>> averagemode expandable parameters
         timebin -- length of time intervals to average
              default: '0'= no time averaging
              example: timebin='60' (average over 60 seconds interval).
         crossscans -- Have time averaging cross scan boundaries 
              default: False=time averaging does not cross scan boundaries 
         crossbls -- Have averaging cross baseline boundaries 
              default: False=averaging does not cross baseline boundaries 
         crossarrays -- Have averaging cross array ids 
              default: False=averaging does not cross array ids 
         width -- number of channels to average
              default: '1'= no channel averaging
              example: width='32' (average 32 channels together).
         stackspw -- whether stack the spw on top of each other (for xaxis='chan') 
              default: False=layout spw side by side
     extendflag -- Have flagging extend to other points
         default: '' = no extension, flag only selected data points
     >>> extendflag expandable parameters
         extendcorr  -- Extend flagging based on correlation:
            default: ''= no correlation based extension
            examples:
            extendcorr = 'all'
            extendcorr = 'half'
         extendchan -- Extend flagging based on channel:
            default: ''= no channel based extension
            example:
            extendchan = 'all'
         extendspw -- Extend flagging based on spectral window:
            default: ''= no spectral window based extension
            example:
            extendspw = 'all'
         extendant -- Extend flagging based on antenna (baseline):
            default: ''= no antenna based extension
            example:
            extendant = 'all'
         extendtime -- Extend flagging based on time:
            default: ''= no time based extension
            example:
            extendtime = 'all'
     subplot -- Panel number on the display screen
            default: 111 (full screen display); example:
            if iteration is non-blank, then
               subplot=yx1 window will produce y by x plots in the window.
            if iteration = '', then
               subplot=yxn; means the window will have y rows, and x columns
               with the present execution placing the plot in location n
               n = 1,..., xy, in order upper left to right, then down.  An
               example is the plotting of four fields on one frame
               field='0'; subplot=221; plotxy()
               field='1'; subplot=222; plotxy()
               field='2'; subplot=223; plotxy()
               field='3'; subplot=224; plotxy()
     overplot -- Overplot these values on current plot (if possible)
            default: False; example: overplot= True
     showflags -- Show the flagged and unflagged data.
            default: False; example: showflags=True
     iteration -- Iterate plots:
            default: ''; no iteration
            Options: 'field', 'antenna', 'baseline'.
            Use the 'NEXT' button on gui to iterate through values.
            To abort an iteration, close the gui window.
     plotsymbol -- plot symbol. (One of the .:,o^v<>s+xDd234hH|_ or
                optionally preceded by one of color code k, r, g, b, c, y)
            default: '.': points, medium-sized dots
            example: plotsymbol='bo' (blue points, large-sized dots
              r=red, b=blue, g=green, y=yellow
              ',' = small points
              '.' = medium points
              'o' = large points
     plotcolor -- pylab color.  Overrides plotsymbol color settings.
            default: 'darkcyan'
            example: plotcolor='g' (green)
                     plotcolor='slateblue'
                     plotcolor='#7FFF34' (RGB tuple)
     multicolor -- Multi-color plotting of channels and correlations
             options: 'none','both','chan','corr'
             default: 'corr'; example: multicolor='chan'
     plotrange -- Specifies the size of the plot [xmin, xmax, ymin, ymax]
            default: [0,0,0,0];  Range is self-selected
	    example: [0,0,0.00,0.20]  only y axis is specified 
            To use a time range (NOTE: somewhat different sytax from timerange
	       ['22:23:24, 23:42:20', 0.02, 0.20]
	       The day must be included if the visibility data span more than one day
	       ['1997/05/09/22:02:00, 1997/05/09/23:00:00', 0.02, 0.20]
     selectplot -- Additional plot control parameters
            default: False; example: selectplot=true
     >>> selectplot expandable parameters
         markersize -- Size of the plotted marks
            default: 5.0; example: markersize=10.0
         linewidth -- Width of plotted lines.
            default: 1; example: linewidth=2.0
         skipnrows -- # of points to skip
            default: 0
            example: skipnrows=100 (plots every 100th point)
         newplot -- When overplotting, replace the last plot only
            default: False; can be useful when doing subplots
	 clearpanel -- Clear nothing on the plot window, automatically
	          clear plotting area, clear the current plot area, or
	          clear the whole plot panel.
	       options: None, Auto, Current, All
	       default: Auto
	       example: clearpanel='Current'
	 title -- Plot title (above plot)
            default: ''; example: title='This is my title'
         xlabels -- Label for x axis
            default: ''; example: xlabels='X Axis'
         ylabels -- Label for y axis
            default: ''; example: ylabels='Y Axis'
         fontsize -- Font size for labels
            default: 10; example: fontsize=2
         windowsize -- Window size
            default: 1.0; not yet implemented
         interactive -- turn on/off on screen display
            options: True = interactively flag data
                     False = direct output to a file without on screen display
            default: True;
	 figfile -- File name to save the plotted figure to.
	    default: ''; example figfile=myPlot.png

	"""

        casalog.origin('plotxy')

	startTime=time.time()
	startProc=time.clock()

	#Python script
	#parameter_printvalues(arg_names,arg_values,arg_types)
	try:
		# Turn the display of the plot GUI on or off
		tp.setgui( interactive );

                if (xaxis.lower()=='velocity' or xaxis.lower()=='vel'):
                   xaxis='velo'
                if (yaxis.lower()=='velocity' or yaxis.lower()=='vel'):
                   yaxis='velo'

                doVel=False
                if (xaxis.lower()=='velo' or xaxis.lower()=='velo'):
                   doVel=True

                if ((type(vis)==str) & (os.path.exists(vis))):

			casalog.post('Adding scratch columns, if necessary.')
			cb.open(vis)
			cb.close(vis)

                        mp.open(vis, doVel, restfreq, frame, doppler)
                else:
                        raise Exception, 'Visibility data set not found - please verify the name'

		# If the user wants the plot window cleared then clear it.
		removeoldplots=False
		if ( clearpanel.lower()=='current' ) :
			mp.clearplot( subplot=subplot );
		elif ( clearpanel.lower()=='all' ) :
			mp.clearplot();
		elif ( clearpanel.lower()=='auto' ) :
			removeoldplots=True

                #print 'xaxis=', xaxis
                if (xaxis.lower()=='frequency'):
                   xaxis='freq'
                if (yaxis.lower()=='frequency'):
                   yaxis='freq'

                

                if (xaxis.lower()=='channel'):
                   xaxis='chan'
                if (yaxis.lower()=='channel'):
                   yaxis='chan'

                if (multicolor.lower()=='channel'):
                   multicolor='chan'
                if (multicolor.lower()=='correlation'):
                   multicolor='corr'

                #if (connect.lower()=='channel'):
                #   connect='chan'

                timeaxis= ['azimuth','elevation','baseline','hourangle',
                           'parallacticangle','time']
                xtime=False
                ytime=False
                for t in range(len(timeaxis)):
                   if (xaxis.lower()==timeaxis[t]):
                      xtime=True
                   if (yaxis.lower()==timeaxis[t]):
                      ytime=True
                if (xtime and ytime): 
                   raise Exception, 'Sorry. it cannot plot '+yaxis+' vs. '+xaxis
			
                #if (xaxis=='chan' and connect=='chan'):
                #   connect='none'
                #if (xaxis=='time' and connect=='time'):
                #   connect='none'


		# Handle the plotrange option making sure each value it
		# a string value.  Note that msplot_cmpt expect a single
		# string of the form: 'xmin,xmax,ymin,ymax'  where each
		# of the values can be a double or a time string.
		plotrange_l=''
		if ( isinstance( plotrange, list ) ) :
			for j in range( 0, len( plotrange )  ) :
				if ( j > 0 ) :
					plotrange_l = plotrange_l + ",";
				plotrange_l = plotrange_l + str( plotrange[j] );
		else:
			plotrange_l = plotrange;

                #print "INFO2 extendflag="+str(extendflag)+" corr="+ \
                #      extendcorr+" chan="+extendchan+" spw="+ \
                #      extendspw+" ant="+extendant+" time="+ \
                #      extendtime
                #   print "INFO2 Extended Flagging Selected"

                flagextend=''
                #do extendflag in the TablePlot
                #if (extendcorr.lower().find('all') >= 0):
                #   flagextend += 'P'
                #if (extendchan.lower().find('all') >= 0):
                #   flagextend += 'C'

                #do extendflag in the MsPlot
                if (extendflag):
                   flagextend='True'

                #connect='none'
		ok=mp.plotoptions(subplot=subplot,plotsymbol=plotsymbol,
                    plotcolor=plotcolor,multicolor=multicolor,
                    markersize=markersize,linewidth=linewidth,
                    #connect=connect,
                    overplot=overplot,replacetopplot=newplot,
                    removeoldpanels=removeoldplots,showflags=showflags,
                    title=title,xlabel=xlabels,ylabel=ylabels,
                    fontsize=fontsize,windowsize=windowsize,
                    plotrange=plotrange_l,timeplot=True,skipnrows=skipnrows,
                    extendflag=flagextend)
                #print "=====plotoptions"
                # check if plot option was okay
                if (not ok):
	           noop='noop'
		   # Returns false when it shouldn't so for now do
		   # nothing
                   #raise Exception, 'An error occured while setting the plot options'
                if (timebin.lower()=='all'):
                   timebin='8640000'
                if (width.lower()=='all'):
                   width='32768'
                if (width.lower()=='allspw'):
                   width='1234567'

                try:
                    nChan=int(width)
                except:
                    width='1'
                    print "Found invalid width value, replaced with width='1'"

                try: 
                    fTime=float(timebin)
                except:
                    timebin='0'
                    print "Found invalid timebin value, replaced with timebin='0'"

                if (averagemode=='scalar'): 
                    if (not (xaxis=='amp' or yaxis=='amp')):
                        averagemode='vector'
                        
                #print width
                #print timebin
                #print averagemode

                #if (timebin=='0' and width=='1'):
                average=''


                #kludge to map average into msplot averaging mode parameters.
                chanavemode='none'
                corravemode='none'
                if (average=='chanandcorr' ):
                    chanavemode='vectorstep';
                    corravemode='vectorstep';
                elif (average=='both'):
                    chanavemode='vectorchunk';
                    timeavemode='scalarchunk';
                elif (average=='correlation'):
                    chanavemode='vectorstep';
                elif (average!='' and average!='none' ):
                    raise Exception, 'Unrecognized average selection'

                if (averagemode==''):
                    averagemode='vector'
                
                ok=mp.avedata(chanavemode=chanavemode,
                              corravemode=corravemode,
                              datacolumn=datacolumn,
                              averagemode=averagemode,
                              averagechan=width,
                              averagetime=timebin,
                              averageflagged=showflags,
                              averagescan=crossscans,
                              averagebl=crossbls,
                              averagearray=crossarrays,
                              averagechanid=stackspw,
                              averagevel=doVel)
                # check if averaging was okay
                if (not ok):
                    raise Exception, 'Failure occured when averaging data!'

                #print "scan=", scan, "uvrange=", uvrange
                ok=mp.setdata(baseline=antenna,field=field,scan=scan,
			      uvrange=uvrange,array=array,feed=feed,
			      spw=spw,correlation=correlation,
			      time=timerange)
                # check if data selection was okay
                if (not ok):
                   raise Exception, 'Data selection resulted in no data!'

                ok=mp.extendflag(extendcorr=extendcorr,extendchan=extendchan,
                              extendspw=extendspw,extendant=extendant,
                              extendtime=extendtime)
                # check if flag extension was okay
                if (not ok):
                   raise Exception, 'Fail to set flag extension!'

		
		if (xaxis=='real' or xaxis=='imag' or
                    xaxis=='phase' or xaxis=='amp'):
			# data vs data plot
			#print 'data vs. data plot ************** '
			if ((yaxis=='amp') | (yaxis=='phase') | 
                            (yaxis=='real') | (yaxis=='imag') ):
                                ok=mp.plotxy(x='data',y='data',
                                      xcolumn=datacolumn,ycolumn=datacolumn,
                                      xvalue=xaxis,yvalue=yaxis,
                                      iteration=iteration)
                                if (not ok):
                                        raise Exception, 'No data selected - please check inputs'
			elif (yaxis=='weight'):
				ok=mp.plotxy(x='data',y=yaxis,
                                      xcolumn=datacolumn,ycolumn=datacolumn,
                                      xvalue=xaxis,yvalue='amp',
                                      iteration=iteration)
			elif (xaxis=='weight'):
				ok=mp.plotxy(x='weight',y='data',
                                      xcolumn=datacolumn,ycolumn=datacolumn,
                                      xvalue='amp',yvalue=yaxis,
                                      iteration=iteration)
                        else:
				raise Exception, 'Must be data versus data plot - illegal yaxis'

		elif ((yaxis=='real' or yaxis=='imag' or 
                       yaxis=='phase' or yaxis=='amp' or yaxis=='weight') and 
                      (xaxis!='x' and xaxis!='u')):
			# something vs data plot
			#print 'something vs data plot ************ '
			if (yaxis=='weight'):
				ok=mp.plotxy(xaxis,yaxis)
			if (xaxis=='chan'): 
                                xaxis='vischannel'
			if (xaxis=='corr' or xaxis=='correlation'): 
                                xaxis='viscorr'
			if (xaxis=='freq'): 
                                xaxis='visfreq'
                        if (xaxis=='velo'):
                                xaxis='visvelocity'
			if (xaxis=='time'): 
                                xaxis='vistime'
			if (yaxis!='weight' and (xaxis!='v' and xaxis!='w')):
				ok=mp.plot(xaxis,datacolumn,yaxis,iteration)
			elif (xaxis=='v' or xaxis=='w'):
				ok=mp.plotxy(x=xaxis,y='data',
                                     xcolumn=datacolumn,ycolumn=datacolumn,
                                     xvalue='amp',yvalue=yaxis,
                                     iteration=iteration)
			if (not ok):
				raise Exception, 'No data selected - please check inputs'
		
		else:
			# something vs something plot
			#print 'something vs. something plot ************ '
			if (xaxis=='chan' or yaxis=='chan'): 
				raise Exception, 'Channel can not be selected versus non-data values'
			if (xaxis=='freq' or yaxis=='freq'): 
				raise Exception, 'Frequency can not be selected versus non-data values'
			if (xaxis=='velo' or yaxis=='velo'): 
				raise Exception, 'Velocity can not be selected versus non-data values'
			if (yaxis=='time'):
				raise Exception, 'yaxis=time is not currently supported'
			if (xaxis=='u'): 
                                ok=mp.plot('uvcoverage')
			if (xaxis=='x'): 
                                ok=mp.plot('array')
			if (xaxis!='x' and xaxis!='u'): 
				#print 'getting to the right spot'
				ok=mp.plotxy(x=xaxis,y=yaxis)
			if (not ok):
				raise Exception, 'No data selected - please check inputs'
		if ( len(iteration) > 0 ):
			mp.iterplotnext()

		if ( len(figfile) > 0 ) :
			mp.savefig( figfile ); 

	except Exception, instance:
		print 'INFO ', instance

	# Close the MS to avoid table locks.  This doesn't close the
	# plot window but frees the MS to be used by other CASA tools
	# And will prevent users from manipulating the
	#if ( not interactive ) :
	#	mp.done();
	#else :
	mp.closeMS();
	
	endProc=time.clock()
	endTime=time.time()
	print 'Total process time %.2f sec.' % (endProc - startProc)
	print 'Total wall clock time %.2f sec.' % (endTime - startTime)


