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
        if(os.path.exists(input_file)):
            os.system('rm -rf ' +input_file)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/ngc5921redux/reference/ngc5921_regression/'
        os.system('cp -r ' +datapath + input_file +' ' + input_file)

    def tearDown(self):
        os.system('rm -rf ' +input_file)
        self.assertTrue(len(tb.showcache()) == 0)

    def test_mode(self):
        '''Imhead: Test list,summary,history,get,put,del,add,empty,bad modes'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        casalog.post( "Staring imhead mode tests!", "NORMAL2" )
    
        
        ################################################################
        # List mode
        casalog.post( "Testing LIST mode", "NORMAL4" );
        results=None
        try:
            results = imhead( input_file, 'list' )
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
            self.assertTrue(got['value'] == testval)
            if ( val != None ):
                imhead( input_file, 'put', 'object', val['value'] )
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
                imhead( input_file, 'put', 'imtype', val['value'] )
        except:
            self.assertTrue(False)
        else:        
            self.assertFalse( val!=None and val == newval )
    
        #######  TELESCOPE  #############
        val   = None
        newval= None
        try:
            val=imhead( input_file, 'get', 'telescope' )
            imhead( input_file, 'put', 'telescope', 'myTelescope' )
            newval=imhead( input_file, 'get', 'telescope' )
            if ( val != None ):        
                imhead( input_file, 'put', 'telescope', val['value'] )
        except:
            self.assertTrue(False)
        else:
            self.assertFalse( val!=None and val == newval )
         
        #######  OBSERVER  #############
        val   = None
        newval= None
        try:
            val=imhead( input_file, 'get', 'observer' )
            imhead( input_file, 'put', 'observer', 'me' )
            newval=imhead( input_file, 'get', 'observer' )
            if ( val != None ):
                imhead( input_file, 'put', 'observer', val['value'] )
        except:
            self.assertTrue(False)
        else:        
            self.assertFalse( val!=None and val == newval )
    
        #######  DATE-OBS  #############
        val   = None
        newval= None
        try:
            val=imhead( input_file, 'get', 'date-obs' )
            imhead( input_file, 'put', 'date-obs', '2007/06/20/00:00:00' )
            newval=imhead( input_file, 'get', 'date-obs' )
            if ( val != None ):
                imhead( input_file, 'put', 'date-obs', str(val['value'])+str(val['unit']) )
        except:
            self.assertTrue(False)
        else:
            self.assertFalse( val!=None and val == newval )
              
        #######  EQUINOX  #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'equinox' )
            imhead( input_file, 'put', 'equinox', 'B1950' )
            newval=imhead( input_file, 'get', 'equinox' )
            if ( val != None ):
                imhead( input_file, 'put', 'equinox', val['value'] )
        except:
            self.assertTrue(False)
        else:
            self.assertFalse(val!=None and val == newval )
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
            imhead( input_file, 'put', 'projection', 'TAN' )
            newval=imhead( input_file, 'get', 'projection' )
            if ( val != None ):
                imhead( input_file, 'put', 'projection', val['value'] )            
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
            imhead( input_file, 'put', 'beamminor', '24arcsec' )
            newval=imhead( input_file, 'get', 'beamminor' )
            if ( val != None ):
                imhead( input_file, 'put', 'beamminor', str(val['value'])+str(val['unit']) )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get beam's minor axis failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set beam's minor axis Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: '24arcsec'"
    
    
        #######  BEAM PA #############
        val   = None
        newval= None
        try:                    
            val=imhead( input_file, 'get', 'beampa' )
            imhead( input_file, 'put', 'beampa', '0.5deg' )
            newval=imhead( input_file, 'get', 'beampa' )
            if ( val != None ):        
                imhead( input_file, 'put', 'beampa', str(val['value'])+str(val['unit']) )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get beam's position angle failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set beam's position angle Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: '0.5deg'"
    
           
        #######  BUNIT  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'bunit' )
            imhead( input_file, 'put', 'bunit', 'kJy/beam' )
            newval=imhead( input_file, 'get', 'bunit' )
            if ( val != None ):
                imhead( input_file, 'put', 'bunit', str(val['value'])+str(val['unit']) )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get beam's unit failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set beam's unit Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: 'kJy/beam'"
    
    
        #######  CTYPE3  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'ctype3' )
            imhead( input_file, 'put', 'ctype3', 'Declination' )
            newval=imhead( input_file, 'get', 'ctype3' )
            if ( val != None ):
                imhead( input_file, 'put', 'ctype3', val['value'] )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get ctype3 failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set ctype3 unit Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: 'Declination'"
        
    
        #######  CRPIX2  #############
        val   = None
        newval= None
        try:                                
            val=imhead( input_file, 'get', 'crpix2' )
            imhead( input_file, 'put', 'crpix2', '64.0' )
            newval=imhead( input_file, 'get', 'crpix2' )
            if ( val != None ):
                imhead( input_file, 'put', 'crpix2', val['value'] )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get beam's unit failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set beam's unit Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: '64.0'"
    
    
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
            imhead( input_file, 'put', 'cunit1', 'deg' )
            newval=imhead( input_file, 'get', 'cunit1' )
            if ( val != None ):
                imhead( input_file, 'put', 'cunit1', val['value'] )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: set/get cunit1 failed in image "+input_file
        else:
            if ( val!=None and val == newval ) :
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError Get/Set cunit1 Failed"\
                        +"\n       Original value: "+str(val)\
                        +"\n       Current value:  "+str(newval)\
                        +"\n       Expected value: 'deg'"
    
    
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
    
        #######  IMTYPE  #############
        val = None
        newval= None
        try:                                    
            val=imhead( input_file, 'get', 'imtype' )
            imhead( input_file, 'del', 'imtype' )
            newval=imhead( input_file, 'get', 'imtype' )
    
            try:
                newval=imhead( input_file, 'get', 'imtype' )
            except:
                pass
            if ( val != None ):
                imhead( input_file, 'add', 'imtype', val['value'] )
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
            self.assertTrue( initVal == val['value'] )
                
        val2='Updated user keyword'
        newval=None
        try:    
            imhead( input_file, 'put', 'test', val2 )
            newval=imhead( input_file, 'get', 'test' )
        except:
            self.assertTrue(False)
        else:
            self.assertTrue( initVal == val['value'] )
    
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
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }    
        casalog.post( "Starting imhead restored values test!", "NORMAL2" )
    
        if(os.path.exists(input_file_copy)):
            os.system('rm -rf ' +input_file_copy)
            
        shutil.copytree(input_file, input_file_copy)
        
        try:
            cur_hdr=imhead( input_file, 'list' )
            orig_hdr=imhead( input_file_copy, 'list' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
              +"\nError: Unable to get values from files: "\
              +"\n\t"+input_file\
              +", and\n\t"+input_file_copy
        else:
            for key in orig_hdr.keys() :
                #print "CHECKING IF "+key+" has CHANGED VALUE."
                #print "has key: ", cur_hdr.has_key(key)
                #print "ORIG value: ", orig_hdr[key]
                #print "CURRENT value: ", cur_hdr[key]
    
                match = False
                if ( cur_hdr.has_key(key) ):
                    if ( isinstance( orig_hdr[key], numpy.ndarray ) ):
                        if ( not isinstance( cur_hdr[key], numpy.ndarray ) ):
                            match = False
                        else:
                            results = orig_hdr[key] == orig_hdr[key]
                            if ( results.all() ):
                                match = True
                    else:
                        if( isinstance( cur_hdr[key], float )\
                            and abs(cur_hdr[key]-orig_hdr[key]) < 0.00000001 ):
                            # Allow for rounding errors when changing values.
                            match = True
                        elif ( cur_hdr[key]==orig_hdr[key] ):
                            match = True
                        
    
                if ( not match ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: "+str(key)+" has not been restored!"\
                         +"\n         Current value:   "+str(cur_hdr[key])\
                         +"\n         Expected value:  "+str(orig_hdr[key])
    
        ## LIST values again.  They should be the same as at the start of the test
        #imhead( input_file, 'list' )
        #print "\nCheck that the values listed are the same as at the"
        #print "beginning of the test"
        
        self.assertTrue(retValue['success'],retValue['error_msgs'])

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
        logfile = 'imhead.log'
        open(logfile,'w').close
        casalog.setlogfile(logfile)
        res = imhead(imagename=input_file, mode='list', verbose=True)
        # restore logfile
        casalog.setlogfile('casapy.log')
        
        cmd = 'grep cdelt1 imhead.log'
        out = commands.getoutput(cmd)
        self.assertNotEqual(out,'','The keyword cdelt1 is not listed')
        cmd = 'grep crval1 imhead.log'
        out = commands.getoutput(cmd)
        self.assertNotEqual(out,'','The keyword crval1 is not listed')
        cmd = 'grep ctype1 imhead.log'
        out = commands.getoutput(cmd)
        self.assertNotEqual(out,'','The keyword ctype1 is not listed')
        cmd = 'grep cunit1 imhead.log'
        out = commands.getoutput(cmd)
        self.assertNotEqual(out,'','The keyword cunit1 is not listed')
        cmd = 'grep shape imhead.log'
        out = commands.getoutput(cmd)
        self.assertNotEqual(out,'','The keyword shape is not listed')
        
        myimd = imdtool()
        myimd.open(input_file)
        res2 = myimd.list()
        myimd.done()
        self.assertTrue(type(res2) == dict)
        self.assertTrue(deep_equality(res, res2))
        
        
    def test_shape(self):
        '''Imhead: CAS-3301: Keyword shape should be included in the output'''
        shape = imhead(imagename=input_file,mode='get',hdkey='shape')
        self.assertTrue(shape,'Keyword shape is missing')
        self.assertTrue(shape['value'].all(), 'Not all values of shape exist.')
        self.assertEqual(shape['value'][0],256, 'Value of shape[0] should be 256 and it is %s instead.'
                         %shape['value'][0])
        
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
        
        
def suite():
    return [imhead_test]    
    
