import os
from taskinit import *

import asap as sd
import pylab as pl
import Tkinter as Tk

def sdplot(sdfile, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, scanaverage, timeaverage, tweight, polaverage, pweight, kernel, kwidth, plottype, stack, panel, flrange, sprange, linecat, linedop, colormap, linestyles, linewidth, histogram, plotfile, overwrite):

        casalog.origin('sdplot')

        ###
        ### Now the actual task code
        ###
        try:
            if sdfile=='':
                raise Exception, 'infile is undefined'

            filename = os.path.expandvars(sdfile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s
            if not overwrite and not plotfile=='':
                plotfilename = os.path.expandvars(plotfile)
                plotfilename = os.path.expanduser(plotfilename)
                if os.path.exists(plotfilename):
                    s = "Output file '%s' exist." % (plotfilename)
                    raise Exception, s

            isScantable=False
            if os.path.isdir(filename) and \
            (not os.path.exists(filename+'/table.f1') and \
             os.path.exists(filename+'/table.info')):
                isScantable=True

            #load the data without averaging
            s=sd.scantable(sdfile,scanaverage)

            # get telescope name
            #'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
            antennaname = s.get_antennaname()

            # determine current fluxunit
            fluxunit_now = s.get_fluxunit()
            if ( antennaname == 'GBT'):
                            if (fluxunit_now == ''):
                                    print "no fluxunit in the data. Set to Kelvin."
                                    s.set_fluxunit('K')
                                    fluxunit_now = s.get_fluxunit()
            print "Current fluxunit = "+fluxunit_now

            # set flux unit string (be more permissive than ASAP)
            if ( fluxunit == 'k' ):
                    fluxunit = 'K'
            elif ( fluxunit == 'JY' or fluxunit == 'jy' ):
                    fluxunit = 'Jy'

            # fix the fluxunit if necessary
            if ( telescopeparm == 'FIX' or telescopeparm == 'fix' ):
                            if ( fluxunit != '' ):
                                    if ( fluxunit == fluxunit_now ):
                                            print "No need to change default fluxunits"
                                    else:
                                            s.set_fluxunit(fluxunit)
                                            print "Reset default fluxunit to "+fluxunit
                                            fluxunit_now = s.get_fluxunit()
                            else:
                                    print "Warning - no fluxunit for set_fluxunit"


            elif ( fluxunit=='' or fluxunit==fluxunit_now ):
                    if ( fluxunit==fluxunit_now ):
                            print "No need to convert fluxunits"

            elif ( type(telescopeparm) == list ):
                    # User input telescope params
                    if ( len(telescopeparm) > 1 ):
                            D = telescopeparm[0]
                            eta = telescopeparm[1]
                            print "Use phys.diam D = %5.1f m" % (D)
                            print "Use ap.eff. eta = %5.3f " % (eta)
                            s.convert_flux(eta=eta,d=D)
                    elif ( len(telescopeparm) > 0 ):
                            jypk = telescopeparm[0]
                            print "Use gain = %6.4f Jy/K " % (jypk)
                            s.convert_flux(jyperk=jypk)
                    else:
                            print "Empty telescope list"

            elif ( telescopeparm=='' ):
                    if ( antennaname == 'GBT'):
                            # needs eventually to be in ASAP source code
                            print "Convert fluxunit to "+fluxunit
                            # THIS IS THE CHEESY PART
                            # Calculate ap.eff eta at rest freq
                            # Use Ruze law
                            #   eta=eta_0*exp(-(4pi*eps/lambda)**2)
                            # with
                            print "Using GBT parameters"
                            eps = 0.390  # mm
                            eta_0 = 0.71 # at infinite wavelength
                            # Ideally would use a freq in center of
                            # band, but rest freq is what I have
                            rf = s.get_restfreqs()[0][0]*1.0e-9 # GHz
                            eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
                            print "Calculated ap.eff. eta = %5.3f " % (eta)
                            print "At rest frequency %5.3f GHz" % (rf)
                            D = 104.9 # 100m x 110m
                            print "Assume phys.diam D = %5.1f m" % (D)
                            s.convert_flux(eta=eta,d=D)

                            print "Successfully converted fluxunit to "+fluxunit
                    elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
                            s.convert_flux()

                    else:
                            # Unknown telescope type
                            print "Unknown telescope - cannot convert"


            # set spectral axis unit
            if ( specunit != '' ):
                    print "Changing spectral axis to "+specunit
                    s.set_unit(specunit)

            # set rest frequency
            if ( specunit == 'km/s' and restfreq != '' ):
                    if ( type(restfreq) == float ):
                            fval = restfreq
                    else:
                            # string with/without unit
                            rf=restfreq.rstrip('Hz')
                            if ( rf[len(rf)-1] == 'T' ):
                                    #THz
                                    fval = float(rf.rstrip('T'))*1.0e12
                            elif ( rf[len(rf)-1] == 'G' ):
                                    #GHz
                                    fval = float(rf.rstrip('G'))*1.0e9
                            elif ( rf[len(rf)-1] == 'M' ):
                                    #MHz
                                    fval = float(rf.rstrip('M'))*1.0e6
                            elif ( rf[len(rf)-1] == 'k' ):
                                    #kHz
                                    fval = float(rf.rstrip('k'))*1.0e3
                            else:
                                    #Hz
                                    fval = float(rf)
                    print 'Set rest frequency to ', fval, ' Hz'
                    s.set_restfreqs(freqs=fval)

            # reset frame and doppler if needed
            if ( frame != '' ):
                    print "Changing frequency frame to "+frame
                    s.set_freqframe(frame)
            else:
                    print 'Using current frequency frame'

            if ( doppler != '' ):
                    if ( doppler == 'radio' ):
                            ddoppler = 'RADIO'
                    elif ( doppler == 'optical' ):
                            ddoppler = 'OPTICAL'
                    elif ( doppler == 'z' ):
                            ddoppler = 'Z'
                    else:
                            ddoppler = doppler

                    s.set_doppler(ddoppler)
            else:
                    print 'Using current doppler convention'

            # Prepare a selection
            sel=sd.selector()

            # Scan selection
            if ( type(scanlist) == list ):
                    # is a list
                    scans = scanlist
            else:
                    # is a single int, make into list
                    scans = [ scanlist ]
            if ( len(scans) > 0 ):
                    sel.set_scans(scans)

            # Select source names
            if ( field != '' ):
                    sel.set_name(field)
                    # NOTE: currently can only select one
                    # set of names this way, will probably
                    # need to do a set_query eventually

            # IF selection
            if ( type(iflist) == list ):
                    # is a list
                    ifs = iflist
            else:
                    # is a single int, make into list
                    ifs = [ iflist ]
            if ( len(ifs) > 0 ):
                    # Do any IF selection
                    sel.set_ifs(ifs)

            # Select polarizations
            if (type(pollist) == list):
              pols = pollist
            else:
              pols = [pollist]
            if(len(pols) > 0 ):
              sel.set_polarisations(pols)

            try:
                #Apply the selection
                s.set_selection(sel)
            except Exception, instance:
                print '***Error***',instance
                return
            del sel

	    # Save the previous plotter settings
	    oldxlim = sd.plotter._minmaxx
	    oldylim = sd.plotter._minmaxy
	    oldpanel = sd.plotter._panelling
	    oldstack = sd.plotter._stacking
	    oldhist = sd.plotter._hist
	    # line properties
	    colormapold=sd.plotter._plotter.colormap
	    linestylesold=sd.plotter._plotter.linestyles
	    linewidthold=pl.rcParams['lines.linewidth']
	    if not hasattr(sd.plotter._plotter.figmgr,'sdplotbar') or sd.plotter._plotter.figmgr.sdplotbar.custombar is None:
		    sd.plotter._plotter.figmgr.sdplotbar=CustomToolbarTkAgg(figmgr=sd.plotter._plotter.figmgr)

            # Plotting
            if plottype=='pointing':
                    if plotfile != '': 
                           sd.plotter.plotpointing(s,plotfile)
                    else:
                           sd.plotter.plotpointing(s)
            elif plottype=='azel':
                    if plotfile != '': 
                           sd.plotter.plotazel(s,plotfile)
                    else:
                           sd.plotter.plotazel(s)
            elif plottype=='totalpower':
                    sd.plotter.plottp(s)
            else:
                    if s.nchan()==1:
                           errmsg="Trying to plot the continuum/total power data in 'spectra' mode,\
                                   please use other plottype options" 
                           raise Exception,errmsg
                    # Average in time if desired
                    if (scanaverage and isScantable):
                           scave=sd.average_time(s,scanav=True)
                    else:
                           scave=s.copy()

                    if ( timeaverage ):
                            if tweight=='none':
                                    errmsg = "Please specify weight type of time averaging"
                                    raise Exception,errmsg
                            stave=sd.average_time(scave,scanav=False, weight=tweight)
                            del scave
                            # Now average over polarization
                            if ( polaverage ):
                                    if pweight=='none':
               				    errmsg = "Please specify weight type of polarization averaging"
                                            raise Exception,errmsg
                                    np = stave.npol()
                                    if ( np > 1 ):
                                            spave=stave.average_pol(weight=pweight)
                                    else:
                                            # only single polarization
                                            print "Single polarization data - no need to average"
                                            spave=stave.copy()
                            else:
                                    spave=stave.copy()
                            del stave
                    else:
                            if ( polaverage ):
                                    if pweight=='none':
                  		            errmsg = "Please specify weight type of polarization averaging"
                                            raise Exception,errmsg
                                    np = s.npol()
                                    if ( np > 1 ):
                                            spave=scave.average_pol(weight=pweight)
                                    else:
                                            # only single polarization
                                            print "Single polarization data - no need to average"
                                            spave=scave.copy()
                            else:
                                    spave=scave.copy()
                            del scave

                    # Smooth the spectrum (if desired)

                    if kernel == '': kernel = 'none'
                    if ( kernel != 'none' and (not (kwidth<=0 and kernel!='hanning'))):
                            print "Smoothing spectrum with kernel "+kernel
                            spave.smooth(kernel,kwidth)

                    # Plot final spectrum
                    # each IF is separate panel, pols stacked
                    sd.plotter.plot(spave)
                    sd.plotter.set_mode(stacking=stack,panelling=panel)

		    # Set colormap, linestyles, and linewidth of plots
		    
		    ncolor = 0
		    if colormap != 'none': 
			    colmap = colormap
			    ncolor=len(colmap.split())
		    elif linestyles == 'none': 
			    colmap = "green red black cyan magenta orange blue purple yellow pink"
			    ucm = sd.rcParams['plotter.colours']
			    if isinstance(ucm,str) and len(ucm) > 0: colmap = ucm
			    ncolor=len(colmap.split())
			    del ucm
		    else: colmap=None

		    if linestyles != 'none': lines=linestyles
		    elif ncolor <= 1: 
			    lines = "line dashed dotted dashdot"
			    uls = sd.rcParams['plotter.linestyles']
			    if isinstance(uls,str) and len(uls) > 0: lines = uls
			    del uls
		    else: lines=None
		
		    if isinstance(linewidth,int) or isinstance (linewidth,float):
			    lwidth = linewidth
		    else:
			    print "WARNING: Invalid linewidth. linewidth is ignored and set to 1."
			    lwidth = 1

		    # set plot colors
		    if colmap is not None:
			    if ncolor > 1 and lines is not None:
				    print "WARNING: 'linestyles' is valid only for single colour plot.\n...Ignoring 'linestyles'."
			    sd.plotter.set_colors(colmap)
		    else:
			    if lines is not None:
				    tmpcol="black"
				    print "INFO: plot colour is set to '",tmpcol,"'"
				    sd.plotter.set_colors(tmpcol)
		    # set linestyles and/or linewidth
		    # so far, linestyles can be specified only if a color is assigned
		    #if lines is not None or linewidth is not None:
		    #        sd.plotter.set_linestyles(lines, linewidth)
		    sd.plotter.set_linestyles(lines, lwidth)
                    # Plot red x-axis at y=0 (currently disabled)
                    sd.plotter.set_histogram(hist=histogram)
                    # sd.plotter.axhline(color='r',linewidth=2)

                    # Set axis ranges (if requested)
                    if len(flrange)==1: print "flrange needs 2 limits - ignoring"
                    if len(sprange)==1: print "sprange needs 2 limits - ignoring"
                    if ( len(sprange) > 1 ):
                            if ( len(flrange) > 1 ):
                                    sd.plotter.set_range(sprange[0],sprange[1],flrange[0],flrange[1])
                            else:
                                    sd.plotter.set_range(sprange[0],sprange[1])
                    elif ( len(flrange) > 1 ):
                            sd.plotter.set_range(ystart=flrange[0],yend=flrange[1])
                    else:
                    # Set default range explicitly (in case range was ever set)
                            sd.plotter.set_range()

		    # Set picker to all the spectra
		    if sd.plotter._visible:
			    npanel=len(sd.plotter._plotter.subplots)
			    for ipanel in range(npanel):
				    ax=sd.plotter._plotter.subplots[ipanel]['axes']
				    for line in ax.lines:
					    line.set_picker(5.0)

                    # Line catalog
                    dolinc=False
                    if ( linecat != 'none' and linecat != '' ):
                            # Use jpl catalog for now (later allow custom catalogs)

                            casapath=os.environ['CASAPATH'].split()
                            catpath=casapath[0]+'/data/catalogs/lines'
                            catname=catpath+'/jpl_asap.tbl'
                            # TEMPORARY: hard-wire to my version
                            # catname='/home/sandrock/smyers/NAUG/Tasks/jpl.tbl'
                            # FOR LOCAL CATALOGS:
                            # catname='jpl.tbl'
                            try:
                                    linc=sd.linecatalog(catname)
                                    dolinc=True
                            except:
                                    print "Could not find catalog at "+catname
                                    dolinc=False
                            if ( dolinc ):
                                    if ( len(sprange)>1 ):
                                            if ( specunit=='GHz' or specunit=='MHz' ):
                                                    linc.set_frequency_limits(sprange[0],sprange[1],specunit)
                                            else:
                                                    print "ERROR: sd.linecatalog.set_frequency_limits accepts onlyGHz and MHz"
                                                    print "continuing without sprange selection on catalog"
                                    if ( linecat != 'all' and linecat != 'ALL' ):
                                            # do some molecule selection
                                            linc.set_name(linecat)
                                    # Plot up the selected part of the line catalog
                                    # use doppler offset
                                    sd.plotter.plot_lines(linc,doppler=linedop)

		    # List observation header
		    # Get antena name
		    antennaname=spave.get_antennaname()
		    # Get an observed source list
		    srclist=spave.get_sourcename()
		    srcset=set([srclist[0]])
		    for i in range(len(srclist)): srcset.add(srclist[i])
		    comma_space=", "
		    srcname=comma_space.join(list(srcset))
		    # Get start date & time of observation
		    obstime=spave.get_time(row=0)
		    # Output strings
		    headdata='Antenna: '+antennaname+\
			      '\nStart time: '+str(obstime)+\
			      '\nSource: '+srcname
		    # Add Observation header to the upper-left corner of plot
		    sd.plotter.figtext(0.03,0.98,headdata,
					horizontalalignment='left',
					verticalalignment='top',
					fontsize=11)

            # Hardcopy
            if ( plottype in ['spectra','totalpower'] and plotfile != '' ):
                    # currently no way w/o screen display first
                    sd.plotter.save(plotfile)

            # Do some clean up
            if plottype=='spectra': 
                    del spave
                    if dolinc: del linc
		    #if colormapold is not None:
		    #	    print "Restoring colormap..."
		    #	    sd.plotter._plotter.colormap=colormapold

	    # Restore the previous line properties
	    sd.plotter._plotter.colormap=colormapold
	    sd.plotter._plotter.linestyles=linestylesold
	    pl.rc('lines', linewidth=linewidthold)
	    # Restore the previous plotter settings
	    sd.plotter._minmaxx = oldxlim
	    sd.plotter._minmaxy = oldylim
	    sd.plotter._panelling = oldpanel
	    sd.plotter._stacking = oldstack
	    sd.plotter._hist = oldhist

	    # Define Pick event
	    if sd.plotter._visible:
		    if sd.plotter._plotter.events['button_press'] is not None:
			    sd.plotter._plotter.canvas.mpl_disconnect(sd.plotter._plotter.events['button_press'])
			    sd.plotter._plotter.events['button_press']=None
		    if plottype=='spectra':
			    sd.plotter._plotter.events['button_press']=sd.plotter._plotter.canvas.mpl_connect('button_press_event',_event_switch)
            # DONE

        except Exception, instance:
                print '***Error***',instance
                return

########################################
##    Helper functions for sdplot     ##
########################################
### Add a custom toolbar to ASAP plotter
class CustomToolbarTkAgg:
	def __init__(self,figmgr=None):
		if figmgr is None: return
		self.figmgr=figmgr
		self.custombar=None
		self._add_custom_toolbar()

	def _add_custom_toolbar(self):
		self.custombar=Tk.Frame(master=self.figmgr.window)
		self.bSpec=self._NewButton(master=self.custombar,
				      text='spec value',
				      command=self.spec_show)
		self.bStat=self._NewButton(master=self.custombar,
				      text='statistics',
				      command=self.stat_cal)
		self.bQuit=self._NewButton(master=self.custombar,
				      text='Quit',
				      command=self.quit,
				      side=Tk.RIGHT)
		self.custombar.pack(side=Tk.BOTTOM,fill=Tk.BOTH)

		### temporary added
		#self.bStat.config(state=Tk.DISABLED)
		###
		self.bSpec.config(relief='sunken')
		self.bStat.config(relief='raised')
		self.mode='spec'
		self.spec_show()
		return self

	def _NewButton(self, master, text, command, side=Tk.LEFT):
		if(os.uname()[0] == 'Darwin'):
			b = Tk.Button(master=master, text=text, command=command)
		else:
			b = Tk.Button(master=master, text=text, padx=2, pady=2, command=command)
		b.pack(side=side)
		return b
	
	def spec_show(self):
		self.figmgr.toolbar.set_message('spec value: drag on a spec')
		if self.mode == 'spec': return
		self.bStat.config(relief='raised')
		self.bSpec.config(relief='sunken')
		self.mode='spec'

	def stat_cal(self):
		self.figmgr.toolbar.set_message('statistics: select a region')
		if self.mode == 'stat': return
		self.bSpec.config(relief='raised')
		self.bStat.config(relief='sunken')
		self.mode='stat'

	def quit(self):
		self.delete_bar()
		sd.plotter._plotter.unmap()

	def delete_bar(self):
		self.custombar.destroy()
		self.custombar=None		

### callback a function according to the selected mode. 
def _event_switch(event):
        # Do not fire event when in zooming/panning mode
	if not sd.plotter._plotter.figmgr.toolbar.mode == '': return
        # When selected point is out of panels
        if event.inaxes == None:
		return
	# Now acutual callback
	if sd.plotter._plotter.figmgr.sdplotbar.mode == 'stat':
		_single_mask(event)
	else: _select_spectrum(event)

### select the nearest spectrum in pick radius
###    and display spectral value on the toolbar. 
def _select_spectrum(event):
        # Do not fire event when in zooming/panning mode
        mode = sd.plotter._plotter.figmgr.toolbar.mode
        if not mode =='':
		return
        # When selected point is out of panels
        if event.inaxes == None:
		return
        # If not left button
        if event.button != 1:
		return

        xclick=event.xdata
        yclick=event.ydata
        dist2=1000.
        pickline=None
	# If the pannel has picable objects
	pflag=False
        for lin in event.inaxes.lines:
		if not lin.pickable(): continue
		pflag=True
		flag,pind = lin.contains(event)
		if not flag: continue
		# Get nearest point
		inds = pind['ind']
		xlin = lin.get_xdata()
		ylin = lin.get_ydata()
		for i in inds:
			d2=(xlin[i]-xclick)**2+(ylin[i]-yclick)**2
			if dist2 >= d2:
				dist2 = d2
				pickline = lin
	# No pickcable line in the pannel
	if not pflag: return
	# Pickable but too far from mouse position
	elif pickline is None:
		picked='No line selected.'
		sd.plotter._plotter.figmgr.toolbar.set_message(picked)
		return
        del pind, inds, xlin, ylin
	# Spectra are Picked
	theplot=sd.plotter._plotter
	thetoolbar = theplot.figmgr.toolbar
	thecanvas = theplot.figmgr.canvas
	# Disconnect the default motion notify event
	# Notice! the other buttons are also diabled!!!
	thecanvas.mpl_disconnect(thetoolbar._idDrag)
        # Get picked spectrum
        xdata = pickline.get_xdata()
        ydata = pickline.get_ydata()
        titl=pickline.get_label()
        titp=event.inaxes.title.get_text()
        panel0=event.inaxes
	picked="Selected: '"+titl+"' in panel '"+titp+"'."
	thetoolbar.set_message(picked)
	# Generate a navigation window
        #naviwin=Navigationwindow(titp,titl)
        #------------------------------------------------------#
        # Show spectrum data at mouse position
        def spec_data(event):
		# Getting spectrum data of neiboring point
		xclick=event.xdata
		if event.inaxes != panel0:
			return
		ipoint=len(xdata)-1
		for i in range(len(xdata)-1):
			xl=xclick-xdata[i]
			xr=xclick-xdata[i+1]
			if xl*xr <= 0.:
				ipoint = i
				break
		# Output spectral value on the navigation window
		posi='[ %s, %s ]:  x = %.2f   value = %.2f'\
		      %(titl,titp,xdata[ipoint],ydata[ipoint])
		#naviwin.posi.set(posi)
		thetoolbar.set_message(posi)
        #------------------------------------------------------#
        # Disconnect from mouse events
        def discon(event):
		#naviwin.window.destroy()
		thecanvas.mpl_disconnect(theplot.events['motion_notify'])
		theplot.events['motion_notify']=None
		thetoolbar._idDrag=thecanvas.mpl_connect('motion_notify_event', thetoolbar.mouse_move)
		thecanvas.mpl_disconnect(theplot.events['button_release'])
		theplot.events['button_release']=None
		return
        #------------------------------------------------------#
        # Show data value along with mouse movement
        theplot.events['motion_notify']=thecanvas.mpl_connect('motion_notify_event',spec_data)
        # Finish events when mouse button is released
        theplot.events['button_release']=thecanvas.mpl_connect('button_release_event',discon)


### Calculate statistics of the selected area. 
def _single_mask(event):
	if event.button ==1: baseval=False
	elif event.button == 3: baseval=True
	else: return

	mask_sel=mask_selection()
	mask_sel.scan=sd.plotter._data
	mask_sel._p=sd.plotter._plotter
	# Create initial mask
	mask_sel.mask=sd._n_bools(mask_sel.scan.nchan(),baseval)
	# Selected mask
	mask_sel._region_start(event)


class mask_selection:
	
	def __init__(self):
		"""
		Create a interactive masking object
		"""
		self.scan=None
		self.rect={}
		self.xold=None
		self.yold=None
		self.xdataold=None
		self.ydataold=None
		self.mask=None
		self._polygons=[]
		self._p=None

	def _region_start(self,event):
		# Do not fire event when in zooming/panning mode
		mode = self._p.figmgr.toolbar.mode
		if not mode =='':
			return
		# Return if selected point is out of panel
		if event.inaxes == None: return
		# Select mask/unmask region with mask
		height = self._p.canvas.figure.bbox.height()
		self.rect = {'button': event.button, 'axes': event.inaxes,
			     'fig': None, 'height': height,
			     'x': event.x, 'y': height - event.y,
			     'world': [event.xdata, event.ydata,
				       event.xdata, event.ydata],
			     'pixel': [event.x, height - event.y,
				       event.x, height -event.y]}
		self._p.register('motion_notify', self._region_draw)
		self._p.register('button_release', self._region_end)

	def _region_draw(self,event):
		self._p.canvas._tkcanvas.delete(self.rect['fig'])
		sameaxes=(event.inaxes == self.rect['axes'])
		if sameaxes: 
			xnow=event.x
			ynow=event.y
			self.xold=xnow
			self.yold=ynow
			self.xdataold=event.xdata
			self.ydataold=event.ydata
		else:
			xnow=self.xold
			ynow=self.yold
			
		self.rect['fig'] = self._p.canvas._tkcanvas.create_rectangle(
			self.rect['x'], self.rect['y'],
			xnow, self.rect['height'] - ynow)

	def _region_end(self,event):
		height = self._p.canvas.figure.bbox.height()
		self._p.register('motion_notify', None)
		self._p.register('button_release', None)
		
		self._p.canvas._tkcanvas.delete(self.rect['fig'])
                
		if event.inaxes == self.rect['axes']: 
			xend=event.x
			yend=event.y
			xdataend=event.xdata
			ydataend=event.ydata
		else:
			xend=self.xold
			yend=self.yold
			xdataend=self.xdataold
			ydataend=self.ydataold
			
		self.rect['world'][2:4] = [xdataend, ydataend]
		self.rect['pixel'][2:4] = [xend, height - yend]
		self._update_mask()
		########## ADDED - 2009.04.01 kana
		masklist=self.scan.get_masklist(mask=self.mask)
		print 'stat region:', masklist
		# Call sdstat()
		invertmask=False
		interactive=False
		statfile=''
		self.scan.stats(stat='max',mask=self.mask)
		self.scan.stats(stat='min',mask=self.mask)
		self.scan.stats(stat='sum',mask=self.mask)
		self.scan.stats(stat='mean',mask=self.mask)
		self.scan.stats(stat='median',mask=self.mask)
		self.scan.stats(stat='rms',mask=self.mask)
		self.scan.stats(stat='stddev',mask=self.mask)
		#if len(self._polygons)>0:
		#	# Remove old polygons
		#	for polygon in self._polygons: polygon.remove()
		#	self._polygons=[]
		#        #mask_sel._p.canvas.draw()

		########## ADDED

	def _update_mask(self):
		from asap import mask_and, mask_or
		# Min and Max for new mask
		xstart=self.rect['world'][0]
		xend=self.rect['world'][2]
		if xstart <= xend: newlist=[xstart,xend]
		else: newlist=[xend,xstart]
		# Mask or unmask
		invmask=None
		if self.rect['button'] == 1:
			invmask=False
			mflg='Mask'
		elif self.rect['button'] == 3:
			invmask=True
			mflg='UNmask'
		print mflg+': ',newlist
		newmask=self.scan.create_mask(newlist,invert=invmask)
		# Logic operation to update mask
		if invmask:
			self.mask=mask_and(self.mask,newmask)
		else:
			self.mask=mask_or(self.mask,newmask)
		# Plot masked regions
		#self._plot_mask()

