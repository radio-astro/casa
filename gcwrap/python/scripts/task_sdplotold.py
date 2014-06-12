import os

from taskinit import casalog, qatool

import asap as sd
from asap.scantable import is_scantable
import sdutil

@sdutil.sdtask_decorator
def sdplotold(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, beamlist, scanaverage, timeaverage, tweight, polaverage, pweight, kernel, kwidth, plottype, stack, panel, flrange, sprange, linecat, linedop, subplot, colormap, linestyles, linewidth, histogram, center, cell, scanpattern, header, headsize, plotstyle, margin, legendloc, outfile, overwrite):
    with sdutil.sdtask_manager(sdplot_worker, locals()) as worker:
        import inspect
        taskname = inspect.currentframe().f_code.co_name
        if True:
            sdutil.issue_deprecation_warning(taskname)
        worker.initialize()
        worker.execute()
        worker.finalize()


class sdplot_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdplot_worker,self).__init__(**kwargs)

    def initialize_scan(self):
        isScantable = is_scantable(self.infile)

        #load the data without time/pol averaging
        sorg = sd.scantable(self.infile,average=self.scanaverage,antenna=self.antenna)

        rfset = (self.restfreq != '') and (self.restfreq != [])
        doCopy = (self.frame != '') or (self.doppler != '') or rfset \
                 or (self.fluxunit != '' and self.fluxunit != sorg.get_fluxunit()) \
                 or (self.specunit != '' and self.specunit != sorg.get_unit())
        doCopy = doCopy and isScantable

        # A scantable selection
        if type(self.scanlist) == str:
            self.scanlist = sorg.parse_idx_selection("SCAN",self.scanlist)
        if type(self.iflist) == str:
            self.iflist = sorg.parse_idx_selection("IF",self.iflist)
        if type(self.pollist) == str:
            self.pollist = sorg.parse_idx_selection("POL",self.pollist)
        if type(self.beamlist) == str:
            self.beamlist = sorg.parse_idx_selection("BEAM",self.beamlist)
        #sel = self.get_selector()
        sel = self.get_selector_by_list()
        sorg.set_selection(sel)
        self.ssel=sel.__str__()
        del sel

        # Copy scantable when usign disk storage not to modify
        # the original table.
        if doCopy and self.is_disk_storage:
            self.scan = sorg.copy()
        else:
            self.scan = sorg
        del sorg

    def execute(self):
        # set various attributes to self.scan
        self.set_to_scan()

        # Average data if necessary
        self.scan = sdutil.doaverage(self.scan, self.scanaverage, self.timeaverage, self.tweight, self.polaverage, self.pweight)

        # Reload plotter if necessary
        sd.plotter._assert_plotter(action="reload")

        # Set subplot layout
        if self.subplot > 10:
            row = int(self.subplot/10)
            col = (self.subplot % 10)
            sd.plotter.set_layout(rows=row,cols=col,refresh=False)
        else:
            if self.subplot > -1:
                casalog.post(("Invalid subplot value, %d, is ignored. It should be in between 11 and 99." % self.subplot),priority="WARN")
            sd.plotter.set_layout(refresh=False)

        # Set subplot margins
        if self.margin != sd.plotter._margins:
            sd.plotter.set_margin(margin=self.margin,refresh=False)

        # Actual plotting
        getattr(self,'plot_%s'%(self.plottype))()

    def save(self):
        # Hardcopy
        #if (self.outfile != '' ) and not ( self.plottype in ['azel','pointing']):
        if (self.outfile != '' ) and not ( self.plottype in ['azel']):
            # currently no way w/o screen display first
            sd.plotter.save(self.outfile)

    def plot_pointing(self):
        kw = {'scan': self.scan}
        #if self.outfile != '': kw['outfile'] = self.outfile
        #sd.plotter.plotpointing(**kw)
        # New pointing plot
        # Set colormap, linestyles, and linewidth of plots
        self.__setup_plotter()
        colbydict = {"t": "type", "s": "scan", "i": "if",
                     "p": "pol", "b": "beam"}
        kw['colorby'] = colbydict[self.stack[0].lower()]
        kw['showline'] = self.scanpattern
        sd.plotter.plotpointing2(**kw)
        
        self.__print_header(asaplot=False)

    def plot_azel(self):
        kw = {'scan': self.scan}
        if self.outfile != '': kw['outfile'] = self.outfile
        sd.plotter.plotazel(**kw)

        self.__print_header(asaplot=False)

    def plot_totalpower(self):
        sd.plotter.plottp(self.scan)

        self.__print_header(asaplot=False)

    def plot_grid(self):
        refresh=False
        (nx,ny,cellx,celly,mapcenter) = self.__get_grid_parameters(True)
        #print "Gridding scantable: npix = (%d, %d), cell = (%s, %s), center = %s" % (nx, ny, cellx, celly, mapcenter)
        self.__dogrid(nx, ny, cellx, celly, mapcenter)

        # Now set fluxunit, specunit, frame, and doppler
        sdutil.set_fluxunit(self.scan, self.fluxunit, 'fix')
        sdutil.set_spectral_unit(self.scan, self.specunit)
        sdutil.set_freqframe(self.scan, self.frame)
        sdutil.set_doppler(self.scan, self.doppler)

        if self.scan.nchan()==1:
            errmsg="Trying to plot the continuum/total power data in 'spectra' mode, please use other plottype options" 
            raise Exception,errmsg

        # Smooth the spectrum (if desired)
        self.__dosmooth()

        sd.plotter.set_data(self.scan,refresh=refresh)

        # Set colormap, linestyles, and linewidth of plots
        self.__setup_plotter()
        # Need to specify center and spacing in the epoch and
        # unit of DIRECTION column (currently assuming J2000 and rad)
        # TODO: need checking epoch and unit of DIRECTION in scantable
        spacing = []
        if cellx != '': spacing.append(cellx)
        if celly != '': spacing.append(celly)
        #sd.plotter.plotgrid(center=crad,spacing=spacing,rows=ny,cols=nx)
        sd.plotter.plotgrid(center=mapcenter,spacing=spacing,rows=ny,cols=nx)

        self.__print_header(asaplot=True)

    def plot_spectra(self):
        asaplot=True
        if self.scan.nchan()==1:
            errmsg="Trying to plot the continuum/total power data in 'spectra' mode, please use other plottype options" 
            raise Exception,errmsg

        # Smooth the spectrum (if desired)
        self.__dosmooth()

        # Plot final spectrum
        # each IF is separate panel, pols stacked
        refresh=False

        # Set colormap, linestyles, and linewidth of plots
        self.__setup_plotter()
        
        # The actual plotting
        sd.plotter.set_data(self.scan,refresh=refresh)
        sd.plotter.plot()

        # Line catalog
        dolinc=False
        if  self.linecat != 'none' and self.linecat != '':
            self.__overlay_linecatalog()

        self.__print_header(asaplot=True)

    def __setup_plotter(self):
        refresh = False
        sd.plotter.set_mode(stacking=self.stack,panelling=self.panel,refresh=refresh)
        ncolor = 0
        cm_default = "green red black cyan magenta orange blue purple yellow pink"
        ls_default = "line"
        if len(self.colormap) < 1: self.colormap = 'none'
        if len(self.linestyles) < 1: self.linestyles = 'none'
        if self.colormap != 'none':
            # user defined colormap
            colmap = self.colormap
            ncolor = len(colmap.split())
        elif self.linestyles == 'none':
            # no colormap and linestyle. using default colormap
            colmap = cm_default
            ucm = sd.rcParams['plotter.colours']
            if isinstance(ucm,str) and len(ucm) > 0: colmap = ucm
            ncolor = len(colmap.split())
            del ucm
        else:
            # user defined linestyle
            colmap = "black"
            casalog.post("plot colour is set to '%s'" % colmap)

        if self.linestyles != 'none':
            # user defined linestyle
            lines = self.linestyles
        elif ncolor <= 1:
            # single color plot. generate variation in linestyle
            lines = "line dashed dotted dashdot"
            uls = sd.rcParams['plotter.linestyles']
            if isinstance(uls,str) and len(uls) > 0: lines = uls
            del uls
        else:
            # multi color plot. 
            lines = None

        if isinstance(self.linewidth,int) or isinstance (self.linewidth,float):
            lwidth = self.linewidth
        else:
            casalog.post("Invalid linewidth. linewidth is ignored and set to 1.", priority='WARN')
            lwidth = 1

        # set plot colors
        if ncolor > 1 and lines is not None:
            casalog.post("'linestyles' is valid only for single colour plot.\n...Ignoring 'linestyles'.", priority='WARN')
        if not lines: lines = ls_default

        sd.plotter.set_colors(colmap,refresh=refresh)

        # set linestyles and/or linewidth
        # so far, linestyles can be specified only if a color is assigned
        #if lines is not None or linewidth is not None:
        #        sd.plotter.set_linestyles(lines, linewidth,refresh=refresh)
        sd.plotter.set_linestyles(lines, lwidth,refresh=refresh)
        # Plot red x-axis at y=0 (currently disabled)
        # sd.plotter.axhline(color='r',linewidth=2)
        sd.plotter.set_histogram(hist=self.histogram,refresh=refresh)

        # Set axis ranges (if requested)
        if len(self.flrange)==1:
            #print "flrange needs 2 limits - ignoring"
            casalog.post( "flrange needs 2 limits - ignoring" )
        if len(self.sprange)==1:
            #print "sprange needs 2 limits - ignoring"
            casalog.post( "sprange needs 2 limits - ignoring" )
        if ( len(self.sprange) > 1 ):
            if ( len(self.flrange) > 1 ):
                sd.plotter.set_range(self.sprange[0],self.sprange[1],self.flrange[0],self.flrange[1],refresh=refresh)
            else:
                sd.plotter.set_range(self.sprange[0],self.sprange[1],refresh=refresh)
        elif ( len(self.flrange) > 1 ):
            sd.plotter.set_range(ystart=self.flrange[0],yend=self.flrange[1],refresh=refresh)
        else:
            # Set default range explicitly (in case range was ever set)
            sd.plotter.set_range(refresh=refresh)

        # legend position
        loc=1
        if self.plotstyle: loc=self.legendloc
        sd.plotter.set_legend(mode=loc,refresh=refresh) 

    def __overlay_linecatalog(self):
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
            if ( len(self.sprange)>1 ):
                if ( specunit=='GHz' or specunit=='MHz' ):
                    linc.set_frequency_limits(sprange[0],sprange[1],specunit)
                else:
                    casalog.post( "sd.linecatalog.set_frequency_limits accepts onlyGHz and MHz", priority = 'WARN' )
                    casalog.post( "continuing without sprange selection on catalog", priority = 'WARN' )
            if ( self.linecat != 'all' and self.linecat != 'ALL' ):
                # do some molecule selection
                linc.set_name(self.linecat)
            # Plot up the selected part of the line catalog
            # use doppler offset
            sd.plotter.plot_lines(linc,doppler=self.linedop)
            del linc

    def __print_header(self, asaplot=False):
        # List observation header
        if self.header and (not self.plotstyle or self.margin==[]):
            # set margin for the header
            sd.plotter._plotter.figure.subplots_adjust(top=0.8)
        datname='Data File:     '+self.infile
        if self.plottype == 'grid':
             datname += " (gridded)"
        sd.plotter.print_header(plot=(self.header and asaplot),fontsize=self.headsize,
                                #logger=True,selstr=self.ssel,extrastr=datname)
                                logger=True,extrastr=datname)

    def __dosmooth(self):
        if self.kernel == '': self.kernel = 'none'
        if ( self.kernel != 'none' and (not (self.kwidth<=0 and self.kernel!='hapnning'))):
            casalog.post( "Smoothing spectrum with kernel "+self.kernel )
            self.scan.smooth(self.kernel,self.kwidth)

    def __dogrid(self, nx, ny, cellx, celly, center):
        # Do only the first IFNO and POLNO
        ifnos = self.scan.getifnos()
        polnos = self.scan.getpolnos()
        if len(ifnos) > 1 or len(polnos) > 1:
            casalog.post("Only the first IFNO (%d) and POLNO (%d) is plotted." % (ifnos[0], polnos[0]),priority="WARN")


        gridder = sd.asapgrid2(self.scan)
        del self.scan
        gridder.setIF(ifnos[0])
        gridder.setPolList([polnos[0]])
        gridder.defineImage(center=center,cellx=cellx,celly=celly,nx=nx,ny=ny)
        gridder.setFunc(func='BOX')
        gridder.setWeight('uniform')
        gridder.grid()

        self.scan = gridder.getResult()
        del gridder

    def __get_grid_parameters(self, calc=False):
        # CAS-5410 Use private tools inside task scripts
        qa = qatool()

        (nx,ny) = self.__get_mapsize()
        mapcenter = sdutil.get_map_center(self.center)
        (cellx,celly) = sdutil.get_cellx_celly(self.cell)
        if calc and \
               ((mapcenter.split() != 3) or not qa.compare(cellx, "rad")):
            #(mapcenter,cellx,celly) = self.__calc_center_and_cell(mapcenter, cellx, celly)
            (mapcenter,cellx,celly) = self.__get_center_and_cell(nx, ny, mapcenter, cellx, celly)
        return (nx,ny,cellx,celly,mapcenter)
        
    def __get_mapsize(self):
        if self.subplot < 11:
            casalog.post("Setting default subplot layout (1x1).",priority="WARN")
            self.subplot = 11
        nx = (self.subplot % 10)
        ny = int(self.subplot/10)
        return (nx,ny)

    def __get_center_and_cell(self, nx, ny, center, cellx, celly):
        from asap._asap import plothelper
        ph = ph = plothelper(self.scan)
        ph.set_grid(nx, ny, cellx=cellx, celly=celly, center=center,
                    projname="SIN")
        gcenter = ph.get_gref()
        gcell = ph.get_gcellval()
        return gcenter, ("%erad" % gcell[0]), ("%erad" % gcell[1])

    def __calc_center_and_cell(self, center, cellx, celly):
        from numpy import array
        # CAS-5410 Use private tools inside task scripts
        qa = qatool()

        # Get map extent (in radian)
        dirarr = array(self.scan.get_directionval()).transpose()
        xmin = dirarr[0].min()
        xmax = dirarr[0].max()
        ymin = dirarr[1].min()
        ymax = dirarr[1].max()
        del dirarr
        # center of directions
        dircent = [0.5*(xmax + xmin), 0.5*(ymax + ymin)]
        centx = None
        centy = None
        # center is not specified
        if center.split() != 3:
            # set map center (string) to center of directions
            center = sdutil.get_map_center(dircent, unit= "rad")
            # direction center in unit of radian
            (centx, centy) = dircent
        # cell is not specified
        if not qa.compare(cellx, "rad"):
            if not centx:
                # center is given. Get the value in radian
                lcent = center.split()
                centx = qa.convert(qa.toangle(lcent[1]), "rad")
                centy = qa.convert(qa.toangle(lcent[2]), "rad")
                # make sure centx is in +-pi of pointing center
                rotnum = round(abs(centx - dircent[0])/(2*pi))
                if centx < dircent[0]: rotnum *= -1
                centx -= rotnum*2*pi
            wx = 2. * max(abs(xmax-centx), abs(xmin-centx))
            wy = 2. * max(abs(ymax-centy), abs(ymin-centy))
            #print "mapsize = [%frad, %frad]" % (wx, wy)
            from numpy import cos
            (nx,ny) = self.__get_mapsize()
            cellx = qa.quantity(wx/float(max(nx-1,1))*cos(centy), "rad")
            celly = qa.quantity(wy/float(max(ny-1,1)), "rad")
        return (center, qa.tos(cellx), qa.tos(celly))

