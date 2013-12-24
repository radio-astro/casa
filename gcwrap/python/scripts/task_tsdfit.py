from taskinit import casalog
from numpy import ma, array, logical_not, logical_and

import asap as sd
import sdutil

@sdutil.sdtask_decorator
def tsdfit(infile, antenna, fluxunit, telescopeparam, field, spw, restfreq, frame, doppler, scan, pol, scanaverage, timeaverage, tweight, polaverage, pweight, fitfunc, fitmode, nfit, thresh, min_nchan, avg_limit, box_size, edge, outfile, overwrite, plotlevel):
    with sdutil.sdtask_manager(sdfit_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        
        return worker.result


class sdfit_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdfit_worker,self).__init__(**kwargs)

    def parameter_check(self):
        self.doguess = not ((self.fitmode.lower()=='list') and (self.invertmask))

    def initialize_scan(self):
        # load the data  without averaging
        sorg = sd.scantable(self.infile,average=False,antenna=self.antenna)
                        
        # restorer
        self.restorer = sdutil.scantable_restore_factory(sorg,
                                                         self.infile,
                                                         self.fluxunit,
                                                         self.specunit,
                                                         self.frame,
                                                         self.doppler,
                                                         self.restfreq)

        # Select scan and field
        #sorg.set_selection(self.get_selector())
        sorg.set_selection(self.get_selector_by_list())

        # this is bit tricky
        # set fluxunit here instead of self.set_to_scan
        # and remove fluxunit attribute to disable additional
        # call of set_fluxunit in self.set_to_scan
        self.scan = sdutil.set_fluxunit(sorg, self.fluxunit, self.telescopeparm, False)
        self.fluxunit_saved = self.fluxunit
        del self.fluxunit

        if self.scan:
            # Restore flux unit in original table before deleting
            self.restorer.restore()
            del self.restorer
            self.restorer = None
        else:
            self.scan = sorg

    def execute(self):
        self.set_to_scan()

        # restore fluxunit
        self.fluxunit = self.fluxunit_saved
        del self.fluxunit_saved

        self.__set_linelist()

        self.__fit()

    def save(self):
        # Store fit
        if ( self.outfile != '' ):
            self.__store_fit()

    def cleanup(self):
        # restore original scantable
        if hasattr(self,'restorer') and self.restorer is not None:
            self.restorer.restore()

    def __fit(self):
        # initialize fitter
        self.fitter = sd.fitter()
        self.result = dict.fromkeys(['nfit','peak','cent','fwhm'],[])
        self.fitparams = []
        if abs(self.plotlevel) > 0:
            self.__init_plot()

        dbw = 1.0
        current_unit = self.scan.get_unit()
        kw = {'thescan':self.scan}
        if len(self.defaultmask) > 0: kw['mask'] = self.defaultmask
        self.fitter.set_scan(**kw)
        firstplot = True
    
        for irow in range(self.scan.nrow()):
            casalog.post( "start row %d" % (irow) )
            numlines = self.nlines[irow] if isinstance(self.nlines,list) \
                       else self.nlines

            if numlines == 0:
                self.fitparams.append([[0,0,0]])
                self.result['nfit']+=[-1]
                self.__warn_fit_failed(irow,'No lines detected.')
                continue
                
            if ( self.fitmode == 'auto'):
                # Auto mode - one comp per line region
                # Overwriting user-supplied nfit
                numfit = numlines
                comps = [1 for i in xrange(numlines)]
            else:
                # Get number of things to fit from nfit list
                comps = self.nfit if isinstance(self.nfit,list) else [self.nfit]
                # Drop extra over numlines
                numfit = min(len(comps),numlines)
            ncomps = sum(comps)
        
            casalog.post( "Will fit %d components in %d regions" % (ncomps, numfit) )

            if numfit <= 0:
                self.fitparams.append([[0,0,0]])
                self.result['nfit']+=[-1]
                self.__warn_fit_failed(irow,'Fit failed.')
                continue

            # Fit the line using numfit gaussians or lorentzians
            # Assume the nfit list matches maskline
            self.fitter.set_function(**{self.fitfunc:ncomps})
            if ( self.doguess ):
                # in auto mode, linelist will be detemined for each spectra
                # otherwise, linelist will be the same for all spectra
                if current_unit != 'channel':
                    xx = self.scan._getabcissa(irow)
                    dbw = abs(xx[1]-xx[0])
                self.__initial_guess(dbw,numfit,comps,irow)
            else:
                # No guesses
                casalog.post( "Fitting lines without starting guess" )

            # Now fit
            self.fitter.fit(row=irow)
            fstat = self.fitter.get_parameters()

            # Check for convergence
            goodfit = ( len(fstat['errors']) > 0 )
            if ( goodfit ):
                self.__update_params(ncomps)
            else:
                # Did not converge
                self.result['nfit'] += [-ncomps]
                self.fitparams.append([[0,0,0]])
                self.__warn_fit_failed(irow,'Fit failed to converge')

            # plot
            if (irow < 16 and abs(self.plotlevel) > 0):
                self.__plot(irow, goodfit, firstplot)
                firstplot = False
        
    def __initial_guess(self, dbw, numfit, comps, irow):
        llist = self.linelist[irow] if self.fitmode == 'auto' \
                else self.linelist
        if len(llist) > 0:
            # guesses: [maxlist, cenlist, fwhmlist]
            guesses = [[],[],[]]
            for x in llist:
                x.sort()
                casalog.post( "detected line: "+str(x) ) 
                msk = self.scan.create_mask(x, row=irow)
                guess = self.__get_initial_guess(msk,x,dbw,irow)
                for i in xrange(3):
                    guesses[i] = guesses[i] + [guess[i]]
        else:
            guess = self.__get_initial_guess(self.defaultmask,[],dbw,irow)
            guesses = [[guess[i]] for i in xrange(3)]

        # Guesses using max, cen, and fwhm=0.7*eqw
        # NOTE: should there be user options here?
        n = 0
        for i in range(numfit):
            # cannot guess for multiple comps yet
            if ( comps[i] == 1 ):
                # use guess
                #getattr(self.fitter,'set_%s_parameters'%(self.fitfunc))(maxl[i], cenl[i], fwhm[i], component=n)
                guess = (guesses[k][i] for k in xrange(3))
                getattr(self.fitter,'set_%s_parameters'%(self.fitfunc))(*guess, component=n)
            n += comps[i]

    def __get_initial_guess(self, msk, linerange, dbw, irow):
        [maxl,suml] = [self.scan._math._statsrow(self.scan,msk,st,irow)[0] \
                       for st in ['max','sum']]
        fwhm = maxl if maxl==0.0 else 0.7*abs(suml/maxl*dbw)
        cen = 0.5*sum(linerange[:2]) if len(linerange) > 1 \
              else self.scan.nchan(self.scan.getif(irow))/2
        return (maxl,cen,fwhm)

    def __update_params(self, ncomps):
        # Retrieve fit parameters
        self.result['nfit'] = self.result['nfit'] + [ncomps]
        keys = ['peak','cent','fwhm']
        retl = dict.fromkeys(keys,[])
        nkeys = len(keys)
        parameters = self.fitter.get_parameters()
        params = parameters['params']
        errors = parameters['errors']
        for i in range(ncomps):
            offset = i*nkeys
            for j in xrange(nkeys):
                key = keys[j]
                retl[key] = retl[key] + [[params[offset+j],\
                                          errors[offset+j]]]
        for key in keys:
            self.result[key] = self.result[key] + [retl[key]]
        pars = parameters['params']
        npars = len(pars) / ncomps
        self.fitparams.append(list(array(pars).reshape((ncomps,npars))))

    def __set_linelist(self):
        self.defaultmask = []
        self.linelist = []
        self.nlines = 1
        getattr(self,'_set_linelist_%s'%(self.fitmode.lower()))()

    def _set_linelist_list(self):
        # Assume the user has given a list of lines
        # e.g. maskline=[[3900,4300]] for a single line
        if ( len(self.maskline) > 0 ):
            # There is a user-supplied channel mask for lines
            if ( not self.invertmask ):
                # Make sure this is a list-of-lists (e.g. [[1,10],[20,30]])
                self.linelist = self.maskline if isinstance(self.maskline[0],list) \
                                else to_list_of_list(self.maskline)
                self.nlines = len(self.linelist)
            self.defaultmask = self.scan.create_mask(self.maskline,invert=self.invertmask)
        else:
            # Use whole region
            if self.invertmask:
                msg='No channel is selected because invertmask=True. Exit without fittinging.'
                raise Exception(msg)

        casalog.post( "Identified %d regions for fitting" % (self.nlines) )
        if ( self.invertmask ):
            casalog.post("No starting guesses available")
        else:
            casalog.post("Will use these as starting guesses")

    def _set_linelist_interact(self):
        # Interactive masking
        new_mask = sdutil.init_interactive_mask(self.scan,
                                                self.maskline,
                                                self.invertmask)
        self.defaultmask = sdutil.get_interactive_mask(new_mask,
                                                       purpose='to fit lines')
        self.linelist=self.scan.get_masklist(self.defaultmask)
        self.nlines=len(self.linelist)
        if self.nlines < 1:
            msg='No channel is selected. Exit without fittinging.'
            raise Exception(msg)
        print '%d region(s) is selected as a linemask' % self.nlines
        print 'The final mask list ('+self.scan._getabcissalabel()+') ='+str(self.linelist)
        print 'Number of line(s) to fit: nfit =',self.nfit
        ans=raw_input('Do you want to reassign nfit? [N/y]: ')
        if (ans.upper() == 'Y'):
            ans=input('Input nfit = ')
            if type(ans) == list: self.nfit = ans
            elif type(ans) == int: self.nfit = [ans]
            else:
                msg='Invalid definition of nfit. Setting nfit=[1] and proceed.'
                casalog.post(msg, priority='WARN')
                self.nfit = [1]
            casalog.post('List of line number reassigned.\n   nfit = '+str(self.nfit))
        sdutil.finalize_interactive_mask(new_mask)

    def _set_linelist_auto(self):
        # Fit mode AUTO and in channel mode
        casalog.post( "Trying AUTO mode - find line channel regions" )
        if ( len(self.maskline) > 0 ):
            # There is a user-supplied channel mask for lines
            self.defaultmask=self.scan.create_mask(self.maskline,
                                                   invert=self.invertmask)
            
        # Use linefinder to find lines
        casalog.post( "Using linefinder" )
        fl=sd.linefinder()
        fl.set_scan(self.scan)
        # This is the tricky part
        # def  fl.set_options(threshold=1.732,min_nchan=3,avg_limit=8,box_size=0.2)
        # e.g. fl.set_options(threshold=5,min_nchan=3,avg_limit=4,box_size=0.1) seem ok?
        fl.set_options(threshold=self.thresh,min_nchan=self.min_nchan,avg_limit=self.avg_limit,box_size=self.box_size)
        # Now find the lines for each row in scantable
        self.nlines=[]
        for irow in range(self.scan.nrow()):
            self.nlines.append(fl.find_lines(mask=self.defaultmask,nRow=irow,edge=self.edge))
            # Get ranges    
            ptout="SCAN[%d] IF[%d] POL[%d]: " %(self.scan.getscan(irow), self.scan.getif(irow), self.scan.getpol(irow))
            if ( self.nlines[irow] > 0 ):
                ll = fl.get_ranges()
                casalog.post( ptout+"Found %d lines at %s" % (self.nlines[irow], str(ll) ) )
            else:
                ll = ()
                casalog.post( ptout+"Nothing found.", priority = 'WARN' )

            # This is a linear list of pairs of values, so turn these into a list of lists
            self.linelist.append(to_list_of_list(ll))

        # Done with linefinder
        casalog.post( "Finished linefinder." )    

    def __store_fit(self):
        outf = file(sdutil.get_abspath(self.outfile),'w')

        #header 
        header="#%-4s %-4s %-4s %-12s " %("SCAN", "IF", "POL", "Function")
        numparam=3     # gaussian fitting is assumed (max, center, fwhm)
        for i in xrange(numparam):
            header+='%-12s '%('P%d'%(i))
        outf.write(header+'\n')

        #data
        for i in xrange(len(self.fitparams)):
            dattmp=" %-4d %-4d %-4d " \
                    %(self.scan.getscan(i), self.scan.getif(i), self.scan.getpol(i))
            for j in xrange(len(self.fitparams[i])):
                if ( self.fitparams[i][j][0]!=0.0): 
                    datstr=dattmp+'%-12s '%('%s%d'%(self.fitfunc,j))
                    for k in xrange(len(self.fitparams[i][j])):
                        datstr+="%3.8f " %(self.fitparams[i][j][k])
                    outf.write(datstr+'\n')
                        
        outf.close()

    def __init_plot(self):
        n = self.scan.nrow()
        if n > 16:
            casalog.post( 'Only first 16 results are plotted.', priority = 'WARN' )
            n = 16
        
        # initialize plotter
        from matplotlib import rc as rcp
        rcp('lines', linewidth=1)
        if not (self.fitter._p and self.fitter._p._alive()):
            self.fitter._p = sdutil.get_plotter(self.plotlevel)
        self.fitter._p.hold()
        self.fitter._p.clear()
        # set nrow and ncol (maximum 4x4)
        self.fitter._p.set_panels(rows=n, cols=0, ganged=False)
        casalog.post( 'nrow,ncol= %d,%d' % (self.fitter._p.rows, self.fitter._p.cols ) )
        self.fitter._p.palette(0,["#777777", "#dddddd", "red", "orange", "purple", "green", "magenta", "cyan"])        
        
    def __plot(self, irow, fitted, firstplot=False ):
        if firstplot:
            labels = ['Spectrum', 'Selected Region', 'Residual', 'Fit']
        else:
            labels = ['spec', 'select', 'res', 'fit']
        myp = self.fitter._p

        myp.subplot(irow)
        # plot spectra
        x = self.fitter.data._getabcissa(irow)
        y = self.fitter.data._getspectrum(irow)
        mr = self.fitter.data._getflagrow(irow)
        if mr: # a whole spectrum is flagged
            themask = False
        else:
            msk = array(self.fitter.data._getmask(irow))
            fmsk = array(self.fitter.mask)
            themask = logical_and(msk,fmsk)
            del msk, fmsk
        # plot masked region if any of channel is not in fit range.
        idx = 0
        if mr or (not all(themask)):
            # dumped region
            plot_line(myp,x,y,themask,label=labels[0],color=1)
            idx = 1
        themask = logical_not(themask)
        
        # fitted region
        plot_line(myp,x,y,themask,label=labels[idx],color=0,scale=True)

        # plot fitted result
        if ( fitted ):
            # plot residual
            if ( self.plotlevel==2 ):
                plot_line(myp,x,self.fitter.get_residual(),themask,label=labels[2],color=7)
            # plot fit
            plot_line(myp,x,self.fitter.fitter.getfit(),themask,label=labels[3],color=2)

        if ( irow == 0 ):
                tlab=self.fitter.data._getsourcename(self.fitter._fittedrow)
                myp.set_axes('title',tlab)
        if (irow%myp.rows == 0):
                ylab=self.fitter.data._get_ordinate_label()
                myp.set_axes('ylabel',ylab)
        if (irow/myp.rows == myp.cols-1):
                xlab=self.fitter.data._getabcissalabel(self.fitter._fittedrow)
                myp.set_axes('xlabel',xlab)
        myp.release()

    def __warn_fit_failed(self,irow,message=''):
        casalog.post( 'Fitting:' )
        casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(self.scan.getscan(irow), self.scan.getbeam(irow), self.scan.getif(irow), self.scan.getpol(irow), self.scan.getcycle(irow)) )
        casalog.post( "   %s"%(message), priority = 'WARN' )

def plot_line(plotter,x,y,msk,label,color,colormap=None,scale=False):
    plotter.set_line(label=label)
    plotter.palette(color,colormap)
    my=ma.masked_array(y,msk)
    if scale:
        xlim=[min(x),max(x)]
        ylim=[min(my),max(my)]
        wy=ylim[1]-ylim[0]
        ylim=[ylim[0]-wy*0.1,ylim[1]+wy*0.1]
        plotter.axes.set_xlim(xlim)
        plotter.axes.set_ylim(ylim)
    plotter.plot(x,my)

def to_list_of_list(l):
    return array(l).reshape(len(l)/2,2).tolist()

