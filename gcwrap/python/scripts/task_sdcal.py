import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
import pylab as pl
from asap import _to_list
import sdutil

#def sdcal(infile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, scanlist, field, iflist, pollist, channelrange, align, reftime, interp, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, verify, outfile, outform, overwrite, plotlevel):
def sdcal(infile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, fraction, noff, width, elongated, markonly, plotpointings, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, verify, outfile, outform, overwrite, plotlevel):
        
        casalog.origin('sdcal')

        ###
        ### Now the actual task code
        ###

        restorer = None
        
        try:
            #load the data with or without averaging
            if infile=='':
                    raise Exception, 'infile is undefined'

            sdutil.assert_infile_exists(infile)

            project = sdutil.get_default_outfile_name(infile,
                                                      outfile,
                                                      '_cal')
            sdutil.assert_outfile_canoverwrite_or_nonexistent(project,
                                                              outform,
                                                              overwrite)

            #s=sd.scantable(infile,average=scanaverage,antenna=antenna)
            s=sd.scantable(infile,average=False,antenna=antenna)

            if not isinstance(s,Scantable):
                    raise Exception, 'Scantable data %s, is not found'

            prior_plot(s, plotlevel)

            # prepare for restore scantable
            restorer = sdutil.scantable_restore_factory(s,
                                                        infile,
                                                        fluxunit,
                                                        specunit,
                                                        frame,
                                                        doppler)

            # set default spectral axis unit
            sdutil.set_spectral_unit(s, specunit)

            # reset frame and doppler if needed
            sdutil.set_freqframe(s, frame)
            sdutil.set_doppler(s, doppler)

            # A scantable selection
            sel = sdutil.get_selector(scanlist, iflist, pollist,
                                      field)
            #Apply the selection
            s.set_selection(sel)
            del sel


            # calibration
            scal = docalibration(s, calmode, fraction, noff, width,
                                 elongated, markonly, plotpointings, verify)

            # Done with scantable s - clean up to free memory
            del s

	    # channel splitting
            sdutil.dochannelrange(scal, channelrange)

            # convert flux
            sdutil.set_fluxunit(scal, fluxunit, telescopeparm, insitu=True)

            # do opacity (atmospheric optical depth) correction
            sdutil.doopacity(scal, tau)

            # Align frequencies if desired
            #if ( align ):
            #        if reftime == '': reftime=None
            #        scal.freq_align(reftime=reftime,method=interp,insitu=True)

            # Average in time if desired
            spave = sdutil.doaverage(scal, scanaverage, timeaverage,
                                     tweight, polaverage, pweight,
                                     averageall)

            posterior_plot(spave, project, plotlevel)


            # Now save the spectrum and write out final ms
            sdutil.save(spave, project, outform, overwrite)

            # Clean up scantable
            del spave

            # DONE

        except Exception, instance:
                sdutil.process_exception(instance)
                raise Exception, instance
                return
        finally:
                if restorer is not None:
                        restorer.restore()
                        del restorer

def docalibration(s, calmode, fraction, noff, width, elongated,
                  markonly, plotpointings, verify=False):
    scanns = s.getscannos()
    sn=list(scanns)
    casalog.post( "Number of scans to be processed: %d" % (len(sn)) )
    if calmode == 'otf' or calmode=='otfraster':
        s2 = _mark( s,
                    (calmode=='otfraster'),
                    fraction=fraction,
                    npts=noff,
                    width=width,
                    elongated=elongated,
                    plot=plotpointings )
        if markonly:
            scal = s2
        else:
            scal = sd.asapmath.calibrate( s2,
                                          scannos=sn,
                                          calmode='ps',
                                          verify=verify )
    else:
        scal = sd.asapmath.calibrate( s,
                                      scannos=sn,
                                      calmode=calmode,
                                      verify=verify )
    return scal

def prior_plot(s, plotlevel):
    if ( abs(plotlevel) > 1 ):
        # print summary of input data
        #print "Initial Raw Scantable:"
        #print s
        casalog.post( "Initial Raw Scantable:" )
        #casalog.post( s._summary() )
        s._summary()

def posterior_plot(s, project, plotlevel):
    if ( abs(plotlevel) > 1 ):
        # print summary of calibrated data
        #print "Final Calibrated Scantable:"
        #print spave
        casalog.post( "Final Calibrated Scantable:" )
        #casalog.post( spave._summary() )
        s._summary()

    # Plot final spectrum
    if ( abs(plotlevel) > 0 ):
        pltfile = project + '_calspec.eps'
        sdutil.plot_scantable(s, pltfile, plotlevel)

def _mark(s, israster, *args, **kwargs):
        marker = sd.edgemarker( israster=israster )
        marker.setdata( s )
        marker.setoption( *args, **kwargs )
        marker.mark()
        if kwargs.has_key('plot') and kwargs['plot']:
                marker.plot()
        return marker.getresult()
