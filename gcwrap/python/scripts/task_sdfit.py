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
            (linelist,nlines,maskforfit,defaultmask,revisednfit) = get_linelist(s,
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
                                         fitmode, fitfunc, revisednfit,
                                         nlines, maskforfit, defaultmask,
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
                

def store_fit(func, in_outname, value, scan, overwrite):
        outname=os.path.expandvars(in_outname)
        outname=os.path.expanduser(outname)
        if os.path.exists(outname):
                if overwrite:
                        os.system('rm '+outname)
                else:
                        casalog.post( in_outname+' already exists.' )
                        return
                
        outfile=file(outname, 'w')

        #header 
        header="#%-4s %-4s %-4s %-12s " %("SCAN", "IF", "POL", "Function")
        numparam=3     # gaussian fitting is assumed (max, center, fwhm)
        for i in xrange(numparam):
            header+='%-12s '%('P%d'%(i))
        header+='\n'
        outfile.write(header)

        #data
        for i in xrange(len(value)):
                dattmp=" %-4d %-4d %-4d " \
                        %(scan.getscan(i), scan.getif(i), scan.getpol(i))
                for j in xrange(len(value[i])):
                        if ( value[i][j][0]!=0.0): 
                                datstr=dattmp+'%-12s '%('%s%d'%(func,j))
                                for k in xrange(len(value[i][j])):
                                        datstr+="%3.8f " %(value[i][j][k])
                                datstr+="\n"
                                outfile.write(datstr)
                        
        outfile.close()

        return


def retrieve_fit(fitter, numcomp):
        values=[]
        parameters=fitter.get_parameters()['params']
        numparams=len(parameters)/numcomp
        for i in xrange(numcomp):
                value=[]
                for j in xrange(numparams):
                        value+=[parameters[numparams*i+j]]
                #print value
                values+=[value]
        
        return values


def init_plot( fitter, n, plotlevel):
        if n > 16:
                casalog.post( 'Only first 16 results are plotted.', priority = 'WARN' )
                n = 16
        
        # initialize plotter
        from matplotlib import rc as rcp
        rcp('lines', linewidth=1)
        if not (fitter._p and fitter._p._alive()):
                from asap.asapplotter import new_asaplot
                visible = False
                if plotlevel > 0:
                        if sd.rcParams['plotter.gui']:
                                visible = True
                        else:
                                casalog.post("GUI plot not available", priority = "ERROR")
                #print "loading new plot"
                fitter._p = new_asaplot(visible=visible)
        fitter._p.hold()
        fitter._p.clear()
        # set nrow and ncol (maximum 4x4)
        fitter._p.set_panels(rows=n, cols=0, ganged=False)
        casalog.post( 'nrow,ncol= %d,%d' % (fitter._p.rows, fitter._p.cols ) )


def plot( fitter, irow, fitted, plotlevel ):
        colors = ["#777777", "#dddddd", "red", "orange", "purple", "green", "magenta", "cyan"]
        myp = fitter._p

        myp.subplot(irow)
        # plot spectra
        myp.palette(1,colors)
        x = fitter.data._getabcissa(irow)
        y = fitter.data._getspectrum(irow)
        mr = fitter.data._getflagrow(irow)
        if mr: # a whole spectrum is flagged
                allmsk = False
                invmsk = True
                nomask = False
        else:
                msk = array(fitter.data._getmask(irow))
                fmsk = array(fitter.mask)
                allmsk = logical_and(msk,fmsk)
                invmsk = logical_not(allmsk)
                del msk, fmsk
                # nomask is False if any of channel is not in fit range.
                nomask = all(allmsk)
        yorg = y
        label0 = ['Selected Region','select']
        label1 = ['Spectrum','spec']
        if ( nomask ):
                label0[0] = label1[0]
                label0[1] = label1[1]
        else:
                # dumped region
                y=ma.masked_array(yorg,mask=allmsk)
                if ( irow==0 ):
                        myp.set_line( label=label1[0] )
                else:
                        myp.set_line( label=label1[1] )
                myp.plot(x,y)
        # fitted region
        myp.palette(0)
        y=ma.masked_array(yorg,mask=invmsk)
        if ( irow==0 ):
                myp.set_line(label=label0[0])
        else:
                myp.set_line(label=label0[1])
        xlim=[min(x),max(x)]
        ymin=min(y)
        ymax=max(y)
        ymin=ymin-(ymax-ymin)*0.1
        ymax=ymax+(ymax-ymin)*0.1
        ylim=[ymin,ymax]
        mya=myp.subplots[irow]['axes']
        myp.axes.set_xlim(xlim)
        myp.axes.set_ylim(ylim)
        myp.plot(x,y)

        # plot fitted result
        if ( fitted ):
                # plot residual
                if ( plotlevel==2 ):
                        myp.palette(7)
                        if ( irow==0 ):
                                myp.set_line(label='Residual')
                        else:
                                myp.set_line(label='res')
                        y=ma.masked_array(fitter.get_residual(),mask=invmsk)
                        myp.plot(x,y)
                # plot fit
                myp.palette(2)
                if ( irow==0 ):
                        myp.set_line(label='Fit')
                else:
                        myp.set_line(label='fit')
                y=ma.masked_array(fitter.fitter.getfit(),mask=invmsk)
                myp.plot(x,y)

        xlab=fitter.data._getabcissalabel(fitter._fittedrow)
        ylab=fitter.data._get_ordinate_label()
        tlab=fitter.data._getsourcename(fitter._fittedrow)
        if ( irow == 0 ):
                myp.set_axes('title',tlab)
        nr=myp.rows
        nc=myp.cols
        if ( irow % nr == 0 ):
                myp.set_axes('ylabel',ylab)
        if ( irow/nr == nc-1 ):
                myp.set_axes('xlabel',xlab)
        myp.release()

def to_list_of_list(l):
    return array(l).reshape(len(l)/2,2).tolist()

def get_initial_guess_list(s, linelist, defaultmask, dbw, irow):
    if len(linelist) > 0:
        maxlt=[]
        fwhmt=[]
        cent=[]
        for x in linelist:
            if ( x[0] > x[1] ):
                x = [x[1],x[0]]
            #print x
            casalog.post( "detected line: "+str(x) ) 
            msk = s.create_mask(x, row=irow)
            (maxl,fwhm,cen)=get_initial_guess(s,msk,x,dbw,irow)
            maxlt = maxlt + [maxl]
            fwhmt = fwhmt + [fwhm]
            cent = cent + [cen]
    else:
        (maxl,fwhm,cen)=get_initial_guess(s,defaultmask,[],dbw,irow)
        maxlt = [maxl]
        fwhmt = [fwhm]
        cent = [cen]
    return (maxlt,fwhmt,cent)

def get_initial_guess(s, msk, linerange, dbw, irow):
    [maxl,suml] = [s._math._statsrow(s,msk,st,irow)[0] for st in ['max','sum']]
    fwhm = maxl if maxl==0.0 else 0.7*abs(suml/maxl*dbw)
    if len(linerange) > 1:
        cen = 0.5*(linerange[0] + linerange[1])
    else:
        # I dont know a better way to specify the center of the spectrum
        cen = s.nchan(irow)/2
    return (maxl,fwhm,cen)
    
def warn_fit_failed(s,irow,message=''):
    casalog.post( 'Fitting:' )
    casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
    casalog.post( "   %s"%(message), priority = 'WARN' )

def dofit(s, fitmode, fitfunc, nfit, nlines, maskforfit, defaultmask, doguess, linelist, plotlevel):
    fitdict = dict.fromkeys(['nfit','peak','cent','fwhm'],[])
    fitparams=[]
    f=sd.fitter()
    funcdict = {'gauss': f.set_gauss_parameters,
                'lorentz': f.set_lorentz_parameters}
    if ( abs(plotlevel) > 0 ):
            init_plot( f, s.nrow(), plotlevel)
    dbw = 1.0
    current_unit = s.get_unit()

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
            ncomps = numlines
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
        f.set_scan(s,maskforfit)
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
                    funcdict[fitfunc](maxl[i], cenl[i], fwhm[i], component=n)
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
            plot(f, irow, goodfit, plotlevel)

    return (fitdict,fitparams)

def get_linelist(s, fitmode, maskline, invertmask, nfit, thresh, min_nchan, avg_limit, box_size, edge):
    defaultmask = []
    linelist = []
    nlines = 1
    maskforfit = None
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
                    maskforfit = defaultmask
            else:
                    # Use whole region
                    if invertmask:
                            msg='No channel is selected because invertmask=True. Exit without fittinging.'
                            raise Exception(msg)

            #print "Identified ",nlines," regions for fitting"
            casalog.post( "Identified %d regions for fitting" % (nlines) )
            if ( invertmask ):
                    casalog.post("No starting guesses available")
            else:
                    casalog.post("Will use these as starting guesses")

    elif (fitmode == 'interact'):
            # Interactive masking
            new_mask = sdutil.init_interactive_mask(s, maskline, invertmask)
            defaultmask = sdutil.get_interactive_mask(new_mask, purpose='to fit lines')
            maskforfit = defaultmask
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
            #print "Trying AUTO mode - find line channel regions"
            casalog.post( "Trying AUTO mode - find line channel regions" )
            if ( len(maskline) > 0 ):
                    # There is a user-supplied channel mask for lines
                    defaultmask=s.create_mask(maskline,invert=invertmask)
                    maskforfit = defaultmask

            # Use linefinder to find lines
            #print "Using linefinder"
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
            #print "Finished linefinder, found ",nlines,"lines"
            del fl
    return (linelist,nlines,maskforfit,defaultmask,revisednfit)
