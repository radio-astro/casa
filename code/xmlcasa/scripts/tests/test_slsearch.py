##########################################################################
# imfit_test.py
#
# Copyright (C) 2008, 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#
# <summary>
# Test suite for the CASA task slsearch
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_slsearch.py:description">slsearch</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the slsearch task
# </etymology>
#
# <synopsis>
# Test the slsearch task and the sl.search() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_slsearch[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the slsearch task to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import shutil
import casac
from tasks import *
from taskinit import *
from __main__ import *
import unittest

good_table = "biglist.tbl"

def run_search(
    table, outfile, freqrange, species, reconly,
    chemnames, qns, intensity, smu2, loga, el,
    eu, rrlinclude, rrlonly, verbose, logfile,
    append
):
    mysl = sltool.create()
    mysl.open(table)
    try: 
        restool = mysl.search(
            outfile=outfile, freqrange=freqrange,
            species=species, reconly=reconly,
            chemnames=chemnames, qns=qns,
            intensity=intensity, smu2=smu2, loga=loga,
            el=el, eu=eu, rrlinclude=rrlinclude,
            rrlonly=rrlonly, verbose=verbose, logfile=logfile,
            append=append
        )
        mysl.close()
        return restool
    except Exception, instance:
        mysl.close()
        
        raise instance
#        throw(instance)

def run_slsearch(
    table, outfile, freqrange, species, reconly,
    chemnames, qns, intensity, smu2, loga, el,
    eu, rrlinclude, rrlonly, verbose, logfile,
    append
):
    default(slsearch)
    return slsearch(
        tablename=table, outfile=outfile, freqrange=freqrange,
        species=species, reconly=reconly,
        chemnames=chemnames, qns=qns,
        intensity=intensity, smu2=smu2, loga=loga,
        el=el, eu=eu, rrlinclude=rrlinclude,
        rrlonly=rrlonly, verbose=verbose, logfile=logfile,
        append=append
    )

class slsearch_test(unittest.TestCase):
    
    def _testit(
        self, table, outfile, freqrange, species, reconly,
        chemnames, qns, intensity, smu2, loga, el,
        eu, rrlinclude, rrlonly, verbose, logfile,
        append, nrows
    ):
        mysl = sltool.create()
        mytb = tbtool.create()
        for i in [0, 1]:
            if (i==0):
                mysl = run_search(table, outfile,
                    freqrange, species, reconly, chemnames,
                    qns, intensity, smu2, loga, el, eu,
                    rrlinclude, rrlonly, verbose, logfile, 
                    append
                )
            else:
                mysl = run_slsearch(
                    table, outfile, freqrange, species,
                    reconly, chemnames, qns, intensity,
                    smu2, loga, el, eu, rrlinclude, rrlonly,
                    verbose, logfile, append
                )
            self.assertEqual(nrows, mysl.nrows())
            mysl.close()

            if (outfile):
                mytb.open(outfile)
                self.assertEqual(nrows, mytb.nrows())
                mytb.close()
                shutil.rmtree(outfile)
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/slsearch/'
        shutil.copytree(datapath + good_table, good_table)

    def tearDown(self):
        shutil.rmtree(good_table)

    def test_exceptions(self):
        """slsearch: Test various exception cases"""
        def testit(
            table, outfile, freqrange, species, reconly,
            chemnames, qns, intensity, smu2, loga, el,
            eu, rrlinclude, rrlonly, verbose, logfile, 
            append
        ):
            for i in [0, 1]:
                if (i==0):
                    self.assertRaises(
                        Exception, run_search, table, outfile,
                        freqrange, species, reconly, chemnames,
                        qns, intensity, smu2, loga, el, eu,
                        rrlinclude, rrlonly, verbose, logfile, 
                        append
                    )
                else:
                    self.assertEqual(
                        run_slsearch(
                            table, outfile, freqrange, species,
                            reconly, chemnames, qns, intensity,
                            smu2, loga, el, eu, rrlinclude, rrlonly,
                            verbose, logfile, append
                        ), None
                    )
        # bogus input table name
        testit(
            table="fred.tbl", outfile="x", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=True,
            verbose=True, logfile="", append=True
        )
        # bad output name
        testit(
            table=good_table, outfile="/x", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=True,
            verbose=True, logfile="", append=True
        )

    def test_table(self):
        """ test various settings of the table parameter"""

        # no table name works because it defaults to the system spectral line table
        self._testit(
            table="", outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=59998
        )
        # test user specified table search
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=67858
        )

    def test_outfile(self):
        """ test various settings of the outfile parameter"""

        outfile = "blah.tbl"
        self._testit(
            table=good_table, outfile=outfile, freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=67858
        )
        
    def test_freqrange(self):
        """ test various settings of the freqrange parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=67858
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 10], species=[],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=15292
        )

    def test_species(self):
        """ test various settings of the species parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=['S18O'],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=9
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=["S18O","HC5Nv11=1"],
            reconly=True, chemnames=[], qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=81
        )
        
    def test_chemnames(self):
        """ test various settings of the chemnames parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=["Silicon Monocarbide"], qns=[],
            intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=6
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=["Silicon Monocarbide", "Potassium chloride"],
            qns=[], intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=32
        )

    def test_qns(self):
        """ test various settings of the qns parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=["11-10"],
            intensity=[-1], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=17
        )

    def test_intensity(self):
        """ test various settings of the intensity parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[-10,-8], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=13447
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[-10,-8], smu2=[-1],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=7626
        )

    def test_smu2(self):
        """ test various settings of the smu2 parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[5, 10],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=12227
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[-1], smu2=[5, 10],
            loga=[-1], el=[-1], eu=[-1], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=6406
        )

    def test_loga(self):
        """ test various settings of the loga parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[-6, -4], el=[-1], eu=[-1], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=22781
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[-6, -4], el=[-1], eu=[-1], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=16960
        )

    def test_eu(self):
        """ test various settings of the eu parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[-1], eu=[150,200], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=9079
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[-1], eu=[150,200], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=3258
        )
        self._testit(
            table=good_table, outfile="", freqrange=[1, 1.1], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[], eu=[1581.52, 1581.53], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=0
        )
        self._testit(
            table=good_table, outfile="", freqrange=[1, 1.1], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[], eu=[1581.57, 1581.58] , rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=1
        )


    def test_el(self):
        """ test various settings of the el parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[150,200], eu=[], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=9023
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[150,200], eu=[], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=3202
        )
        self._testit(
            table=good_table, outfile="", freqrange=[1, 1.1], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[1581.52, 1581.53], eu=[], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=1
        )
        self._testit(
            table=good_table, outfile="", freqrange=[1, 1.1], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[1581.57, 1581.58] , eu=[], rrlinclude=False, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=0
        )

    def test_rrlonly(self):
        """ test various settings of the rrlonly parameter"""

        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[], eu=[], rrlinclude=True, rrlonly=False,
            verbose=False, logfile="", append=True, nrows=67858
        )
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[], eu=[], rrlinclude=True, rrlonly=True,
            verbose=False, logfile="", append=True, nrows=5821
        )

    def test_logfile(self):
        """ test various settings of the logfile and append parameters"""

        logfile = "xx.log"
        # verbose = False so no logfile should be written
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[], eu=[], rrlinclude=True, rrlonly=True,
            verbose=False, logfile=logfile, append=False, nrows=5821
        )
        self.assertFalse(os.path.exists(logfile))
        # verbose and overwrite
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[], eu=[], rrlinclude=True, rrlonly=True,
            verbose=True, logfile=logfile, append=False, nrows=5821
        )
        self.assertTrue(os.path.exists(logfile))

        num_lines = sum(1 for line in open(logfile))
        # append (twice)
        self.assertEquals(num_lines, 5822)
        self._testit(
            table=good_table, outfile="", freqrange=[0, 100], species=[],
            reconly=True, chemnames=[], qns=[],
            intensity=[], smu2=[],
            loga=[], el=[], eu=[], rrlinclude=True, rrlonly=True,
            verbose=True, logfile=logfile, append=True, nrows=5821
        )
        self.assertTrue(os.path.exists(logfile))
        num_lines = sum(1 for line in open(logfile))
        self.assertEquals(num_lines, 3*5822)
        os.remove(logfile)


def suite():
    return [slsearch_test]
