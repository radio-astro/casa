import os
from taskinit import *

def importatca (
    files=None,
    vis=None,
    options=None,
    spw=None,
    nscans=None,
    lowfreq=None,
    highfreq=None,
    fields=None,
    edge=8
    ):
    """Convert an RPFITS file into a CASA visibility file (MS).
           The conversion of the RPFITS format into a measurement set.  
           This version has been tested for both old ATCA and CABB data.
................          
           Keyword arguments:
       files -- Name of input RPFITS file(s)
               default: none; example: file='2010-01-02_1234.c999'

....   vis -- Output ms name, note a postfix (.ms) is NOT appended to this name
               default: none
               
....   options -- Processing options, comma separated list
                 birdie - flag parts of spectrum known to be bad
                 reweight - (pre-CABB) reweight lag spectrum to avoid ringing
                 noautoflag - don't apply automatic flags (e.g. pointing scans)
                 noxycorr - don't apply xyphase correction
                 fastmosaic - use for large mosaics to speed up data access
                 hires  - turn time binned data into fast sampled data
                 notsys - undo online Tsys calibration
                 noac - don't load autocorrelations
....   spw -- specify the input spectral windows to use. For CABB the order is
              first continuum, 2nd continuum, then any zooms for first band,
              followed by zooms for 2nd band. Pre-CABB data just has 0 and 1.
              The output may have more spectral windows if there are frequency
              changes.
........ default: all

....   nscans -- Number of scans to skip followed by number of scans to read
....       default: 0,0 (read all)

....   lowfreq -- Lowest reference frequency to select
....       default: 0 (all)

....   highfreq -- highest reference frequency to select
....       default: 0 (all)

....   fields -- List of field names to select
........ default: all

....   edge -- Percentage of edge channels to flag. For combined zooms, this 
               specifies the percentage for a single zoom
........ default: 8 (flags 4% of channels at lower and upper edge)
            
        """

    # Python script
    myaf = casac.atcafiller()
    try:
        try:
            casalog.origin('importatca')
            # -----------------------------------------
            # beginning of importatca implementation
            # -----------------------------------------
            myaf.open(vis,files,options)
            firstscan=0
            lastscan=9999
            if (nscans != None):
              if len(nscans)>0:
                firstscan=nscans[0]
              if len(nscans)>1:
                lastscan=nscans[1]
            myaf.select(firstscan,lastscan,spw,lowfreq,highfreq,
            fields,edge)
            myaf.fill()
        except Exception, e:
          print e;
          casalog.post("Failed to import atca rpfits file(s) %s" % files)
          raise
        # Write the args to HISTORY.
        try:
            param_names = \
                importatca.func_code.co_varnames[:importatca.func_code.co_argcount]
            param_vals = [eval(p) for p in param_names]
            write_history(
                myaf, vis, 'importatca', param_names, 
                param_vals, casalog
            )
        except Exception, instance:
            casalog.post("Failed to updated HISTORY", 'WARN')
    except:
        pass
    finally:
        if (myaf):
            del myaf 
        # -----------------------------------
        # end of importatca implementation
        # -----------------------------------

