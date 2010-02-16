import report
import unittest

class version_test(unittest.TestCase):

    def shortDescription(self):
        return "Unit tests of comparing version strings"
    
    def setUp(self):
        pass
    
    def tearDown(self):
        pass

    def test_fails(self):
        assert 2 + 2 == 5

    def test_execution_failure(self):
        raise Exception("die")

    def test_r_r_1(self):
        "test revision vs revision"
        a = "CASA Version 3.0.1 (r10006)"
        b = "CASA Version 3.0.1 (r9933)"

        self.order(b, a)

    def test_r_r_2(self):
        a = "CASA Version 3.0.1 (r9936)"
        b = "CASA Version 3.0.1 (r9933)"

        self.order(b, a)

    def test_r_r_3(self):
        a = "CASA Version 3.0.0 (r9888)"
        b = "CASA Version 3.0.1 (r9913)"

        self.order(a, b)

    def test_build_build_1(self):
        a = "CASA Version 2.4.0 (build #8115)"
        b = "CASA Version 2.4.0 (build #7782)"

        self.order(b, a)
        
    def test_build_build_2(self):
        a = "CASA Version 2.4.0 (build #8115)"
        b = "CASA Version 3.0.0 (build #9684)"

        self.order(a, b)

    def test_build_r_1(self):
        a = "CASA Version 3.0.0 (r9886)"
        b = "CASA Version 3.0.0 (build #9684)"

        self.order(b, a)

    def test_build_r_2(self):
        a = "CASA Version 3.0.1 (r10006)"
        b = "CASA Version 3.0.0 (build #9684)"

        self.order(b, a)


    def order(self, a, b):
        """Verify that the cmp_version function behaves
        as it should, given that a is earlier than b"""
        
        assert report.cmp_version(a, b) < 0
        assert report.cmp_version(b, a) > 0
        assert report.cmp_version(a, a) == 0
        assert report.cmp_version(b, b) == 0

def suite():
    return [version_test]

def run_suite():
    set = unittest.TestLoader().loadTestsFromTestCase(version_test)
    unittest.TextTestRunner(verbosity=2).run(set)

