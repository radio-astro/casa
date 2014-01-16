import unittest
import pCASA
from tasks import *

class pcasa_test(unittest.TestCase):

    def shortDescription(self):
        return "Test of pCASA parallel task wrapper"

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

    def tearDown(self):
        for v in self.vis:
            os.system("rm -rf " + v)
            os.system("rm -rf " + v + ".flagversions")

    def test1(self):
        vis = "multi.ms"

        pCASA.create(vis)
        for i in range(6):
            pCASA.add(vis, self.vis[i], "localhost")
        
        pCASA.list(vis)

        print "Remove a few subMSs"
        pCASA.remove(vis, self.vis[0])
        pCASA.remove(vis, self.vis[3])
        pCASA.remove(vis, self.vis[4])

        pCASA.list(vis)

        listobs(vis)

        print "Re-add what was removed, on default=localhost"
        pCASA.add(vis, self.vis[0])
        pCASA.add(vis, self.vis[3])
        pCASA.add(vis, self.vis[4])

        pCASA.list(vis)

        listobs(vis)

        flagdata(vis,
                  scan = "3",
                  antenna = "8",
                  mode='unflag',
                  flagbackup = False)
        


        flagcmd(vis, inpmode="list")

        # Commented out because of CAS-2661.
        #for log in pCASA.pc.cluster.get_casalogs():
        #    assert os.system("cat " + log) == 0

def suite():
    return [pcasa_test]
