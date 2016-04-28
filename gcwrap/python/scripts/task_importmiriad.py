import os
from taskinit import *

def importmiriad (
    mirfile=None,
    vis=None,
    tsys=None,
    spw=None,
    vel=None,
    linecal=None,
    wide=None,
    debug=None,
    ):
    """Convert a Miriad visibility file into a CASA visibility file (MS).
           The conversion of the Miriad visibility format into a measurement set.  This version
           has been tested for both ATNF and CARMA Miriad files.
................          
           Keyword arguments:
        mirfile -- Name of input Miriad visibility file (directory)
               default: none; example: mirfile='mydata.uv'

....   vis      -- Output ms name
               default: mirfile name with suffix replaced by '.ms'

....   tsys   -- Set True to use the system temperature to set the visibility weights
               default: False

....   spw -- specify the spectral windows to use
........ default: all

....   vel -- Velocity system to use: LSRK, LSRD or TOPO
....       default: TOPO for ATCA, LSRK for CARMA

....   linecal -- (CARMA only) apply CARMA linecal on the fly?
....       default: False

....   wide    -- (CARMA only) specify the window averages to use
........ default: all
........ 
....   debug  -- specify level of debug messages (0,1,2,3)
                 default: 0 (=None)

           
        """

    # Python script
    mymf = casac.miriadfiller()
    try:
        try:
            casalog.origin('importmiriad')
            # -----------------------------------------'
            # beginning of importmiriad implementation
            # -----------------------------------------
            mymf.fill(vis,mirfile,tsys,spw,vel,linecal,wide,debug)
        except Exception, e:
          print e;
          casalog.post("Failed to import miriad file %s" % mirfile)
          raise
        # Write the args to HISTORY.
        try:
            param_names = \
                importmiriad.func_code.co_varnames[:importmiriad.func_code.co_argcount]
            param_vals = [eval(p) for p in param_names]
            write_history(
                mymf, vis, 'importmiriad', param_names, 
                param_vals, casalog
            )
        except Exception, instance:
            casalog.post("Failed to updated HISTORY", 'WARN')
    except:
        pass
    finally:
        if (mymf):
            del mymf 
        # -----------------------------------
        # end of importmiriad implementation
        # -----------------------------------

