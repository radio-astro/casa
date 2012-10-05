import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
from asap import _to_list
from asap.scantable import is_scantable
import sdutil

def sdsmooth(infile, antenna, scanaverage, scanlist, field, iflist, pollist, kernel, kwidth, chanwidth, verify, outfile, outform, overwrite, plotlevel):

        casalog.origin('sdsmooth')


        try:
            #load the data with or without averaging
            if infile=='':
                    raise Exception, 'infile is undefined'
            
            sdutil.assert_infile_exists(infile)

            # Default file name
            project = sdutil.get_default_outfile_name(infile,
                                                      outfile,
                                                      '_sm')
            sdutil.assert_outfile_canoverwrite_or_nonexistent(project,
                                                              outform,
                                                              overwrite)

            sorg=sd.scantable(infile,average=False,antenna=antenna)
            if not (isinstance(sorg,Scantable)):
                    raise Exception, 'infile=%s is not found' % infile


            # A scantable selection
            sel = sdutil.get_selector(scanlist, iflist, pollist,
                                      field)
            try:
                #Apply the selection
                sorg.set_selection(sel)
                del sel
            except Exception, instance:
                casalog.post( str(instance), priority = 'ERROR' )
                casalog.post( 'No output written.' )
                raise Exception, instance
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
                    s = sdutil.doaverage(s, scanaverage, True, 'tint',
                                         false, 'none')

            # Smooth the spectrum
            if kernel=='' or kernel=='none':
                errstr = "kernel need to be specified"
                raise Exception, errstr
            elif kernel!='hanning' and kwidth<=0:
                errstr = "kernel should be > 0"
                raise Exception, errstr
            else:
                    prior_plot(s, project, plotlevel)

                    dosmooth(s, kernel, kwidth, chanwidth, verify)

                    posterior_plot(s, project, plotlevel)

            sdutil.save(s, project, outform, overwrite)

            del s

        except Exception, instance:
                casalog.post( str(instance), priority = 'ERROR' )
                raise Exception, instance
                return


def dosmooth(s, kernel, kwidth, chanwidth, verify):
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

def prior_plot(s, project, plotlevel):
    if ( abs(plotlevel) > 1 ):
        # print summary of input data
        casalog.post( "Initial Scantable:" )
        #casalog.post( s._summary() )
        s._summary()
        
    if ( abs(plotlevel) > 0 ):
        pltfile=project+'_rawspec.eps'
        sdutil.plot_scantable(s, pltfile, plotlevel, 'Raw spectra')

def posterior_plot(s, project, plotlevel):
    if ( abs(plotlevel) > 0 ):
        pltfile=project+'_smspec.eps'
        sdutil.plot_scantable(s, pltfile, plotlevel, 'Smoothed spectra')
