import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
from asap import _to_list
from asap.scantable import is_scantable

def sdsmooth(infile, antenna, scanaverage, scanlist, field, iflist, pollist, kernel, kwidth, chanwidth, verify, outfile, outform, overwrite, plotlevel):

        casalog.origin('sdsmooth')


        try:
            #load the data with or without averaging
            if infile=='':
                    raise Exception, 'infile is undefined'

            filename = os.path.expandvars(infile)
            filename = os.path.expanduser(filename)
            if not os.path.exists(filename):
                errstr = "File '%s' not found." % (filename)
                raise Exception, errstr

            # Default file name
            if ( outfile == '' ):
                    project = infile.rstrip('/') + '_sm'
            else:
                    project = outfile
            outfilename = os.path.expandvars(project)
            outfilename = os.path.expanduser(outfilename)
            if not overwrite and (outform!='ascii' and outform!='ASCII'):
                if os.path.exists(outfilename):
                    errstr = "Output file '%s' exist." % (outfilename)
                    raise Exception, errstr

            sorg=sd.scantable(infile,average=False,antenna=antenna)
            if not (isinstance(sorg,Scantable)):
                    raise Exception, 'infile=%s is not found' % infile

            if ( abs(plotlevel) > 1 ):
                    # print summary of input data
                    casalog.post( "Initial Scantable:" )
                    #casalog.post( s._summary() )
                    sorg._summary()


            # A scantable selection
            # Scan selection
            scans = _to_list(scanlist,int) or []

            # IF selection
            ifs = _to_list(iflist,int) or []

            # Select polarizations
            pols = _to_list(pollist,int) or []

            # Actual selection
            sel = sd.selector(scans=scans, ifs=ifs, pols=pols)

            # Select source names
            if ( field != '' ):
                    sel.set_name(field)
                    # NOTE: currently can only select one
                    # set of names this way, will probably
                    # need to do a set_query eventually


            try:
                #Apply the selection
                sorg.set_selection(sel)
                del sel
            except Exception, instance:
                casalog.post( str(instance), priority = 'ERROR' )
                casalog.post( 'No output written.' )
                return

            # Copy scantable when usign disk storage not to modify
            # the original table.
            if is_scantable(infile) and \
                   sd.rcParams['scantable.storage'] == 'disk':
                    s = sorg.copy()
            else:
                    s = sorg
            del sorg

            #Average within each scan
            if scanaverage:
                    stmp = s.copy()
                    s=stmp.average_time(scanav=True)
                    del stmp

            # Smooth the spectrum
            if kernel=='' or kernel=='none':
                errstr = "kernel need to be specified"
                raise Exception, errstr
            elif kernel!='hanning' and kwidth<=0:
                errstr = "kernel should be > 0"
                raise Exception, errstr
            else:
                    if ( abs(plotlevel) > 0 ):
                            # plot spectrum before smoothing
                            # reset plotter
                            if sd.plotter._plotter:
                                    sd.plotter._plotter.quit()
                            visible = sd.plotter._visible
                            sd.plotter.__init__(visible=visible)
                            sd.plotter.set_linestyles(linewidth=1,refresh=False)
                            # each IF is separate panel, pols stacked
                            sd.plotter.set_mode(stacking='p',panelling='i',refresh=False)
                            sd.plotter.plot(s)
                            # somehow I need to put text() twice in order to the second
                            # text() actually displays on the plot...
                            sd.plotter.text(0.0, 1.0,'',coords='relative')
                            sd.plotter.text(0.0, 1.0,'Raw spectra', coords='relative')
                            if ( plotlevel < -1 ):
                                    # Hardcopy - currently no way w/o screen displayfirst
                                    pltfile=project+'_rawspec.eps'
                                    sd.plotter.save(pltfile)

                    if kernel == 'regrid':
                            if not qa.isquantity(chanwidth):
                                    errstr = "Invalid quantity chanwidth "+chanwidth
                                    raise Exception, errstr
                            qchw = qa.quantity(chanwidth)
                            oldUnit = s.get_unit()
                            if qchw['unit'] in ("", "channel", "pixel"):
                                    s.set_unit("channel")
                            elif qa.compare(chanwidth,"1Hz") or \
                               qa.compare(chanwidth,"1m/s"):
                                    s.set_unit(qchw['unit'])
                            else:
                                    errstr = "Invalid dimension of quantity chanwidth "+chanwidth
                                    raise Exception, errstr
                            casalog.post( "Regridding spectra in width "+chanwidth )
                            s.regrid_channel(width=qchw['value'],plot=verify,insitu=True)
                            s.set_unit(oldUnit)
                    else:
                            casalog.post( "Smoothing spectra with kernel "+kernel )
                            s.smooth(kernel=kernel,width=kwidth,plot=verify,insitu=True)

                    if ( abs(plotlevel) > 0 ):
                            # plot spectrum after smoothing
                            # reset plotter
                            if sd.plotter._plotter:
                                    sd.plotter._plotter.quit()
                            visible = sd.plotter._visible
                            sd.plotter.__init__(visible=visible)
                            sd.plotter.set_linestyles(linewidth=1,refresh=False)
                            # each IF is separate panel, pols stacked
                            sd.plotter.set_mode(stacking='p',panelling='i',refresh=False)
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

            s.save(smfile,outform,overwrite)
            if outform!='ASCII':
                    casalog.post( "Writing output "+outform+" file "+smfile )

            del s

        except Exception, instance:
                casalog.post( str(instance), priority = 'ERROR' )
                return


