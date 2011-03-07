import os
import shutil
from nose.tools import *
from asap import linecatalog

FNAME = "12mmlines.dat"

class TestLinecatalog:
    @classmethod
    def setupClass(self):
        f = file(FNAME, "w")
        txt = """\
He88B 18669.75 0.0 1.0
H87B 19304.69 0.0 2
He87B 19312.55 0.0 3
U 19416.0 0.0 0
"""
        f.write(txt)
        f.close()

    @classmethod
    def teardownClass(self):
        os.remove(FNAME)

    def setup(self):
        self.lc = linecatalog(FNAME)

    def test_init(self):
        pass

    def test_len(self):
        assert_equals(len(self.lc), 4)

    def test_set_name_pattern(self):
        self.lc.set_name("H*")
        assert_equals(len(self.lc), 3)

    @raises(RuntimeError)
    def test_fail_set_name_pattern(self):
        self.lc.set_name("Z*")

    def test_set_name_regex(self):
        self.lc.set_name("^H[0-9]+B$", 'regex')
        assert_equals(len(self.lc), 1)

    @raises(RuntimeError)
    def test_fail_set_name_regex(self):
        self.lc.set_name("^Z[0-9]+B$", 'regex')

    def test_reset(self):
        self.lc.set_name("^H[0-9]+B$", 'regex')
        self.lc.reset()
        assert_equals(len(self.lc), 4)

    def test_set_frequency_limits(self):
        for args in [(1.0, 19.0, 'GHz', 1),
                     (1000.0, 19000.0, 'MHz', 1),
                     (19.30, 19.4, 'GHz', 2),
                    ]:
            yield self.freq_limits, args

    def freq_limits(self, args):
        self.lc.set_frequency_limits(*args[:-1])
        assert_equals(len(self.lc), args[-1])

    @raises(RuntimeError)
    def test_fails_set_frequency_limits(self):
        self.lc.set_frequency_limits(20, 30)

    def test_set_strength_limits(self):
        self.lc.set_strength_limits(0.5, 2.5)
        assert_equals(len(self.lc), 2)

    @raises(RuntimeError)
    def test_fails_set_strength_limits(self):
        self.lc.set_strength_limits(10, 20)


    def test_get_row(self):
        r = self.lc.get_row(0)
        refrow = {'name': 'He88B', 'value': 18669.75}
        assert_equals(r, refrow)

    @raises(IndexError)
    def test_fail_get_row(self):
        r = self.lc.get_row(-1)

    def test_get_item(self):
        r = self.lc[0]
        refrow = {'name': 'He88B', 'value': 18669.75}
        assert_equals(r, refrow)

    def test_save(self):
        outname = 'tmp_lincatalog.tbl'
        self.lc.save(outname)
        lc  = linecatalog(outname)
        lctxt = str(lc)
        origlctxt = str(self.lc)
        lc = None
        shutil.rmtree(outname, True)
        assert lctxt == origlctxt
