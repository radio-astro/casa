from nose.tools import *
from asap import scantable, selector
from asap.logging import asaplog
# no need for log messages
#asaplog.disable()

class TestAverage(object):
    def setup(self):
        s = scantable("data/2011-10-13_1609-MX025.rpf", average=False)
        # make sure this order is always correct - in can be random
        sel = selector()
        sel.set_order=(["SCANNO", "IFNO", "POLNO"])
        s.set_selection(sel)
        self.st = s.copy()
        
        
#        assert_almost_equals(res_rms[0], 0.38346, 5)
#        assert_almost_equals(res_rms[1], 0.38780, 5)        
   
    def test_average(self):
        av = self.st.average_time()
        assert_equals(av.nrow(),self.st.nbeam()*self.st.nif()*self.st.npol())
