import os
from taskinit import *

import asap as sd
from asap._asap import Scantable

def sdsmooth(sdfile, scanaverage, scanlist, field, iflist, pollist, kernel, kwidth, outfile, outform, overwrite, plotlevel):

        casalog.origin('sdsmooth')


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
                    project = sdfile.rstrip('/') + '_sm'
            else:
                    project = outfile
            outfilename = os.path.expandvars(project)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    s = "Output file '%s' exist." % (outfilename)
                    raise Exception, s

            s=sd.scantable(sdfile,scanaverage)
            if (isinstance(s,Scantable)):
                    stmp = s.copy()
                    s=stmp.average_time(scanav=True)
            else:
                    raise Exception, 'sdfile=%s is not found' % sdfile

            if ( abs(plotlevel) > 1 ):
                    # print summary of input data
                    #print "Initial Scantable:"
                    #print s
                    casalog.post( "Initial Scantable:" )
                    casalog.post( s._summary() )
                    casalog.post( "--------------------------------------------------------------------------------" )


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

            # Selec polarizations
            if (type(pollist) == list):
              pols = pollist
            else:
              pols = [pollist]
            if(len(pols) > 0 ):
              sel.set_polarisations(pols)

            try:
                #Apply the selection
                s.set_selection(sel)
                del sel
            except Exception, instance:
                #print '***Error***',instance
                #print 'No output written.'
                casalog.post( instance.message, priority = 'ERROR' )
                casalog.post( 'No output written.' )
                return

            # Smooth the spectrum
            if kernel=='' or kernel=='none':
                s = "kernel need to be specified"
                raise Exception, s
            if ( kernel != '' and (not (kwidth<=0 and kernel!='hanning'))):
                    if ( abs(plotlevel) > 0 ):
                            # plot spectrum before smoothing
                            # each IF is separate panel, pols stacked
                            sd.plotter.set_mode(stacking='p',panelling='i')
                            sd.plotter.plot(s)
                            # somehow I need to put text() twice in order to the second
                            # text() actually displays on the plot...
                            sd.plotter.text(0.0, 1.0,'',coords='relative')
                            sd.plotter.text(0.0, 1.0,'Raw spectra', coords='relative')
                            if ( plotlevel < -1 ):
                                    # Hardcopy - currently no way w/o screen displayfirst
                                    pltfile=project+'_rawspec.eps'
                                    sd.plotter.save(pltfile)
                    #print "Smoothing spectrum with kernel "+kernel
                    casalog.post( "Smoothing spectrum with kernel "+kernel )
                    s.smooth(kernel,kwidth,insitu=True)
                    if ( abs(plotlevel) > 0 ):
                           # plot spectrum after smoothing
                            # each IF is separate panel, pols stacked
                            sd.plotter.set_mode(stacking='p',panelling='i')
                            sd.plotter.plot(s)
                            sd.plotter.text(0.0, 1.0,'', coords='relative')
                            sd.plotter.text(0.0, 1.0,'Smoothed spectra', coords='relative')
                            if ( plotlevel < -1 ):
                                    # Hardcopy - currently no way w/o screen displayfirst
                                    pltfile=project+'_smspec.eps'
                                    sd.plotter.save(pltfile)

            if ( (outform == 'ASCII') or (outform == 'ascii') ):
                    outform = 'ASCII'
                    smfile = project + '_'
            elif ( (outform == 'ASAP') or (outform == 'asap') ):
                    outform = 'ASAP'
                    smfile = project
            elif ( (outform == 'SDFITS') or (outform == 'sdfits') ):
                    outform = 'SDFITS'
                    smfile = project
            elif ( (outform == 'MS') or (outform == 'ms') or (outform == 'MS2')or (outform == 'ms2') ):
                    outform = 'MS2'
                    smfile = project
            else:
                    outform = 'ASAP'
                    smfile = project

            if overwrite and os.path.exists(outfilename):
                    os.system('rm -rf %s' % outfilename)

            # when subset of the data selected by selector
            # scantable.save() does not work well, so create a copy first
            # before saving.
            s2=s.copy()
            s2.save(smfile,outform,overwrite)
            if outform!='ASCII':
                    #print "Writing output "+outform+" file "+smfile
                    casalog.post( "Writing output "+outform+" file "+smfile )
            # Clean up scantable
            del s, s2

        except Exception, instance:
                #print '***Error***',instance
                casalog.post( instance.message, priority = 'ERROR' )
                return


