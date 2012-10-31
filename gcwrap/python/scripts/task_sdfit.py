import os
from taskinit import *

import sdutil
import asap as sd
import pylab as pl
from numpy import ma, array, logical_not, logical_and
import sdutil

def sdfit(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, fitfunc, fitmode, maskline, invertmask, nfit, thresh, min_nchan, avg_limit, box_size, edge, outfile, overwrite, plotlevel):


        casalog.origin('sdfit')

        restorer = None
        retValue={}

        try:
            sdutil.assert_infile_exists(infile)

            #load the data  without averaging
            s = sd.scantable(infile,average=False,antenna=antenna)
                        
            # Select scan and field
            sel = sdutil.get_selector(in_scans=scanlist,
                                      in_ifs=iflist,
                                      in_pols=pollist,
                                      in_field=field)
            #Apply the selection (if any)
            s.set_selection(sel)
            del sel

            # restorer
            restorer = sdutil.scantable_restore_factory(s,
                                                        infile,
                                                        fluxunit,
                                                        specunit,
                                                        frame,
                                                        doppler,
                                                        restfreq)
	    
	    # set restfreq
	    if (specunit == 'km/s'):
		    if (restfreq == '') and (len(s.get_restfreqs()[0]) == 0):
			    mesg = "Restfreq must be given."
			    raise Exception, mesg
                    sdutil.set_restfreq(s, restfreq)

            # set default spectral axis unit
            sdutil.set_spectral_unit(s, specunit)

            # reset frame and doppler if needed
            sdutil.set_freqframe(s, frame)
            sdutil.set_doppler(s, doppler)

            # convert flux
            stmp = sdutil.set_fluxunit(s, fluxunit, telescopeparm, False)
            if stmp:
                restorer.restore()
                del restorer
                restorer = None
                s = stmp
                del stmp

            # Make line region masks and list of line regions
            (linelist,nlines,defaultmask,revisednfit) = get_linelist(s,
                                                                     fitmode,
                                                                     maskline,
                                                                     invertmask,
                                                                     nfit,
                                                                     thresh,
                                                                     min_nchan,
                                                                     avg_limit,
                                                                     box_size,
                                                                     edge)

            # Now the line fitting for each rows in scantable
            doguess = not ((fitmode.lower()=='list') and (invertmask))
            (retValue,fitparams) = dofit(s,
                                         fitmode, fitfunc.lower(), revisednfit,
                                         nlines, defaultmask,
                                         doguess, linelist, plotlevel)
            # Store fit
            if ( outfile != '' ):
                    store_fit(fitfunc, outfile, fitparams, s, overwrite)
	    
            # Final clean up
            del s

            return retValue

        except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance
        finally:
                if restorer is not None:
                    restorer.restore()
                    del restorer
                casalog.post('')
                
def get_linelist(s, fitmode, maskline, invertmask, nfit, thresh, min_nchan, avg_limit, box_size, edge):
    defaultmask = []
    linelist = []
    nlines = 1
    revisednfit = nfit
    if ( fitmode == 'list' ):
            # Assume the user has given a list of lines
            # e.g. maskline=[[3900,4300]] for a single line
            if ( len(maskline) > 0 ):
                    # There is a user-supplied channel mask for lines
                    if ( not invertmask ):
                            # Make sure this is a list-of-lists (e.g. [[1,10],[20,30]])
                            linelist = maskline if isinstance(maskline[0],list) \
                                       else to_list_of_list(maskline)
                            nlines = len(linelist)
                    defaultmask = s.create_mask(maskline,invert=invertmask)
            else:
                    # Use whole region
                    if invertmask:
                            msg='No channel is selected because invertmask=True. Exit without fittinging.'
                            raise Exception(msg)

            casalog.post( "Identified %d regions for fitting" % (nlines) )
            if ( invertmask ):
                    casalog.post("No starting guesses available")
            else:
                    casalog.post("Will use these as starting guesses")

    elif (fitmode == 'interact'):
            # Interactive masking
            new_mask = sdutil.init_interactive_mask(s, maskline, invertmask)
            defaultmask = sdutil.get_interactive_mask(new_mask, purpose='to fit lines')
            linelist=s.get_masklist(defaultmask)
            nlines=len(linelist)
            if nlines < 1:
                    msg='No channel is selected. Exit without fittinging.'
                    raise Exception(msg)
            print '%d region(s) is selected as a linemask' % nlines
            print 'The final mask list ('+s._getabcissalabel()+') ='+str(linelist)
            print 'Number of line(s) to fit: nfit =',nfit
            ans=raw_input('Do you want to reassign nfit? [N/y]: ')
            if (ans.upper() == 'Y'):
                    ans=input('Input nfit = ')
                    if type(ans) == list: revisednfit=ans
                    elif type(ans) == int: revisednfit=[ans]
                    else:
                            msg='Invalid definition of nfit. Setting nfit=[1] and proceed.'
                            casalog.post(msg, priority='WARN')
                            revisednfit=[1]
                    casalog.post('List of line number reassigned.\n   nfit = '+str(nfit))
            sdutil.finalize_interactive_mask(new_mask)

    else:
            # Fit mode AUTO and in channel mode
            casalog.post( "Trying AUTO mode - find line channel regions" )
            if ( len(maskline) > 0 ):
                    # There is a user-supplied channel mask for lines
                    defaultmask=s.create_mask(maskline,invert=invertmask)

            # Use linefinder to find lines
            casalog.post( "Using linefinder" )
            fl=sd.linefinder()
            fl.set_scan(s)
            # This is the tricky part
            # def  fl.set_options(threshold=1.732,min_nchan=3,avg_limit=8,box_size=0.2)
            # e.g. fl.set_options(threshold=5,min_nchan=3,avg_limit=4,box_size=0.1) seem ok?
            fl.set_options(threshold=thresh,min_nchan=min_nchan,avg_limit=avg_limit,box_size=box_size)
            # Now find the lines for each row in scantable
            nlines=[]
            for irow in range(s.nrow()):
                nlines.append(fl.find_lines(mask=defaultmask,nRow=irow,edge=edge))
                # Get ranges

                ptout="SCAN[%d] IF[%d] POL[%d]: " %(s.getscan(irow), s.getif(irow), s.getpol(irow))
                if ( nlines[irow] > 0 ):
                        ll = fl.get_ranges()
                        casalog.post( ptout+"Found %d lines at %s" % (nlines[irow], str(ll) ) )
                else:
                        ll = ()
                        casalog.post( ptout+"Nothing found.", priority = 'WARN' )

                # This is a linear list of pairs of values, so turn these into a list of lists
                linelist.append(to_list_of_list(ll))
            # Done with linefinder
            casalog.post( "Finished linefinder." )
    return (linelist,nlines,defaultmask,revisednfit)

def dofit(s, fitmode, fitfunc, nfit, nlines, defaultmask, doguess, linelist, plotlevel):
    fitdict = dict.fromkeys(['nfit','peak','cent','fwhm'],[])
    fitparams=[]
    f=sd.fitter()
    if ( abs(plotlevel) > 0 ):
            init_plot( f, s.nrow(), plotlevel)
    dbw = 1.0
    current_unit = s.get_unit()
    kw = {'thescan':s}
    if len(defaultmask) > 0: kw['mask'] = defaultmask
    f.set_scan(**kw)
    firstplot = True
    
    for irow in range(s.nrow()):
        casalog.post( "start row %d" % (irow) )
        numlines = nlines[irow] if isinstance(nlines,list) else nlines

        if numlines == 0:
            fitparams.append([[0,0,0]])
            fitdict['nfit']+=[-1]
            warn_fit_failed(s,irow,'No lines detected.')
            continue
                
        if ( fitmode == 'auto'):
            # Auto mode - one comp per line region
            # Overwriting user-supplied nfit
            numfit = numlines
            comps = [1 for i in xrange(numlines)]
        else:
            # Get number of things to fit from nfit list
            comps = nfit if isinstance(nfit,list) else [nfit]
            # Drop extra over numlines
            numfit = min(len(comps),numlines)
        ncomps = sum(comps)
        
        casalog.post( "Will fit %d components in %d regions" % (ncomps, numfit) )

        if numfit <= 0:
            fitparams.append([[0,0,0]])
            fitdict['nfit']+=[-1]
            warn_fit_failed(s,irow,'Fit failed.')
            continue

        # Fit the line using numfit gaussians or lorentzians
        # Assume the nfit list matches maskline
        f.set_function(**{fitfunc:ncomps})
        if ( doguess ):
            # in auto mode, linelist will be detemined for each spectra
            # otherwise, linelist will be the same for all spectra
            llist = linelist[irow] if fitmode == 'auto' else linelist
            if current_unit != 'channel':
                    xx = s._getabcisssa(irow)
                    dbw = abs(xx[1]-xx[0])
            (maxl,fwhm,cenl) = get_initial_guess_list(s,llist,defaultmask,dbw,irow)
            # Guesses using max, cen, and fwhm=0.7*eqw
            # NOTE: should there be user options here?
            n = 0
            for i in range(numfit):
                # cannot guess for multiple comps yet
                if ( comps[i] == 1 ):
                    # use guess
                    getattr(f,'set_%s_parameters'%(fitfunc))(maxl[i], cenl[i], fwhm[i], component=n)
                n += comps[i]
        else:
            # No guesses
            casalog.post( "Fitting lines without starting guess" )

        # Now fit
        f.fit(row=irow)
        fstat = f.get_parameters()

        # Check for convergence
        goodfit = ( len(fstat['errors']) > 0 )
        if ( goodfit ):
            # Retrieve fit parameters
            fitdict['nfit'] = fitdict['nfit'] + [ncomps]
            keys = ['peak','cent','fwhm']
            retl = dict.fromkeys(keys,[])
            for i in range(ncomps):
                fstat = f.get_parameters(i)
                for j in xrange(len(retl)):
                    key = keys[j]
                    retl[key] = retl[key] + [[fstat['params'][j],\
                                              fstat['errors'][j]]]
            for key in keys:
                fitdict[key] = fitdict[key] + [retl[key]]
            fitparams+=[retrieve_fit(f, ncomps)]
        else:
            # Did not converge
            fitdict['nfit'] += [-ncomps]
            fitparams.append([[0,0,0]])
            warn_fit_failed(s,irow,'Fit failed to converge')

        # plot
        if (irow < 16 and abs(plotlevel) > 0):
            plot(f, irow, goodfit, plotlevel, firstplot)
            firstplot = False

    return (fitdict,fitparams)

def store_fit(func, outfile, value, scan, overwrite):
        sdutil.assert_outfile_canoverwrite_or_nonexistent(outfile, 'none', overwrite)
        outf = file(sdutil.get_abspath(outfile),'w')

        #header 
        header="#%-4s %-4s %-4s %-12s " %("SCAN", "IF", "POL", "Function")
        numparam=3     # gaussian fitting is assumed (max, center, fwhm)
        for i in xrange(numparam):
            header+='%-12s '%('P%d'%(i))
        outf.write(header+'\n')

        #data
        for i in xrange(len(value)):
            dattmp=" %-4d %-4d %-4d " \
                    %(scan.getscan(i), scan.getif(i), scan.getpol(i))
            for j in xrange(len(value[i])):
                if ( value[i][j][0]!=0.0): 
                    datstr=dattmp+'%-12s '%('%s%d'%(func,j))
                    for k in xrange(len(value[i][j])):
                        datstr+="%3.8f " %(value[i][j][k])
                    outf.write(datstr+'\n')
                        
        outf.close()
        return

def retrieve_fit(fitter, numcomp):
        parameters=fitter.get_parameters()['params']
        numparams=len(parameters)/numcomp
        values = list(array(parameters).reshape((numcomp,numparams)))
        return values

def init_plot( fitter, n, plotlevel):
        if n > 16:
                casalog.post( 'Only first 16 results are plotted.', priority = 'WARN' )
                n = 16
        
        # initialize plotter
        from matplotlib import rc as rcp
        rcp('lines', linewidth=1)
        if not (fitter._p and fitter._p._alive()):
            fitter._p = sdutil.get_plotter(plotlevel)
        fitter._p.hold()
        fitter._p.clear()
        # set nrow and ncol (maximum 4x4)
        fitter._p.set_panels(rows=n, cols=0, ganged=False)
        casalog.post( 'nrow,ncol= %d,%d' % (fitter._p.rows, fitter._p.cols ) )
        fitter._p.palette(0,["#777777", "#dddddd", "red", "orange", "purple", "green", "magenta", "cyan"])

def plot( fitter, irow, fitted, plotlevel, firstplot=False ):
        if firstplot:
            labels = ['Spectrum', 'Selected Region', 'Residual', 'Fit']
        else:
            labels = ['spec', 'select', 'res', 'fit']
        myp = fitter._p

        myp.subplot(irow)
        # plot spectra
        x = fitter.data._getabcissa(irow)
        y = fitter.data._getspectrum(irow)
        mr = fitter.data._getflagrow(irow)
        if mr: # a whole spectrum is flagged
                themask = False
        else:
                msk = array(fitter.data._getmask(irow))
                fmsk = array(fitter.mask)
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
                if ( plotlevel==2 ):
                        plot_line(myp,x,fitter.get_residual(),themask,label=labels[2],color=7)
                # plot fit
                plot_line(myp,x,fitter.fitter.getfit(),themask,label=labels[3],color=2)

        if ( irow == 0 ):
                tlab=fitter.data._getsourcename(fitter._fittedrow)
                myp.set_axes('title',tlab)
        if (irow%myp.rows == 0):
                ylab=fitter.data._get_ordinate_label()
                myp.set_axes('ylabel',ylab)
        if (irow/myp.rows == myp.cols-1):
                xlab=fitter.data._getabcissalabel(fitter._fittedrow)
                myp.set_axes('xlabel',xlab)
        myp.release()

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

def get_initial_guess_list(s, linelist, defaultmask, dbw, irow):
    if len(linelist) > 0:
        # guesses: [maxlist,fwhmlist,cenlist]
        guesses = [[],[],[]]
        for x in linelist:
            x.sort()
            casalog.post( "detected line: "+str(x) ) 
            msk = s.create_mask(x, row=irow)
            guess = get_initial_guess(s,msk,x,dbw,irow)
            for i in xrange(3):
                guesses[i] = guesses[i] + [guess[i]]
    else:
        guess=get_initial_guess(s,defaultmask,[],dbw,irow)
        guesses = [[guess[i]] for i in xrange(3)]
    return tuple(guesses)

def get_initial_guess(s, msk, linerange, dbw, irow):
    [maxl,suml] = [s._math._statsrow(s,msk,st,irow)[0] for st in ['max','sum']]
    fwhm = maxl if maxl==0.0 else 0.7*abs(suml/maxl*dbw)
    cen = 0.5*sum(linerange[:2]) if len(linerange) > 1 else s.nchan(s.getif(irow))/2
    return (maxl,fwhm,cen)
    
def warn_fit_failed(s,irow,message=''):
    casalog.post( 'Fitting:' )
    casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
    casalog.post( "   %s"%(message), priority = 'WARN' )

