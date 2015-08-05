import numpy
import os
import time, datetime
from taskinit import gentools, casalog
import sdutil
ms, sdms, tb = gentools(['ms','sdms','tb'])


from numpy import ma, array, logical_not, logical_and


@sdutil.sdtask_decorator
def tsdfit(infile=None, datacolumn=None, antenna=None, field=None, spw=None, timerange=None, scan=None, pol=None,
           fitfunc=None, fitmode=None, nfit=None, thresh=None, min_nchan=None, avg_limit=None, box_size=None,
           edge=None, outfile=None, overwrite=None):
    casalog.origin('tsdbaseline')

    try:
        if os.path.exists(outfile):
            if overwrite:
                os.system('rm -rf %s' % outfile)
            else:
                raise ValueError(outfile + ' exists.')
        if (fitmode != 'list'):
            raise ValueError, "fitmode='%s' is not supported yet" % fitmode
        if (spw == ''): spw = '*'

        selection = ms.msseltoindex(vis=infile, spw=spw, field=field, 
                                    baseline=str(antenna), time=timerange, 
                                    scan=scan)

        sdms.open(infile)
        sdms.set_selection(spw=sdutil.get_spwids(selection), field=field, 
                           antenna=str(antenna), timerange=timerange, 
                           scan=scan)

        tempfile = 'temp_tsdfit_'+str(datetime.datetime.fromtimestamp(time.time())).replace('-','').replace(' ','').replace(':','')
        if os.path.exists(tempfile):
            tempfile += str(datetime.datetime.fromtimestamp(time.time())).replace('-','').replace(' ','').replace(':','')
            if os.path.exists(tempfile):
                raise Exception('temporary file ' + tempfile + ' exists...')
        tempoutfile = tempfile + '_temp_output_ms'
        if os.path.exists(tempoutfile):
            tempoutfile += str(datetime.datetime.fromtimestamp(time.time())).replace('-','').replace(' ','').replace(':','')
            if os.path.exists(tempoutfile):
                raise Exception('temporary ms file ' + tempoutfile + ' exists...')

        sdms.fit_line(datacolumn=datacolumn, spw=spw, pol=pol, fitfunc=fitfunc,
                      nfit=str(nfit)[1:-1].replace(' ', ''),
                      tempfile=tempfile, tempoutfile=tempoutfile)

        if os.path.exists(tempfile):
            return get_results(tempfile, fitfunc, nfit, outfile)
        else:
            raise Exception('temporary file was unexpectedly not created.')

    except Exception, instance:
        raise Exception, instance
    finally:
        if 'tempfile' in locals() and os.path.exists(tempfile):
            os.system('rm -f %s' % tempfile)
        if 'tempoutfile' in locals() and os.path.exists(tempoutfile):
            os.system('rm -rf %s' % tempoutfile)

def get_results(tempfile, fitfunc, nfit, outfile):
    res = {'cent':[], 'peak':[], 'fwhm':[], 'nfit':[]}
    if (fitfunc == 'gaussian'): func = 'gauss'
    ncomp = sum(nfit)
    iline = 0
    with open(tempfile, 'r') as f:
        outfile_exists = (outfile != '')
        if outfile_exists:
            fout = open(outfile, 'a')
            s = '#SCAN\tTIME\t\tANT\tBEAM\tSPW\tPOL\tFunction\tP0\t\tP1\t\tP2\n'
            fout.write(s)
        
        for line in f:
            component = line.strip().split(':')   # split into each component
            if (0 < ncomp): 
                assert(len(component) == ncomp)
            res['cent'].append([])
            res['peak'].append([])
            res['fwhm'].append([])
            res['nfit'].append(ncomp)
            for icomp in range(ncomp):
                fit_result = component[icomp].split(',')   # split into each parameter
                num_ids = 6 # scan, time, ant, beam, spw, pol
                assert(len(fit_result) == 2*(len(res.keys())-1)+num_ids)
                res['cent'][iline].append([float(fit_result[num_ids+0]), float(fit_result[num_ids+1])])
                res['peak'][iline].append([float(fit_result[num_ids+2]), float(fit_result[num_ids+3])])
                res['fwhm'][iline].append([float(fit_result[num_ids+4]), float(fit_result[num_ids+5])])

                if outfile_exists:
                    s =  fit_result[0] + '\t'       #scanID
                    s += fit_result[1] + '\t'       #time
                    s += fit_result[2] + '\t'       #antennaID
                    s += fit_result[3] + '\t'       #beamID
                    s += fit_result[4] + '\t'       #spwID
                    s += fit_result[5] + '\t'       #polID
                    s += func + str(icomp) + '\t\t' #function
                    s += fit_result[8] + '\t'       #P0 (peak)
                    s += fit_result[6] + '\t'       #P1 (cent)
                    s += fit_result[10] + '\t'      #P2 (fwhm)
                    s += '\n'
                    fout.write(s)
            iline += 1
        
        if outfile_exists: fout.close()
    
    return res


class sdfit_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdfit_worker,self).__init__(**kwargs)

    def parameter_check(self):
        self.doguess = True

    def initialize_scan(self):
        # load the data  without averaging
        sorg = sd.scantable(self.infile,average=False,antenna=self.antenna)
                        
        # restorer
        self.restorer = sdutil.scantable_restore_factory(sorg,
                                                         self.infile,
                                                         self.fluxunit,
                                                         sorg.get_unit(),
                                                         self.frame,
                                                         self.doppler,
                                                         self.restfreq)

        # Select scan and field
        sorg.set_selection(self.get_selector(sorg))

        # this is bit tricky
        # set fluxunit here instead of self.set_to_scan
        # and remove fluxunit attribute to disable additional
        # call of set_fluxunit in self.set_to_scan
        self.scan = sdutil.set_fluxunit(sorg, self.fluxunit, self.telescopeparam, False)
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

        #WORKAROUND for new tasks (in future this should be done in sdutil)
        if not self.timeaverage: self.scanaverage = False
        # average data
        self.scan = sdutil.doaverage(self.scan, self.scanaverage, self.timeaverage, self.tweight, self.polaverage, self.pweight)

        # restore fluxunit
        self.fluxunit = self.fluxunit_saved
        del self.fluxunit_saved

        self.result = dict.fromkeys(['nfit','peak','cent','fwhm'],[])
        self.fitparams = []
        
        if self.assert_no_channel_selection_in_spw(mode='result'):
            maskline_dict = self.scan.parse_spw_selection(self.spw)
            
            valid_spw_list = []
            for (k,v) in maskline_dict.items():
                if len(v) > 0 and numpy.all(numpy.array(map(len, v)) > 0):
                    valid_spw_list.append(k)

            sel_org = self.scan.get_selection()
            for irow in xrange(self.scan.nrow()):
                scantab = self.scan.get_row(irow, insitu=False)
                spw = scantab.getif(0)
                if not spw in valid_spw_list:
                    continue
                maskline = maskline_dict[spw]
                self.maskline = maskline
                self.__set_linelist(scantab)
                self.__fit(scantab)
            ifs_org = sel_org.get_ifs()
            ifs_new = list(set(ifs_org) & set(valid_spw_list))
            sel_org.set_ifs(ifs_new)
            self.scan.set_selection(sel_org)
        else:
            self.maskline = []
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

    def __fit(self, scantab=None):
        # initialize fitter
        self.fitter = sd.fitter()
        if abs(self.plotlevel) > 0:
            self.__init_plot()

        dbw = 1.0
        if scantab is None:
            scantab = self.scan

        current_unit = scantab.get_unit()

        kw = {'thescan':scantab}
        if len(self.defaultmask) > 0: kw['mask'] = self.defaultmask
        self.fitter.set_scan(**kw)
        firstplot = True
    
        for irow in range(scantab.nrow()):
            casalog.post( "start row %d" % (irow) )
            # check for FLAGROW
            if scantab._getflagrow(irow):
                casalog.post( "Row %d is flagged. skip fitting " % irow)
                self.fitparams.append([[0,0,0]])
                self.result['nfit']+=[-1]
                continue
            numlines = self.nlines[irow] if isinstance(self.nlines,list) \
                       else self.nlines

            if numlines == 0:
                self.fitparams.append([[0,0,0]])
                self.result['nfit']+=[-1]
                self.__warn_fit_failed(scantab,irow,'No lines detected.')
                continue
                
            if ( self.fitmode != 'auto'):
                # Get number of things to fit from nfit list
                comps = self.nfit if isinstance(self.nfit,list) else [self.nfit]
                # Drop extra over numlines
                numfit = min(len(comps),numlines)
            ncomps = sum(comps)
        
            casalog.post( "Will fit %d components in %d regions" % (ncomps, numfit) )

            if numfit <= 0:
                self.fitparams.append([[0,0,0]])
                self.result['nfit']+=[-1]
                self.__warn_fit_failed(scantab,irow,'Fit failed.')
                continue

            # Fit the line using numfit gaussians or lorentzians
            # Assume the nfit list matches maskline
            self.fitter.set_function(**{self.fitfunc:ncomps})
            if ( self.doguess ):
                # in auto mode, linelist will be detemined for each spectra
                # otherwise, linelist will be the same for all spectra
                if current_unit != 'channel':
                    xx = scantab._getabcissa(irow)
                    dbw = abs(xx[1]-xx[0])
                self.__initial_guess(scantab,dbw,numfit,comps,irow)
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
                self.__warn_fit_failed(scantab,irow,'Fit failed to converge')

            # plot
            if (irow < 16 and abs(self.plotlevel) > 0):
                self.__plot(irow, goodfit, firstplot)
                firstplot = False
        
    def __initial_guess(self, scantab, dbw, numfit, comps, irow):
        llist = self.linelist[irow] if self.fitmode == 'auto' \
                else self.linelist
        if len(llist) > 0:
            # guesses: [maxlist, cenlist, fwhmlist]
            guesses = [[],[],[]]
            for x in llist:
                x.sort()
                casalog.post( "detected line: "+str(x) ) 
                msk = scantab.create_mask(x, row=irow)
                guess = self.__get_initial_guess(scantab,msk,x,dbw,irow)
                for i in xrange(3):
                    guesses[i] = guesses[i] + [guess[i]]
        else:
            guess = self.__get_initial_guess(scantab,self.defaultmask,[],dbw,irow)
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

    def __get_initial_guess(self, scantab, msk, linerange, dbw, irow):
        sp = array(scantab._getspectrum(irow))
        msk = scantab.get_mask(irow)
        linerange = [int(chan) for chan in linerange]
        if len(linerange) == 0:
            data = ma.masked_array(sp,[ (not val) for val in msk])
            mx = ma.masked_array(range(len(sp)), data.mask)
        else:
            data = ma.masked_array(sp[linerange[0]:linerange[1]+1],
                                   [ (not msk[idx]) for idx in xrange(linerange[0],linerange[1]+1) ])
            mx = ma.masked_array(xrange(linerange[0],linerange[1]+1),
                                 data.mask)
        maxl = data.max()
        suml = data.sum()
        fwhm = maxl if maxl==0.0 else 0.7*abs(suml/maxl*dbw)
        casalog.post("Calculating initial guess of row=%d, range=[%d, %d]" % (irow, mx.min(), mx.max()), priority='DEBUG')
        # center of channel range
        rcen = 0.5*sum(linerange[:2]) if len(linerange) > 1 \
               else scantab.nchan(scantab.getif(irow))/2
        # the valid channel closest to rcen
        #cen = mx[ numpy.where( abs(mx-rcen)==abs(mx-rcen).min() ) ][0]
        cen = mx[ numpy.where(data==maxl)][0]
        casalog.post("Initial guess: [max, center, fwhm] = [%f, %d, %f]" % (maxl, cen, fwhm), priority='DEBUG')
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

    def __set_linelist(self, scantab=None):
        self.defaultmask = []
        self.linelist = []
        self.nlines = 1
        getattr(self,'_set_linelist_%s'%(self.fitmode.lower()))(scantab)

    def _set_linelist_list(self, scantab=None):
        # Assume the user has given a list of lines
        # e.g. maskline=[[3900,4300]] for a single line
        if scantab is None:
            scantab = self.scan
        if ( len(self.maskline) > 0 ):
            # There is a user-supplied channel mask for lines
            # Make sure this is a list-of-lists (e.g. [[1,10],[20,30]])
            self.linelist = self.maskline if isinstance(self.maskline[0],list) \
                else to_list_of_list(self.maskline)
            self.nlines = len(self.linelist)
            self.defaultmask = scantab.create_mask(self.maskline,invert=False)

        casalog.post( "Identified %d regions for fitting" % (self.nlines) )
        casalog.post("Will use these as starting guesses")

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

    def __warn_fit_failed(self,scantab,irow,message=''):
        casalog.post( 'Fitting:' )
        casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(scantab.getscan(irow), scantab.getbeam(irow), scantab.getif(irow), scantab.getpol(irow), scantab.getcycle(irow)) )
        casalog.post( "   %s"%(message), priority = 'WARN' )

def to_list_of_list(l):
    return array(l).reshape(len(l)/2,2).tolist()

