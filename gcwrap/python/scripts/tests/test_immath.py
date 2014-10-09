#######################################################################3
#  immath_test.py
#
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
# dmehring
# </author>
#
# <summary>
# Test suite for the CASA immath Task
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="immath.py:description">immath</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# immath_test stands for image math test
# </etymology>
#
# <synopsis>
# immath_test.py is a Python script that tests the correctness
# of the immath task in CASA.
#
# The tests include:
#   ????
#
# In the immath task ????
#
# </synopsis> 
#
# <example>
# # This test was designed to run in the automated CASA test system.
# # This example shows who to run it manually from with casapy.
# runUnitTest.main(['test_immath'])
#
# or outside casapy like this:
# casapy -c runUnitTest.py test_immath
#
# </example>
#
# <motivation>
# To provide a test standard to the immath task to try and ensure
# coding changes do not break the 
# </motivation>
#
# <todo>
# mode input/output parameter test
# varnames input/output parameter test
# polariziation/spectral mode tests.
# </todo>
########################################################################3

import sys
import traceback
import os
import shutil
import random
import time
import casac
import numpy
import glob
import struct
from tasks import *
from taskinit import *
import unittest

sep = os.sep
datapath=os.environ.get('CASAPATH').split()[0] + sep + 'data' + sep\
    + 'regression' + sep + 'unittest' + sep + 'immath' + sep

cas1452_1_im = 'CAS-1452-1.im'
cas1910_im = 'CAS-1910.im'
cas1830_im = 'ngc5921.demo.cleanimg.image'
n5921im = 'ngc5921.clean.image'
n1333im = 'n1333_both.image'
n1333rgn = 'n1333_both.image.rgn'
c129bcim = '3C129BC.clean.image'
im0im = 'immath0.im'
im1im = 'immath1.im'
im2im = 'immath2.im'
im3im = 'immath3.im'
im4im = 'immath4.im'
im5im = 'immath5.im'
im6im = 'immath6.im'
im7im = 'immath7.im'
im8im = 'immath8.im'
im9im = 'immath9.im'
im10im = 'immath10.im'

IQU_im = '3C129IQU.im'
IQUV_im = '3C129IQUV.im' 
POLA_im = '3C129_POLA.im'
POLL_im = '3C129_POLL.im'  
POLT_im = '3C129_POLT.im'  
Q_im = '3C129Q.im'
QU_im = '3C129QU.im'
U_im = '3C129U.im'  
UV_im = '3C129UV.im'  
V_im = '3C129V.im'
thresh_mask = '30uJy_thresh_mask.tbl'

imageList =['ngc5921.clean.image', 'n1333_both.image', 'n1333_both.image.rgn', '3C129BC.clean.image']

imageList2 =['ngc5921.clean.image', 'n1333_both.image', 'n1333_both.image.rgn', '3C129BC.clean.image',
cas1910_im,cas1452_1_im, cas1830_im]

imageList3 =['immath0.im', 'immath1.im', 'immath2.im', 'immath3.im', 'immath4.im', 'immath5.im',
'immath6.im', 'immath7.im', 'immath8.im', 'immath9.im','immath10.im']

imageList4 = [IQU_im, IQUV_im, POLA_im, POLL_im, POLT_im, Q_im, QU_im, U_im, UV_im, V_im, thresh_mask]


################      HELPER FUNCTIONS      ###################

# Retrieve the pixel value at a particular postion in an image.
def _getPixelValue( imageName, point ):
    ia.open( imageName );
    retValue = ia.pixelvalue( point );
    ia.close();
    
    if len( retValue ) < 1 :
        # Create an exception
        msg = str( "Unable to obtain value at: " ) + str(point) + str( " from file " ) + imageName
        raise ValueError, str
    else:
        retValue = retValue['value']['value']

    return retValue

def _exceptionInfo( maxLevel=5 ):
    cla, exc, trbk = sys.exc_info()
    excName = cla.__name__
    try:
        excArgs = exc.__dict__["args"]
    except KeyError:
        excArgs = "<no args>"
    excTb = traceback.format_tb(trbk, maxLevel)
    return(excName, excArgs, excTb)

def make_data(imshape):
    data = ia.makearray(0, imshape)
    for i in range(imshape[0]):
        data[i] = list(data[i])
        for j in range(imshape[1]):
            data[i][j] = list(data[i][j])
            for k in range(imshape[2]):
                data[i][j][k] = (k+1) + (j+1)*imshape[0] + (i+1)
            data[i][j] = tuple(data[i][j])
        data[i] = tuple(data[i])
    return data



################      THE TESTS         ###################


####################################################################
# Valid/Invalid Input to parameters.  The parameters are:
#    imagename
#    outfile
#    mode
#    mask
#    region
#    box
#    chans
#    stokes
#
# Returns a dictionary contain two keywords as follows:
#        success:     set to True if all tests passed
#        error_msgs:  detailed message(s) of any error(s) that occured.
####################################################################

class immath_test1(unittest.TestCase):
    
    def setUp(self):
        if(os.path.exists(imageList[0])):
            for img in imageList:
                os.system('rm -rf ' +img)
            
        for img in imageList:
            os.system('cp -RL ' +datapath + img +' ' + img)
            

    def tearDown(self):
        for img in imageList:
            os.system('rm -rf ' +img)
            os.system('rm -rf input_test*')
        self.assertTrue(len(tb.showcache()) == 0)
                       
    def test_input2(self):
        '''Immath 2: Test bad input file'''
        #######################################################################
        # Testing the imagename parameter.
        #    1. Bad file name should throw and exception
        #    2. Good file name, a file should be
        #    3. List with bad file in it.
        #    4. List with good files in it.
        #######################################################################
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "The IMAGENAME parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    
        result = None
        try:
            results = immath( 'junk', outfile='input_test1' )
        except:
            pass
        else:
            if ( ( results != None and  not isinstance(results, bool) )\
               or  results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Badfile, 'junk', was not reported as missing."\
                     + "\n RESULTS: "+str(results)

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test_input3(self):
        '''Immath 3: test good input file'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        results = None
        try:
            results = immath( imageList[0], outfile='input_test2', expr='IM0' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Unable to do math on "+imageList[0]
            
        if (  not os.path.exists( 'input_test2' ) or results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: outfile 'input_test2' was not created."

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test_input4(self):
        '''Immath 4: Test bad list of input files'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        result = None
        try:
            results = immath( [imageList[0],3,imageList[2]], outfile='input_test3')
        except:
            pass
        else:
            if ( ( results != None and  not isinstance(results, bool) )\
               or  results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Badfile, '3', was not reported."

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test_input5(self):
        '''Immath 5: test list with good input files'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        results=None
        try:
            results = immath( imageList, outfile='input_test4', expr='IM0' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Unable to do math on image "+imageList[0]
            
        if (  not os.path.exists( 'input_test4' ) or results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: 'input_test4' output file was not created."
    
    
        #######################################################################
        # Testing the outfile parameter.
        #    1. Bad file, file already exists, exception should be thrown
        #    2. Non-string value
        #    3. Good file name, a file should be
        #######################################################################
        casalog.post( "The OUTFILE parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    
        results = None
        try:
            results = immath( imageList[0], outfile='input_test4', expr='IM0*1' )
        except:
            pass
        else:
            if ( ( results != None and  not isinstance(results, bool) )\
               or  results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                      +"\nError: Bad outfile, 'input_test4', was not reported as already existing."

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test_input7(self):
        '''Immath 7: test outfile'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        results = None
        try:
            results = immath( imageList[0], outfile='input_test5', expr='IM0*1' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Unable to create image 'input_test5'"
        if ( not os.path.exists( 'input_test5' ) or results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: output file, 'input_test5', was not created."

        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
        #######################################################################
        # Testing the mode parameter.
        #    TODO
        #    Valid modes are: evalexpr, spix, pola, poli
        #    Invalid ???
        #######################################################################
          
    def test_input8(self):
        '''Immath 8: Test bad region parameter'''
        #######################################################################
        # Testing REGION parameter
        # Expects a file containing a region record, as created by the viewer.
        # Tests include bad file name, file with bad content, and good file.
        ####################################################################### 
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "The REGION parameter tests will cause errors to occur, do not be alarmed", 'WARN' )

        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass
        
        results=None    
        try:
            results = immath( imageList[1], region='garbage.rgn' )
        except:
            #We want this to fail
            pass
        else:
            if ( ( results != None and  not isinstance(results, bool) )\
               or  results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                              +"\nError: Bad region file, 'garbage.rgn', was not reported as missing."+"\nREsults: "+str(results)
    
        try:
            filename = os.getcwd() + sep + 'garbage.rgn'
            fp=open( filename, 'w' )
            fp.writelines('This file does NOT contain a valid CASA region specification\n')
            fp.close()
    
            results=None
            # First remove the default results file
            try:
                shutil.rmtree( 'immath_results.im' )
            except:
                pass        
            try:
                results = immath( imageList[1], region=filename )
            except:
                pass
            else:
                if ( ( results != None and not isinstance(results, bool) )\
                       or results==True ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                              + "\nError: Bad region file, 'garbage.rgn',"\
                              + " was not reported as bad."+str(results)
        except Exception, err:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Unable to create bad region file.\n\t"
            raise Exception, err

        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    def test_input9(self):
        '''Immath 9: Test good region parameter'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        results = None
        print "ARG"
        print "immath( imagename="+imageList[1]+", expr='IM0', "\
                        +"region="+imageList[2]+", outfile='input_test12' )"
        try:
            rec = rg.fromfiletorecord(imageList[2])
            results=immath( imagename=imageList[1], expr='IM0', \
                        region=imageList[2], outfile='input_test12' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Unable to do math with region file "+imageList[1]
            
        if ( not os.path.exists('input_test12') or results==None or results==False ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                  +"\nError: output file 'input_test12', was not created."\
                  +"\n  RESULTS "+str(results)

        self.assertTrue(retValue['success'],retValue['error_msgs'])            
    
    def test_input10(self):
        '''Immath 10: Test bad x and y box values'''
        #######################################################################
        # Testing BOX parameter
        # The input, imageList[0]=ngc5921.clean.image, file has pixel
        # values ranging from:
        #   0-255, 0-255
        # Tests include -3, -1, 0, 1 random valid value, 254, 255, 256
        #   for both the x, and y coords
        #
        # Note: -1 is a special case implying use the full range, so to
        #       be out of bounds we need -2 or less.
        #######################################################################
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "The BOX parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
        
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], box='-3,0,254,254', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) )\
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Bad box value, 'x=-3', was not reported as bad."
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass
        try:
            results = immath( imageList[0], box='0,-3,254,254', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Bad box value, 'y=-3', was not reported as bad."
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass
        try:
            results = immath( imageList[0], box='-2,0,254,254', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Bad box value, 'x=-2', was not reported."\
                     +"\n\tRESULTS: "+str(results)
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], box='0,-2,254,254', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Bad box value, 'y=-2', was not reported as bad."


        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], box='0,0,256,254', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Bad box value, 'x=256', was not reported as bad."
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], box='0,0,254,256', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Bad box value, 'y=256', was not reported as bad."


        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], box='0, 0,266,200', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and not ( isinstance(results, bool) )
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Bad box value, 'x=266', was not reported as bad."
    
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], box='0,0,254,266', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Bad box value, 'y=266', was not reported as bad."
    
        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test_input12(self):
        '''Immath 12: Test bad values of channels'''
        #######################################################################
        # Testing CHANS parameter: valid values 0-39 for our image
        # Values used for testing, -5,-2,0,22~35, 44,45,46
        #
        # NOTE: a coord value of -1 indicates use all, so -1 is a valid
        #       coordinate.
        #######################################################################
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "The CHANS parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
        
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try: 
            results = immath( imageList[0], chans='-5', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Bad channel value, '-5', was not reported."
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], chans='-2', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Bad channel value, '-2', was not reported."\
                       +"\n\tRESULTS: "+str(results)
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], chans='-18', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Bad channel value of -18 was not reported."
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass
        try:
            results = immath( imageList[0], chans='47', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Bad channel value of 47 was not reported."
    
        results = None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], chans='46', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: Bad channel value of 46 was not reported."

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def test_input13(self):
        '''Immath 12: Test selection of channels'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        results = None            
        try:
            print "immath( imagename="+imageList[0]+", varnames='A', expr='A', chans='22~35', outfile='input_test14' )"
            results = immath( imagename=imageList[0], chans='22~35', expr='A', varnames='A', outfile='input_test14' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Unable to do image math on chans=22~35 only "
        if ( not os.path.exists( 'input_test14' ) or results==None or results==False ): 
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: output file, 'input_test14', was not created."\
                     +"\n\tResults: "+str(results)
        
        results = None
        try:
            results = immath( imageList[0], expr='IM0', chans='0', outfile='input_test15' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Unable to do image math with chans=0 only"
        if ( not os.path.exists( 'input_test15' ) or results==None or results==False ): 
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: output file 'input_test15`' was not created."\
                     +"\nRESULT: "+str(results)
    
        results=None
        try:
            results = immath( imageList[0], chans='39', expr='IM0', outfile='input_test16' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Unable to do image math with chans=39 only"
        if ( not os.path.exists( 'input_test16' ) or results==None or results==False ): 
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: output file 'input_test16' was not created."

        self.assertTrue(retValue['success'],retValue['error_msgs'])    

    def test_input14(self):
        '''Immath 13: Test STOKES parameter, values: I, Q, yellow'''
        #######################################################################
        # Testing STOKES parameter, valid values: 'I'
        #    Tests are 'Q', 'yellow' (invalid) and 'I'
        #######################################################################
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "The STOKES parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
        
        results=None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], stokes='Q', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Bad stokes value, 'Q', was not reported."
                
        results=None
        # First remove the default results file
        try:
            shutil.rmtree( 'immath_results.im' )
        except:
            pass    
        try:
            results = immath( imageList[0], stokes='yellow', expr='IM0' )
        except:
            pass
        else:
            if ( results != None and  not ( isinstance(results, bool) ) \
                 or results==True ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Bad stokes value, 'yellow', was not reported."
    
        results = None
        try:
            results = immath( imageList[0], stokes='I', outfile='input_test17', expr='IM0' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Image math failed with stokes=I"
        if ( not os.path.exists( 'input_test17' ) or results==None or results==False ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: output file 'input_test17' was not created."
    
    
        self.assertTrue(retValue['success'],retValue['error_msgs'])    
    

class immath_test2(unittest.TestCase):   

    def setUp(self):
        if(os.path.exists(imageList2[0])):
            for img in imageList2:
                shutil.rmtree(img)
                
        # FIXME: add links to repository
        for img in imageList2:
            self.assertTrue(os.path.exists(datapath + img))
            if os.path.isdir(datapath + img):
                shutil.copytree(datapath + img, img)
            else:
                shutil.copy(datapath + img, img)


    def tearDown(self):
        for img in imageList2:
            if os.path.isdir(img):
                shutil.rmtree(img)
            else:
                os.remove(img)
        for img in glob.glob("pol_test*"):
            if os.path.isdir(img):
                shutil.rmtree(img)
            else:
                os.remove(img)
        # FIXME need to figure out how to close this table correctly
        me.done()
        cache_tables = tb.showcache()
        self.assertTrue(len(cache_tables) == 0)
                       
    def copy_img(self):
        '''Copy images to local disk'''
        for img in imageList3:
            os.system('cp -r ' +datapath + img +' ' + img)

    def rm_img(self):
        '''Remove images from disk'''
        for img in imageList3:
            os.system('rm -rf ' +img)
        
    ####################################################################
    # Testing various math expression.
    #
    # These tests use ngc5921 which is 256x256x1x46
    #
    # Returns a dictionary contain two keywords as follows:
    #        success:     set to True if all tests passed
    #        error_msgs:  detailed message(s) of any error(s) that occured.
    ####################################################################
    def test_expr1(self):
        '''Immath expr1: Testing various math expressions'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "Starting immath INPUT/OUTPUT tests.", 'NORMAL2' )            
        
        ################        TEST 1          ###################
        # Test to see if all the points of a single image are doubled.
        # Note that we a few values from the input and output images to see
        # if we truly doubled each value.
    
        casalog.post( "Testing the doubling of every value in an image.", 'NORMAL2' )
        results=None
        try:
            results=immath( imagename=imageList2[0], outfile='expr_test1', expr='IM0 * 2' );
        except Exception, e:
            casalog.post( "Exception occured doubling image ... "+str(e), 'DEBUG1')
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +'\nError: Failed to create "doubled image" on '+imageList2[0]
            
        if (  not os.path.exists( 'expr_test1' ) or results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: outfile 'expr_test1' was not created."
        else:
            myia = iatool()
            myia.open(imageList2[0])
            expected = 2*myia.getchunk()
            myia.done()
            myia.open("expr_test1")
            got = myia.getchunk()
            myia.done()
            self.assertTrue((expected == got).all(), "got not equal to expected for doubling pixel values")
            

        self.assertTrue(retValue['success'],retValue['error_msgs'])
                
    def _create_expr_test2(self):
        outimage='expr_test2'
        if (os.path.exists(outimage)):
            return True
        return immath(
            imagename=imageList2[0], outfile=outimage,
            expr='IM0', chans='5'
        );

    def test_expr2(self):
        '''Immath expr2: Test extraction of single plane'''
        ################        TEST 2          ###################
        # Test to see if we can grab a single plane out of the image.
        # Then use this single plane and add it to the whole image.
        # In this case we want the plane at channel 5.  NOTE: We can't
        # do this with LEL at the moment, but will be able to in the
        # future.  The commented out immath() calls use LEL
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "**** Testing extraction of a single plane.", 'NORMAL2' )
    
        # First test that we can extract a plane from the bigger image.
        # Checking the size of the resulting image and a few of the
        # points to make sure they are from the correct plane.
        outimage='expr_test2'

        results=None
        try:    
            results = self._create_expr_test2()
            #immath( outimage, 'evalexpr', str('"')+image1+str('"[INDEXIN(4,[5])]' ) );
        except Exception, e:
            casalog.post( "Exception occured getting image slice ... "+str(e), 'DEBUG1')        
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +'\nError: Failed to create image" slice on '+imageList2[0]
            
        if (  not os.path.exists( outimage ) or results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: outfile '"+outimage+"' was not created."
        else:
            # Verify the size of the output image, and check to make sure
            # the values are copied over correctly
            ia.open( outimage );
            size = ia.shape();
            ia.close()
            if ( len(size) < 4 or size[0]!=256 or size[1]!=256 or \
                    size[2]!=1 or size[3]!=1 ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                       +'\nError: Size of subimage is incorrect: '\
                       + size + ".\nExpected the resulting image to"\
                       + "be 256x256x1x1 pixels."
            else:
                myia = iatool()
                myia.open(imageList2[0])
                expected = myia.getchunk()[:,:,:,5:5]
                myia.done()
                myia.open(outimage)
                got = myia.getchunk()
                myia.done()
                self.assertTrue((got == expected).all()) 
                
        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    def test_expr3(self):
        '''Immath expr3: Add plane to an image'''
        # We know how to make a subimage so now lets actually add this plane
        # to the whole image.
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "Adding slice to a cube image.", 'NORMAL2' )
        outimage="expr_test3"
        self.assertTrue(os.path.exists(imageList2[0]))
        results = self._create_expr_test2()
        results = immath(
            mode='evalexpr', outfile=outimage,
            imagename=[ 'expr_test2', imageList2[0] ],
            expr='IM0+IM1'
        )
        # Now do some value checks
        # of these images and from that get their sizes with the topixel
        # function, however, we are assuming that its a 256x256x1x46 image.
        size=[]
        try:
            ia.open( outimage )
            size=ia.shape()
            ia.done()
        except Exception, e:
            casalog.post( "Exception occured getting image shape ... "+str(e), 'DEBUG1')
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Unable to get shape of image "+outimage
        
        self.assertTrue((size == [256, 256, 1, 46]).all())
        
        myia = iatool()
        myia.open("expr_test2")
        chunk1 = myia.getchunk()
        myia.done()
        myia.open(imageList2[0])
        chunk2 = myia.getchunk()
        myia.done()
        expected = chunk1 + chunk2
        myia.open(outimage)
        got = myia.getchunk()
        myia.done()
        self.assertTrue((got - expected < 0.000001 ).all())

        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    
    ####################################################################
    # Testing various the various polarization modes for data
    # correctness.
    #
    # The 3C129 test set has 4 polarizations and is great for
    # testing this one.
    #
    #
    # Returns a dictionary contain two keywords as follows:
    #        success:     set to True if all tests passed
    #        error_msgs:  detailed message(s) of any error(s) that occured.
    ####################################################################
    def test_pol(self):
        '''Immath pol: Testing various polarization modes for data correctness'''
        success = True
        errors = ''
    #    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "Starting immath INPUT/OUTPUT tests.", 'NORMAL2' )
        self.assertTrue(len(tb.showcache()) == 0)
        # First make the I, Q, U, and V files.  This step may not be
        # needed if immath learns to do this for the user.
        immath( imageList2[3], expr='IM0', stokes='I', outfile='pol_test_I.im' )

        immath( imageList2[3], expr='IM0', stokes='Q', outfile='pol_test_Q.im' )
        immath( imageList2[3], expr='IM0', stokes='U', outfile='pol_test_U.im' )
        immath( imageList2[3], expr='IM0', stokes='V', outfile='pol_test_V.im' )
        imList = ['pol_test_Q.im', 'pol_test_U.im', 'pol_test_V.im']
        myia = iatool()
        # total polarization intensity
        outfile = 'pol_test1'
        self.assertTrue(immath( imagename=imList, outfile=outfile, mode='poli' ))
        myia.open(outfile)
        self.assertTrue(myia.coordsys().stokes()[0] == 'Ptotal')
        myia.done()
        
        # linear polarization intensity
        outfile = 'linear_pol_intensity_test'
        self.assertTrue(immath(imagename=imList[0:2], outfile=outfile, mode='poli' ))
        myia.open(outfile)
        self.assertTrue(myia.coordsys().stokes()[0] == 'Plinear')
        myia.done()
        imList = ['pol_test_Q.im', 'pol_test_U.im']
        self.assertTrue(immath( imagename=imList, outfile='pol_test2', mode='pola' ))
 
    # verification of fix to CAS-1678
    # https://bugs.aoc.nrao.edu/browse/CAS-1678
    def test_many_images(self):
        '''Immath: verification of fix to CAS-1678'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "Starting immath many (>9) image test.", 'NORMAL2' )
    
        self.copy_img()
        imagename = [
            'immath0.im', 'immath1.im', 'immath2.im', 'immath3.im', 'immath4.im', 'immath5.im',
            'immath6.im', 'immath7.im', 'immath8.im', 'immath9.im','immath10.im'
        ]
        expr = 'IM0+IM1+IM2+IM3+IM4+IM5+IM6+IM7+IM8+IM9+IM10'
        myia = iatool()
        try:
            # full image test
            outfile = 'full_image_sum.im'
            if (immath(imagename=imagename, expr=expr, outfile=outfile)):
                expected = numpy.ndarray([2,2])
                expected.put(range(expected.size),66)
                myia.open(outfile)
                got = myia.getchunk()
                myia.done()
                if (not (got == expected).all()):
                    retValue['success'] = False
                    retValue['error_msgs'] += "\n Full image sum not correctly calculated"
            else:
                retValue['success'] = False
                retValue['error_msgs'] += "\nimmath returned False for full image sum"            
        except:
            retValue['success'] = False
            retValue['error_msgs'] += "\nFull image calculation threw an exception: " + str(sys.exc_info()[0])
    
        shutil.rmtree(outfile)        
        try:
            # subimage image test
            outfile = 'subimage_sum.im'
            if (immath(imagename=imagename, expr=expr, outfile=outfile, box='0,0,0,0')):
                expected = numpy.ndarray([1,1])
                expected.put(range(expected.size), 66)
                myia.open(outfile)
                got = myia.getchunk()
                myia.done()
                if (not (got == expected).all()):
                    retValue['success'] = False
                    retValue['error_msgs'] += "\n sub image sum not correctly calculated"
            else:
                retValue['success'] = False
                retValue['error_msgs'] += "\nimmath returned False for sub image sum: " + str(sys.exc_info()[0])
        except:
            retValue['success'] = False
            retValue['error_msgs'] += "\nSub image calculation threw an exception"

        self.rm_img()
        shutil.rmtree(outfile)
        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    
    # fixed setting trc brokenness with a kludge (as, given the state of the pre-existing code, was the best that could be done)
    def test_CAS_1910_fix(self):
        '''Immath: verification of fix to CAS-1910'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        try:
    
            if (
                not immath(
                    imagename=cas1910_im, outfile='cas-1910_out.im',
                    mode='evalexpr', expr='IM0*1',
                    box='708,1158,1006,1456'
                )
            ):
                retValue['success'] = False
                retValue['error_msgs'] += "\nimmath returned False for setting box correctly (CAS-1910)"
        except:
            retValue['success'] = False
            retValue['error_msgs'] += "\nimmath threw exception for setting box correctly (CAS-1910)"

        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    # cas-1452
    def test_one_function(self):
        '''Immath: verification of fix to CAS-1452'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        funcs = ['sin', 'SIN', 'Sin']
        imagename = cas1452_1_im
        ia.open(imagename)
        expected = numpy.sin(ia.getchunk())
        ia.close()
        #try:
        for f in funcs:
            try:
                shutil.rmtree('cas-1452_out_' + f + '.im')
            except:
                pass
            
            outfile = 'cas-1452_out_' + f + '.im'
            expr = f + "(\'" + imagename + "\')" 
            self.assertTrue(
                immath(
                    imagename=imagename, outfile=outfile,
                    mode='evalexpr', expr=expr
                )
            )
            ia.open(outfile)
            got = ia.getchunk()
            ia.close()
            maxdiff = numpy.abs(got - expected).max()
            self.assertTrue( maxdiff <= 1e-7)
            os.system('rm -rf ' + outfile)
    
        #except:
        #    retValue['success'] = False
        #    retValue['error_msgs'] += "\nimmath threw exception for " + f + " calculation"
        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    def test_CAS_1830_fix(self):
        '''Immath: verification of fix to CAS-1830'''
        test = "cas_1830_fix_test"
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        expected = [256, 256, 1, 1]
        i = 0
        for expr in ["IM0", "'" + cas1830_im + "'", '"' + cas1830_im + '"']:
            if (expr == "IM0"):
                imagenames = [cas1830_im]
            else:
                imagenames = [cas1830_im, ""]
            for imagename in imagenames:
                outfile = 'cas1830_out+' + str(i)
                if (imagename):
                    res = immath(imagename=imagename, expr=expr, chans='22', outfile=outfile)
                else:
                    res = immath(expr=expr, chans='22', outfile=outfile)
                self.assertTrue(res)
                myia = iatool()
                myia.open(outfile)
                self.assertTrue((myia.shape() == expected).all())
                myia.close()
                i += 1
        self.assertTrue(retValue['success'],retValue['error_msgs'])


class immath_test3(unittest.TestCase):

    def setUp(self):
            
        for img in imageList4:
            shutil.copytree(datapath + img, img)
    
    def tearDown(self):
        for img in imageList4:
            shutil.rmtree(img)     
        
        os.system('rm -rf pola*')
        os.system('rm -rf poli*')
        # FIXME need to figure out how to close this table correctly
        cache_tables = tb.showcache()
        if (len(cache_tables) > 0):
            for table in cache_tables:
                self.assertTrue(table.rfind("/IERSeop97") >= 0)
        

    def _comp(self, imagename, mode, outfile, expected, epsilon, polithresh=''):
        self.assertTrue(immath(imagename=imagename, outfile=outfile, mode=mode, polithresh=polithresh))
        self.assertTrue(os.path.exists(outfile))
        ia.open(outfile)
        got = ia.getchunk()
        ia.done()
        diff = expected - got
        if (epsilon == 0):
            #<debug>
            maxdiff = numpy.abs(numpy.max(got - expected))
            maxdiffrel = numpy.abs(numpy.max(diff/got))
            if (maxdiff > 0):
                print "maxdiff " + str(maxdiff)
                print "maxdiffrel " + str(maxdiffrel)

                
            self.assertTrue((diff == 0).all())     
        else:
            self.assertTrue((numpy.abs(diff)/got < epsilon).all())        

    def test_CAS2120(self):
        '''immath: verification of old functionality and similar new functionality introduced by CAS-2120'''

        sysbits = struct.calcsize("P") * 8
        # epsilon necessary because of differences between 32 and 64 bit images
        epsilon = 3e-7

        # POLA
        mode = 'pola'
        
        myia = iatool()

        myia.open(POLA_im)
        expected = myia.getchunk()
        myia.done()

        # pola the old way
        self._comp([Q_im, U_im], mode, 'pola_1.im', expected, epsilon)  

        # test that order of Q, U doesn't matter
        self._comp([U_im, Q_im], mode, 'pola_2.im', expected, epsilon)  
        outfile = 'pola_2.im'

        # test making pola image from multi-stokes image
        self._comp(IQUV_im, mode, 'pola_3.im', expected, epsilon)  

        # test making pola image from multi-stokes image without V
        self._comp(IQU_im, mode, 'pola_4.im', expected, epsilon)  

        # test making pola image from multi-stokes image without I and V
        self._comp(QU_im, mode, 'pola_5.im', expected, epsilon)  

        # no Q, this should fail
        outfile = 'pola6.im'
        try:
            immath(imagename=UV_im, outfile=outfile, mode=mode)
            # should not get here
            self.assertTrue(false)
        except:
            self.assertFalse(os.path.exists(outfile))

        # no U, this should fail
        outfile = 'pola7.im'
        try:
            immath(imagename=Q_im, outfile=outfile, mode=mode)
            # should not get here
            self.assertTrue(false)
        except:
            self.assertFalse(os.path.exists(outfile))

        # no U, this should fail
        outfile = 'pola7.im'
        try:
            immath(imagename=[Q_im, V_im], outfile=outfile, mode=mode)
            # should not get here
            self.assertTrue(false)
        except:
            self.assertFalse(os.path.exists(outfile))

        # with a linear polarization threshold applied
        outfile = 'pola_8.im'
        self._comp(QU_im, mode, outfile, expected, epsilon, polithresh='30uJy/beam')
        mask_tbl = outfile + os.sep + 'mask0'
        self.assertTrue(os.path.exists(mask_tbl))
        
        mytb = tbtool()
        mytb.open(thresh_mask)
        col = 'PagedArray'
        maskexp = mytb.getcell(col, 0)
        mytb.done()
        mytb.open(mask_tbl)
        maskgot = mytb.getcell(col, 0)
        mytb.close()

        self.assertTrue((maskgot == maskexp).all())

        # POLI
        mode = 'poli'
        myia.open(POLL_im)
        poll = myia.getchunk()
        myia.done()

        myia.open(POLT_im)
        polt = myia.getchunk()
        myia.done()

        # linear polarization the old way
        self._comp([Q_im, U_im], mode, 'poli_1.im', poll, epsilon)  

        # Q, U order shouldn't matter
        self._comp([Q_im, U_im], mode, 'poli_2.im', poll, epsilon)  

        # new way, one multi-stokes image
        self._comp(IQU_im, mode, 'poli_3.im', poll, epsilon)  

        # new way, one multi-stokes image with only Q and U
        self._comp(QU_im, mode, 'poli_4.im', poll, epsilon)  
       
        # poli for IQUV image will produce a total, not linear, polarization image 
        self._comp([Q_im, U_im, V_im], mode, 'poli_5.im', polt, epsilon)  
        self._comp(IQUV_im, mode, 'poli_6.im', polt, epsilon)

        # fail, no U
        outfile = 'poli7.im'
        try:
            immath(imagename=[Q_im, V_im], outfile=outfile, mode=mode)
            # should not get here
            self.assertTrue(false)
        except:
            self.assertFalse(os.path.exists(outfile))

        # fail, no Q
        outfile = 'poli8.im'
        try:
            immath(imagename=UV_im, outfile=outfile, mode=mode)
            # should not get here
            self.assertTrue(false)
        except:
            self.assertFalse(os.path.exists(outfile))


    def test_CAS2943(self):
        """Test the stretch parameter"""
        myia = iatool()
        myia.fromshape("myim.im", [10, 20, 4, 40])
        myia.done()
        myia.fromshape("mask1.im", [10, 20, 4, 40])
        myia.done()
        myia.fromshape("mask2.im", [10, 20, 1, 1])
        myia.done()
        myia.fromshape("mask3.im", [10, 20, 4, 2])
        myia.done()
        outfile = "out1.im"
        immath(
            imagename="myim.im", outfile=outfile, mode="evalexpr",
            expr="1*IM0", mask="mask1.im > 5", stretch=False
        )
        myia.open(outfile)
        self.assertTrue((myia.shape() == [10, 20, 4, 40]).all())
        myia.done()
        outfile = "out2.im"
        self.assertFalse(
            immath(
                imagename="myim.im", outfile=outfile, mode="evalexpr",
                expr="1*IM0", mask="mask2.im > 5", stretch=False
            )
        )
        outfile = "out3.im"
        immath(
            imagename="myim.im", outfile=outfile, mode="evalexpr",
            expr="1*IM0", mask="mask2.im > 5", stretch=True
        )
        myia.open(outfile)
        self.assertTrue((myia.shape() == [10, 20, 4, 40]).all())
        myia.done()
        outfile = "out4.im"
        self.assertFalse(
            immath(
                imagename="myim.im", outfile=outfile, mode="evalexpr",
                expr="1*IM0", mask="mask3.im > 5", stretch=False
            )
        )
        
    def test21(self):
        """Test moved from imagetest regreesion"""
        
        myia = iatool()
        # Define some arrays to be stored in the test images.
        # Define 2 local symbols to hold the results.
        # Delete the image files in case they are still present.
        #a1 = array(1:96, 8,12)
        #a2 = a1 + 100
        a1 = myia.makearray(0.0,[8,12])
        a2 = myia.makearray(0.0,[8,12])
        sum = myia.makearray(0.0,[8,12])
        num = 1
        for j in range(8):
            for k in range(12):
                a1[j][k] = num
                a2[j][k] = a1[j][k]+100
                sum[j][k] = a1[j][k]+a2[j][k]
                num = num+1
        names=""

        # Create 2 images.
        # Keep the first one in a global symbol (with a strange name),
        # so it can be used in a LEL expression with the $-notation.
        imname1 = 'im.1'
        global_iet_im1 = myia.newimagefromarray(imname1, a1)
        self.assertTrue(global_iet_im1)
        imname2 = 'im.2'
        im2 = myia.newimagefromarray(imname2, a2)
        self.assertTrue(im2)
        
        im2.done()
        
        # Form a simple expression and check if the result is correct.
        #global_iet_im1.done()
        ex = '"'+imname1+'" + "'+imname2+'"'
        outf = 'ex1.1'
        global_iet_ex1 = myia.imagecalc(outfile=outf, pixels=ex, overwrite=True)
        self.assertTrue(global_iet_ex1)
            
        pixels = global_iet_ex1.getchunk()
        self.assertTrue(abs(pixels - sum).all() < .0001)

        # Now form and check an expression using the $-notation.
        # The mask should be all true (in fact, there is no mask).
        #ex2 = imagecalc(pixels='$global_iet_ex1 - $global_iet_im1')
        #$-notation no longer works
        global_iet_ex1.done() #need to close before re-opening
        ex2 = myia.imagecalc(pixels='"'+outf+'" - "'+imname1+'"')
        self.assertTrue(ex2)
            
        pixels = ex2.getregion()
        mask = ex2.getregion(getmask=True)
        self.assertTrue(len(pixels) > 0 and len(mask) > 0)
        ok = ex2.done()
        self.assertTrue(abs(pixels - a2).all() < 0.0001)
            
        self.assertTrue(mask.all())
        

        # Define a region as a global symbol.
        # Use it in an expression using the $-notation.
        # The mask should be all true (in fact, there is no mask).
        #global_iet_reg1 = rg.quarter()
        global_iet_reg1 = rg.box(blc=[.25,.25],trc=[.75,.75], frac=True)
        #ex3 = imagecalc(pixels='$global_iet_im1[$global_iet_reg1]')
        ex3all = myia.imagecalc(pixels='"'+imname1+'"')
        self.assertTrue(ex3all)
           
        ex3 = ex3all.subimage(region=global_iet_reg1)
        self.assertTrue(ex3)
          
        ok = ex3all.done()
        pixels = ex3.getregion()
        mask = ex3.getregion(getmask=True)
        ## ex3.maskhandler ("get", names)
        ok = ex3.done()
        #if not alleq(pixels, a1[3:6,4:9], tolerance=0.0001):
        self.assertTrue(abs(pixels - a1[2:6,3:9]).all() < 0.0001)
        self.assertTrue(mask.all())
        
        ##    if (len(names) != 0) {
        ##      global_iet_im1.done()
        ##      fail ("imageexprtest: there is a mask in '$im1[$reg1]'")

        # Close the first image.
        # Note that after im4.done te image server might be closed because
        # there are no active objects anymore.
        imname4 = 'im.4'
        ex = '"'+imname1+'"['+'"'+imname1+'"%2==0]'
        im4 = myia.imagecalc (imname4, ex)
        self.assertTrue(im4)
        pixels = im4.getregion()
        mask = im4.getregion(getmask=True)
        ok = im4.maskhandler ("get",names)
        ok = im4.done()
        self.assertTrue(abs(pixels - a1).all() < 0.0001)

        a1mod2 = a1 % 2 == 0
        self.assertTrue((mask == a1mod2).all())
        
        self.assertTrue(len(names) == 0)
         
        # Move the image to test if its mask table can still be found
        # and if the default mask was set.
        imname5= 'im.5'
        myia.close()  # close imname4
        ok = os.renames(imname4, imname5)
        ex = '"'+imname5+'"'
        ex3 = myia.imagecalc(pixels=ex)
        self.assertTrue(ex3)
            
        pixels = ex3.getregion()
        mask = ex3.getregion(getmask=True)
        ok = ex3.done()
        self.assertTrue(abs(pixels - a1).all() <0.0001)
         
        self.assertTrue((mask==a1mod2).all())

        # Now issue some incorrect expressions.
        ex = '"'+imname1+'"+'+'"im..2"'
        self.assertRaises(Exception, myia.imagecalc, pixels=ex)
       

        ex = '"'+imname2+'" - max("'+imname1+'", "'+imname2+'", "'+imname1+'")'
        self.assertRaises(Exception, myia.imagecalc, pixels=ex)


        # Close last open image.
        # Remove the image files created.
        # Note that im.4 has been moved into im.1.
        ok = global_iet_im1.done()
      
    def test_complex(self):
        """Test creating and manipulating complex images"""
        myia = iatool()
        floatim = "float.im"
        myia.fromshape(floatim, [2,2], type='f')
        self.assertTrue(type(myia.getchunk()[0,0]) == numpy.float64)
        myia.done()
        complexim = "complex.im"
        zz = myia.imagecalc(complexim, "complex(" + floatim + ")")
        self.assertTrue(type(zz.getchunk()[0,0]) == numpy.complex128)
        complexim2 = "complex2.im"
        yy = zz.subimage(complexim2)
        zz.done()
        yy.done()
        zz = myia.imagecalc("", complexim + "+" + complexim2)
        self.assertTrue(type(zz.getchunk()[0,0]) == numpy.complex128)
        zz.done()
        
    def test_8(self):
        """Tests moved from imagetest regression, some are probably useless"""
        myia = iatool()
        imname = 'ia.fromarray1.image'
        imname2 = 'ia.fromshape2.image'
        imname3 = 'imagecalc.image'
        imshape = [10,20,5]
        data = make_data(imshape)
        
        self.assertRaises(
            Exception, myia.imagecalc, outfile=imname3,
            pixels='i_like_doggies'
        )
       
        myim = myia.newimagefromarray(outfile=imname, pixels=data)
        
        stats = myim.statistics(force=True, list=False)
        myim.done()
        #
        myim = myia.newimagefromshape(
            outfile=imname2,
            shape=[2*imshape[0], 2*imshape[1],
            2*imshape[2]]
        )
        myim.done()
        ex = imname + '+' + imname2
        self.assertRaises(
            Exception, myia.imagecalc, outfile=imname3, pixels=ex
        )
        
        # Need the double quotes because of / in expression
        ex = '"'+imname+'"'+'+'+'"'+imname+'"'
        myim = myia.imagecalc(outfile=imname3, pixels=ex)
        self.assertTrue(myim)
        stats2 = myim.statistics(force=True, list=False)
        self.assertTrue(
            stats2['max'] == 2*stats['max']
        )
        self.assertTrue(
            stats2['min'] == 2*stats['min']
        )
        myia.close()  # needed to remove table lock preventing myim.done
        myim.remove(done=True)
        
    def test_complex_calc(self):
        """Test ia.calc on complex images"""
        myia = iatool()
        shape = [4, 4]
        im1 = 'complex_calc1.im'
        im2 = 'complex_calc2.im'
        im3 = 'complex_calc3.im'
        im4 = 'float_calc4.im'

        myia.fromshape(im1, shape, type='c')
        myia.fromshape(im2, shape, type='c')
        myia.fromshape(im3, shape, type='c')

        myia.calc(im1 + '+' + im2)
        data = myia.getchunk()
        myia.done()
        self.assertTrue(type(data[0,0]) == numpy.complex128)

        myia.fromshape(im4, shape, type='f')
        self.assertRaises(Exception, myia.calc, im1 + '+' + im2)
        




def suite():
    return [immath_test1, immath_test2, immath_test3]
    
    
    
