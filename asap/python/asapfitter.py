import _asap
from asap.parameters import rcParams
from asap.logging import asaplog, asaplog_post_dec
from asap.utils import _n_bools, mask_and


class fitter:
    """
    The fitting class for ASAP.
    """
    def __init__(self):
        """
        Create a fitter object. No state is set.
        """
        self.fitter = _asap.fitter()
        self.x = None
        self.y = None
        self.mask = None
        self.fitfunc = None
        self.fitfuncs = None
        self.fitted = False
        self.data = None
        self.components = 0
        self._fittedrow = 0
        self._p = None
        self._selection = None
        self.uselinear = False

    def set_data(self, xdat, ydat, mask=None):
        """
        Set the absissa and ordinate for the fit. Also set the mask
        indicationg valid points.
        This can be used for data vectors retrieved from a scantable.
        For scantable fitting use 'fitter.set_scan(scan, mask)'.
        Parameters:
            xdat:    the abcissa values
            ydat:    the ordinate values
            mask:    an optional mask

        """
        self.fitted = False
        self.x = xdat
        self.y = ydat
        if mask == None:
            self.mask = _n_bools(len(xdat), True)
        else:
            self.mask = mask
        return

    @asaplog_post_dec
    def set_scan(self, thescan=None, mask=None):
        """
        Set the 'data' (a scantable) of the fitter.
        Parameters:
            thescan:     a scantable
            mask:        a msk retrieved from the scantable
        """
        if not thescan:
            msg = "Please give a correct scan"
            raise TypeError(msg)
        self.fitted = False
        self.data = thescan
        self.mask = None
        if mask is None:
            self.mask = _n_bools(self.data.nchan(), True)
        else:
            self.mask = mask
        return

    @asaplog_post_dec
    def set_function(self, **kwargs):
        """
        Set the function to be fit.
        Parameters:
            poly:    use a polynomial of the order given with nonlinear least squares fit
            lpoly:   use polynomial of the order given with linear least squares fit
            gauss:   fit the number of gaussian specified
            lorentz: fit the number of lorentzian specified
        Example:
            fitter.set_function(poly=3)  # will fit a 3rd order polynomial via nonlinear method
            fitter.set_function(lpoly=3)  # will fit a 3rd order polynomial via linear method
            fitter.set_function(gauss=2) # will fit two gaussians
            fitter.set_function(lorentz=2) # will fit two lorentzians
        """
        #default poly order 0
        n=0
        if kwargs.has_key('poly'):
            self.fitfunc = 'poly'
            n = kwargs.get('poly')
            self.components = [n]
            self.uselinear = False
        elif kwargs.has_key('lpoly'):
            self.fitfunc = 'poly'
            n = kwargs.get('lpoly')
            self.components = [n]
            self.uselinear = True
        elif kwargs.has_key('gauss'):
            n = kwargs.get('gauss')
            self.fitfunc = 'gauss'
            self.fitfuncs = [ 'gauss' for i in range(n) ]
            self.components = [ 3 for i in range(n) ]
            self.uselinear = False
        elif kwargs.has_key('lorentz'):
            n = kwargs.get('lorentz')
            self.fitfunc = 'lorentz'
            self.fitfuncs = [ 'lorentz' for i in range(n) ]
            self.components = [ 3 for i in range(n) ]
            self.uselinear = False
        else:
            msg = "Invalid function type."
            raise TypeError(msg)

        self.fitter.setexpression(self.fitfunc,n)
        self.fitted = False
        return

    @asaplog_post_dec
    def fit(self, row=0, estimate=False):
        """
        Execute the actual fitting process. All the state has to be set.
        Parameters:
            row:        specify the row in the scantable
            estimate:   auto-compute an initial parameter set (default False)
                        This can be used to compute estimates even if fit was
                        called before.
        Example:
            s = scantable('myscan.asap')
            s.set_cursor(thepol=1)        # select second pol
            f = fitter()
            f.set_scan(s)
            f.set_function(poly=0)
            f.fit(row=0)                  # fit first row
        """
        if ((self.x is None or self.y is None) and self.data is None) \
               or self.fitfunc is None:
            msg = "Fitter not yet initialised. Please set data & fit function"
            raise RuntimeError(msg)

        else:
            if self.data is not None:
                self.x = self.data._getabcissa(row)
                self.y = self.data._getspectrum(row)
                self.mask = mask_and(self.mask, self.data._getmask(row))
                asaplog.push("Fitting:")
                i = row
                out = "Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]" % (self.data.getscan(i),
                                                                      self.data.getbeam(i),
                                                                      self.data.getif(i),
                                                                      self.data.getpol(i),
                                                                      self.data.getcycle(i))
                asaplog.push(out,False)
        self.fitter.setdata(self.x, self.y, self.mask)
        if self.fitfunc == 'gauss' or self.fitfunc == 'lorentz':
            ps = self.fitter.getparameters()
            if len(ps) == 0 or estimate:
                self.fitter.estimate()
        fxdpar = list(self.fitter.getfixedparameters())
        if len(fxdpar) and fxdpar.count(0) == 0:
             raise RuntimeError,"No point fitting, if all parameters are fixed."
        if self.uselinear:
            converged = self.fitter.lfit()
        else:
            converged = self.fitter.fit()
        if not converged:
            raise RuntimeError,"Fit didn't converge."
        self._fittedrow = row
        self.fitted = True
        return

    def store_fit(self, filename=None):
        """
        Save the fit parameters.
        Parameters:
            filename:    if specified save as an ASCII file, if None (default)
                         store it in the scnatable
        """
        if self.fitted and self.data is not None:
            pars = list(self.fitter.getparameters())
            fixed = list(self.fitter.getfixedparameters())
            from asap.asapfit import asapfit
            fit = asapfit()
            fit.setparameters(pars)
            fit.setfixedparameters(fixed)
            fit.setfunctions(self.fitfuncs)
            fit.setcomponents(self.components)
            fit.setframeinfo(self.data._getcoordinfo())
            if filename is not None:
                import os
                filename = os.path.expandvars(os.path.expanduser(filename))
                if os.path.exists(filename):
                    raise IOError("File '%s' exists." % filename)
                fit.save(filename)
            else:
                self.data._addfit(fit,self._fittedrow)

    @asaplog_post_dec
    def set_parameters(self,*args,**kwargs):
        """
        Set the parameters to be fitted.
        Parameters:
              params:    a vector of parameters
              fixed:     a vector of which parameters are to be held fixed
                         (default is none)
              component: in case of multiple gaussians, the index of the
                         component
        """
        component = None
        fixed = None
        params = None

        if len(args) and isinstance(args[0],dict):
            kwargs = args[0]
        if kwargs.has_key("fixed"): fixed = kwargs["fixed"]
        if kwargs.has_key("params"): params = kwargs["params"]
        if len(args) == 2 and isinstance(args[1], int):
            component = args[1]
        if self.fitfunc is None:
            msg = "Please specify a fitting function first."
            raise RuntimeError(msg)
        if (self.fitfunc == "gauss" or self.fitfunc == 'lorentz') and component is not None:
            if not self.fitted and sum(self.fitter.getparameters()) == 0:
                pars = _n_bools(len(self.components)*3, False)
                fxd = _n_bools(len(pars), False)
            else:
                pars = list(self.fitter.getparameters())
                fxd = list(self.fitter.getfixedparameters())
            i = 3*component
            pars[i:i+3] = params
            fxd[i:i+3] = fixed
            params = pars
            fixed = fxd
        self.fitter.setparameters(params)
        if fixed is not None:
            self.fitter.setfixedparameters(fixed)
        return

    @asaplog_post_dec
    def set_gauss_parameters(self, peak, centre, fwhm,
                             peakfixed=0, centrefixed=0,
                             fwhmfixed=0,
                             component=0):
        """
        Set the Parameters of a 'Gaussian' component, set with set_function.
        Parameters:
            peak, centre, fwhm:  The gaussian parameters
            peakfixed,
            centrefixed,
            fwhmfixed:           Optional parameters to indicate if
                                 the paramters should be held fixed during
                                 the fitting process. The default is to keep
                                 all parameters flexible.
            component:           The number of the component (Default is the
                                 component 0)
        """
        if self.fitfunc != "gauss":
            msg = "Function only operates on Gaussian components."
            raise ValueError(msg)
        if 0 <= component < len(self.components):
            d = {'params':[peak, centre, fwhm],
                 'fixed':[peakfixed, centrefixed, fwhmfixed]}
            self.set_parameters(d, component)
        else:
            msg = "Please select a valid  component."
            raise ValueError(msg)

    @asaplog_post_dec
    def set_lorentz_parameters(self, peak, centre, fwhm,
                             peakfixed=0, centrefixed=0,
                             fwhmfixed=0,
                             component=0):
        """
        Set the Parameters of a 'Lorentzian' component, set with set_function.
        Parameters:
            peak, centre, fwhm:  The gaussian parameters
            peakfixed,
            centrefixed,
            fwhmfixed:           Optional parameters to indicate if
                                 the paramters should be held fixed during
                                 the fitting process. The default is to keep
                                 all parameters flexible.
            component:           The number of the component (Default is the
                                 component 0)
        """
        if self.fitfunc != "lorentz":
            msg = "Function only operates on Lorentzian components."
            raise ValueError(msg)
        if 0 <= component < len(self.components):
            d = {'params':[peak, centre, fwhm],
                 'fixed':[peakfixed, centrefixed, fwhmfixed]}
            self.set_parameters(d, component)
        else:
            msg = "Please select a valid  component."
            raise ValueError(msg)

    def get_area(self, component=None):
        """
        Return the area under the fitted gaussian/lorentzian component.
        Parameters:
              component:   the gaussian/lorentzian component selection,
                           default (None) is the sum of all components
        Note:
              This will only work for gaussian/lorentzian fits.
        """
        if not self.fitted: return
        if self.fitfunc == "gauss" or self.fitfunc == "lorentz":
            pars = list(self.fitter.getparameters())
            from math import log,pi,sqrt
            if self.fitfunc == "gauss":
                fac = sqrt(pi/log(16.0))
            elif self.fitfunc == "lorentz":
                fac = pi/2.0
            areas = []
            for i in range(len(self.components)):
                j = i*3
                cpars = pars[j:j+3]
                areas.append(fac * cpars[0] * cpars[2])
        else:
            return None
        if component is not None:
            return areas[component]
        else:
            return sum(areas)

    @asaplog_post_dec
    def get_errors(self, component=None):
        """
        Return the errors in the parameters.
        Parameters:
            component:    get the errors for the specified component
                          only, default is all components
        """
        if not self.fitted:
            msg = "Not yet fitted."
            raise RuntimeError(msg)
        errs = list(self.fitter.geterrors())
        cerrs = errs
        if component is not None:
            if self.fitfunc == "gauss" or self.fitfunc == "lorentz":
                i = 3*component
                if i < len(errs):
                    cerrs = errs[i:i+3]
        return cerrs


    @asaplog_post_dec
    def get_parameters(self, component=None, errors=False):
        """
        Return the fit paramters.
        Parameters:
             component:    get the parameters for the specified component
                           only, default is all components
        """
        if not self.fitted:
            msg = "Not yet fitted."
            raise RuntimeError(msg)
        pars = list(self.fitter.getparameters())
        fixed = list(self.fitter.getfixedparameters())
        errs = list(self.fitter.geterrors())
        area = []
        if component is not None:
            if self.fitfunc == "gauss" or self.fitfunc == "lorentz":
                i = 3*component
                cpars = pars[i:i+3]
                cfixed = fixed[i:i+3]
                cerrs = errs[i:i+3]
                a = self.get_area(component)
                area = [a for i in range(3)]
            else:
                cpars = pars
                cfixed = fixed
                cerrs = errs
        else:
            cpars = pars
            cfixed = fixed
            cerrs = errs
            if self.fitfunc == "gauss" or self.fitfunc == "lorentz":
                for c in range(len(self.components)):
                  a = self.get_area(c)
                  area += [a for i in range(3)]
        fpars = self._format_pars(cpars, cfixed, errors and cerrs, area)
        asaplog.push(fpars)
        return {'params':cpars, 'fixed':cfixed, 'formatted': fpars,
                'errors':cerrs}

    def _format_pars(self, pars, fixed, errors, area):
        out = ''
        if self.fitfunc == 'poly':
            c = 0
            for i in range(len(pars)):
                fix = ""
                if len(fixed) and fixed[i]: fix = "(fixed)"
                if errors :
                    out += '  p%d%s= %3.6f (%1.6f),' % (c,fix,pars[i], errors[i])
                else:
                    out += '  p%d%s= %3.6f,' % (c,fix,pars[i])
                c+=1
            out = out[:-1]  # remove trailing ','
        elif self.fitfunc == 'gauss' or self.fitfunc == 'lorentz':
            i = 0
            c = 0
            aunit = ''
            ounit = ''
            if self.data:
                aunit = self.data.get_unit()
                ounit = self.data.get_fluxunit()
            while i < len(pars):
                if len(area):
                    out += '  %2d: peak = %3.3f %s , centre = %3.3f %s, FWHM = %3.3f %s\n      area = %3.3f %s %s\n' % (c,pars[i],ounit,pars[i+1],aunit,pars[i+2],aunit, area[i],ounit,aunit)
                else:
                    out += '  %2d: peak = %3.3f %s , centre = %3.3f %s, FWHM = %3.3f %s\n' % (c,pars[i],ounit,pars[i+1],aunit,pars[i+2],aunit,ounit,aunit)
                c+=1
                i+=3
        return out


    @asaplog_post_dec
    def get_estimate(self):
        """
        Return the parameter estimates (for non-linear functions).
        """
        pars = self.fitter.getestimate()
        fixed = self.fitter.getfixedparameters()
        asaplog.push(self._format_pars(pars,fixed,None))
        return pars

    @asaplog_post_dec
    def get_residual(self):
        """
        Return the residual of the fit.
        """
        if not self.fitted:
            msg = "Not yet fitted."
            raise RuntimeError(msg)
        return self.fitter.getresidual()

    @asaplog_post_dec
    def get_chi2(self):
        """
        Return chi^2.
        """
        if not self.fitted:
            msg = "Not yet fitted."
            raise RuntimeError(msg)
        ch2 = self.fitter.getchi2()
        asaplog.push( 'Chi^2 = %3.3f' % (ch2) )
        return ch2

    @asaplog_post_dec
    def get_fit(self):
        """
        Return the fitted ordinate values.
        """
        if not self.fitted:
            msg = "Not yet fitted."
            raise RuntimeError(msg)
        return self.fitter.getfit()

    @asaplog_post_dec
    def commit(self):
        """
        Return a new scan where the fits have been commited (subtracted)
        """
        if not self.fitted:
            msg = "Not yet fitted."
            raise RuntimeError(msg)
        from asap import scantable
        if not isinstance(self.data, scantable):
            msg = "Not a scantable"
            raise TypeError(msg)
        scan = self.data.copy()
        scan._setspectrum(self.fitter.getresidual())
        return scan

    @asaplog_post_dec
    def plot(self, residual=False, components=None, plotparms=False,
             filename=None):
        """
        Plot the last fit.
        Parameters:
            residual:    an optional parameter indicating if the residual
                         should be plotted (default 'False')
            components:  a list of components to plot, e.g [0,1],
                         -1 plots the total fit. Default is to only
                         plot the total fit.
            plotparms:   Inidicates if the parameter values should be present
                         on the plot
        """
        if not self.fitted:
            return
        if not self._p or self._p.is_dead:
            if rcParams['plotter.gui']:
                from asap.asaplotgui import asaplotgui as asaplot
            else:
                from asap.asaplot import asaplot
            self._p = asaplot()
        self._p.hold()
        self._p.clear()
        self._p.set_panels()
        self._p.palette(0)
        tlab = 'Spectrum'
        xlab = 'Abcissa'
        ylab = 'Ordinate'
        from numpy import ma,logical_not,logical_and,array
        m = self.mask
        if self.data:
            tlab = self.data._getsourcename(self._fittedrow)
            xlab = self.data._getabcissalabel(self._fittedrow)
            m =  logical_and(self.mask,
                             array(self.data._getmask(self._fittedrow),
                                   copy=False))

            ylab = self.data._get_ordinate_label()

        colours = ["#777777","#dddddd","red","orange","purple","green","magenta", "cyan"]
        nomask=True
        for i in range(len(m)):
            nomask = nomask and m[i]
        label0='Masked Region'
        label1='Spectrum'
        if ( nomask ):
            label0=label1
        else:
            y = ma.masked_array( self.y, mask = m )
            self._p.palette(1,colours)
            self._p.set_line( label = label1 )
            self._p.plot( self.x, y )
        self._p.palette(0,colours)
        self._p.set_line(label=label0)
        y = ma.masked_array(self.y,mask=logical_not(m))
        self._p.plot(self.x, y)
        if residual:
            self._p.palette(7)
            self._p.set_line(label='Residual')
            y = ma.masked_array(self.get_residual(),
                                  mask=logical_not(m))
            self._p.plot(self.x, y)
        self._p.palette(2)
        if components is not None:
            cs = components
            if isinstance(components,int): cs = [components]
            if plotparms:
                self._p.text(0.15,0.15,str(self.get_parameters()['formatted']),size=8)
            n = len(self.components)
            self._p.palette(3)
            for c in cs:
                if 0 <= c < n:
                    lab = self.fitfuncs[c]+str(c)
                    self._p.set_line(label=lab)
                    y = ma.masked_array(self.fitter.evaluate(c),
                                          mask=logical_not(m))

                    self._p.plot(self.x, y)
                elif c == -1:
                    self._p.palette(2)
                    self._p.set_line(label="Total Fit")
                    y = ma.masked_array(self.fitter.getfit(),
                                          mask=logical_not(m))
                    self._p.plot(self.x, y)
        else:
            self._p.palette(2)
            self._p.set_line(label='Fit')
            y = ma.masked_array(self.fitter.getfit(),
                                  mask=logical_not(m))
            self._p.plot(self.x, y)
        xlim=[min(self.x),max(self.x)]
        self._p.axes.set_xlim(xlim)
        self._p.set_axes('xlabel',xlab)
        self._p.set_axes('ylabel',ylab)
        self._p.set_axes('title',tlab)
        self._p.release()
        if (not rcParams['plotter.gui']):
            self._p.save(filename)

    @asaplog_post_dec
    def auto_fit(self, insitu=None, plot=False):
        """
        Return a scan where the function is applied to all rows for
        all Beams/IFs/Pols.

        """
        from asap import scantable
        if not isinstance(self.data, scantable) :
            msg = "Data is not a scantable"
            raise TypeError(msg)
        if insitu is None: insitu = rcParams['insitu']
        if not insitu:
            scan = self.data.copy()
        else:
            scan = self.data
        rows = xrange(scan.nrow())
        # Save parameters of baseline fits as a class attribute.
        # NOTICE: This does not reflect changes in scantable!
        if len(rows) > 0: self.blpars=[]
        asaplog.push("Fitting:")
        for r in rows:
            out = " Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]" % (scan.getscan(r),
                                                                   scan.getbeam(r),
                                                                   scan.getif(r),
                                                                   scan.getpol(r),
                                                                   scan.getcycle(r))
            asaplog.push(out, False)
            self.x = scan._getabcissa(r)
            self.y = scan._getspectrum(r)
            self.mask = mask_and(self.mask, scan._getmask(r))
            self.data = None
            self.fit()
            x = self.get_parameters()
            fpar = self.get_parameters()
            if plot:
                self.plot(residual=True)
                x = raw_input("Accept fit ([y]/n): ")
                if x.upper() == 'N':
                    self.blpars.append(None)
                    continue
            scan._setspectrum(self.fitter.getresidual(), r)
            self.blpars.append(fpar)
        if plot:
            self._p.unmap()
            self._p = None
        return scan
