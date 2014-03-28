##########################################################################
# imhead_test.py
#
# Copyright (C) 2008, 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This scripts free software; you can redistribute it and/or modify it
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
# Shannon Jaeger (University of Calgary)
# </author>
#
# <summary>
# Test suite for the CASA imhead Task
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="imhead.py:description">imhead</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# imhead_test stands for image header test
# </etymology>
#
# <synopsis>
# imhead_test.py is a Python script that tests the correctness
# of the imhead task in CASA.  This is essentially a unit test
# for the imhead task.
#
# However, it is also used as a regression test for the imhead task.
#
# </synopsis> 
#
# <example>
# # This test was designed to run using the CASA unit test system.
# # This example shows who to run it manually from within casapy.
# runUnitTest.main(['test_imhead'])
#
# or outside casapy like this:
# casapy -c runUnitTest.py test_imhead
#
# </example>
#
# <motivation>
# To provide a test standard to the imhead task to try and ensure
# coding changes do not break the 
# </motivation>
#
# <todo>
#  1. produce summary data
#  2 make sure failure_msgs is used everywere, and display them#
# </todo>
#     
#
# SDJ Sep. 8, 2008 Created.
# SDJ May 20, 2009 Updated to testing Framework


###########################################################################
import numpy
import os
import shutil
import casac
from tasks import *
from taskinit import *
import commands
import unittest

###########################################################################
# NAME: mode_test
#
# SHORT DESCRIPTION: Make sure all of the modes are supported
#                    and that a graceful exit occurs for an
#                    invalid mode.
#
# DESCRIPTION: Call each of the modes to make sure they work.
#              The VALID modes are: list, get, put, history, summary,
#              fits, add, and del.
# 
#              The INVALID modes called are: '', 2.2, and bad
#
############################################################################

# Input file names
input_file = 'ngc5921.clean.image'
input_file_copy = 'ngc5921.clean.image.copy'
datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imhead/'
complexim = 'complex.im'

def deep_equality(a, b): 
    if (type(a) != type(b)):
        print "types don't match, a is a " + str(type(a)) + " b is a " + str(type(b))
        return False
    if (type(a) == dict):
        if (a.keys() != b.keys()):
            print "keys don't match, a is " + str(a.keys()) + " b is " + str(b.keys())
            return False
        for k in a.keys():
            if (
                k == "telescope" or k == "observer"
                or k == "telescopeposition"
            ):
                continue
            elif (not deep_equality(a[k], b[k])):
                print "dictionary member inequality a[" + str(k) \
                    + "] is " + str(a[k]) + " b[" + str(k) + "] is " + str(b[k])
                return False
        return True
    if (type(a) == float):
        if not (a == b or abs((a-b)/a) <= 1e-6):
            print "float mismatch, a is " + str(a) + ", b is " + str(b)
        return a == b or abs((a-b)/a) <= 1e-6
    if (type(a) == numpy.ndarray):
        if (a.shape != b.shape):
            print "shape mismatch a is " + str(a.shape) + " b is " + str(b.shape)
            return False
        x = a.tolist()
        y = b.tolist()
        for i in range(len(x)):
            if (not deep_equality(x[i], y[i])):
                print "array element mismatch, x is " + str(x[i]) + " y is " + str(y[i])
                return False
        return True
    return a == b



class imhead_test(unittest.TestCase):
    
    def setUp(self):
        for f in [input_file, complexim]:
            if(os.path.exists(f)):
                os.system('rm -rf ' + f)
            os.system('cp -r ' +datapath + f +' ' + f)

    def tearDown(self):
        for f in [input_file, complexim]:
            if(os.path.exists(f)):
                os.system('rm -rf ' + f)
        self.assertTrue(len(tb.showcache()) == 0)

    def test_mode(self):
        '''Imhead: Test list,summary,history,get,put,del,add,empty,bad modes'''
        self.assertTrue(len(tb.showcache()) == 0)
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        casalog.post( "Staring imhead mode tests!", "NORMAL2" )
    
        
        ################################################################
        # List mode
        casalog.post( "Testing LIST mode", "NORMAL4" );
        results=None
        try:
            results = imhead( input_file, 'list' )
            self.assertTrue(len(tb.showcache()) == 0)
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: list mode test has failed on image "+input_file

        if ( results==None \
             or (isinstance(results,bool) and results==False )\
             or (isinstance(results,dict) and results=={} ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: list mode test has failed on image "+input_file\
                     +"\nThe results are: "+str(results)
        ################################################################
        # Summary mode
        casalog.post( "Testing SUMMARY mode", "NORMAL4" );
        results=None
        try:
            results = imhead( input_file, 'summary' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: summary mode test has failed on image "+input_file
    
        if ( results==None \
             or (isinstance(results,bool) and results==False )\
             or (isinstance(results,dict) and results=={} ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: summary mode test has failed on image "+input_file\
                     +"\nThe results are: "+str(results)
    
    
        ################################################################
        # History mode
        casalog.post( "Testing HISTORY mode", "NORMAL4" );
        results=None
        self.assertTrue(len(tb.showcache()) == 0)
        try:
            results = imhead( input_file, 'history' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: history mode test has failed on image "+input_file
    
        if ( results==None \
             or (isinstance(results,bool) and results==False )\
             or (isinstance(results,dict) and results=={} ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: history mode test has failed on image "+input_file\
                     +"\nThe results are: "+str(results)
    
        ################################################################
        # Fits mode
        #
        # Commented out since this isn't implemented yet!
        #casalog.post( "Testing FITS mode", "NORMAL4" );
        #results=None
        #try:
        #    results = imhead( input_file, 'fits' )
        #except:
        #    retValue['success']=False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #             +"\nError: fits mode test has failed on image "+input_file
        #
        #if ( results==None \
        #     or (isinstance(results,bool) and results==False )\
        #     or (isinstance(results,dict) and results=={} ) ):
        #    retValue['success']=False
        #    retValue['error_msgs']=retValue['error_msgs']\
        #             +"\nError: fits mode test has failed on image "+input_file\
        #             +"\nThe results are: "+str(results)
        #    
    
    
        ################################################################
        # Get mode
        casalog.post( "Testing GET  mode", "NORMAL4" );
        results=None
        self.assertTrue(len(tb.showcache()) == 0)
        try:
            results = imhead( input_file, 'get', 'telescope' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: get mode test has failed on image "+input_file
    
        if ( results==None \
             or (isinstance(results,bool) and results==False )\
             or (isinstance(results,dict) and results=={} ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: get mode test has failed on image "+input_file\
                     +"\nThe results are: "+str(results)
    
    
        ################################################################
        # Put mode
        casalog.post( "Testing PUT mode", "NORMAL4" );
        results=None
        try:
            results = imhead( input_file, 'put', 'lastupdated', 'today' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: put mode test has failed on image "+input_file
    
        if ( results==None \
             or (isinstance(results,bool) and results==False )\
             or (isinstance(results,dict) and results=={} ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: put mode test has failed on image "+input_file\
                     +"\nThe results are: "+str(results)
    
        ################################################################
        # Del mode
        casalog.post( "Testing DEL mode", "NORMAL4" );
        results=None
        self.assertTrue(len(tb.showcache()) == 0)
        try:
            results = imhead( input_file, 'del', 'lastupdated')
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: del mode test has failed on image "+input_file
    
        if ( results==None \
             or (isinstance(results,bool) and results==False )\
             or (isinstance(results,dict) and results=={} ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: del mode test has failed on image "+input_file\
                     +"\nThe results are: "+str(results)    
    
    
        ################################################################
        # Add mode
        casalog.post( "Testing ADD mode", "NORMAL4" );
        results=None
        try:
            results = imhead( input_file, 'add', 'garbage', "added while testing imhead task.")
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: add mode test has failed on image "+input_file
    
        if ( results==None \
             or (isinstance(results,bool) and results==False )\
             or (isinstance(results,dict) and results=={} ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: add mode test has failed on image "+input_file\
                     +"\nThe results are: "+str(results)    
    
    
        ################################################################
        # Empty mode
        results=None
        self.assertTrue(len(tb.showcache()) == 0)
        try:
            results = imhead( input_file, '' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: empty mode was not detected as invalid!"
    
    
        ################################################################
        # Empty mode
        results=None
        try:
            results = imhead( input_file, 2.2 )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: mode='2.2' was not detected as invalid!"
                
    
        ################################################################
        # bad mode
        results=None
        self.assertTrue(len(tb.showcache()) == 0)
        try:
            results = imhead( input_file, 'bad' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: 'bad' mode was not detected!"
    
        self.assertTrue(retValue['success'],retValue['error_msgs'])
        
    ###########################################################################
    # NAME: set_get_test
    #
    # SHORT DESCRIPTION: Set/get a variety of header items make sure each
    #                    of them work
    #
    # DESCRIPTION: Call each of the modes to make sure they work.
    #              The VALID hditems tested are: object, imtype, telescope,
    #                 observer, date-obs, equinox, rest frequency, projection,
    #                 beam major axis, beam minor axis, beam position angle,
    #                 beam unit, coord axis type. num pixels for coord axis,
    #                 coord axis reference value, coord axis delta,
    #                 coord axis units
    # 
    #              The INVALID hditems used are: '', and bad
    #
    ############################################################################
    def test_set_get(self):
        '''Imhead: test put/get modes'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        casalog.post( "Staring imhead get/put tests!", "NORMAL2" )
        
        # Test the set/get value routines.  All of them
    
        #######  OBJECT  #############
        val   = None
        newval= None
        try:
            val = imhead( input_file, 'get', 'object' )
            testval = 'theCoolObject'
            imhead( input_file, 'put', 'object', testval )
            newval = imhead( input_file, 'get', 'object' )
            got = imhead( input_file, 'get', 'object' )
            self.assertTrue(got == testval)
            if ( val != None ):
                imhead( input_file, 'put', 'object', val )
        except:
            self.assertTrue(False)
        else:
            self.assertFalse( val!=None and val == newval )
    
        #######  IMTYPE  #############
        val   = None
        newval= None
        try:
            val=imhead( input_file, 'get', 'imtype' )
            imhead( input_file, 'put', 'imtype', '' )
            newval=imhead( input_file, 'get', 'imtype' )
            if ( val != None ):
                imhead( input_file, 'put', 'imtype', val )
        except:
            self.assertTrue(False)
        else:        
            self.assertTrue( val!=None and val == newval )
    
        #######  TELESCOPE  #############
        val   = None
        newval= None
        try:
            val=imhead( input_file, 'get', 'telescope' )
            telescope = 'myTelescope'
            imhead( input_file, 'put', 'telescope', telescope )
            newval=imhead( input_file, 'get', 'telescope' )
            self.assertTrue(newval == telescope)

            if ( val != None ):        
                imhead( input_file, 'put', 'telescope', val )
        except:
            self.assertTrue(False)
         
        #######  OBSERVER  #############
        val   = None
        newval= None
        try:
            val=imhead( input_file, 'get', 'observer' )
            imhead( input_file, 'put', 'observer', 'me' )
            newval=imhead( input_file, 'get', 'observer' )
            if ( val != None ):
                imhead( input_file, 'put', 'observer', val )
        except:
            self.assertTrue(False)
        else:        
            self.assertFalse( val!=None and val == newval )
    
        #######  DATE-OBS  #############
        val   = None
        newval= None
        try:
            val=imhead( input_file, 'get', 'date-obs' )
            date = '2007/06/20/00:00:00'
            imhead( input_file, 'put', 'date-obs', date )
            newval=imhead( input_file, 'get', 'date-obs' )
            self.assertTrue(newval == date)
            if ( val != None ):
                imhead( input_file, 'put', 'date-obs', val )
        except:
            self.assertTrue(False)
              
        #######  EQUINOX  #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'equinox' )
            equinox = 'B1950'
            imhead( input_file, 'put', 'equinox', equinox )
            newval=imhead( input_file, 'get', 'equinox' )
            self.assertTrue(newval == equinox)
            if ( val != None ):
                imhead( input_file, 'put', 'equinox', val )
        except Exception, e:
            print str(e)
            self.assertTrue(False)
        
        #######  RESTFREQUENCY  #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'restfreq' )
            print "*** val " + str(val)
            imhead( input_file, 'put', 'restfreq', '1.421GHz' )
            print "kk"
            imhead( input_file, 'put', 'restfreq', '15.272GHz, 1.67GHz' )
            print "yy"
            newval=imhead( input_file, 'get', 'restfreq' )
            print "newval " + str(newval)
            if ( val != None ):  
                print "val2 " + str(val)      
                #imhead( input_file, 'put', 'restfreq', str(val['value'])+str(val['unit']) )
                imhead( input_file, 'put', 'restfreq', val['value'])
                print "done"
        except Exception, instance:
            print str(instance)
            self.assertTrue(False)
        else:
            self.assertFalse( val!=None and val == newval )
                
    
    
        #######  PROJECTION  #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'projection' )
            proj = 'TAN'
            imhead( input_file, 'put', 'projection', proj )
            newval=imhead( input_file, 'get', 'projection' )
            self.assertTrue(newval == proj)
            if ( val != None ):
                imhead( input_file, 'put', 'projection', val )            
        except:
            self.assertTrue(False)
        else:
            self.assertFalse( val!=None and val == newval )
        #######  BEAM MAJOR  #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'beammajor' )
            imhead( input_file, 'put', 'beammajor', '12arcsec')
            newval=imhead( input_file, 'get', 'beammajor' )
            if ( val != None ):        
                imhead( input_file, 'put', 'beammajor', val )
        except:
            self.assertTrue(False)
        else:
            self.assertFalse( val!=None and val != newval )
         
        #######  BEAM  MINOR #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'beamminor' )
            bmaj = '24arcsec'
            imhead( input_file, 'put', 'beamminor', bmaj )
            newval=imhead( input_file, 'get', 'beamminor' )
            self.assertTrue(newval == qa.quantity(bmaj))
            if ( val != None ):
                imhead( input_file, 'put', 'beamminor', str(val['value'])+str(val['unit']) )
        except:
            self.assertTrue(False)
    
    
        #######  BEAM PA #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'beampa' )
            bpa = '0.5deg'
            imhead( input_file, 'put', 'beampa', bpa )
            newval=imhead( input_file, 'get', 'beampa' )
            self.assertTrue(newval == qa.quantity(bpa))
            if ( val != None ):        
                imhead( input_file, 'put', 'beampa', str(val['value'])+str(val['unit']) )
        except:
            self.assertTrue(False)
           
        #######  BUNIT  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'bunit' )
            bunit = 'kJy/beam'
            imhead( input_file, 'put', 'bunit',  bunit)
            newval=imhead( input_file, 'get', 'bunit' )
            self.assertTrue(newval == bunit)
            if ( val != None ):
                imhead( input_file, 'put', 'bunit', val )
        except:
            self.assertTrue(False)
    
        #######  CTYPE3  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'ctype3' )
            dec = 'Declination'
            imhead( input_file, 'put', 'ctype3',  dec)
            newval=imhead( input_file, 'get', 'ctype3' )
            self.assertTrue(newval == dec)
            if ( val != None ):
                imhead( input_file, 'put', 'ctype3', val )
        except:
            self.assertTrue(False)
    
        #######  CRPIX2  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'crpix2' )
            pix = 64.0
            imhead( input_file, 'put', 'crpix2', pix )
            newval=imhead( input_file, 'get', 'crpix2' )
            self.assertTrue(newval == pix)
            if ( val != None ):
                imhead( input_file, 'put', 'crpix2', val )
        except:
            self.assertTrue(False)
    
    
        #######  CRVAL4  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'crval4' )
            imhead( input_file, 'put', 'crval4', '1.6' )
            newval=imhead( input_file, 'get', 'crval4' )
            if ( val != None ):
                imhead( input_file, 'put', 'crval4', val['value'] )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get crval4 failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set crval4 Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: '1.6'"
    
    
        #######  CDELT4  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'cdelt4' )
            imhead( input_file, 'put', 'cdelt4', '2.5'  )
            newval=imhead( input_file, 'get', 'cdelt4' )
            if ( val != None ):
                imhead( input_file, 'put', 'cdelt4', val['value'] )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get cdelt4 failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set cdelt4 Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: '2.5'"
    
    
    
        #######  CUNIT1  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'cunit1' )
            unit = 'deg'
            imhead( input_file, 'put', 'cunit1', unit )
            newval=imhead( input_file, 'get', 'cunit1' )
            self.assertTrue(newval == unit)
            if ( val != None ):
                imhead( input_file, 'put', 'cunit1', val )
        except:
           self.assertTrue(False)
    
        ################################################################
        # bad header time
        results=None
        try:
            results = imhead( input_file, 'get', 'bad' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: 'bad' header item was not detected!"
    
        # This will get a default value of '', one could argue
        # that this maybe should not succeed, but this way is ok too.
        results=None
        try:
            results = imhead( input_file, 'put', 'bad' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: 'bad' header item was not placed in the header!"
    
        try:
            results = imhead( input_file, 'get', 'bad' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: 'bad' header item was not found after put."
            
    
    
    
        ################################################################
        # Empty header item
        results=None
        try:
            results = imhead( input_file, 'get', '' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: Empty header item was not detected!"
    
        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    
    ###########################################################################
    # NAME: add_del_test
    #
    # SHORT DESCRIPTION: Set/get a variety of header items make sure each
    #                    of them work
    #
    # DESCRIPTION: Call each of the modes to make sure they work.
    #              The VALID hditems tested are: ???
    # 
    #              The INVALID hditems used are: TODO
    #
    ############################################################################
    def test_add_del(self):
        '''Imhead: test add/del modes'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        casalog.post( "Starting imhead add/del tests!", "NORMAL2" )
        
        #######  BEAM  #############
        val_maj = val_min = val_pa   = None
        newval= None
        try:                                
            val_maj=imhead( input_file, 'get', 'beammajor' )
            val_min=imhead( input_file, 'get', 'beamminor' )
            val_pa=imhead( input_file, 'get', 'beampa' )
            imhead( input_file, 'del', 'beammajor' )
    
            try:
                newval=imhead( input_file, 'get', 'beammajor' )
            except:
                pass
            if ( val_maj != None ):
                imhead( input_file, 'add', 'beammajor', \
                        str(val_maj['value'])+str(val_maj['unit']) )
            if ( val_min != None ):            
                imhead( input_file, 'add', 'beamminor', \
                        str(val_min['value'])+str(val_min['unit']) )
            if ( val_pa != None ):
                imhead( input_file, 'add', 'beampa', \
                        str(val_pa['value'])+str(val_pa['unit']) )
        except:
            self.assertTrue(false)
        else:
            self.assertFalse(
                isinstance(newval,str)
                and newval.replace(' ','').find('NotKnown') > -1
            )
    
    
        #######  OBJECT  #############
        val = None
        newval= None
        try:                                    
            val=imhead( input_file, 'get', 'object' )
            imhead( input_file, 'del', 'object' )
            newval=imhead( input_file, 'get', 'object' )
            print "*** val " + str(val)
            print "*** new " + str(newval)
        except:
            self.assertTrue(False)
        else:
            self.assertTrue( val != newval )
    
        #######  USER DEFINED KEYWORD #############
        ###
        ### Add, get, put, and del a user defined
        ### keyword to the header.
        initVal="Testing the addition of user defined keyword"
        val=None
        try:
            imhead( input_file, 'add', 'test', initVal )
            val=imhead( input_file, 'get', 'test', val )
        except:
            self.assertTrue(False)
        else:
            self.assertTrue( initVal == val )
                
        val2='Updated user keyword'
        newval=None
        try:    
            imhead( input_file, 'put', 'test', val2 )
            newval=imhead( input_file, 'get', 'test' )
        except:
            self.assertTrue(False)
        else:
            self.assertTrue( initVal == val )
    
        casalog.post( 'The next operation will cause an exception.', 'WARN' )
        results=None
        try:
            imhead( input_file, 'del', 'test')
        except:
            self.assertTrue(False)
        try:
            results=imhead( input_file, 'get', 'test' )
        except:
            pass
        else:
            self.assertFalse(
                results!=None
                 and (
                    (isinstance(results,bool) and results==True )
                    or (isinstance(results,dict) and results!={} )
                )
            )
        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    
    ###########################################################################
    # NAME: restored_test
    #
    # SHORT DESCRIPTION: Verify that the file has been resorted
    #                    of them work
    #
    # DESCRIPTION: Check to see if we have the original values again.  All
    #              of the tests should change the values back to what they
    #              were intially.
    #
    ############################################################################
    
    ##########  CHECK ORIG VALUES ############
    #
                
    def test_values_restored(self):
        '''Imhead: test if original values are restored'''    
        if(os.path.exists(input_file_copy)):
            os.system('rm -rf ' +input_file_copy)
        shutil.copytree(input_file, input_file_copy)
        cur_hdr = imhead( input_file, 'list' )
        orig_hdr = imhead( input_file_copy, 'list' )
        gotkeys = cur_hdr.keys()
        gotkeys.sort()
        expkeys = orig_hdr.keys()
        expkeys.sort()
        self.assertTrue(expkeys == gotkeys)
        for key in expkeys:
            got = cur_hdr[key]
            expec = orig_hdr[key]
            exptype = type(expec)
            self.assertTrue(isinstance(got, exptype))
            if ( isinstance(exptype, numpy.ndarray ) ):
                self.assertTrue((got == expec).all())
            elif isinstance(exptype, str):
                self.assertTrue(got == expec)
            elif  isinstance(exptype, float ):
                self.assertTrue(abs(got - expec) < 0.00000001 )

    def test_types(self):
        '''Imhead: CAS-3285 Test types of keys'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        ima = input_file
        for i in range(4):
            cdelt = imhead(imagename=ima, mode='get', hdkey='cdelt'+str(i+1))
            if (type(cdelt['value']) == str):
                retValue['success'] = False
                retValue['error_msgs'] = retValue['error_msgs']\
                    +"Error: CDELT"+str(i+1)+" type should not be a string."
                    
                self.assertTrue(retValue['success'], retValue['error_msgs']) 
    

    def test_units(self):
        '''Imhead: CAS-3285 Test if units are printed'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        ima = input_file
        cdelt1 = imhead(imagename=ima, mode='get', hdkey='cdelt1')
        if (cdelt1['unit'] == ''):
            retValue['success'] = False
            retValue['error_msgs'] = retValue['error_msgs']\
                +"Error: CDELT1"+" has no units."
            
        cdelt2 = imhead(imagename=ima, mode='get', hdkey='cdelt2')
        if (cdelt2['unit'] == ''):
            retValue['success'] = False
            retValue['error_msgs'] = retValue['error_msgs']\
                +"Error: CDELT2"+" has no units."
                
        cdelt4 = imhead(imagename=ima, mode='get', hdkey='cdelt4')
        if (cdelt4['unit'] == ''):
            retValue['success'] = False
            retValue['error_msgs'] = retValue['error_msgs']\
                +"Error: CDELT4"+" has no units."
            
        self.assertTrue(retValue['success'], retValue['error_msgs'])

    def test_list(self):
        '''Imhead: CAS-3300 Test the printing of some keywords in list mode'''
        imf = input_file
        imc = complexim
        
        for image in [imf, imc]:
            logfile = image + ".log"
            open(logfile,'w').close
            casalog.setlogfile(logfile)
            res = imhead(imagename=image, mode='list', verbose=True)
            self.assertTrue(res)
            # restore logfile
            casalog.setlogfile('casapy.log')
            
            cmd = 'grep cdelt1 ' + logfile
            out = commands.getoutput(cmd)
            self.assertNotEqual(out,'','The keyword cdelt1 is not listed')
            cmd = 'grep crval1 ' + logfile
            out = commands.getoutput(cmd)
            self.assertNotEqual(out,'','The keyword crval1 is not listed')
            cmd = 'grep ctype1 ' + logfile
            out = commands.getoutput(cmd)
            self.assertNotEqual(out,'','The keyword ctype1 is not listed')
            cmd = 'grep cunit1 ' + logfile
            out = commands.getoutput(cmd)
            self.assertNotEqual(out,'','The keyword cunit1 is not listed')
            cmd = 'grep shape ' + logfile
            out = commands.getoutput(cmd)
            self.assertNotEqual(out,'','The keyword shape is not listed')
            
            myimd = imdtool()
            myimd.open(image)
            res2 = myimd.list()
            myimd.done()
            self.assertTrue(type(res2) == dict)
            self.assertTrue(deep_equality(res, res2))
        
    def test_get(self):
        """Test the get method"""
        myia = iatool()
        for xx in ['f', 'c']:
            imagename = "xx1d.im_" + xx
            shape = [1, 1, 6]
            myia.fromshape(imagename, shape, type=xx)
            major = {'value': 4, 'unit': "arcsec"}
            minor = {'value': 2, 'unit': "arcsec"}
            pa = {'value': 30, 'unit': "deg"}
            myia.setrestoringbeam(major=major, minor=minor, pa=pa)
            bunit = "Jy/beam"
            myia.setbrightnessunit(bunit)
            if xx == 'f':
                myia.addnoise()
                myia.calcmask(imagename + "<= 0")
            
            myia.done()
            got = imhead(imagename=imagename, mode="get", hdkey="imtype")
            self.assertTrue(got == "Intensity")
            object = "sgrb2n"
            mytb = tbtool()
            mytb.open(imagename, nomodify=False)
            info = mytb.getkeyword("imageinfo")
            info['objectname'] = object
            mytb.putkeyword("imageinfo", info)
            mytb.done()
            imhead(imagename=imagename, mode="put", hdkey="object", hdvalue=object)
            got = imhead(imagename=imagename, mode="get", hdkey="object")
            self.assertTrue(got == object)
            got = imhead(imagename=imagename, mode="get", hdkey="equinox")
            self.assertTrue(got == "J2000")
            got = imhead(imagename=imagename, mode="get", hdkey="date-obs")
            self.assertTrue(got == "2000/01/01/00:00:00")
            got = imhead(imagename=imagename, mode="get", hdkey="epoch")
            self.assertTrue(got == "2000/01/01/00:00:00")
            got = imhead(imagename=imagename, mode="get", hdkey="observer")
            self.assertTrue(got == "Karl Jansky")
            got = imhead(imagename=imagename, mode="get", hdkey="projection")
            self.assertTrue(got == "SIN")
            got = imhead(imagename=imagename, mode="get", hdkey="restfreq")
            self.assertTrue(abs(got['value']/1420405751.7860003 - 1) < 1e-6)
            self.assertTrue(got['unit'] == 'Hz')
            got = imhead(imagename=imagename, mode="get", hdkey="reffreqtype")
            self.assertTrue(got == "LSRK")
            got = imhead(imagename=imagename, mode="get", hdkey="telescope")
            self.assertTrue(got == "ALMA")
            myia.open(imagename)
            myia.setrestoringbeam(remove=True)
            myia.done()
            self.assertFalse(
                imhead(imagename=imagename, mode="get", hdkey="beammajor")
            )
            myia.open(imagename)
            myia.setrestoringbeam(major=major, minor=minor, pa=pa)
            myia.done()
            got = imhead(imagename=imagename, mode="get", hdkey="beammajor")
            self.assertTrue(got == major)
            got = imhead(imagename=imagename, mode="get", hdkey="beamminor")
            self.assertTrue(got == minor)
            got = imhead(imagename=imagename, mode="get", hdkey="beampa")
            self.assertTrue(got == pa)
            got = imhead(imagename=imagename, mode="get", hdkey="bunit")
            self.assertTrue(got == bunit)
            got = imhead(imagename=imagename, mode="get", hdkey="masks")
            if xx == 'f':
                self.assertTrue(got == ["mask0"])
            else:
                self.assertTrue(len(got) == 0)
            got = imhead(imagename=imagename, mode="get", hdkey="shape")
            self.assertTrue((got == shape).all())
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="ctype6"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="ctype0"))
            got = imhead(imagename=imagename, mode="get", hdkey="ctype1")
            self.assertTrue(got == "Right Ascension")
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="crpix6"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="crpix0"))
            got = imhead(imagename=imagename, mode="get", hdkey="crpix1")
            self.assertTrue(got == 0)
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="crval6"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="crval0"))
            got = imhead(imagename=imagename, mode="get", hdkey="crval3")
            self.assertTrue(got == qa.quantity("1.415e9Hz"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="cdelt6"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="cdelt0"))
            got = imhead(imagename=imagename, mode="get", hdkey="cdelt3")
            self.assertTrue(got == qa.quantity("1000Hz"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="cunit6"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="cunit0"))
            got = imhead(imagename=imagename, mode="get", hdkey="cunit3")
            self.assertTrue(got == "Hz")
            if xx == 'f':
                myia.open(imagename)
                stats = myia.statistics()
                myia.done()
            got = imhead(imagename=imagename, mode="get", hdkey="datamin")
            if xx == 'f':
                self.assertTrue(got == stats['min'])
            else:
                self.assertTrue(got == None)
            got = imhead(imagename=imagename, mode="get", hdkey="datamax")
            if xx == 'f':
                self.assertTrue(got == stats['max'])
            else:
                self.assertTrue(got == None)
            got = imhead(imagename=imagename, mode="get", hdkey="minpos")
            if xx == 'f':
                self.assertTrue(got == stats['minposf'].translate(None, ","))
            else:
                self.assertTrue(got == None)
            got = imhead(imagename=imagename, mode="get", hdkey="maxpos")
            if xx == 'f':
                self.assertTrue(got == stats['maxposf'].translate(None, ","))
            else:
                self.assertTrue(got == None)
            got = imhead(imagename=imagename, mode="get", hdkey="minpixpos")
            if xx == 'f':
                self.assertTrue((got == stats['minpos']).all())
            else:
                self.assertTrue(got == None)
            got = imhead(imagename=imagename, mode="get", hdkey="maxpixpos")
            if xx == 'f':
                self.assertTrue((got == stats['maxpos']).all())
            else:
                self.assertTrue(got == None)
            value = "fred"   
            key = "userkey"     
            imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=value)
            got = imhead(imagename=imagename, mode="get", hdkey=key)
            self.assertTrue(got == value)

    def test_del(self):
        """Test deletion/clearing of keys"""
        myia = iatool()
        for t in ['f', 'c']:
            imagename = "xx1d_del.im_" + t
            shape = [1, 1, 6]
            myia.fromshape(imagename, shape, type=t)
            major = {'value': 4, 'unit': "arcsec"}
            minor = {'value': 2, 'unit': "arcsec"}
            pa = {'value': 30, 'unit': "deg"}
            myia.setrestoringbeam(major=major, minor=minor, pa=pa)
            bunit = "Jy/beam"
            myia.setbrightnessunit(bunit)
            if t == 'f':
                myia.addnoise()
                myia.calcmask(imagename + "<= 0")
                stats = myia.statistics()
            myia.done()
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="bunit"))
            got = imhead(imagename=imagename, mode="get", hdkey="bunit")
            self.assertTrue(got == "")
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="cdelt1"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="crpix1"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="crval1"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="cunit1"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="ctype1"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="equinox"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="imtype"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="masks", hdvalue="blue"))
            got = imhead(imagename=imagename, mode="del", hdkey="masks", hdvalue="")
            self.assertTrue(got)
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="masks")) == 0)
            if t == 'f':
                myia.open(imagename)
                myia.calcmask(imagename + "<= 0")
                myia.done()
                self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="masks")) == 1)
                self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="masks", hdvalue="mask0"))
                self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="masks")) == 0)
            
            mytb = tbtool()
            mytb.open(imagename, nomodify=False)
            info = mytb.getkeyword("imageinfo")
            info['objectname'] = "xyz"
            mytb.putkeyword("imageinfo", info)
            mytb.done()
            
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="object")) > 0)
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="object"))
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="object")) == 0)
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="epoch"))
            
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="epoch"))
            
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="observer")) > 0)
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="observer"))
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="observer")) == 0)
    
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="projection"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="reffreqtype"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="restfreq"))
            self.assertFalse(imhead(imagename=imagename, mode="del", hdkey="shape"))
    
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="telescope")) > 0)
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="telescope"))
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="telescope")) == 0)
    
            self.assertTrue(len(imhead(imagename=imagename, mode="get", hdkey="bmaj")) > 0)
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="bmaj"))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey="bmaj") == None)
            
            for x in ['datamin', 'minpos', 'minpixpos', 'datamax', 'maxpos', 'maxpixpos']:
                self.assertFalse(imhead(imagename=imagename, mode="del", hdkey=x))
                
            val = "afdasdf"
            imhead(imagename=imagename, mode="put", hdkey="jj", hdvalue=val)
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey="jj") == val)
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="jj"))
            self.assertFalse(imhead(imagename=imagename, mode="get", hdkey="jj"))

    def test_add(self):
        myia = iatool()
        for t in ['f', 'c']:
            imagename = "xx1d_add.im_" + t
            shape = [1, 1, 6]
            myia.fromshape(imagename, shape, type=t)
            major = {'value': 4, 'unit': "arcsec"}
            minor = {'value': 2, 'unit': "arcsec"}
            pa = {'value': 30, 'unit': "deg"}
            myia.setrestoringbeam(major=major, minor=minor, pa=pa)
            bunit = "Jy/beam"
            myia.setbrightnessunit(bunit)
            if t == 'f':
                myia.addnoise()
                myia.calcmask(imagename + "<= 0")
                stats = myia.statistics()
            myia.done()
            
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="bunit", hdvalue="K"))
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey="bunit"))
            self.assertTrue(imhead(imagename=imagename, mode="add", hdkey="bunit", hdvalue="K"))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey="bunit") == "K")
    
            for key in ['cdelt1', 'crpix1', 'crval1', 'cunit1', 'ctype1']:
                self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=0.5))
        
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="equinox", hdvalue='b1900'))
    
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="imtype", hdvalue='rm'))
    
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="masks", hdvalue="xx"))
    
            key = "object"
            value = "sgrb2"
            self.assertTrue(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == value)
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
    
            epoch = imhead(imagename=imagename, mode="get", hdkey="epoch")
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="epoch", hdvalue=epoch))
    
            key = "observer"
            value = "Edwin Hubble"
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey=key))
            self.assertTrue(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == value)
            
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="projection", hdvalue='sin'))
    
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="reffreqtype", hdvalue='lsrk'))
            
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="restfreq", hdvalue='10GHz'))
    
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey="shape", hdvalue='10GHz'))
    
            key = "telescope"
            value = "bima"
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey=key))
            self.assertTrue(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == value)
    
            key = "bmaj"
            value = 4
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            self.assertTrue(imhead(imagename=imagename, mode="del", hdkey=key))
            key = "bpa"
            value = 4
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            key = "bmaj"
            value = 4
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            value = "4m"
            self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            value = "4arcmin"
            self.assertTrue(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == qa.quantity(value))
            key = "bmin"
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == qa.quantity(value))
            key = "bpa"
            value = "0deg"
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == qa.quantity(value))
    
            for key in [
                "datamin", "datamax", "maxpos", "minpos",
                "maxpixpos", "minpixpos"
            ]: 
                self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=4))
                
            
            key = "user-specified"
            for value in ["test-val", 6, qa.quantity("4km/s")]:
                self.assertTrue(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
                self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == value)
                self.assertFalse(imhead(imagename=imagename, mode="add", hdkey=key, hdvalue=value))
                self.assertTrue(imhead(imagename=imagename, mode="del", hdkey=key, hdvalue=value))
    

    def test_put(self):
        myia = iatool()
        for t in ['f', 'c']:
            imagename = "xx1d_put.im" + t
            shape = [1, 1, 6]
            myia.fromshape(imagename, shape, type=t)
            major = {'value': 4, 'unit': "arcsec"}
            minor = {'value': 2, 'unit': "arcsec"}
            pa = {'value': 30, 'unit': "deg"}
            myia.setrestoringbeam(major=major, minor=minor, pa=pa)
            bunit = "Jy/beam"
            self.assertTrue(myia.setbrightnessunit(bunit))
            if type == 'f':
                # addnoise currently only supports float images
                myia.addnoise()
                myia.calcmask(imagename + "<= 0")
                stats = myia.statistics()
            units = myia.coordsys().units()
            myia.done()
            
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey="bunit", hdvalue="K"))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey="bunit") == "K")
    
            key = "cdelt1"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="3arcsec"))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            exp = qa.quantity("0.05'")
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("3arcsec")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("0.05")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity(0.05)))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertFalse(
                imhead(
                    imagename=imagename, mode="put",
                    hdkey=key, hdvalue="3bogus"
                )
            )
            
            key = "crpix1"
            exp = 4.5
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(got == exp)
            exp = -8.6
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=str(exp)))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(got == exp)
            self.assertFalse(
                imhead(
                    imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("3arcsec")
                )
            )
            
            key = "crval1"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="3arcsec"))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            exp = qa.quantity("0.05'")
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("3arcsec")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("0.05")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity(0.05)))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
    
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertFalse(
                imhead(
                    imagename=imagename, mode="put",
                    hdkey=key, hdvalue="3bogus"
                )
            )
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="3:00:00"))
            got = imhead(imagename=imagename, mode="get", hdkey=key)
            exp = qa.quantity("45deg")
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
    
            key = "ctype1"
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=4.5))
            exp = "universe Number"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(got == exp)
            
            key = "cunit1"
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=4.5))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="bogus"))
            exp = "K"
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            exp = "deg"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(got == exp)
            
            key = "equinox"
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=4.5))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="bogus"))
            exp = "GALACTIC"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(got == exp)
            
            
            key = "imtype"
            exp = "Rotation Measure"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == exp)
            exp = "Jack"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == "Intensity")
    
            key = "masks"
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="x"))
            
            key = "object"
            exp = "Restaraunt at the end of the universe"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == exp)
            exp = 4
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            
            key = "epoch"
            exp = "2009/05/30/05:21:45"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == exp)
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="5km"))
    
            key = "observer"
            exp = "Observer at the end of the universe"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == exp)
            exp = 4
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            
            key = "projection"
            exp = "TAN"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            got = imhead(imagename=imagename, mode="get", hdkey=key)
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == exp)
            exp = "blah"
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            exp = 4
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            
            key = "reffreqtype"
            exp = "CMB"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            got = imhead(imagename=imagename, mode="get", hdkey=key)
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == exp)
            exp = "blah"
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            exp = 4
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            
            key = "restfreq"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="45GHz"))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            exp = qa.quantity("45GHz")
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("45GHz")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("4.5e10")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity(4.5e10)))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertFalse(
                imhead(
                    imagename=imagename, mode="put",
                    hdkey=key, hdvalue="3bogus"
                )
            )
            
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey="shape", hdvalue=[5,5,5]))
    
            key = "telescope"
            exp = "Telescope at the end of the universe"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == exp)
            exp = 4
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=exp))
            
            key = "bpa"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="45deg"))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            exp = qa.quantity("45deg")
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("45deg")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("45")))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity(45)))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="3bogus"))
            
            key = "bmaj"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="5arcsec"))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            exp = qa.quantity("5arcsec")
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("5arcsec")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("5")))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity(5)))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="3bogus"))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="1arcsec"))
    
            key = "bmin"
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="1arcsec"))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            exp = qa.quantity("1arcsec")
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("1arcsec")))
            got = imhead(imagename=imagename, mode="get", hdkey=key) 
            self.assertTrue(abs(qa.sub(got, exp)['value']) < 1e-8)
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity("1")))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=qa.quantity(1)))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="3bogus"))
            self.assertFalse(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue="10arcsec"))
    
            for key in [
                "datamin", "datamax", "maxpos", "minpos",
                "maxpixpos", "minpixpos"
            ]: 
                self.assertFalse(imhead(imagename=imagename, mode="set", hdkey=key, hdvalue=4))
               
            key = "user-specified"
            for value in ["test-val", 6, qa.quantity("4km/s")]:
                self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=value))
                self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == value)
                self.assertTrue(imhead(imagename=imagename, mode="put", hdkey=key, hdvalue=value))
                self.assertTrue(imhead(imagename=imagename, mode="get", hdkey=key) == value)
                self.assertTrue(imhead(imagename=imagename, mode="del", hdkey=key, hdvalue=value))
            
    def test_CAS4355(self):
        """ verify puthead can take sesigimal values where appropriate (CAS-4355)"""
        myia = iatool()
        image = "cas4355.im"
        myia.fromshape(image, [10,10])
        ra = "14:33:10.5"
        key = "crval1"
        imhead(imagename=image, mode="put", hdkey=key, hdvalue=ra)
        got = imhead(imagename=image, mode="get", hdkey=key)
        got = qa.canon(got)
        exp = qa.canon(qa.toangle(ra))
        self.assertTrue(qa.getunit(got) == qa.getunit(exp))
        self.assertTrue(abs(qa.getvalue(got)/qa.getvalue(exp) - 1) < 1e-8)
        
        dec = "-22.44.55.66"
        key = "crval2"
        imhead(imagename=image, mode="put", hdkey=key, hdvalue=dec)
        got = imhead(imagename=image, mode="get", hdkey=key)
        got = qa.canon(got)
        exp = qa.canon(qa.toangle(dec))
        self.assertTrue(qa.getunit(got) == qa.getunit(exp))
        self.assertTrue(abs(qa.getvalue(got)/qa.getvalue(exp) - 1) < 1e-8)
        
    def test_put_crval_stokes(self):
        """Test updating stokes, CAS-6352"""
        myia = iatool()
        imagename = "CAS-6352.im"
        myia.fromshape(imagename, [1, 1, 3])
        myia.done()
        self.assertFalse(
            imhead(
                imagename=imagename, mode="put", hdkey="crval3",
                hdvalue="I"
            )
        )
        expec = ["Q", "XX", "LL"]
        self.assertTrue(
            imhead(
                imagename=imagename, mode="put", hdkey="crval3",
                hdvalue=expec
            )
        )
        self.assertTrue(
            (
                imhead(imagename=imagename, mode="get", hdkey="crval3")
                == expec
            ).all()
        )
        
        
def suite():
    return [imhead_test]    
    
