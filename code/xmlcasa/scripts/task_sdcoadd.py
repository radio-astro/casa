import os
from taskinit import *

import asap as sd
import pylab as pl

def sdcoadd(sdfilelist, fluxunit, telescopeparm, specunit, frame, doppler, scanaverage, timeaverage, polaverage, outfile, outform, overwrite):

        casalog.origin('sdcoadd')


        ###
        ### Now the actual task code
        ###

        try:
            import os.path
            scanlist = []
            nrow=0
            if len(sdfilelist)<2:
                 raise Exception, 'Need at least two data file names'

            # check output file name
            if outfile=='':
                outfile=sdfilelist[0].rstrip('/')+'_coadd'
            outfilename = os.path.expandvars(outfile)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s


            #load each the data in the list with or without averaging
            for i in range(len(sdfilelist)):
                filename = os.path.expandvars(sdfilelist[i])
                filename = os.path.expanduser(filename)
                if not os.path.exists(filename):
                    s = "File '%s' not found." % (filename)
                    raise Exception, s
                else:
                    scanlist.append(sd.scantable(sdfilelist[i],scanaverage))
                    nrow+=scanlist[i].nrow()

                    # get telescope name
                    antennaname = scanlist[i].get_antennaname()
                    fluxunit_now = scanlist[i].get_fluxunit()

                    if ( antennaname == 'GBT'):
                            if (fluxunit_now == ''):
                                    #print "No fluxunit in the data. Set to Kelvin."
                                    casalog.post( "No fluxunit in the data. Set to Kelvin." )
                                    scanlist[i].set_fluxunit('K')
                                    fluxunit_now = scanlist[i].get_fluxunit()
                    #print "Current fluxunit = "+fluxunit_now

                    # set default spectral axis unit
                    if ( specunit != '' ):
                        scanlist[i].set_unit(specunit)

                    # reset frame and doppler if needed
                    if ( frame != '' ):
                        scanlist[i].set_freqframe(frame)
                    #else:
                    #    print 'Using current frequency frame'

                    if ( doppler != '' ):
                        if ( doppler == 'radio' ):
                            ddoppler = 'RADIO'
                        elif ( doppler == 'optical' ):
                            ddoppler = 'OPTICAL'
                        elif ( doppler == 'z' ):
                            ddoppler = 'Z'
                        else:
                            ddoppler = doppler

                        scanlist[i].set_doppler(ddoppler)
                    #else:
                    #    print 'Using current doppler convention'

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
                                scanlist[i].set_fluxunit(fluxunit)
                                #print "Reset default fluxunit to "+fluxunit
                                casalog.post( "Reset default fluxunit to "+fluxunit )
                                fluxunit_now = scanlist[i].get_fluxunit()
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
                            scanlist[i].convert_flux(eta=eta,d=D)
                        elif ( len(telescopeparm) > 0 ):
                            jypk = telescopeparm[0]
                            #print "Use gain = %6.4f Jy/K " % (jypk)
                            casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
                            scanlist[i].convert_flux(jyperk=jypk)
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
                            rf = scanlist[i].get_restfreqs()[0][0]*1.0e-9 # GHz
                            eta = eta_0*pl.exp(-0.001757*(eps*rf)**2)
                            #print "Calculated ap.eff. eta = %5.3f " % (eta)
                            #print "At rest frequency %5.3f GHz" % (rf)
                            casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
                            casalog.post( "At rest frequency %5.3f GHz" % (rf) )
                            D = 104.9 # 100m x 110m
                            #print "Assume phys.diam D = %5.1f m" % (D)
                            casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
                            scanlist[i].convert_flux(eta=eta,d=D)

                            #print "Successfully converted fluxunit to "+fluxunit
                            casalog.post( "Successfully converted fluxunit to "+fluxunit )
                        elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
                            scanlist[i].convert_flux()

                        else:
                            # Unknown telescope type
                            #print "Unknown telescope - cannot convert"
                            casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )

            merged=sd.merge(scanlist)
            #print "Coadded %s" % sdfilelist
            casalog.post( "Coadded %s" % sdfilelist )
            if (nrow>merged.nrow()): 
                #print "WARNING: Actual number of rows is less than the number of rows expected in merged data."
                #print "         Possibly, there are conformance error among the input data."
                casalog.post( "Actual number of rows is less than the number of rows expected in merged data.", priority = 'WARN' )
                casalog.post( "Possibly, there are conformance error among the input data.", priority = 'WARN' )
            
            # Average in time if desired
            if ( timeaverage ):
                stave=sd.average_time(merged,weight='tintsys')
                #print "Averaged scans in time"
                casalog.post( "Averaged scans in time" )
                # Now average over polarizations;Tsys-weighted (1/Tsys**2) average
                if ( polaverage ):
                    np = stave.npol()
                    if ( np > 1 ):
                        spave=stave.average_pol(weight='tsys')
                        #print "Averaged polarization"
                        casalog.post( "Averaged polarization" )
                    else:
                        # only single polarization
                        #print "Single polarization data - no need to average"
                        casalog.post( "Single polarization data - no need to average" )
                        spave=stave.copy()
                else:
                    spave=stave.copy()

                del stave
            else:
                if ( polaverage ):
                    np = merged.npol()
                    if ( np > 1 ):
                        spave=merged.average_pol(weight='tsys')
                        #print "Averaged polarization"
                        casalog.post( "Averaged polarization" )
                    else:
                        # only single polarization
                        #print "Single polarization data - no need to average"
                        casalog.post( "Single polarization data - no need to average" )
                        spave=merged.copy()
                else:
                    spave=merged.copy()
            
            #if outfile=='':
            #    outfile=sdfilelist[0]+'_coadd'
            # save
            if ( (outform == 'ASCII') or (outform == 'ascii') ):
                    outform = 'ASCII'
            elif ( (outform == 'ASAP') or (outform == 'asap') ):
                    outform = 'ASAP'
            elif ( (outform == 'SDFITS') or (outform == 'sdfits') ):
                    outform = 'SDFITS'
            elif ( (outform == 'MS') or (outform == 'ms') or (outform == 'MS2') or (outform == 'ms2') ):
                    outform = 'MS2'
            else:
                    outform = 'ASAP'


            if overwrite and os.path.exists(outfilename):
               os.system('rm -rf %s' % outfilename) 
      
            spave.save(outfile,outform,overwrite)
            if outform!='ASCII':
                    #print "Wrote output "+outform+" file "+outfile
                    casalog.post( "Wrote output "+outform+" file "+outfile )

            # Clean up scantable
            del merged, spave, scanlist
            # DONE
        except Exception, instance:
                #print '***Error***',instance
                casalog.post( str(instance), priority = 'ERROR' )
                return


