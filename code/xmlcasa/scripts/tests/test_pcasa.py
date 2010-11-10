import unittest
import pCASA
from tasks import *

class pcasa_test(unittest.TestCase):

    def setUp(self):
        print "Importing data..."
        self.vis = ["ngc5921-1.ms",
                    "ngc5921-2.ms",
                    "ngc5921-3.ms",
                    "ngc5921-4.ms",
                    "ngc5921-5.ms",
                    "ngc5921-6.ms"]
        if True:
            importuvfits(os.environ.get('CASAPATH').split()[0] + \
                         '/data/regression/ngc5921/ngc5921.fits', \
                         self.vis[0])
        else:
            assert os.system("cp -r " +
                             os.environ.get('CASAPATH').split()[0] + \
                             "/data/regression/wproject_regression/coma.ms " + \
                             self.vis[0]) == 0

        for i in range(1, 6):
            assert os.system("cp -r " + self.vis[0] + " " + self.vis[i]) == 0

    def shortDescription(self):
        return "Test of pCASA parallel task wrapper"

    def test1(self):
        vis = "multi.ms"

        submss = []
        for i in range(6):
            submss.append(pCASA.subMS(self.vis[i], "localhost"))
        pCASA.create(vis, submss)
        
        pCASA.show(vis)

        listobs(vis)

        flagdata2(vis,
                  scan = "3",
                  antenna = "8",
                  unflag=True,
                  flagbackup = False)
        
        flagdata(vis,
                 scan = "3",
                 antenna = "8",
                 unflag=True,
                 flagbackup = False)
        
        for log in pCASA.pc.cluster.get_casalogs():
            assert os.system("cat " + log) == 0

        

def suite():
    return [pcasa_test]
