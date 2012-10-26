import os
from taskinit import *

import asap as sd
from asap.scantable import is_scantable
import pylab as pl
import sdutil

def sdcoadd(infiles, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanaverage, timeaverage, tweight, polaverage, pweight, outfile, outform, overwrite):

        casalog.origin('sdcoadd')


        ###
        ### Now the actual task code
        ###

        try:
            import os.path
            scantablist = []
            # List of restorers
            restore = []
            
            if len(infiles)<2:
                 raise Exception, 'Need at least two data file names'

            # check output file name
            project = sdutil.get_default_outfile_name(infiles[0],
                                                      outfile,
                                                      '_coadd')
            sdutil.assert_outfile_canoverwrite_or_nonexistent(project,
                                                              outform,
                                                              overwrite)
            outfilename = sdutil.get_abspath(project)

            #load each the data in the list with or without averaging
            for i in range(len(infiles)):
                sdutil.assert_infile_exists(infiles[i])
                filename = sdutil.get_abspath(infiles[i])
            
                thisscan = sd.scantable(infiles[i],average=scanaverage,antenna=antenna)
                # prepare restorer object
                restorer = sdutil.scantable_restore_factory(thisscan,
                                                            filename,
                                                            fluxunit,
                                                            specunit,
                                                            frame,
                                                            doppler)


                # set default spectral axis unit
                sdutil.set_spectral_unit(thisscan, specunit)
                
                # reset frame and doppler if needed
                sdutil.set_freqframe(thisscan, frame)
                sdutil.set_doppler(thisscan, doppler)

                # convert flux
                stmp = sdutil.set_fluxunit(thisscan, fluxunit, telescopeparm, False)
                if stmp:
                    # Restore header in original table before deleting
                    if restorer is not None:
                        restorer.restore()
                        del restorer
                        restorer = None
                    del thisscan
                    thisscan = stmp
                    del stmp

                scantablist.append(thisscan)
                restore.append(restorer)

            # merge scantables
            merged = merge(scantablist)
            #print "Coadded %s" % infiles
            casalog.post( "Coadded %s" % infiles )
            
            # Average in time if desired
            spave = sdutil.doaverage(merged, scanaverage, timeaverage,
                                     tweight, polaverage, pweight)
            
            # save
            sdutil.save(spave, project, outform, overwrite)

            # Clean up scantable
            del merged, spave
            # DONE
        except Exception, instance:
                #print '***Error***',instance
                import traceback
                print traceback.format_exc()
                casalog.post( str(instance), priority = 'ERROR' )
                raise Exception, instance
                return
        finally:
                try:
                        # Restore header information in the table
                        for i in range(len(scantablist)):
                            if restore[i] is not None:
                                restore[i].restore()
                        del restore

                       # Final clean up
                        del scantablist
                except:
                        pass
                casalog.post('')
                

def merge(scantablist):
    merged=sd.merge(scantablist)

    # check for nrow
    nrow = 0
    for scan in scantablist:
        nrow += scan.nrow()    
    if (nrow>merged.nrow()): 
        #print "WARNING: Actual number of rows is less than the number of rows expected in merged data."
        #print "         Possibly, there are conformance error among the input data."
        casalog.post( "Actual number of rows is less than the number of rows expected in merged data.", priority = 'WARN' )
        casalog.post( "Possibly, there are conformance error among the input data.", priority = 'WARN' )
        
    return merged
