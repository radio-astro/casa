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
#   <li> <linkto class="imregion.py:description">imregion</linkto> 
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
#
#
# </synopsis> 
#
# <example>
# # This test was designed to run in the automated CASA test system.
# # This example shows who to run it manually from with casapy.
# import os
# sys.path.append( os.environ["CASAPATH"].split()[0]+'/code/xmlcasa/scripts/regressions/admin' )
# import publish_summary
# publish_summary.runTest( 'immath_test' )
#
# To test the script by itself, ie. not with test infrastructure.
# where WORKING_DIR is where you'll run the script
#       CASA_REPOS is where casa is installed
#
# cp CASA_REPOS/????
# cp CASA_REPOS/code/xmmlcasa/tasts/tests/immath_test.py WORKING_DIR
# casapy
# CASA <2>: cd WORKING_DIR
# CASA <3>: import immath_test
# CASA <4>: immath_test.run()
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
from tasks import *
from taskinit import *

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
def input_test( imageList ):
    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    casalog.post( "Starting immath INPUT/OUTPUT tests.", 'NORMAL2' )

    #######################################################################
    # Test invalid paramter name provided.
    ######################################################################
    result = None
    try:
        results = immath( imageList[0], outfile='input_test0', invalid='invalid' )
    except:
        pass
    else:
        if ( results != None and \
             ( not isinstance(results, bool) or results==True ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Invalid paramter, 'invalid', was not reported."    
    

    #######################################################################
    # Testing the imagename parameter.
    #    1. Bad file name should throw and exception
    #    2. Good file name, a file should be
    #    3. List with bad file in it.
    #    4. List with good files in it.
    #######################################################################
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

    results = None
    try:
        results = immath( imageList[0], outfile={0: 'test', 1: 'magic'}, expr='IM0*1' )
    except:
        pass
    else:
        if ( ( results != None and not ( isinstance(results, bool) )
             or results==True ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                  +"\nError: Bad outfile was not reported."
            
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

    #######################################################################
    # Testing the mode parameter.
    #    TODO
    #    Valid modes are: evalexpr, spix, pola, poli
    #    Invalid ???
    #######################################################################



    

    #######################################################################
    # Testing REGION parameter
    # Expects a file containing a region record, as created by the viewer.
    # Tests include bad file name, file with bad content, and good file.
    ####################################################################### 
    casalog.post( "The REGION parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    # First remove the default results file
    try:
        shutil.rmtree( 'immath_results.im' )
    except:
        pass
    
    results=None
    try:
        results = immath( imageList[1], region=7 )
    except:
        pass
    else:
        if ( ( results != None and not isinstance(results, bool) )\
               or results==True ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Bad region file, 7, was not reported as bad."+str(results)


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
        filename = os.getcwd()+'/garbage.rgn'
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


    results = None
    print "ARG"
    print "immath( imagename="+imageList[1]+", expr='IM0', "\
                    +"region="+imageList[2]+", outfile='input_test12' )"
    try:
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


    # Find a random region in the file, legal one.
    x1=random.randint(0,254)
    x2=random.randint(x1,254)
    y1=random.randint(0,254)
    y2=random.randint(y1,254)
    boxstr=str(x1)+','+str(y1)+','+str(x2)+','+str(y2)
    
    try:
        results = None
        results = immath( imageList[0], box=boxstr, outfile='input_test13', expr='IM0' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to do image math in box="+boxstr
    if ( not os.path.exists( 'input_test13' ) or results==None ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: output file 'input_test13' was not "\
                 +"created at "+boxstr


    #######################################################################
    # Testing CHANS parameter: valid values 0-39 for our image
    # Values used for testing, -5,-2,0,22~35, 44,45,46
    #
    # NOTE: a coord value of -1 indicates use all, so -1 is a valid
    #       coordiante.
    #######################################################################
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

        
    #######################################################################
    # Testing STOKES parameter, valid values: 'I'
    #    Tests are 'Q', 'yellow' (invalid) and 'I'
    #######################################################################
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


    return retValue



####################################################################
# Testing various math expression.
#
# These tests use ngc5921 which is 256x256x1x46
#
# Returns a dictionary contain two keywords as follows:
#        success:     set to True if all tests passed
#        error_msgs:  detailed message(s) of any error(s) that occured.
####################################################################
def expr_test():
    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    casalog.post( "Starting immath INPUT/OUTPUT tests.", 'NORMAL2' )

    imageList=data()
    
    
    ################        TEST 1          ###################
    # Test to see if all the points of a single image are doubled.
    # Note that we a few values from the input and output images to see
    # if we truly doubled each value.

    casalog.post( "Testing the doubling of every value in an image.", 'NORMAL2' )
    results=None
    try:
        results=immath( imagename=imageList[0], outfile='expr_test1', expr='IM0 * 2' );
    except Exception, e:
        casalog.post( "Exception occured doubling image ... "+str(e), 'DEBUG1')
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +'\nError: Failed to create "doubled image" on '+imageList[0]
        
    if (  not os.path.exists( 'expr_test1' ) or results == None ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: outfile 'expr_test1' was not created."
    else:
        # Check all of the corners, note we really should get the coordsys
        # of these images and from that get their sizes with the topixel
        # function, however, we are assuming that its a 256x256x1x46 image.
        points = [ [0,0,0,0], [255,255,0,45], [0, 255, 0, 0], [0, 255, 0, 45],\
                   [255, 0, 0, 45], [255, 0, 0, 0], [127,127,0,21], \
                   [12,43,0,3] ];
        try: 
            for i in range( len(points) ):
                point=points[i]
            
                inValue = _getPixelValue( imageList[0], point )
                outValue = _getPixelValue( 'expr_test1', point )
        
                if ( inValue*2 != outValue ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                          +'\nError: Values have not been doubled in image '\
                          +imageList[0]\
                          +'\n       at position '+str(point)\
                          +'\n       The values are:'+str(inValue)\
                          +" and "+str(outValue)
        except Exception, e:
            casalog.post( "Exception occured evaluating doubled image ... "+str(e), 'DEBUG1')
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to get pixel values of doubled image."
            

    ################        TEST 2          ###################
    # Test to see if we can grab a single plane out of the image.
    # Then use this single plane and add it to the whole image.
    # In this case we want the plane at channel 5.  NOTE: We can't
    # do this with LEL at the moment, but will be able to in the
    # future.  The commented out immath() calls use LEL
    casalog.post( "**** Testing extraction of a single plane.", 'NORMAL2' )

    # First test that we can extract a plane from the bigger image.
    # Checking the size of the resulting image and a few of the
    # points to make sure they are from the correct plane.
    outimage='expr_test2'
    results=None
    try:    
        results=immath( imagename=imageList[0], outfile=outimage, \
        	         expr='IM0', chans='5',  );
        #immath( outimage, 'evalexpr', str('"')+image1+str('"[INDEXIN(4,[5])]' ) );
    except Exception, e:
        casalog.post( "Exception occured getting image slice ... "+str(e), 'DEBUG1')        
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +'\nError: Failed to create image" slice on '+imageList[0]
        
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
            inPoints = [ [0,0,0,5], [255,255,0,5], [127,127,0,5], [9,178,0,5] ];
            outPoints = [ [0,0,0,0], [255,255,0,0], [127,127,0,0], [9,178,0,0] ];
            try:
                for i in range( len(inPoints) ):
                    inValue  = _getPixelValue( imageList[0], inPoints[i] );
                    outValue = _getPixelValue( outimage, outPoints[i] );
        
                    if ( inValue != outValue ):
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                          +'\nError: Values have not been copied properly to '\
                          +'the image slice.'\
                          +"\n       at positions "\
                          +str(inPoints[i])+' = '+str(inValue)+"   and  "\
                          +str(outPoints[i])+' = '+str(outValue)
            except Exception, e:
                casalog.post( "Exception occured getting image slice values ... "+str(e), 'DEBUG1')
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to get pixel values of image slice."

    
    # We know how to make a subimage so now lets actually add this plane
    # to the whole image.
    casalog.post( "Adding slice to a cube image.", 'NORMAL2' )
    outimage="expr_test3"

    results=None
    try:
        results = immath( mode='evalexpr', outfile=outimage,  \
                imagename=[ 'expr_test2', imageList[0] ], \
                expr='IM0+IM1' )

        #expr=str('"')+image1+str('"' ) + str(" + " ) + str('"')+image1+str('"[INDEXIN(4,[5])]' );

    except Exception, e:
        casalog.post( "Exception occured getting image slice ... "+str(e), 'DEBUG1')
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
           +"\nError: Unable to add a image slice to an image cube."


    if (  not os.path.exists( outimage ) or results == None ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: outfile '"+outimage+"' was not created."
    else:
        # Now do some value checks
        # of these images and from that get their sizes with the topixel
        # function, however, we are assuming that its a 256x256x1x46 image.
        size=[]
        try:
            ia.open( outimage )
            size=ia.shape()
            ia.close()
        except Exception, e:
            casalog.post( "Expception occured getting image shape ... "+str(e), 'DEBUG1')
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Unable to get shape of image "+outimage
        

        if ( len(size) < 4 or ( size[0]!=256 or size[1]!=256 or \
                            size[2]!=1 or size[3]!=46 ) ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                  +"\nError: Size of output plane is incorrect: "+str(size)\
                  +"\n       Excepted a shape of 256x256x1x46"
        else: 
            # Note that the first 3 axes need to match was was used for the
            # inPoints list above in order to use the planeValues gathered above.
            try :
                points = [ [0,0,0,0], [255,255,0,0], [127,127,0,12], [9,178,0,33] ];
                slicePoints = [ [0,0,0,5], [255,255,0,5], [127,127,0,5], [9,178,0,5] ]
                
                for i in range( len(points) ):
                    point=points[i]
                    inValue    = _getPixelValue( imageList[0], point );
                    sliceValue = _getPixelValue( imageList[0], slicePoints[i] );
                    outValue   = _getPixelValue( outimage, point );
                    
                    # NOTE: There can be rounding errors so we don't expect it
                    #       to be exact.
                    if ( abs(inValue+sliceValue-outValue) > 0.000001 ):
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                          +'\nError: Values have not been doubled in image '\
                          +imageList[0]\
                          +"\n       at position "+str(point)\
                          +' are '+str(inValue)+"   and   " +str(outValue)
            except Exception, e:
                casalog.post( "Exception occured comparing image points ... "+str(e), 'DEBUG1')
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                                   +"\nError: Unable to get pixel values."

    return retValue


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
def pol_test( imageList ):
    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    casalog.post( "Starting immath INPUT/OUTPUT tests.", 'NORMAL2' )

    # First make the I, Q, U, and V files.  This step may not be
    # needed if immath learns to do this for the user.
    immath( imageList[3], expr='IM0', stokes='I', outfile='pol_test_I.im' )
    immath( imageList[3], expr='IM0', stokes='Q', outfile='pol_test_Q.im' )
    immath( imageList[3], expr='IM0', stokes='U', outfile='pol_test_U.im' )
    immath( imageList[3], expr='IM0', stokes='V', outfile='pol_test_V.im' )

    imList = ['pol_test_Q.im', 'pol_test_U.im', 'pol_test_V.im']
    results = None
    try:
        results = immath( imagename=imList, outfile='pol_test1', mode='poli' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Failed to create polarization intensity image."
    if ( not os.path.exists( 'pol_test1' ) or results == None ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: output file, 'pol_test1', was not created."

    imList = ['pol_test_Q.im', 'pol_test_U.im']
    results = None
    try:
        results = immath( imagename=imList, outfile='pol_test2', mode='pola' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Failed to create polarization angle image."
    if ( not os.path.exists( 'pol_test2' ) or results == None ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: output file, 'pol_test2', was not created."
        

    #imList = [ Need at least two images at different frequencies]
    #results = None
    #try:
    #    results = immath( imagenames=imList, outfile='pol_test3', mode='spix' )
    #except:
    #    retValue['success']=False
    #    retValue['error_msgs']=retValue['error_msgs']\
    #               +"\nError: Failed to create spectral index image."
    #if ( not os.path.exists( 'pol_test3' ) or results == None ):
    #    retValue['success']=False
    #    retValue['error_msgs']=retValue['error_msgs']\
    #               +"\nError: output file, 'pol_test3', was not created."
    
    return retValue
####################################################################
# Methods for the automated CASA testing
####################################################################

def description():
    return " Tests the correctness of the immath task in CASA including:"\
           + "\n\n"\
           +"\n1. Valid/invalid input"\
           +"\n2. Simple math on image(s) performed and verified"\
           +"\n3. Test polarization and spectral modes."

def data():
    # ATST3/Orion/orion_gbt.im                         300x300x1x1
    # ATST3/Orion/orion_vlamem.im                      300x300x1x1
    #
    # ATSTS3/NGC4826/n4826_bima.im                     256x256x1x30
    # ATSTS3/NGC4826/n4826_12mmom0.im                  32x32x1x1
    # ATSTS3/NGC4826/n4826_mom0.im                     256x256x1x1
    #
    # g192redux/reference/g192_a2.image                512x512x1x40
    #
    # secondNGC4826/reference/n4826_tjoint1.image      256x256x1x30
    # secondNGC4826/reference/n4826_tjoint2.image      256x256x1x30
    #
    # ngc1333/reference/ngc1333_regression/n1333_both.image  800x800x1x10
    #
    # ngc5921redux/reference/ngc5921_regression/ngc5921.clean.image  256x256x1x46
    #
    # has 4 polarizations!
    # 3C129/reference/3C129BC.clean.image              2048x2048x4x1
    
    return ['ngc5921.clean.image', 'n1333_both.image', 'n1333_both.image.rgn', '3C129BC.clean.image' ]


def doCopy():
    #print "\n\nIn IMMATH doCopy()\n\n"
    return [1, 1, 1 ]

def run():
    test_list = [ 'input_test()', 'math_test()', \
                  'region_test()' ]


    # This would be really, really, really, really nice to run in a loop
    # and use the eval() command to execute the various methods BUT for
    # some idiotic reason, which I do not know, when you do this. The
    # casapy function, update_params() can not be found and you get an
    # error msg but if you call each method directly then there is no issue.
    
    passed = True
    error_msgs = ""
    testResults=[]
    testResults.append( input_test( data() ) )
    testResults.append( expr_test() )
    testResults.append( pol_test( data() ) )    
    print "TEST RESULTS: ", testResults

    for results in testResults:
        #print "RESULTS: ", results
        #print "ERRORS: ", results['error_msgs']
        if ( not results['success'] ):
            passed = False
            error_msgs = error_msgs + "\n" + results['error_msgs']

    print "ERROR MES: ", error_msgs
    print "PASSED: ", passed
    if ( not passed ):
        casalog.post( error_msgs, 'EXCEPTION' )
        raise Exception, 'immath test has failed!\n'+error_msgs
    
    return []
