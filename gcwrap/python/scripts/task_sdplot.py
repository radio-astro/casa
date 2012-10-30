import os
from taskinit import *

import asap as sd
import pylab as pl
from asap import _to_list
from asap.scantable import is_scantable
import sdutil

def sdplot(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, beamlist, scanaverage, timeaverage, tweight, polaverage, pweight, kernel, kwidth, plottype, stack, panel, flrange, sprange, linecat, linedop, subplot, colormap, linestyles, linewidth, histogram, center, cell, header, headsize, plotstyle, margin, legendloc, outfile, overwrite):

    casalog.origin('sdplot')

    ###
    ### Now the actual task code
    ###
    try:
        # Make sure infile exists
        sdutil.assert_infile_exists(infile)
            
        if not overwrite and not outfile=='':
            outfilename = os.path.expandvars(outfile)
            outfilename = os.path.expanduser(outfilename)
            if os.path.exists(outfilename):
                s = "Output file '%s' exist." % (outfilename)
                raise Exception, s
            
        isScantable = is_scantable(infile)

        #load the data without time/pol averaging
        sorg = sd.scantable(infile,average=scanaverage,antenna=antenna)

        rfset = (restfreq != '') and (restfreq != [])
        doCopy = (frame != '') or (doppler != '') or rfset \
                 or (fluxunit != '' and fluxunit != sorg.get_fluxunit()) \
                 or (specunit != '' and specunit != sorg.get_unit())
        doCopy = doCopy and isScantable

        # A scantable selection
        if type(scanlist) == str:
            scanlist = sorg.parse_idx_selection("SCAN",scanlist)
        if type(iflist) == str:
            iflist = sorg.parse_idx_selection("IF",iflist)
        if type(pollist) == str:
            pollist = sorg.parse_idx_selection("POL",pollist)
        if type(beamlist) == str:
            beamlist = sorg.parse_idx_selection("BEAM",beamlist)
        sel = sdutil.get_selector(in_scans=scanlist, in_ifs=iflist,
                                  in_pols=pollist, in_field=field,
                                  in_beams=beamlist)
        sorg.set_selection(sel)
        ssel=sel.__str__()
        del sel

        # Copy scantable when usign disk storage not to modify
        # the original table.
        if doCopy and sd.rcParams['scantable.storage'] == 'disk':
            s = sorg.copy()
        else:
            s = sorg
        del sorg

        # convert flux
        sdutil.set_fluxunit(s, fluxunit, telescopeparm)

        # set spectral axis unit
        sdutil.set_spectral_unit(s, specunit)

        # set rest frequency
        if ( specunit == 'km/s' and rfset ):
            fval = sdutil.normalise_restfreq(restfreq)
            casalog.post( 'Set rest frequency to %s Hz' % str(fval) )
            s.set_restfreqs(freqs=fval)

        # reset frame and doppler if needed
        sdutil.set_freqframe(s, frame)
        sdutil.set_doppler(s, doppler)

        # Averaging
        # average over time (scantable is already scan averaged if necessary)
        spave = sdutil.doaverage(s, scanaverage, timeaverage, tweight, polaverage, pweight)

        # Grid scantable
        if plottype == 'grid':
            if subplot < 11:
                casalog.post("Setting default subplot layout (1x1).",priority="WARN")
                subplot = 11
            nx = (subplot % 10)
            ny = int(subplot/10)
            if center != '':
                ldirstr = center.split(' ')
                if len(ldirstr) == 2:
                    center = "J2000 "+center
                elif len(ldirstr) == 3:
                    if ldirstr[0].upper() != "J2000":
                        raise ValueError, "Currently only J2000 is supported"
                else:
                    errmsg = "Invalid grid center: %s" % (center)
                    raise ValueError, errmsg
            cellx = ''
            celly = ''
            if type(cell) == str:
                cellx = cell
                celly = cell
            elif len(cell) == 2:
                cellx = cell[0]
                celly = cell[1]
            elif len(cell) == 1:
                cellx = cell[0]
                celly = cell[0]
            # Do only the first IFNO and POLNO
            ifnos = spave.getifnos()
            polnos = spave.getpolnos()
            if len(ifnos) > 1 or len(polnos) > 1:
                casalog.post("Only the first IFNO (%d) and POLNO (%d) is plotted." % (ifnos[0], polnos[0]),priority="WARN")

            
            gridder = sd.asapgrid2(spave)
            del spave
            gridder.setIF(ifnos[0])
            gridder.setPolList([polnos[0]])
            gridder.defineImage(center=center,cellx=cellx,celly=celly,nx=nx,ny=ny)
            gridder.setFunc(func='BOX')
            gridder.setWeight('uniform')
            gridder.grid()
            
            spave = gridder.getResult()
            del gridder

            # Now set fluxunit, specunit, frame, and doppler
            if ( fluxunit != '' ): spave.set_fluxunit(fluxunit)
            if ( specunit != '' ):
                spave.set_unit(specunit)
                if ( specunit == 'km/s' and rfset ):
                    fval = sdutil.normalise_restfreq(restfreq)
                    spave.set_restfreqs(freqs=fval)

            if ( frame != '' ): spave.set_freqframe(frame)
            if ( doppler != '' ): spave.set_doppler(ddoppler)


        # Reload plotter if necessary
        sd.plotter._assert_plotter(action="reload")

        # Set subplot layout
        if subplot > 10:
            row = int(subplot/10)
            col = (subplot % 10)
            sd.plotter.set_layout(rows=row,cols=col,refresh=False)
        else:
            if subplot > -1:
                casalog.post(("Invalid subplot value, %d, is ignored. It should be in between 11 and 99." % subplot),priority="WARN")
            sd.plotter.set_layout(refresh=False)

        # Set subplot margins
        if margin != sd.plotter._margins:
            sd.plotter.set_margin(margin=margin,refresh=False)

        # Plotting
        asaplot=False
        if plottype=='pointing':
            if outfile != '': 
                sd.plotter.plotpointing(spave,outfile)
            else:
                sd.plotter.plotpointing(spave)
            del spave
        elif plottype=='azel':
            if outfile != '': 
                sd.plotter.plotazel(spave,outfile)
            else:
                sd.plotter.plotazel(spave)
            del spave
        elif plottype=='totalpower':
            asaplot=True
            sd.plotter.plottp(spave)
            del spave
        else:
            asaplot=True
            if spave.nchan()==1:
                errmsg="Trying to plot the continuum/total power data in 'spectra' mode, please use other plottype options" 
                raise Exception,errmsg

            # Smooth the spectrum (if desired)

            if kernel == '': kernel = 'none'
            if ( kernel != 'none' and (not (kwidth<=0 and kernel!='hanning'))):
                casalog.post( "Smoothing spectrum with kernel "+kernel )
                spave.smooth(kernel,kwidth)

            # Plot final spectrum
            # each IF is separate panel, pols stacked
            refresh=False
            #sd.plotter.plot(spave)
            sd.plotter.set_data(spave,refresh=refresh)
            del spave
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
                casalog.post( "Invalid linewidth. linewidth is ignored and set to 1.", priority = 'WARN' )
                lwidth = 1

            # set plot colors
            if colmap is not None:
                if ncolor > 1 and lines is not None:
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
            sd.plotter.set_legend(mode=loc,refresh=refresh)

            # The actual plotting
            if plottype == 'grid':
                # Need to specify center and spacing in the epoch and
                # unit of DIRECTION column (currently assuming J2000 and rad)
                # TODO: need checking epoch and unit of DIRECTION in scantable
                crad = None
                spacing = None
                if center != '':
                    epoch, ra, dec = center.split(' ')
                    cme = me.direction(epoch, ra, dec)
                    crad = [qa.convert(cme['m0'],'rad')['value'], \
                            qa.convert(cme['m1'],'rad')['value']]
                if cellx != '' and celly != '':
                    spacing = [qa.convert(cellx, 'rad')['value'], \
                               qa.convert(celly, 'rad')['value']]
                sd.plotter.plotgrid(center=crad,spacing=spacing,rows=ny,cols=nx)
            else:
                sd.plotter.plot()

            # Line catalog
            dolinc=False
            if plottype != 'grid' and ( linecat != 'none' and linecat != '' ):
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
                    casalog.post( "Could not find catalog at "+catname, priority = "WARN" )
                    dolinc=False
                if ( dolinc ):
                    if ( len(sprange)>1 ):
                        if ( specunit=='GHz' or specunit=='MHz' ):
                            linc.set_frequency_limits(sprange[0],sprange[1],specunit)
                        else:
                            casalog.post( "sd.linecatalog.set_frequency_limits accepts onlyGHz and MHz", priority = 'WARN' )
                            casalog.post( "continuing without sprange selection on catalog", priority = 'WARN' )
                    if ( linecat != 'all' and linecat != 'ALL' ):
                        # do some molecule selection
                        linc.set_name(linecat)
                    # Plot up the selected part of the line catalog
                    # use doppler offset
                    sd.plotter.plot_lines(linc,doppler=linedop)
                    del linc

        # List observation header
        if header and (not plotstyle or margin==[]):
            # set margin for the header
            sd.plotter._plotter.figure.subplots_adjust(top=0.8)
        datname='Data File:     '+infile
        if plottype == 'grid':
             datname += " (gridded)"
        sd.plotter.print_header(plot=(header and asaplot),fontsize=headsize,
                                logger=True,selstr=ssel,extrastr=datname)
        del ssel, datname

        # Hardcopy
        if (outfile != '' ) and not ( plottype in ['azel','pointing']):
            # currently no way w/o screen display first
            sd.plotter.save(outfile)

        # Do some clean up
        #import gc
        #gc.collect()

            # DONE

    except Exception, instance:
        sdutil.process_exception(instance)
        raise Exception, instance
        return

