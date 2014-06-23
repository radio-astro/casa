#
# Unit test of importasdm task with signledish=True
#
# Here, the script tests whether,
#   1) ASDM can be imported without any exception,
#   2) all necessary tables exist,
#   3) (part of) table contents are valid.
#
# Same tests will be done for useversion='v2' and 'v3'.
#
import os
import sys
import shutil
import string
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest

# ASDM dataset name
myasdm_dataset_name = 'uid___X5f_X18951_X1'

# Scantable name
asapname = myasdm_dataset_name + '.asap'

class asdmsd_import( unittest.TestCase ):
    def setUp( self ):
        self.res = None
        self.subtables = [ 'FIT',
                           'FOCUS',
                           'FREQUENCIES',
                           'HISTORY',
                           'MOLECULES',
                           'TCAL',
                           'WEATHER' ]
        self.nrows = { 'MAIN': 3576,
                       'FIT': 0,
                       'FOCUS': 1,
                       'FREQUENCIES': 2,
                       'HISTORY': 0,
                       'MOLECULES': 1,
                       'TCAL': 1,
                       'WEATHER': 1 }
        if(os.path.exists(myasdm_dataset_name)):
            shutil.rmtree(myasdm_dataset_name)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/asdm-import/input/'
        shutil.copytree(datapath + myasdm_dataset_name, myasdm_dataset_name)
        default(importasdm)
        
    def tearDown(self):
        shutil.rmtree(myasdm_dataset_name)
        shutil.rmtree(asapname,ignore_errors=True)

##     def test0( self ):
##         """asdmsd_import_v2: Test direct import to single-dish format (oldasdm2ASAP)"""
##         # 1) test import
##         self.doImport( version='v2' )

##         # 2) table existence check
##         self.tableExistenceCheck()

##         # 3) table contents check
##         self.tableContentsCheck()
        
    def test1( self ):
        """asdmsd_import_v3: Test direct import to single-dish format (asdm2ASAP)"""
        # 1) test import
        self.doImport( version='v3' )

        # 2) table existence check
        self.tableExistenceCheck()

        # 3) table contents check
        self.tableContentsCheck()

    def doImport( self, version ):
        self.res = importasdm(asdm=myasdm_dataset_name,
                              vis=asapname,
                              singledish=True,
                              antenna=0,
                              useversion=version)
        self.assertEqual( self.res, None )


    def tableExistenceCheck( self ):
        result={}
        item = { 'success': True, 'message':'' }
        msgTemplate = string.Template( '%s/$table does not exist.'%(asapname) )
        
        # MAIN
        result['MAIN'] = item
        if not os.path.exists( asapname ):
            result['MAIN']['success'] = False
            result['MAIN']['message'] = msgTemplate.safe_substitute( table='MAIN' )

        # subtables
        for t in self.subtables:
            result[t] = item
            pathToSubtable = asapname + '/' + t
            if not os.path.exists( pathToSubtable ):
                result[t]['success'] = False
                result[t]['message'] = msgTemplate.safe_substitute( table = t )

        self.__checkResult( result )

    def tableContentsCheck( self ):
        result={}
        item = { 'success': True, 'message':'' }
        msgTemplate = string.Template( '%s/$table: nrow must be $valid (was $thevalue)' )

        # MAIN
        result['MAIN'] = item
        tb.open( asapname )
        nrow = tb.nrows()
        tb.close()
        if nrow != self.nrows['MAIN']:
            result['MAIN']['success'] = False
            result['MAIN']['message'] = msgTemplate.safe_substitute( table='MAIN', valid=self.nrows['MAIN'], thevalue=nrow )

        # subtables
        for t in self.subtables:
            result[t] = item
            pathToSubtable = asapname + '/' + t
            tb.open( pathToSubtable )
            nrow = tb.nrows()
            tb.close()
            if nrow != self.nrows[t]:
                result[t]['success'] = False
                result[t]['message'] = msgTemplate.safe_substitute( table=t, valid=self.nrows[t], thevalue=nrow )

        self.__checkResult( result )

        # MAIN header
        items = { 'nIF': 2,
                  'nPol': 1,
                  'nBeam': 1,
                  'nChan': 256,
                  'AntennaName': 'OSF//DV01@TF1' }
        self.__checkHeader( items )

        # MAIN column
        items = { 0: 0.50806641578674316,
                  1788: 10.553685188293457 }
        self.__checkColumn( 'SPECTRA', items )

        # FREQUENCIES column
        items = { 0: 8.6276162900858246e10,
                  1: 8.6287882216325546e10 }
        self.__checkColumn( 'REFVAL', items, 'FREQUENCIES' )

        # SCANNO (CAS-5841)
        items = numpy.arange(1,7)
        self.__checkValue('SCANNO', items)
        
    def __checkResult( self, result ):
        for key in result.keys():
            r = result[key]
            self.assertTrue( r['success'], msg=r['message'] )        

    def __checkHeader( self, items ):
        msgTemplate = string.Template( 'HEADER: $key must be $valid (was &value).' )
        vals = {}
        tb.open( asapname )
        for key in items.keys():
            vals[key] = tb.getkeyword( key )
        tb.close()

        for key in items.keys():
            #print '%s: %s'%(key,vals[key])
            self.assertEqual( vals[key], items[key],
                              msg=msgTemplate.safe_substitute(key=key,valid=items[key],value=vals[key]) )

    def __checkColumn( self, column, items, table='MAIN' ):
        msgTemplate = string.Template( '$table/$column: row $row differ (difference is $diff)' )
        vals = {}
        if table is 'MAIN':
            tb.open( asapname )
        else:
            tb.open( asapname + '/' + table )
        for key in items.keys():
            v = tb.getcell( column, key )
            if isinstance( v, numpy.ndarray ):
                vals[key] = v[0]
            else:
                vals[key] = v
        tb.close()

        for key in items.keys():
            #print '%s: %s'%(key,vals[key])
            diff = (vals[key] - items[key]) / vals[key]
            #print 'diff = %12.10f'%(diff)
            self.assertTrue( diff < 1.0e-8,
                             msg=msgTemplate.safe_substitute(table=table,column=column,row=key,diff=diff) )

    def __checkValue(self, column, ref):
        tb.open(asapname)
        val = numpy.unique(tb.getcol(column))
        print 'checking %s...'%(column)
        print 'reference=%s'%(ref)
        print 'resulting value=%s'%(val)
        self.assertEqual(len(ref), len(val))
        self.assertTrue(all(ref == val))
            
def suite():
    return [asdmsd_import]
