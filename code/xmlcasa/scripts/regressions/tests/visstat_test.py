from tasks import *
from taskinit import *
import os

epsilon = 0.0001

def description():
    return "Test of visstat task"

def run():
    expected = {'ngc5921.ms':
                {'DATA': {'rms': 16.493364334106445,
                          'medabsdevmed': 0.043623808771371841,
                          'min': 2.2130521756480448e-05,
                          'max': 73.75,
                          'sum': 12927749.2227745,
                          'quartile': 0.28097864985466003,
                          'median': 0.052413057535886765,
                          'sumsq': 776452297.33716965,
                          'stddev': 15.859288085070023,
                          'var': 251.517018565244,
                          'npts': 2854278.0,
                          'mean': 4.5292537106667607}},
                'flagdatatest.ms':
                {'DATA': {'rms': 0.19930592179298401,
                          'medabsdevmed': 0.036990493535995483,
                          'min': 0.0,
                          'max': 0.32321548461914062,
                          'sum': 386384.63405715674,
                          'quartile': 0.074486657977104187,
                          'median': 0.19719454646110535,
                          'sumsq': 79473.508742819497,
                          'stddev': 0.049251333048233308,
                          'var': 0.0024256938070279983,
                          'npts': 2000700.0,
                          'mean': 0.19312472337539699}}}
    

    for vis in ['ngc5921.ms', 'flagdatatest.ms']:

        print "Getting data", vis, "..."
        
        if vis == 'ngc5921.ms':
            importuvfits(os.environ.get('CASAPATH').split()[0] + \
                         '/data/regression/ngc5921/ngc5921.fits', \
                         vis)
        else:
            os.system('cp -R ' + os.environ.get('CASAPATH').split()[0] +\
                      '/data/regression/flagdata/flagdatatest.ms ' + vis)


        s = visstat(vis=vis, axis='amp', datacolumn='data')

        if s.keys() != expected[vis].keys():
            raise Exception("Wrong dictionary keys. Expected %s, got %s" % \
                            (expected[vis], s))
                            

        print "Expected =", expected[vis]
        print "Got = ", s
        if not s.has_key('DATA'):
            raise Exception("Dictionary returned from visstat does not have key DATA")
        for e in expected[vis]['DATA'].keys():
            print "Checking %s: %s vs %s" % \
                   (e, expected[vis]['DATA'][e], s['DATA'][e])
            failed = False
            if expected[vis]['DATA'][e] == 0:
                if s['DATA'][e] != 0:
                    failed = True
            else:
                if abs((expected[vis]['DATA'][e] - s['DATA'][e])/expected[vis]['DATA'][e]) > 0.0001:
                    failed = True
            if failed:
                raise Exception("Numbers differ, expected %s, got %s" % \
                      (str(expected[vis]['DATA'][e]), str(s['DATA'][e])))
            
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
                print "Call with axis =", col, "; datacolumn =", dc
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
