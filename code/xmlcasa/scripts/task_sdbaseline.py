import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
import pylab as pl

def sdbaseline(sdfile, fluxunit, telescopeparm, specunit, frame, doppler, scanlist, field, iflist, pollist, tau, blmode, blpoly, verify, masklist, thresh, avg_limit, edge, outfile, outform, overwrite, plotlevel):


        casalog.origin('sdbaseline')


        ###
        ### Now the actual task code
        ###

        try:
            #load the data with or without averaging
            if sdfile=='':
                    raise Exception, 'sdfile is undefined'
            
            filename = os.path.expandvars(sdfile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                s = "File '%s' not found." % (filename)
                raise Exception, s

            # Default file name
            if ( outfile == '' ):
                    project = sdfile.rstrip('/') + '_bs'
            else:
                    project = outfile
            outfilename = os.path.expandvars(project)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s

            s=sd.scantable(sdfile,False)
            if ( abs(plotlevel) > 1 ):
                    # print summary of input data
                    print "Initial Raw Scantable:"
                    print s
            
            # check if the data contains spectra
            if (s.nchan()==1):
               s = "Cannot process the input data. It contains only single channel data."
               raise Exception, s

            # get telescope name
            #'ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43' (Tid), 'CEDUNA', and 'HOBART'
            antennaname = s.get_antennaname()

            # check current fluxunit
            # for GBT if not set, set assumed fluxunit, Kelvin
            fluxunit_now = s.get_fluxunit()
            if ( antennaname == 'GBT'):
                            if (fluxunit_now == ''):
                                    print "No fluxunit in the data. Set to Kelvin."
                                    s.set_fluxunit('K')
                                    fluxunit_now = s.get_fluxunit()

            print "Current fluxunit = "+fluxunit_now

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
                                            print "No need to change default fluxunits"
                                    else:
                                            s.set_fluxunit(fluxunit)
                                            print "Reset default fluxunit to "+fluxunit
                                            fluxunit_now = s.get_fluxunit()
                            else:
                                    print "Warning - no fluxunit for set_fluxunit"


            elif ( fluxunit=='' or fluxunit==fluxunit_now ):
                    if ( fluxunit==fluxunit_now ):
                            print "No need to convert fluxunits"

            elif ( type(telescopeparm) == list ):
                    # User input telescope params
                    if ( len(telescopeparm) > 1 ):
                            D = telescopeparm[0]
                            eta = telescopeparm[1]
                            print "Use phys.diam D = %5.1f m" % (D)
                            print "Use ap.eff. eta = %5.3f " % (eta)
                            s.convert_flux(eta=eta,d=D)
                    elif ( len(telescopeparm) > 0 ):
                            jypk = telescopeparm[0]
                            print "Use gain = %6.4f Jy/K " % (jypk)
                            s.convert_flux(jyperk=jypk)
                    else:
                            print "Empty telescope list"

            elif ( telescopeparm=='' ):
                    if ( antennaname == 'GBT'):
                            # needs eventually to be in ASAP source code
                            print "Convert fluxunit to "+fluxunit
                            # THIS IS THE CHEESY PART
                            # Calculate ap.eff eta at rest freq
                            # Use Ruze law
                            #   eta=eta_0*exp(-(4pi*eps/lambda)**2)
                            # with
                            print "Using GBT parameters"
                            eps = 0.390  # mm
                            eta_0 = 0.71 # at infinite wavelength
                            # Ideally would use a freq in center of
                            # band, but rest freq is what I have
                            rf = s.get_restfreqs()[0][0]*1.0e-9 # GHz
                            eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
                            print "Calculated ap.eff. eta = %5.3f " % (eta)
                            print "At rest frequency %5.3f GHz" % (rf)
                            D = 104.9 # 100m x 110m
                            print "Assume phys.diam D = %5.1f m" % (D)
                            s.convert_flux(eta=eta,d=D)

                            print "Successfully converted fluxunit to "+fluxunit
                    elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
                            s.convert_flux()

                    else:
                            # Unknown telescope type
                            print "Unknown telescope - cannot convert"


            # set default spectral axis unit
            if ( specunit != '' ):
                    s.set_unit(specunit)

            # reset frame and doppler if needed
            if ( frame != '' ):
                    s.set_freqframe(frame)
            else:
                    print 'Using current frequency frame'

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
                    print 'Using current doppler convention'

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
                print '***Error***',instance
                return
            del sel

            scanns = s.getscannos()
            sn=list(scanns)
            print "Number of scans to be processed:", len(sn)

	    # Warning for multi-IF data
	    if len(s.getifnos()) > 1:
		print '\nWarning - The scantable contains multiple IF data.'
		print '          Note the same mask(s) are applied to all IFs based on CHANNELS.'
		print '          Baseline ranges may be incorrect for all but IF=%d.\n' % (s.getif(0))

            # do opacity (atmospheric optical depth) correction
            if ( tau > 0.0 ):
                    # recalculate az/el (NOT needed for GBT data)
                    if ( antennaname != 'GBT'): s.recalc_azel()
                    s.opacity(tau)

	    # Header data for saving parameters of baseline fit
	    header = "Source Table: "+sdfile+"\n"
	    header += " Output File: "+project+"\n"
	    header += "   Flux Unit: "+s.get_fluxunit()+"\n"
	    header += "     Abscissa: "+s.get_unit()+"\n"
	    header += "   Fit order: %d\n"%(blpoly)
	    header += "    Fit mode: "+blmode+"\n"
	    if blmode == 'auto':
		    header += "   Threshold: %f\n"%(thresh)
		    header += "   avg_limit: %d\n"%(avg_limit)
		    header += "        Edge: "+str(edge)+"\n"
	    elif blmode == 'list':
		    header += "   Fit Range: "+str(masklist)+"\n"

            # Polynomial baseline (if requested, e.g. blpoly=5)
            if ( blmode == 'auto' ):
                    # Auto-fit baseline
                    if( blpoly >= 0 ):
                            if (len(masklist) > 0):
                                    # Use baseline mask for regions to INCLUDE in baseline fit
                                    # Create mask using list, e.g. masklist=[[500,3500],[5000,7500]]
                                    basemask=s.create_mask(masklist)
                                    s.auto_poly_baseline(mask=basemask,order=blpoly,edge=edge,
                                                             threshold=thresh,chan_avg_limit=avg_limit,
                                                             plot=verify)
				    # NOTICE: Do not modify scantable before formatting output
				    masklists=s.masklists
				    rmsl=[]
				    if not isinstance(masklists,list) or len(masklists) != s.nrow():
					    raise Exception, "Number of rows != masklists."
				    for r in range(s.nrow()):
					    sr=s.get_row(row=r,insitu=False)
					    if masklists[r] is not None:
						    msk=sr.create_mask(masklists[r])
						    rmsl.append(sr.stats('rms',msk))
						    del msk
					    else:
						    rmsl.append(sr.stats('rms'))
				    # Format output
				    dataout=_format_output(s,s.blpars,rmsl,masklists)
				    del sr, rmsl, masklists
                                    del basemask
                            else:
                                    # Automatic polynomial baseline without mask
                                    s.auto_poly_baseline(order=blpoly,edge=edge,threshold=thresh,
                                                             chan_avg_limit=avg_limit,plot=verify)
				    # NOTICE: Do not modify scantable before formatting output
				    masklists=s.masklists
				    rmsl=[]
				    if not isinstance(masklists,list) or len(masklists) != s.nrow():
					    raise Exception, "Number of rows != masklists."
				    for r in range(s.nrow()):
					    sr=s.get_row(row=r,insitu=False)
					    if masklists[r] is not None:
						    msk=sr.create_mask(masklists[r])
						    rmsl.append(sr.stats('rms',msk))
						    del msk
					    else:
						    rmsl.append(sr.stats('rms'))
				    # Format output
				    dataout=_format_output(s,s.blpars,rmsl,masklists)
				    del sr, rmsl, masklists

            elif ( blmode == 'list' ):
                    # Fit in specified region
                    if( blpoly >= 0 ):
                            if (len(masklist) > 0):
                                    # Use baseline mask for regions to INCLUDE in baseline fit
                                    # Create mask using list, e.g. masklist=[[500,3500],[5000,7500]]
                                    basemask=s.create_mask(masklist)
                                    s.poly_baseline(mask=basemask,order=blpoly,plot=verify)
				    rmsl=list(s.stats('rms',basemask))
				    # NOTICE: Do not modify scantable before formatting output
				    dataout=_format_output(s,s.blpars,rmsl)

                                    del basemask
                            else:
                                    # Without mask
                                    s.poly_baseline(order=blpoly,plot=verify)
				    rmsl=list(s.stats('rms'))
				    # NOTICE: Do not modify scantable before formatting output
				    dataout=_format_output(s,s.blpars,rmsl)

	    elif ( blmode == 'interact'):
                    # Interactive masking
		    new_mask=sd.interactivemask()
		    if (len(masklist) > 0):
			    new_mask.select_mask(s,masklist)
		    else:
			    new_mask.select_mask(s)

		    # Get final mask list
		    msk=new_mask.get_mask()
		    del new_mask
		    msks=s.get_masklist(msk)
		    if len(msks) < 1:
			    print 'No channel is selected. Exit without baselining.'
			    return
		    print 'final mask list ('+s._getabcissalabel()+') =',msks
		    header += "   Fit Range: "+str(msks)+"\n"
		    # Calculate base-line RMS
		    if len(msks) > 0:
			    s.poly_baseline(mask=msk,order=blpoly,plot=verify)
			    rmsl=list(s.stats('rms',msk))
			    # NOTICE: Do not modify scantable before formatting output
			    dataout=_format_output(s,s.blpars,rmsl)
		    else:
			    s.poly_baseline(order=blpoly,plot=verify)
			    rmsl=list(s.stats('rms'))
			    # NOTICE: Do not modify scantable before formatting output
			    dataout=_format_output(s,s.blpars,rmsl)
		    del msk, msks

            # Plot final spectrum
            if ( abs(plotlevel) > 0 ):
                    # each IF is separate panel, pols stacked
                    sd.plotter.plot(s)
                    sd.plotter.set_mode(stacking='p',panelling='i')
                    sd.plotter.set_histogram(hist=True)
                    sd.plotter.axhline(color='r',linewidth=2)
                    if ( plotlevel < 0 ):
                            # Hardcopy - currently no way w/o screen display first
                            pltfile=project+'_bsspec.eps'
                            sd.plotter.save(pltfile)


            # Now save the spectrum and write out final ms
            if ( (outform == 'ASCII') or (outform == 'ascii') ):
                    outform = 'ASCII'
                    spefile = project + '_'
            elif ( (outform == 'ASAP') or (outform == 'asap') ):
                    outform = 'ASAP'
                    spefile = project
            elif ( (outform == 'SDFITS') or (outform == 'sdfits') ):
                    outform = 'SDFITS'
                    spefile = project
            elif ( (outform == 'MS') or (outform == 'ms') or (outform == 'MS2') or (outform == 'ms2') ):
                    outform = 'MS2'
                    spefile = project
            else:
                    outform = 'ASAP'
                    spefile = project

            if overwrite and os.path.exists(outfilename):
                    os.system('rm -rf %s' % outfilename)

            # to apply data selections by selector, make copy
            tmpscn=s.copy()
            tmpscn.save(spefile,outform,overwrite)
            if outform!='ASCII':print "Wrote output "+outform+" file "+spefile

	    # Save parameters of baseline fit
	    blfile=project+'_blparam.txt'
	    separator = "#"*60+"\n"
	    blf=open(blfile,'w')
	    blf.write(separator)
	    blf.write(header)
	    blf.write(separator)
	    blf.write(dataout)
	    blf.close()
	    
            # Clean up scantable
            del s, tmpscn

        except Exception, instance:
                print '***Error***',instance
                return


### Format baseline parameters for output
def _format_output(scan=None,pars=None,rms=None,masklists=None):
	out = ""
	# Check input scantable
	if not isinstance(scan, Scantable):
		print "Error: Data is not scantable"
		return
	# Check baseline fit parameters
	if not isinstance(pars,list):
		print "Error: Invalid baseline parameters."
		return
	elif scan.nrow() != len(pars):
		print "Error: Number of rows != baseline params sets."
		return
        # Check rms data
        if isinstance(rms,list) and len(rms) == scan.nrow(): sflag=True
        else: 
		print "Error: Invalid rms data"
		return
        # Check masklists data
        mflag=False
        if masklists is None: mflag=False
        elif isinstance(masklists,list) and len(masklists) == scan.nrow():
		mflag=True
        else: 
		print "Error: Invalid masklists"
		return

	# Format data output
	sep = "-"*60+"\n"
	out += sep
	
	for r in xrange(scan.nrow()):
		out+=" Scan[%d] Beam[%d] IF[%d] Pol[%d] Cycle[%d]: \n" % \
		      (scan.getscan(r), scan.getbeam(r), scan.getif(r), \
		       scan.getpol(r), scan.getcycle(r))
		if mflag:
			out += "Fitted range = "+str(masklists[r])+"\n"
		out+="Baseline parameters\n"
		if pars[r] is not None: 
			cpars = pars[r]['params']
			cfixed = pars[r]['fixed']
			c = 0
			for i in range(len(cpars)):
				fix = ""
				if len(cfixed) and cfixed[i]: fix = "(fixed)"
				out += '  p%d%s= %3.6f,' % (c,fix,cpars[i])
				c+=1
			out = out[:-1]  # remove trailing ','
			out+="\n"
		else: out += '  Not fitted\n'
		out += "Results of baseline fit\n"
		out += "  rms = %3.6f\n" % (rms[r])
		out+=sep
	return out

