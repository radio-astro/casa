import os
from taskinit import *

def importuvfits(fitsfile, vis, antnamescheme=None):
    """

    Convert a UVFITS file to a CASA visibility data set (MS):

    Keyword arguments:
    fitsfile -- Name of input UV FITS file
        default = none; example='3C273XC1.fits'
    vis -- Name of output visibility file (MS)
        default = none; example: vis='3C273XC1.ms'
    antnamescheme -- Naming scheme for VLA/JVLA/CARMA antennas
        default = old;
        old: Antenna name is a number, '04'
             This option exists for backwards compatibility
             but can lead to ambiguous results when antenna
             indices are used for data selection.
        new: Antenna name is not a number, 'VA04' or 'EA04'
             With this scheme, data selection via
             antenna names and indices is non-ambiguous.
    async --  Run asynchronously
        default = false; do not run asychronously


    """
    myms = mstool()
    try:
        try:
            casalog.origin('importuvfits')
            casalog.post("")
            myms.fromfits(vis, fitsfile, antnamescheme=antnamescheme)
            myms.close()
        except Exception, instance: 
            casalog.post( str( '*** Error ***') + str(instance), 'SEVERE')
            raise
        # Write the args to HISTORY.
        try:
            param_names = \
                importuvfits.func_code.co_varnames[:importuvfits.func_code.co_argcount]
            param_vals = [eval(p) for p in param_names]
            write_history(
                myms, vis, 'importuvfits', param_names, 
                param_vals, casalog
            )
        except Exception, instance:
            casalog.post("Failed to updated HISTORY table", 'WARN')
    except:
        pass
    finally:
        if (myms):
            #myms.close()
            del myms 


