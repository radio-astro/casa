import os
from taskinit import *

import asap as sd
import pylab as pl
#import Tkinter as Tk

def sdplot(sdfile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, scanaverage, timeaverage, tweight, polaverage, pweight, kernel, kwidth, plottype, stack, panel, flrange, sprange, linecat, linedop, colormap, linestyles, linewidth, histogram, header, headsize, plotstyle, layout, legendloc, plotfile, overwrite):

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
            s=sd.scantable(sdfile,average=scanaverage,antenna=antenna)

            # get telescope name
            #'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
            antennaname = s.get_antennaname()

            # determine current fluxunit
            fluxunit_now = s.get_fluxunit()
            if ( antennaname == 'GBT'):
                            if (fluxunit_now == ''):
                                    #print "no fluxunit in the data. Set to Kelvin."
                                    casalog.post( "no fluxunit in the data. Set to Kelvin." )
                                    s.set_fluxunit('K')
                                    fluxunit_now = s.get_fluxunit()
            #print "Current fluxunit = "+fluxunit_now
            casalog.post( "Current fluxunit = "+fluxunit_now )

            # set flux unit string (be more permissive than ASAP)
            if ( fluxunit == 'k' ):
                    fluxunit = 'K'
            elif ( fluxunit == 'JY' or fluxunit == 'jy' ):
                    fluxunit = 'Jy'

            # fix the fluxunit if necessary
            if ( telescopeparm == 'FIX' or telescopeparm == 'fix' ):
                            if ( fluxunit != '' ):
                                    if ( fluxunit == fluxunit_now ):
                                            #print "No need to change default fluxunits"
                                            casalog.post( "No need to change default fluxunits" )
                                    else:
                                            s.set_fluxunit(fluxunit)
                                            #print "Reset default fluxunit to "+fluxunit
                                            casalog.post( "Reset default fluxunit to "+fluxunit )
                                            fluxunit_now = s.get_fluxunit()
                            else:
                                    #print "Warning - no fluxunit for set_fluxunit"
                                    casalog.post( "no fluxunit for set_fluxunit", priority = 'WARN' )


            elif ( fluxunit=='' or fluxunit==fluxunit_now ):
                    if ( fluxunit==fluxunit_now ):
                            #print "No need to convert fluxunits"
                            casalog.post( "No need to convert fluxunits" )

            elif ( type(telescopeparm) == list ):
                    # User input telescope params
                    if ( len(telescopeparm) > 1 ):
                            D = telescopeparm[0]
                            eta = telescopeparm[1]
                            #print "Use phys.diam D = %5.1f m" % (D)
                            #print "Use ap.eff. eta = %5.3f " % (eta)
                            casalog.post( "Use phys.diam D = %5.1f m" % (D) )
                            casalog.post( "Use ap.eff. eta = %5.3f " % (eta) )
                            s.convert_flux(eta=eta,d=D)
                    elif ( len(telescopeparm) > 0 ):
                            jypk = telescopeparm[0]
                            #print "Use gain = %6.4f Jy/K " % (jypk)
                            casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
                            s.convert_flux(jyperk=jypk)
                    else:
                            #print "Empty telescope list"
                            casalog.post( "Empty telescope list" )

            elif ( telescopeparm=='' ):
                    if ( antennaname == 'GBT'):
                            # needs eventually to be in ASAP source code
                            #print "Convert fluxunit to "+fluxunit
                            casalog.post( "Convert fluxunit to "+fluxunit )
                            # THIS IS THE CHEESY PART
                            # Calculate ap.eff eta at rest freq
                            # Use Ruze law
                            #   eta=eta_0*exp(-(4pi*eps/lambda)**2)
                            # with
                            #print "Using GBT parameters"
                            casalog.post( "Using GBT parameters" )
                            eps = 0.390  # mm
                            eta_0 = 0.71 # at infinite wavelength
                            # Ideally would use a freq in center of
                            # band, but rest freq is what I have
                            rf = s.get_restfreqs()[0][0]*1.0e-9 # GHz
                            eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
                            #print "Calculated ap.eff. eta = %5.3f " % (eta)
                            #print "At rest frequency %5.3f GHz" % (rf)
                            casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
                            casalog.post( "At rest frequency %5.3f GHz" % (rf) )
                            D = 104.9 # 100m x 110m
                            #print "Assume phys.diam D = %5.1f m" % (D)
                            casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
                            s.convert_flux(eta=eta,d=D)

                            #print "Successfully converted fluxunit to "+fluxunit
                            casalog.post( "Successfully converted fluxunit to "+fluxunit )
                    elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
                            s.convert_flux()

                    else:
                            # Unknown telescope type
                            #print "Unknown telescope - cannot convert"
                            casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )


            # set spectral axis unit
            if ( specunit != '' ):
                    #print "Changing spectral axis to "+specunit
                    casalog.post( "Changing spectral axis to "+specunit )
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
                    #print 'Set rest frequency to ', fval, ' Hz'
                    casalog.post( 'Set rest frequency to %d Hz' %(fval) )
                    s.set_restfreqs(freqs=fval)

            # reset frame and doppler if needed
            if ( frame != '' ):
                    #print "Changing frequency frame to "+frame
                    casalog.post( "Changing frequency frame to "+frame )
                    s.set_freqframe(frame)
            else:
                    #print 'Using current frequency frame'
                    casalog.post( 'Using current frequency frame' )

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
                    #print 'Using current doppler convention'
                    casalog.post( 'Using current doppler convention' )

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
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return
	    # For printing header information
	    ssel=sel.__str__()
            del sel

	    # Save the previous plotter settings
	    oldxlim = sd.plotter._minmaxx
	    oldylim = sd.plotter._minmaxy
	    oldpanel = sd.plotter._panelling
	    oldstack = sd.plotter._stacking
	    oldhist = sd.plotter._hist
	    oldlayout = sd.plotter._panellayout
	    # Line properties
	    colormapold=sd.plotter._plotter.colormap
	    linestylesold=sd.plotter._plotter.linestyles
	    linewidthold=pl.rcParams['lines.linewidth']

	    # Reload plotter if necessary
	    if not sd.plotter._plotter or sd.plotter._plotter.is_dead:
		    if hasattr(sd.plotter,'_plotter') and \
			   hasattr(sd.plotter._plotter.figmgr,'casabar'):
			    del sd.plotter._plotter.figmgr.casabar
		    sd.plotter._plotter = sd.plotter._newplotter()
		    sd.plotter._plotter.figmgr.casabar=sd.plotter._newcasabar()

	    # The new toolbar
	    #if not hasattr(sd.plotter._plotter.figmgr,'sdplotbar') or sd.plotter._plotter.figmgr.sdplotbar.custombar is None:
	    #	    sd.plotter._plotter.figmgr.sdplotbar=CustomToolbarTkAgg(figmgr=sd.plotter._plotter.figmgr)

	    # Set panel layout
	    if layout != oldlayout: sd.plotter.set_panellayout(layout=layout,refresh=False)
	    
            # Plotting
	    asaplot=False
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
		    asaplot=True
                    sd.plotter.plottp(s)
            else:
		    asaplot=True
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
                                            #print "Single polarization data - no need to average"
                                            casalog.post( "Single polarization data - no need to average" )
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
                                            #print "Single polarization data - no need to average"
                                            casalog.post( "Single polarization data - no need to average" )
                                            spave=scave.copy()
                            else:
                                    spave=scave.copy()
                            del scave

                    # Smooth the spectrum (if desired)

                    if kernel == '': kernel = 'none'
                    if ( kernel != 'none' and (not (kwidth<=0 and kernel!='hanning'))):
                            #print "Smoothing spectrum with kernel "+kernel
                            casalog.post( "Smoothing spectrum with kernel "+kernel )
                            spave.smooth(kernel,kwidth)

                    # Plot final spectrum
                    # each IF is separate panel, pols stacked
		    refresh=False
                    #sd.plotter.plot(spave)
                    sd.plotter.set_data(spave,refresh=refresh)
                    sd.plotter.set_mode(stacking=stack,panelling=panel,refresh=refresh)

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
			    #print "WARNING: Invalid linewidth. linewidth is ignored and set to 1."
                            casalog.post( "Invalid linewidth. linewidth is ignored and set to 1.", priority = 'WARN' )
			    lwidth = 1

		    # set plot colors
		    if colmap is not None:
			    if ncolor > 1 and lines is not None:
				    #print "WARNING: 'linestyles' is valid only for single colour plot.\n...Ignoring 'linestyles'."
                                    casalog.post( "'linestyles' is valid only for single colour plot.\n...Ignoring 'linestyles'.", priority = 'WARN' )
			    sd.plotter.set_colors(colmap,refresh=refresh)
		    else:
			    if lines is not None:
				    tmpcol="black"
				    #print "INFO: plot colour is set to '",tmpcol,"'"
                                    casalog.post( "plot colour is set to '"+tmpcol+"'" )
				    sd.plotter.set_colors(tmpcol,refresh=refresh)
		    # set linestyles and/or linewidth
		    # so far, linestyles can be specified only if a color is assigned
		    #if lines is not None or linewidth is not None:
		    #        sd.plotter.set_linestyles(lines, linewidth,refresh=refresh)
		    sd.plotter.set_linestyles(lines, lwidth,refresh=refresh)
                    # Plot red x-axis at y=0 (currently disabled)
                    # sd.plotter.axhline(color='r',linewidth=2)
		    sd.plotter.set_histogram(hist=histogram,refresh=refresh)

                    # Set axis ranges (if requested)
                    if len(flrange)==1:
                            #print "flrange needs 2 limits - ignoring"
                            casalog.post( "flrange needs 2 limits - ignoring" )
                    if len(sprange)==1:
                            #print "sprange needs 2 limits - ignoring"
                            casalog.post( "sprange needs 2 limits - ignoring" )
                    if ( len(sprange) > 1 ):
                            if ( len(flrange) > 1 ):
                                    sd.plotter.set_range(sprange[0],sprange[1],flrange[0],flrange[1],refresh=refresh)
                            else:
				    sd.plotter.set_range(sprange[0],sprange[1],refresh=refresh)
                    elif ( len(flrange) > 1 ):
			    sd.plotter.set_range(ystart=flrange[0],yend=flrange[1],refresh=refresh)
                    else:
                    # Set default range explicitly (in case range was ever set)
                            sd.plotter.set_range(refresh=refresh)

		    # legend position
		    loc=1
		    if plotstyle: loc=legendloc
		    sd.plotter._plotter.legend(loc)
		    
		    # Need the actual plotting before setting picker
		    sd.plotter.plot()
		    
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
                                    #print "Could not find catalog at "+catname
                                    casalog.post( "Could not find catalog at "+catname, priority = False )
                                    dolinc=False
                            if ( dolinc ):
                                    if ( len(sprange)>1 ):
                                            if ( specunit=='GHz' or specunit=='MHz' ):
                                                    linc.set_frequency_limits(sprange[0],sprange[1],specunit)
                                            else:
                                                    #print "ERROR: sd.linecatalog.set_frequency_limits accepts onlyGHz and MHz"
                                                    #print "continuing without sprange selection on catalog"
                                                    casalog.post( "sd.linecatalog.set_frequency_limits accepts onlyGHz and MHz", priority = 'WARN' )
                                                    casalog.post( "continuing without sprange selection on catalog", priority = 'WARN' )
                                    if ( linecat != 'all' and linecat != 'ALL' ):
                                            # do some molecule selection
                                            linc.set_name(linecat)
                                    # Plot up the selected part of the line catalog
                                    # use doppler offset
                                    sd.plotter.plot_lines(linc,doppler=linedop)

	    # List observation header
	    if not plotstyle or layout==[]:
		    # automatic layout
		    sd.plotter._plotter.figure.subplots_adjust(top=0.8)
	    datname='Data File:     '+sdfile
	    sd.plotter.print_header(plot=(header and asaplot),fontsize=headsize,
				    logger=True,selstr=ssel,extrastr=datname)
	    del ssel, datname

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
	    sd.plotter._panellayout = oldlayout

	    # Define Pick event
	    #if sd.plotter._visible:
	    #	    sd.plotter._plotter.register('button_press',None)
	    #	    if plottype=='spectra':
	    #		    sd.plotter._plotter.register('button_press',_event_switch)

            # DONE

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return

