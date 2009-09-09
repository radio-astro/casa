from tasks import *
from taskinit import *
import os

epsilon = 0.0001

def description():
    return "Test of visstat task"

def run():
    for vis in ['ngc5921.ms', 'pointingtest.ms']:

        print "Getting data", vis, "..."
        
        if vis == 'ngc5921.ms':
            importuvfits(os.environ.get('CASAPATH').split()[0] + \
                         '/data/regression/ngc5921/ngc5921.fits', \
                         vis)
        else:
            os.system('cp -R ' + os.environ.get('CASAPATH').split()[0] +\
                      '/data/regression/pointing/pointingtest.ms ' + vis)


        print "Create scratch columns. Expect error messages from applycal,"
        print "that's fine we just want the scratch columns"
        applycal(vis=vis)

          
        tb.open(vis)
        cols = tb.colnames()
        tb.close()

        cplx = ['amp', 'amplitude', 'phase', 'imag', 'imaginary', 'real']
        for x in cplx:
            cols.append(x)
        print cols
        cols.remove('DATA')
        cols.remove('CORRECTED_DATA')
        cols.remove('MODEL_DATA')

        cols = [x.lower() for x in cols]

        print "Trying these column names", cols

        for col in cols:
            data_cols = ['']
            if col in cplx:
                data_cols = ['data', 'corrected', 'model'] # not supported: 'residual'
                
            for dc in data_cols:
                print "Call with axis =", col, "; datacolumn=", dc
                if dc != '':
                    s = visstat(vis=vis, axis=col, datacolumn=dc)
                else:
                    s = visstat(vis=vis, axis=col)
                if col.upper() == "FLAG_CATEGORY":
                    # The MSs used have no data in FLAG_CATEGORY, therefore
                    # visstat() should fail
                    if s != None:
                        raise Exception("Error! " + str(s))
                elif not type(s) is dict:
                    raise Exception("Error! Return value " + str(s) + " is not a dictionary")

        for a in range(1, 5):
            s = visstat(vis=vis, axis='ANTENNA1', antenna=str(a)+'&26')
            print "antenna =", a, "; mean = ", s['ANTENNA1']['mean']

            # Note there's a counting from 0 or 1 issue here
            # with the antenna numbering
            if abs((s['ANTENNA1']['mean']-1) - a > epsilon):
                raise Exception("Error!")

        for scan in range(1, 8):
            s = visstat(vis=vis, axis='SCAN_NUMBER', scan=str(scan))
            
            print "scan =", scan, "; mean = ", s['SCAN_NUMBER']['mean']
            if abs(s['SCAN_NUMBER']['mean'] - scan > epsilon):
                raise Exception("Error!")

    return []

def data():
    return []
