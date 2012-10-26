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
            try:
                    sel = sdutil.get_selector(in_scans=scanlist,
                                              in_ifs=iflist,
                                              in_pols=pollist,
                                              in_field=field)
		    #Apply the selection (if any)
                    s.set_selection(sel)
            except Exception, instance:
                casalog.post( str(instance), priority = 'ERROR' )
                raise Exception, instance

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
##             (linemask,linelist,nlines,domask,doguess) = get_mask(s,
##                                                                  fitmode,
##                                                                  maskline,
##                                                                  invertmask,
##                                                                  nfit,
##                                                                  thresh,
##                                                                  min_nchan,
##                                                                  avg_limit,
##                                                                  box_size,
##                                                                  edge)
            maskforstat = []
            maskforfit = None
            if ( fitmode == 'list' ):
                    # Assume the user has given a list of lines
                    # e.g. maskline=[[3900,4300]] for a single line
                    if ( len(maskline) > 0 ):
                            # There is a user-supplied channel mask for lines
                            if ( not invertmask ):
                                    maskforstat = s.create_mask(maskline)
                                    maskforfit = maskforstat
                                    doguess = True
                                    # Make sure this is a list-of-lists (e.g. [[1,10],[20,30]])
                                    if ( type(maskline[0]) == list ):
                                            ll = len(maskline)
                                            nlines = len(maskline)
                                            linelist = maskline
                                    else:
                                            # Not a list, turn into one
                                            linelist = to_list_of_list(maskline)
                                            nlines = len(linelist)
                            else:
                                    # invert regions
                                    maskforstat=s.create_mask(maskline,invert=True)
                                    maskforfit = maskforstat
                                    nlines = 1
                                    linelist=[]
                                    doguess = False

                    else:
                            if invertmask:
                                    msg='No channel is selected because invertmask=True. Exit without fittinging.'
                                    casalog.post( msg, priority = 'WARN' )
                                    return
                            # Use whole region
                            nlines = 1
                            linelist=[]
                            doguess = True

                    #print "Identified ",nlines," regions for fitting"
                    casalog.post( "Identified %d regions for fitting" % (nlines) )
                    if ( doguess ):
                            "Will use these as starting guesses"
                    else:
                            "No starting guesses available"

	    elif (fitmode == 'interact'):
		    # Interactive masking
                    maskforstat = sdutil.get_interactive_mask(s, maskline, invertmask)
                    maskforfit = maskforstat
		    linelist=s.get_masklist(maskforstat)
		    nlines=len(linelist)
		    if nlines < 1:
			    msg='No channel is selected. Exit without fittinging.'
			    casalog.post( msg, priority = 'WARN' )
			    return
		    doguess=True
		    print '%d region(s) is selected as a linemask' % nlines
		    print 'The final mask list ('+s._getabcissalabel()+') ='+str(linelist)
		    print 'Number of line(s) to fit: nfit =',nfit
		    ans=raw_input('Do you want to reassign nfit? [N/y]: ')
		    if (ans.upper() == 'Y'):
			    ans=input('Input nfit = ')
			    if type(ans) == list: nfit=ans
			    elif type(ans) == int:nfit=[ans]
			    else:
				    msg='Invalid definition of nfit. Setting nfit=[1] and proceed.'
				    casalog.post(msg, priority='WARN')
				    nfit=[1]
			    casalog.post('List of line number reassigned.\n   nfit = '+str(nfit))

            else:
                    # Fit mode AUTO and in channel mode
                    #print "Trying AUTO mode - find line channel regions"
                    casalog.post( "Trying AUTO mode - find line channel regions" )
                    if ( len(maskline) > 0 ):
                            # There is a user-supplied channel mask for lines
                            maskforstat=s.create_mask(maskline,invert=invertmask)
                            maskforfit = maskforstat

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
                    linelist=[]
                    nlines=[]
                    for irow in range(s.nrow()):
                        nlines.append(fl.find_lines(mask=maskforstat,nRow=irow,edge=edge))
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
                    doguess = True
                    del fl

            # If we have line regions, get starting guesses
            linemax=[]
            lineeqw=[]
            linecen=[]
            if ( doguess ):
                    # For each line get guess of max, cen and estimated equivalent width (sum/max)
                    for irow in range(s.nrow()):
                        casalog.post( "start row %d" % (irow) )
                        if( fitmode=='auto' ):
                                # in auto mode, linelist will be determined
                                # for each spectra
                                llist=linelist[irow]
                        else:
                                # otherwise, linelist will be the same
                                # for all spectra
                                llist=linelist
                        if( len(llist) > 0):
                            maxlt=[]
                            eqwt=[]
                            cent=[]
                            for x in llist:
                                    if ( x[0] > x[1] ):
                                            tmp=x[0]
                                            x[0]=x[1]
                                            x[1]=tmp
                                    #print x
                                    casalog.post( "detected line: "+str(x) ) 
                                    msk = s.create_mask(x, row=irow)
                                    #maxl = s.stats('max',msk)[irow]
                                    #suml = s.stats('sum',msk)[irow]
                                    maxl = s._math._stats(s,msk,'max')[irow]
                                    suml = s._math._stats(s,msk,'sum')[irow]

                                    if ( maxl != 0.0 ):
                                            eqw = suml/maxl
                                            if ( s.get_unit() != 'channel' ):
                                                    xx=s._getabcissa(irow)
                                                    dbw=abs(xx[1]-xx[0])
                                                    eqw = eqw * dbw
                                    else:
                                            eqw = 0.0
                                    cen = 0.5*(x[0] + x[1])
                                    maxlt = maxlt + [maxl]
                                    eqwt = eqwt + [eqw]
                                    cent = cent + [cen]
                            linemax = linemax + [maxlt]
                            lineeqw = lineeqw + [eqwt]
                            linecen = linecen + [cent]
                        else:
                            # For what its worth, do stats on unmasked region
                            maxl = s._math._stats(s,maskforstat,'max')[irow]
                            suml = s._math._stats(s,maskforstat,'sum')[irow]
                                    
                            linemax.append([maxl])
                            if (maxl != 0.0):
                                    eqw = suml/maxl
                            else:
                                    eqw = 0.0
                            lineeqw.append([eqw])
                            # I dont know a better way to specify the center of the spectrum
                            cen = s.nchan()/2
                            linecen.append([cen])

            # Now the line fitting for each rows in scantable
            retValue['nfit']=[]
            retValue['peak']=[]
            retValue['cent']=[]
            retValue['fwhm']=[]
            fitparams=[]
            f=sd.fitter()
            if ( abs(plotlevel) > 0 ):
                    init_plot( f, s.nrow(), plotlevel)
            for irow in range(s.nrow()):
                # for plotting
                fitted=False
                residual=False
                
                if (type(nlines) == list):
                    numlines=nlines[irow]
                else:
                    numlines=nlines
                
                if (numlines > 0):
		    if ( fitmode == 'list' or fitmode == 'interact'):
                            # Get number of things to fit from nfit list
                            if ( type(nfit) == list ):
                                    numfit = len(nfit)
                                    nlist = nfit
                            else:
                                    numfit = 1
                                    nlist = [nfit]
                            # Drop extra over numlines
                            numfit = min(numfit,numlines)

                            ncomps = 0
                            comps = []
                            for i in range(numfit):
                                    ncomps = ncomps + nlist[i]
                                    comps += [nlist[i]]
                    else:
                            # Auto mode - one comp per line region
                            # Overwriting user-supplied nfit
                            numfit = numlines
                            ncomps = numlines
                            comps = []
                            for i in range(numlines):
                                    comps += [1]

                    casalog.post( "Will fit %d components in %d regions" % (ncomps, numfit) )

                    if (numfit > 0):
                            # Fit the line using numfit gaussians or lorentzians
                            # Assume the nfit list matches maskline
                            #f=sd.fitter()
			    if (fitfunc == 'lorentz'):
				    f.set_function(lorentz=ncomps)
			    else:
				    f.set_function(gauss=ncomps)
				    
                            f.set_scan(s,maskforfit)
                            #for irow in range(s.nrow()):
                            if ( doguess ):
                                    # Guesses using max, cen, and eqw
                                    # NOTE: should there be user options here?
                                    n = 0
                                    for i in range(numfit):
                                        if ( comps[i] == 1 ):
                                            # use guess
                                            maxl = linemax[irow][i]
                                            fwhm = 0.7*abs( lineeqw[irow][i] )
                                            cenl = linecen[irow][i]
					    if (fitfunc == 'lorentz'):
						    f.set_lorentz_parameters(maxl, cenl, fwhm, component=n)
					    else:
						    f.set_gauss_parameters(maxl, cenl, fwhm, component=n)
                                            n = n + 1
                                        else:
                                            # cannot guess for multiple comps yet
                                            n = n + comps[i]
                            else:
                                    # No guesses
                                    casalog.post( "Fitting lines without starting guess" )

                            # Now fit
                            f.fit(row=irow)
                            fstat = f.get_parameters()
                            #print 'row',i,':',fstat

                            # Check for convergence
                            goodfit = ( len(fstat['errors']) > 0 )
                            if ( goodfit ):
                                    # Plot residuals
                                    fitted=True
                                    if ( abs(plotlevel) == 2 ): 
                                            residual=True
                                    #if ( abs(plotlevel) > 0):
                                        # each IF is separate panel, pols stacked
                                    #    f.plot(residual=True)
                                     
                                    # Retrieve fit parameters
                                    retValue['nfit'] = retValue['nfit'] + [ncomps]
                                    retp = []
                                    retc = []
                                    retf = []
                                    for i in range(ncomps):
                                        fstat = f.get_parameters(i)
                                        retp = retp + [[fstat['params'][0],\
                                                        fstat['errors'][0]]]
                                        retc = retc + [[fstat['params'][1],\
                                                        fstat['errors'][1]]]
                                        retf = retf + [[fstat['params'][2],\
                                                        fstat['errors'][2]]]
                                    retValue['peak']=retValue['peak'] + [retp]
                                    retValue['cent']=retValue['cent'] + [retc]
                                    retValue['fwhm']=retValue['fwhm'] + [retf] 
                                    fitparams+=[retrieve_fit(f, ncomps)]
                            else:
                                    # Did not converge
                                    retValue['nfit'] += [-ncomps]
                                    fitparams.append([[0,0,0]])
                                    casalog.post( 'Fitting:' )
                                    casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
                                    casalog.post( "   Fit failed to converge", priority = 'WARN' )
                            # Clean up
                            #del f
                    else:
                            fitparams.append([[0,0,0]])
                            retValue['nfit']+=[-1]
                            casalog.post( 'Fitting:' )
                            casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
                            casalog.post( '   Fit failed.', priority = 'WARN' )
                else:
                    fitparams.append([[0,0,0]])
                    retValue['nfit']+=[-1]
                    casalog.post( 'Fitting:' )
                    casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
                    casalog.post( '   No lines detected.', priority = 'WARN' )

                # plot
                if ( abs(plotlevel) > 0 ):
                        if ( irow < 16 ):
                                plot( f, irow, fitted, residual )
                
            # Store fit
            if ( outfile != '' ):
                    store_fit(fitfunc, outfile, fitparams, s, overwrite)
	    
            # Final clean up
            del f
            del s
##             if (domask): del linemask

            return retValue

        except Exception, instance:
                import traceback
                print traceback.format_exc()
                casalog.post( str(instance), priority = 'ERROR' )
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
        parstr=""
        numparam=3     # gaussian fitting is assumed (max, center, fwhm)
        for i in xrange(numparam):  
                pname="P%d" %(i)
                parstr+="%-12s " %(pname)
        header+="%s\n" %(parstr)
        outfile.write(header)

        #data
        for i in xrange(len(value)):
                dattmp=" %-4d %-4d %-4d " \
                        %(scan.getscan(i), scan.getif(i), scan.getpol(i))
                for j in xrange(len(value[i])):
                        if ( value[i][j][0]!=0.0): 
                                funcname=func+("%d" %(j))
                                datstr=dattmp+"%-12s " %(funcname)
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


def plot( fitter, irow, fitted, residual ):
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
                nomask = True
                for i in range(len(allmsk)):
                        # nomask is False if any of channel is not in fit range.
                        nomask = nomask and allmsk[i]
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
                if ( residual ):
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
