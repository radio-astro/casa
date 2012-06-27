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
# Test suite for the CASA task splattotable
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_splattotable.py:description">splattotable</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the splattotable task
# </etymology>
#
# <synopsis>
# Test the splattotable task and the sl.splattotable() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_splattotable[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the splattotable task to ensure
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

good_list = "list1.txt"
bad_list = "list2.txt"

def run_sttmethod(list, table):
    mysl = sltool.create()
    restool = mysl.splattotable(filenames=list, table=table)
    mysl.close()
    return restool

def run_stttask(list, table):
    default(splattotable)
    return splattotable(filenames=list, table=table)


class splattotable_test(unittest.TestCase):
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/splattotable/'
        shutil.copy(datapath + good_list, good_list)
        shutil.copy(datapath + bad_list, bad_list)

    
    def tearDown(self):
        os.remove(good_list)
        os.remove(bad_list)

    def test_exceptions(self):
        """splattotable: Test various exception cases"""
        
        def testit(filenames, table):
            for i in [0,1]:
                if (i==0):
                    self.assertRaises(Exception, run_sttmethod, filenames, table)
                else:
                    self.assertEqual(run_stttask(filenames, table), None)

        # blank output table name
        testit("list1.txt", "")
        
        # bad list 
        testit("list2.txt", "myout");
        
        # unwritable table
        testit("list1.txt", "/myout");
        

    def test_good_list(self):
        """splattotable: Test converting a good list"""
        def testit(filenames, table):
            mytb = tbtool.create()
            for i in [0,1]:
                table = table + str(i)
                if (i==0):
                    newsl = run_sttmethod(filenames, table)
                else:
                    newsl = run_stttask(filenames, table)
                self.assertTrue(mytb.open(table))
            tb.done()
                    
        testit("list1.txt", "good_table")
      

def suite():
    return [splattotable_test]
