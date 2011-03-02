import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
import pylab as pl

def sdaverage(sdfile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, verify, outfile, outform, overwrite, plotlevel):

        casalog.origin('sdaverage')

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

            if ( outfile == '' ):
                    project = sdfile.rstrip('/') + '_cal'
            else:
                    project = outfile
            outfilename = os.path.expandvars(project)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s


            s=sd.scantable(sdfile,average=scanaverage,antenna=antenna)

            if not isinstance(s,Scantable):
                    raise Exception, 'Scantable data %s, is not found'

            if ( abs(plotlevel) > 1 ):
                    # print summary of input data
                    #print "Initial Raw Scantable:"
                    #print s
                    casalog.post( "Initial Raw Scantable:" )
                    casalog.post( s._summary() )
                    casalog.post( "--------------------------------------------------------------------------------" )

            # Default file name
            #if ( outfile == '' ):
            #        project = sdfile + '_cal'
            #else:
            #        project = outfile

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
            casalog.post( "Current fluxunit = " + fluxunit_now ) 

            # set default spectral axis unit
            if ( specunit != '' ):
                    s.set_unit(specunit)

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
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return
            del sel

	    # channel splitting
	    if ( channelrange != [] ):
		    if ( len(channelrange) == 1 ):
                            #print "Split spectrum in the range [%d, %d]" % (0, channelrange[0])
                            casalog.post( "Split spectrum in the range [%d, %d]" % (0, channelrange[0]) )
			    s._reshape( 0, int(channelrange[0]) )
		    else:
                            #print "Split spectrum in the range [%d, %d]" % (channelrange[0], channelrange[1])
                            casalog.post( "Split spectrum in the range [%d, %d]" % (channelrange[0], channelrange[1]) )
			    s._reshape( int(channelrange[0]), int(channelrange[1]) )

            scanns = s.getscannos()
            sn=list(scanns)
            #print "Number of scans to be processed:", len(sn)
            casalog.post( "Number of scans to be processed: %d" % (len(sn)) )
            scal = sd.asapmath.calibrate( s, scannos=sn, calmode=calmode, verify=verify )

            # Done with scantable s - clean up to free memory
            del s

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
                                            scal.set_fluxunit(fluxunit)
                                            #print "Reset default fluxunit to "+fluxunit
                                            casalog.post( "Reset default fluxunit to "+fluxunit )
                                            fluxunit_now = scal.get_fluxunit()
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
                            casalog.post( "Use ap.eff. eta = %5.3f " % (eta) )
                            scal.convert_flux(eta=eta,d=D)
                    elif ( len(telescopeparm) > 0 ):
                            jypk = telescopeparm[0]
                            #print "Use gain = %6.4f Jy/K " % (jypk)
                            casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
                            scal.convert_flux(jyperk=jypk)
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
                            rf = scal.get_restfreqs()[0][0]*1.0e-9 # GHz
                            eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
                            #print "Calculated ap.eff. eta = %5.3f " % (eta)
                            #print "At rest frequency %5.3f GHz" % (rf)
                            casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
                            casalog.post( "At rest frequency %5.3f GHz" % (rf) )
                            D = 104.9 # 100m x 110m
                            #print "Assume phys.diam D = %5.1f m" % (D)
                            casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
                            scal.convert_flux(eta=eta,d=D)

                            #print "Successfully converted fluxunit to "+fluxunit
                            casalog.post( "Successfully converted fluxunit to "+fluxunit )
                    elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
                            scal.convert_flux()

                    else:
                            # Unknown telescope type
                            #print "Unknown telescope - cannot convert"
                            casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )

            # do opacity (atmospheric optical depth) correction
            if ( tau > 0.0 ):
                    # recalculate az/el (NOT needed for GBT data)
                    if ( antennaname != 'GBT'): scal.recalc_azel()
                    scal.opacity(tau)

            # Average in time if desired
            if ( timeaverage ):
                    if tweight=='none':
                            errmsg = "Please specify weight type of time averaging"
                            raise Exception,errmsg
                    stave=sd.average_time(scal,weight=tweight,compel=averageall)
                    del scal
                    # Now average over polarizations;
                    if ( polaverage ):
                            if pweight=='none':
                                    errmsg = "Please specify weight type of polarization averaging"
                                    raise Exception,errmsg
                            np = len(stave.getpolnos())
                            if ( np > 1 ):
                                    spave=stave.average_pol(weight=pweight)
                            else:
                                    # only single polarization
                                    #print "Single polarization data - no need to average"
                                    casalog.post( "Single polarization data - no need to average" )
                                    spave=stave.copy()
                    else:
                            spave=stave.copy()
                    del stave
            else:
                    if ( scanaverage ):
                            # scan average if the input is a scantable
                            spave=sd.average_time(scal,scanav=True)
                            scal=spave.copy()
                    if ( polaverage ):
                            if pweight=='none':
                                    errmsg = "Please specify weight type of polarization averaging"
                                    raise Exception,errmsg
                            np = scal.npol()
                            if ( np > 1 ):
                                    spave=scal.average_pol(weight=pweight)
                            else:
                                    # only single polarization
                                    #print "Single polarization data - no need to average"
                                    casalog.post( "Single polarization data - no need to average" )
                                    spave=scal.copy()
                    else:
                            spave=scal.copy()
                    del scal

            if ( abs(plotlevel) > 1 ):
                    # print summary of calibrated data
                    #print "Final Calibrated Scantable:"
                    #print spave
                    casalog.post( "Final Calibrated Scantable:" )
                    casalog.post( spave._summary() )
                    casalog.post( "--------------------------------------------------------------------------------" )


            # Plot final spectrum
            if ( abs(plotlevel) > 0 ):
                    # each IF is separate panel, pols stacked
                    sd.plotter.plot(spave)
                    sd.plotter.set_mode(stacking='p',panelling='i')
                    sd.plotter.set_histogram(hist=True)
                    #sd.plotter.axhline(color='r',linewidth=2)
                    if ( plotlevel < 0 ):
                            # Hardcopy - currently no way w/o screen display first
                            pltfile=project+'_calspec.eps'
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

            spave.save(spefile,outform,overwrite)
            if outform!='ASCII':
                    #print "Wrote output "+outform+" file "+spefile
                    casalog.post( "Wrote output "+outform+" file "+spefile )

            # Clean up scantable
            del spave

            # DONE

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority='ERROR' )
                return

