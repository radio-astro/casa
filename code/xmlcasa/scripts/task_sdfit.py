import os
from taskinit import *

import asap as sd
import pylab as pl

def sdfit(sdfile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, fitfunc, fitmode, maskline, invertmask, nfit, thresh, min_nchan, avg_limit, box_size, edge, fitfile, overwrite, plotlevel):


        casalog.origin('sdfit')

        ###
        ### Now the actual task code
        ###
        retValue={}

        try:
            if sdfile=='':
                    raise Exception, 'sdfile is undefined'

            filename = os.path.expandvars(sdfile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s

            #load the data  without averaging
            s=sd.scantable(sdfile,average=False,antenna=antenna)


            # get telescope name
            #'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
            antennaname = s.get_antennaname()

            # check current fluxunit
            # for GBT if not set, set assumed fluxunit, Kelvin
            fluxunit_now = s.get_fluxunit()
            if ( antennaname == 'GBT'):
                            if (fluxunit_now == ''):
                                    #print "No fluxunit in the data. Set to Kelvin."
                                    casalog.post( "No fluxunit in the data. Set to Kelvin." )
                                    s.set_fluxunit('K')
                                    fluxunit_now = s.get_fluxunit()

            #print "Current fluxunit = "+fluxunit_now
            casalog.post( "Current fluxunit = "+fluxunit_now )

            # set default spectral axis unit
            if ( specunit != '' ):
                    s.set_unit(specunit)
                    specunit_now = specunit
            else:
                    specunit_now = s.get_unit()

            # reset frame and doppler if needed
            if ( frame != '' ):
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

            # Select scan and field
            sel = sd.selector()

            # Set up scanlist
            if ( type(scanlist) == list ):
                    # is a list
                    scans = scanlist
            else:
                    # is a single int, make into list
                    scans = [ scanlist ]
            # Now select them
            if ( len(scans) > 0 ):
                    sel.set_scans(scans)

            # Select source names
            if ( field != '' ):
                    sel.set_name(field)
                    # NOTE: currently can only select one
                    # set of names this way, will probably
                    # need to do a set_query eventually

            # Select IFs
            if ( type(iflist) == list ):
                    # is a list
                    ifs = iflist
            else:
                    # is a single int, make into list
                    ifs = [ iflist ]
            if ( len(ifs) > 0 ):
                    # Do any IF selection
                    sel.set_ifs(ifs)

            # Select polarization
            if ( type(pollist) == list ):
                    # is a list
                    pols = pollist
            else:
                    pols = [ polist ] 
            if ( len(pols) > 0 ):
                    sel.set_polarisations(pols)
            try:
               # Apply the selection (if any)
                    s.set_selection(sel)
            except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return
            del sel

            # convert flux
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
                            casalog.post(  "Use ap.eff. eta = %5.3f " % (eta) )
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

            # Make line region masks and list of line regions
            if ( fitmode == 'list' ):
                    # Assume the user has given a list of lines
                    # e.g. maskline=[[3900,4300]] for a single line
                    if ( len(maskline) > 0 ):
                            # There is a user-supplied channel mask for lines
                            if ( not invertmask ):
                                    linemask=s.create_mask(maskline)
                                    domask = True
                                    doguess = True
                                    # Make sure this is a list-of-lists (e.g. [[1,10],[20,30]])
                                    if ( type(maskline[0]) == list ):
                                            ll = len(maskline)
                                            nlines = len(maskline)
                                            linelist = maskline
                                    else:
                                            # Not a list, turn into one
                                            nlines = len(maskline)/2
                                            linelist = []
                                            for i in range(nlines):
                                                    lo = maskline[2*i]
                                                    up = maskline[2*i+1]
                                                    linelist = linelist + [[lo,up]]
                            else:
                                    # invert regions
                                    linemask=s.create_mask(maskline,invert=True)
                                    domask = True
                                    nlines = 1
                                    linelist=[]
                                    doguess = False

                    else:
                            # Use whole region
                            nlines = 1
                            linelist=[]
                            doguess = True
                            domask = False

                    #print "Identified ",nlines," regions for fitting"
                    casalog.post( "Identified %d regions for fitting" % (nlines) )
                    if ( doguess ):
                            "Will use these as starting guesses"
                    else:
                            "No starting guesses available"

	    elif (fitmode == 'interact'):
		    # Interactive masking
		    new_mask=sd.interactivemask(scan=s)
		    if (len(maskline) > 0):
			    new_mask.set_basemask(masklist=maskline,invert=False)
		    new_mask.select_mask(once=False,showmask=True)
		    # Wait for user to finish mask selection
		    finish=raw_input("Press return to calculate statistics.\n")

		    # Get final mask list
		    linemask=new_mask.get_mask()
		    linelist=s.get_masklist(linemask)
		    nlines=len(linelist)
		    if nlines < 1:
			    msg='No channel is selected. Exit without fittinging.'
			    casalog.post( msg, priority = 'WARN' )
			    return
		    domask=True
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
		    new_mask.finish_selection()
		    del new_mask

            else:
                    # Fit mode AUTO and in channel mode
                    #print "Trying AUTO mode - find line channel regions"
                    casalog.post( "Trying AUTO mode - find line channel regions" )
                    if ( len(maskline) > 0 ):
                            # There is a user-supplied channel mask for lines
                            linemask=s.create_mask(maskline,invert=invertmask)
                            domask = True
                    else:
                            # Use whole region
                            domask = False

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
                        if ( domask ):
                            #nlines=fl.find_lines(mask=linemask,nRow=irow,edge=edge)
                            nlines.append(fl.find_lines(mask=linemask,nRow=irow,edge=edge))
                        else:
                            #nlines=fl.find_lines(nRow=irow,edge=edge)
                            nlines.append(fl.find_lines(nRow=irow,edge=edge))
                        # Get ranges

                        ptout="SCAN[%d] IF[%d] POL[%d]: " %(s.getscan(irow), s.getif(irow), s.getpol(irow))
                        if ( nlines[irow] > 0 ):
                                ll = fl.get_ranges()
                                #print ptout, "Found ", nlines[irow], " lines at ", ll
                                casalog.post( ptout+"Found %d lines at %s" % (nlines[irow], str(ll) ) )
                        else:
                                ll = ()
                                #print ptout, "Nothing found."
                                casalog.post( ptout+"Nothing found.", priority = 'WARN' )

                        # This is a linear list of pairs of values, so turn these into a list of lists
                        llisttmp = []
                        for i in range(nlines[irow]):
                            lo = ll[2*i]
                            up = ll[2*i+1]
                            if specunit == 'km/s':
                                    tmp=lo
                                    lo=up
                                    up=tmp
                            llisttmp = llisttmp + [[lo,up]]
                        linelist.append(llisttmp)
                    # Done with linefinder
                    #print "Finished linefinder."
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
                        #print "start ", irow
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
                            if ( domask ):
                                    #maxl = s.stats('max',linemask)[irow]
                                    #suml = s.stats('sum',linemask)[irow]
                                    maxl = s._math._stats(s,linemask,'max')[irow]
                                    suml = s._math._stats(s,linemask,'sum')[irow]
                            else:
                                    #maxl = s.stats('max')[irow]
                                    #suml = s.stats('sum')[irow]
                                    maxl = s._math._stats(s,[],'max')[irow]
                                    suml = s._math._stats(s,[],'sum')[irow]
                                    
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
                    init_plot( f, s.nrow())
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

                    #print "Will fit ",ncomps," components in ",numfit," regions"
                    casalog.post( "Will fit %d components in %d regions" % (ncomps, numfit) )

                    if (numfit > 0):
                            # Fit the line using numfit gaussians or lorentzians
                            # Assume the nfit list matches maskline
                            #f=sd.fitter()
			    if (fitfunc == 'lorentz'):
				    f.set_function(lorentz=ncomps)
			    else:
				    f.set_function(gauss=ncomps)
				    
                            if ( domask ):
                                    f.set_scan(s,linemask)
                            else:
                                    f.set_scan(s)
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
                                    #print "Fitting lines without starting guess"
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
                                    #print 'Fitting:'
                                    #print 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow))
                                    #print "   Fit failed to converge"
                                    casalog.post( 'Fitting:' )
                                    casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
                                    casalog.post( "   Fit failed to converge", priority = 'WARN' )
                            # Clean up
                            #del f
                    else:
                            fitparams.append([[0,0,0]])
                            retValue['nfit']+=[-1]
                            #print 'Fitting:'
                            #print 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow))
                            #print '   Fit failed.'
                            casalog.post( 'Fitting:' )
                            casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
                            casalog.post( '   Fit failed.', priority = 'WARN' )
                else:
                    fitparams.append([[0,0,0]])
                    retValue['nfit']+=[-1]
                    #print 'Fitting:'
                    #print 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow))
                    #print '   No lines detected.'
                    casalog.post( 'Fitting:' )
                    casalog.post( 'Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]' %(s.getscan(irow), s.getbeam(irow), s.getif(irow), s.getpol(irow), s.getcycle(irow)) )
                    casalog.post( '   No lines detected.', priority = 'WARN' )

                # plot
                if ( abs(plotlevel) > 0 ):
                        if ( irow < 16 ):
                                plot( f, irow, fitted, residual )

            # Store fit
            if ( fitfile != '' ):
                    #f.store_fit(fitfile)
                    #print fitparams
                    store_fit(fitfunc, fitfile, fitparams, s, overwrite)
                
            # Final clean up
            del f
            del s
            if (domask): del linemask

            return retValue
            # DONE

        except Exception, instance:
                #print '***Error***', instance
                casalog.post( str(instance), priority = 'ERROR' )
                return

        finally:
                casalog.post('')
                

### store_fit
def store_fit(func, fitfile, value, scan, overwrite):
        outname=os.path.expandvars(fitfile)
        outname=os.path.expanduser(outname)
        if os.path.exists(outname):
                if overwrite:
                        os.system('rm '+outname)
                else:
                        #print fitfile+' already exists.'
                        casalog.post( fitfile+' already exists.' )
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


### retrieve_fit
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


### init_plot
def init_plot( fitter, n ):
        # set nrow and ncol (maximum 4x4)
        if n < 3:
                nrow=1
                ncol=n
        elif n < 5:
                nrow=2
                ncol=2
        elif n < 7:
                nrow=2
                ncol=3
        elif n < 10:
                nrow=3
                ncol=3
        else:
                nrow=4
                ncol=4
        #print 'nrow,ncol=', nrow, ',', ncol
        casalog.post( 'nrow,ncol= %d,%d' % (nrow, ncol ) )
        if n > 16:
                #print 'Only first 16 results are plotted.'
                casalog.post( 'Only first 16 results are plotted.', priority = 'WARN' )
        
        # initialize plotter
        fitter._p
        if not fitter._p or fitter._p.is_dead:
                if sd.rcParams['plotter.gui']:
                        from asap.asaplotgui import asaplotgui as asaplot
                else:
                        from asap.asaplot import asaplot
        fitter._p=asaplot()
        fitter._p.hold()
        fitter._p.clear()
        fitter._p.set_panels(nrow, ncol, ganged=False)


### plot
def plot( fitter, irow, fitted, residual ):
        colors=["#777777", "#dddddd", "red", "orange", "purple", "green", "magenta", "cyan"]
        from matplotlib.numerix import ma, array, logical_not, logical_and
        myp=fitter._p

        myp.subplot(irow)
        # plot spectra
        myp.palette(1,colors)
        x=fitter.data._getabcissa(irow)
        y=fitter.data._getspectrum(irow)
        msk=array(fitter.data._getmask(irow))
        fmsk=array(fitter.mask)
        allmsk=logical_and(msk,fmsk)
	yorg=y
	nomask=True
	for i in range(len(allmsk)):
		nomask = nomask and allmsk[i]
	label0=['Selected Region','select']
	label1=['Spectrum','spec']
	if ( nomask ):
		label0[0]=label1[0]
		label0[1]=label1[1]
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
	y=ma.masked_array(yorg,mask=logical_not(allmsk))
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
                        y=ma.masked_array(fitter.get_residual(),mask=logical_not(allmsk))
                        myp.plot(x,y)
                # plot fit
                myp.palette(2)
                if ( irow==0 ):
                        myp.set_line(label='Fit')
                else:
                        myp.set_line(label='fit')
                y=ma.masked_array(fitter.fitter.getfit(),mask=logical_not(allmsk))
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
        
