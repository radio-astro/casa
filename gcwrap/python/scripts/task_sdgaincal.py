import sys
import os
import numpy
import numpy.random as random
import shutil

from taskinit import gentools, casalog
from applycal import applycal
import types
import sdutil


def sdgaincal(infile=None, calmode=None, radius=None, smooth=None, 
              antenna=None, field=None, spw=None, scan=None, intent=None, 
              applytable=None, outfile='', overwrite=False): 
    
    casalog.origin('sdgaincal')
    
    # Calibrator tool
    (mycb,) = gentools(['cb'])

    try:
        # outfile must be specified
        if (outfile == '') or not isinstance(outfile, str):
            raise ValueError, "outfile is empty."
        
        # overwrite check
        if os.path.exists(outfile) and not overwrite:
            raise RuntimeError(outfile + ' exists.')
        
        # TODO: pre-application functionality is not supported yet
        if type(applytable) == types.ListType or \
          (isinstance(applytable, str) and len(applytable) > 0):
            casalog.post('Pre-application of calibration solutions is not supported yet.'
                         + 'Just ignore specified caltables.')
        
        # open MS
        if isinstance(infile, str) and os.path.exists(infile):
            mycb.open(filename=infile, compress=False, addcorr=False, addmodel=False)
        else:
            raise RuntimeError, 'infile not found - please verify the name'
        
        # select data
        if isinstance(antenna, str) and len(antenna) > 0:
            baseline = '{ant}&&&'.format(ant=antenna)
        else:
            baseline = ''
        mycb.selectvis(spw=spw, scan=scan, field=field, intent=intent, baseline=baseline)
        
        # set solve
        if calmode == 'doublecircle':
            if radius is None:
                raise RuntimeError('radius must be specified.')
            elif not isinstance(radius, str):
                rcenter = '%sarcsec'%(radius)
            else:
                try:
                    # if radius is a string only consists of numeric value without unit, 
                    # it will succeed.
                    rcenter = '%sarcsec'%(float(radius))
                except:
                    # if the above fails, it may indicate that the string contains unit
                    rcenter = radius
            mycb.setsolve(type='SDGAIN_OTFD', table=outfile, radius=rcenter, smooth=smooth)
        else:
            raise RuntimeError('Unknown calibration mode: \'{mode}\''.format(mode=calmode))

        # solve
        mycb.solve()

        ## reporting calibration solution
        #reportsolvestats(mycb.activityrec());

    except Exception, e:
        import traceback
        casalog.post(traceback.format_exc(), priority='DEBUG')
        casalog.post(errmsg(e), priority='SEVERE')
        raise e
    
    finally:
        mycb.close()

def errmsg(e):
    return '{type}: {msg}'.format(type=e.__class__.__name__, msg=e.message)