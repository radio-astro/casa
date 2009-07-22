########################################################################3
#  imcontsub_test.py
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
# Test suite for the CASA imcontsub Task
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="imcontsub.py:description">imregion</linkto> 
#   <li> <linkto class="imregion.py:description">imregion</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# imcontsub_test stands for image momemnts test
# </etymology>
#
# <synopsis>
# imcontsub_test.py is a Python script that tests the correctness
# of the imcontsub task in CASA.
#
# The tests include:
#    1. Incorrect input for each paramter.  Incorrect input includes
#       one input of the incorrect data type, out-of-bounds (where
#       applicable, and correct data type but non-sensical.
#    2. A set of sample continuum subtractions with expected
#       output
#    3. Calculating one example for each allowed fitorder
#    4. Continuum subtraction with region selection on the sky,
#       channels, and stokes values, as well as using an input
#       region file.
#
# In the imcontsub task a specified continuum channel is subtracted
# from spectral line data.
#
# The expected input is a spectral line image. Both a continuum
# image and a continuum-subtracted spectral line are created as
# output, as long as the user has provided filenames for them.
#
# By default the continuum subtraction is applied to the whole
# image, however, if a region file is given and/or the user specifies
# a box range, channels, or stokes values then the subtraction is
# performed on this portion of the image only.
#
# </synopsis> 
#
# <example>
# # This test was designed to run in the automated CASA test system.
# # This exmple shows who to run it manually from with casapy.
# import os
# sys.path.append( os.environ["CASAPATH"].split()[0]+'/code/xmlcasa/scripts/regressions/admin' )
# import publish_summary
# publish_summary.runTest( 'imcontsub_test' )
#
# To test the script by itself, ie. not with test infrastructure.
# where WORKING_DIR is where you'll run the script
#       CASA_REPOS is where casa is installed
#
# cp CASA_REPOS/data/regression/g192redux/reference/* WORKING_DIR
# cp CASA_REPOS/code/xmmlcasa/tasts/tests/imcontsub_test.py WORKING_DIR
# casapy
# CASA <2>: cd WORKING_DIR
# CASA <3>: import imcontsub_test
# CASA <4>: imcontsub_test.run()
# </example>
#
# <motivation>
# To provide a test standard to the imcontsub task to try and ensure
# coding changes do not break the 
# </motivation>
#
# <todo>
# Almost everything!
#
# imcontsub doesn't return anything currently on success or failure.
# If this changes the tests will need to change to check for this.
# The basic design of the tests is based on the immoments test which
# returns an image tool upon success.
# </todo>

import random
import os
import shutil
import casac
from tasks import *
from taskinit import *

    
####################################################################
# Incorrect inputs to parameters.  The parameters are:
#    imagename
#    linefile
#    contfile
#    fitorder
#    region
#    box
#    chans
#    stokes
#
# Returns True if successful, and False if it has failed.
####################################################################
def input_test():
    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    casalog.post( "Starting imcontsub INPUT/OUTPUT tests.", 'NORMAL2' )

    # First step get rid of all the old test files!
    for file in os.listdir( '.' ):
        if file.startswith( 'input_test_' ):
            shutil.rmtree( file )
    if os.path.exists( 'garbage.rgn' ):
        os.remove('garbage.rgn')


    #######################################################################
    # Testing the imagename parameter.
    #    1. Bad file name should throw and exception
    #    2. Good file name, a file should be
    #######################################################################
    casalog.post( "The IMAGENAME parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    
    try:
        results = imcontsub( 'g192', contfile='input_test_cont1', linefile='input_test_line1' )
    except:
        no_op='noop'
    else:
        if ( results ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Badfile, 'g192', was not reported as missing."
        
    
    try:
        results = imcontsub( 'g192_a2.image', contfile='input_test_cont1', linefile='input_test_line1' )

    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Unable to do continuum subtraction on g192_a2.image"
        
    if ( ( not os.path.exists( 'input_test_cont1' ) \
         or not os.path.exists( 'input_test_line1' ) ) and results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: continuum files for input_test 1 were not created."


    #######################################################################
    # Testing the linefile parameter.
    #    1. Bad file, file already exists, exception should be thrown
    #    2. Good file name, a file should be
    #######################################################################
    casalog.post( "The LINEFILE parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    
    results = imcontsub( 'g192_a2.image', linefile='input_test_line1' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Badfile, 'input_test_line1', was not reported as already existing."
        
    
    try:
        results=imcontsub( 'g192_a2.image', linefile='input_test_line2' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to create spectral line file on g192"
    if ( not os.path.exists( 'input_test_line2' ) and results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Spectral line file, 'input_test_line2', was not created."


    #######################################################################
    # Testing the contfile parameter.
    #    1. Bad file, file already exists, exception should be thrown
    #    2. Good file name, a file should be
    #######################################################################
    casalog.post( "The CONTFILE parameter tests will cause errors to occur, do not be alarmed", 'WARN' )

    Results = False
    try:
        results = imcontsub( 'g192_a2.image', contfile='input_test_cont1' )
    except:
        no_op='noop'
    else:
        if ( results ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Badfile, 'input_test_cont1', was not "\
                 +"reported as already existing."        
    
    try:
        results=imcontsub( 'g192_a2.image', contfile='input_test_cont2' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to create continuum file for g192"
    if ( not os.path.exists( 'input_test_cont2' ) or not results ): 
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Continuum file, 'input_test_cont2', was not created."\
                   +"\nResults: "+str(results)
        
    
        
    #######################################################################
    # Testing FITORDER parameter, valid values 0 and greater
    #    1. Below valid range: -2, 2.3, and -10
    #    3. Within range: 0, 1, 2
    #######################################################################
    casalog.post( "The FITORDER parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    
    try:
        results=imcontsub( 'g192_a2.image', fitorder=-1, contfile='moment_test' )
    except:
        no_op='noop'
    else:
        if ( results ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: No exception thrown for bad fit order, -1"

        
    try:
        results=imcontsub( 'g192_a2.image', fitorder=2.3, contfile='moment_test' )
    except:
        no_op='noop'
    else:
        if ( results ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: No exception thrown for bad fit order, 2.3" 
             
        
    try:
        results=imcontsub( 'g192_a2.image', fitorder=-10, contfile='moment_test' )
    except:
        no_op='noop'
    else:
        if ( results ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: No exception thrown for bad fit order, -10"

    
        
    try:
        results=imcontsub( 'g192_a2.image', fitorder=0, contfile='input_test_cont3', linefile='input_test_line3' )
    except Exception, err:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Unable to subtract continuum with a fit order of 0 "
    if ( not os.path.exists( 'input_test_cont3' ) or not os.path.exists( 'input_test_line3' ) or not results ): 
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: continuum 3 output files were NOT created."\
                   + "\n\tRESULTS: "+str(results)
        

        
    try:
        results=imcontsub( 'g192_a2.image', fitorder=1, contfile='input_test_cont4', linefile='input_test_line4' )
    except Exception, err:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Unable to subtract continuum with a fit order of 1 "
    if ( not os.path.exists( 'input_test_cont4' ) or not os.path.exists( 'input_test_cont4' ) or not results ): 
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: continuum 4 output files were NOT created."

        
    try:
        results=imcontsub( 'g192_a2.image', fitorder=2, contfile='input_test_cont5', linefile='input_test_line5' )
    except Exception, err:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Unable to subtract continuum with a fit order of 2 "

    if ( not os.path.exists( 'input_test_cont5' ) or not os.path.exists( 'input_test_cont5' ) or not results ): 
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: continuum 5 output files were NOT created."        
        


    #######################################################################
    # Testing REGION parameter
    # Expects a file containing a region record, as created by the viewer.
    # Tests include bad file name, file with bad content, and good file.
    ####################################################################### 
    casalog.post( "The REGION parameter tests will cause errors to occur, do not be alarmed", 'WARN' )

    results = imcontsub( 'g192_a2.image', region=7 )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad region file, 7, was not reported as bad."



    try:
        results = imcontsub( 'g192_a2.image', region='garbage.rgn' )
    except:
        #We want this to fail
        no_op = 'noop'
    else:
        if ( results ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                                    +"\nError: Bad region file, 'garbage.rgn', was not reported as missing."

    try:
        filename = os.getcwd()+'garbage.rgn'
        fp=open( filename, 'w' )
        fp.writelines('This file does NOT contain a valid CASA region specification\n')
        fp.close()

        try:
            results = imcontsub( 'g192_a2.image', region=filename )
        except:
            no_op='noop'
        else:
            if ( results ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                          + "\nError: Bad region file, 'garbage.rgn',"\
                          + " was not reported as bad."
    except Exception, err:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Unable to create bad region file.\n\t"
        raise Exception, err

    
    try:
        results=imcontsub( 'g192_a2.image', region='g192_a2.image-2.rgn', linefile='input_test_line6' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to do continuum subtraction with region file g192_a2.image-2.rgn"
    if ( not os.path.exists( 'input_test_line6' ) or not results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Spetral line file, 'input_test_line6', was not created."



    #######################################################################
    # Testing BOX parameter
    # The input file has pixel values ranging from
    #   0-511, 0-511
    # Tests include -3, -1, 0, 1 random valid value, 500, 511, 525
    #   for both the x, and y coords
    #
    # Note: -1 is a special case implying use the full range, so to
    #       be out of bounds we need -2 or less.
    #######################################################################
    casalog.post( "The BOX parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    
    
    results = imcontsub( 'g192_a2.image', box='-3,0,511,511' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'x=-3', was not reported as missing."

    
    results = imcontsub( 'g192_a2.image', box='0,-3,511,511' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'y=-3', was not reported as missing."


    
    results = imcontsub( 'g192_a2.image', box='-2,0,511,511' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'x=-2', was not reported."\
             +"\n\tRESULTS: "+str(results)

    
    results = imcontsub( 'g192_a2.image', box='0,-2,511,511' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'y=-2', was not reported as missing."

    
    results = imcontsub( 'g192_a2.image', box='0,0,512,511' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'x=512', was not reported as missing."

    
    results = imcontsub( 'g192_a2.image', box='0,0,511,512' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'y=512', was not reported as missing."

    
    results = imcontsub( 'g192_a2.image', box='0, 0,525,511' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'x=525', was not reported as missing."

    
    results = imcontsub( 'g192_a2.image', box='0,0,511,525' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad box value, 'y=525', was not reported as missing."

    x1=random.randint(0,511)
    x2=random.randint(x1,511)
    y1=random.randint(0,511)
    y2=random.randint(y1,511)
    boxstr=str(x1)+','+str(y1)+','+str(x2)+','+str(y2)
    
    try:
        results = imcontsub( 'g192_a2.image', box=boxstr, linefile='input_test_line7' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to create moment with box="+boxstr
    if ( not os.path.exists( 'input_test_line7' ) or not results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Moment file, 'input_test_box_1', was not "\
                 +"created at "+boxstr


    #######################################################################
    # Testing CHANS parameter: valid values 0-39 for our image
    # Values used for testing, -5,-2,0,22~35, 39,40,45
    #
    # NOTE: a coord value of -1 indicates use all, so -1 is a valid
    #       coordiante.
    #######################################################################
    casalog.post( "The CHANS parameter tests will cause errors to occur, do not be alarmed", 'WARN' )
    
    
    results = imcontsub( 'g192_a2.image', chans='-5' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad channel value, '-5', was not reported."

    
    results = imcontsub( 'g192_a2.image', chans='-2' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad channel value, '-2', was not reported."\
             +"\n\tRESULTS: "+str(results)

    
    results = imcontsub( 'g192_a2.image', chans='-18' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad channel value of -18 was not reported."

    
    results = imcontsub( 'g192_a2.image', chans='45' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad channel value of 45 was not reported."

    
    results = imcontsub( 'g192_a2.image', chans='40' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad channel value of 40 was not reported."

    
    try:
        results = imcontsub( 'g192_a2.image', chans='22~35', linefile='input_test_line8' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to spectral line image with chans=22~35"
    if ( not os.path.exists( 'input_test_line8' ) or not results ): 
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Spectral line image file, 'input_test_line8', was not created."

    
    try:
        results = imcontsub( 'g192_a2.image', chans='0', linefile='input_test_line9' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to create moment wit chans=0"
    if ( not os.path.exists( 'input_test_line9' ) or not results ): 
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Spectral line file, 'input_test_line9', was not created."


    
    try:
        results = imcontsub( 'g192_a2.image', chans='39', linefile='input_test_line10' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to create moment with chans=39"
    if ( not os.path.exists( 'input_test_line10' ) or not results ): 
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Spectral line file, 'input_test_line10', was not created."

        
    #######################################################################
    # Testing STOKES parameter, valid values: 'I'
    #    Tests are 'Q', 'yellow' (invalid) and 'I'
    #######################################################################
    casalog.post( "The STOKES parameter tests will cause errors to occur, do not be alarmed", 'WARN' )

    
    results = imcontsub( 'g192_a2.image', stokes='Q' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad stokes value, 'Q', was not reported."

    
    results = imcontsub( 'g192_a2.image', stokes='yellow' )
    if ( results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
             +"\nError: Bad stokes value, 'yellow', was not reported."

    
    try:
        results = imcontsub( 'g192_a2.image', stokes='I', linefile='input_test_line11' )
    except:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Unable to create moment with stokes=Q"
    if ( not os.path.exists( 'input_test_line11' ) or not results ):
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Spectral line file, 'input_test_line11', was not created."

    #print "RETURNING", retValue
    return retValue


####################################################################
# Continuum subtraction correctness test.
#
# This test subtacts the continuum from the g192 data file
# and compares the results (both continuum and spectral line
# with subtracted continuum files) with pervious results.
#
# Random values are selected in the files and compared.
#
# Returns True if successful, and False if it has failed.
####################################################################

def continuum_test():
    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    casalog.post( "Starting imcontsub CONTINUUM SUB CORRECTNESS tests.", 'NORMAL2' )

    # First step get rid of all the old test files!
    for file in os.listdir( '.' ):
        if file.startswith( 'cont_test_' ):
            shutil.rmtree( file )

    
    try:
        results=imcontsub( 'g192_a2.image', fitorder=0, contfile='cont_test_cont1', linefile='cont_test_line1' )
    except Exception, err:
        retValue['success']=False
        retValue['error_msgs']=retValue['error_msgs']\
                 +"\nError: Unable to subtract continuum with a fit order of 0 "\
                 +"\n\t REULTS: "+str(results)
    else:
        if ( not os.path.exists( 'cont_test_cont1' ) or not os.path.exists( 'cont_test_line1' ) or not results ): 
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: continuum 3 output files were NOT created."
        else:
            # Now that we know something has been done lets check some values
            # with previously created files to see if the values are the same.
            # We randomly pick 50 points (almost 10%)
            for count in range(0,50):
                x = random.randint(0,511)
                y = random.randint(0,511)
                box=str(x)+','+str(y)+','+str(x)+','+str(y)
                chan = str(random.randint(0,39))

                line_prev_value={}
                line_cur_value={'empty':''}
                try: 
                    line_prev_value = imval( 'g192_a2.contfree', box=box, chans=chan, stokes='I' )
                    line_cur_value  = imval( 'cont_test_line1', box=box, chans=chan, stokes='I' )
                except:
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Unable to compare spectral line files."
                else:
                   #print "Spec line prev value: ", line_prev_value
                   #print "spec line current value: ", line_cur_value        
                   if ( line_prev_value != line_cur_value ):
                    retValue['success']    = False
                    retValue['error_msgs'] = '\nError: spectral line value differs with '\
                          + "previously calculated value at: "\
                          + "\t["+str(x)+','+str(y)+','+chan+',I].'\
                          + "\tvalues are "+str(line_prev_value)+" and "+str(line_cur_value)
                try:
                    cont_prev_value = imval( 'g192_a2.cont', box=box, chans=chan, stokes='I' )
                    cont_cur_value  = imval( 'cont_test_cont1', box=box, chans=chan, stokes='I' )
                except:
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                        +"\nError: Unable to compare continuum files."
                else:
                   #print "Continuum prev value: ", cont_prev_value
                   #print "Continuum current value: ", cont_cur_value        
                   if ( cont_prev_value != cont_cur_value ):
                    retValue['success']    = False
                    retValue['error_msgs'] = '\nError: continuum value differs with '\
                        + "previously calculated value at: "\
                        + "\t["+str(x)+','+str(y)+','+chan+',I].'
                        #+ "\tvalues are "+str(cont_prev_value)+" and "+str(cont_cur_value)

    return retValue


####################################################################
# Region selection correction test.
#
# This test selects a region for continuum subtraction. Checks
# are done to make sure only the data in the selected region
# changes.
#
# Returns True if successful, and False if it has failed.
####################################################################

def region_test():
    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    casalog.post( "Starting imcontsub REGION tests.", 'NORMAL2' )

    # First step get rid of all the old test files!
    for file in os.listdir( '.' ):
        if file.startswith( 'rgn_test_' ):
            shutil.rmtree( file )



    return retValue


####################################################################
# Fitorder tests correctness test.
#
# This test subtacts the continuum from the g192 data file
# for fitorder =1 and fitorder=2, note that fitorder=0 is
# tested in the continuum test and valid/invalid inputs to
# the fitorder paramter are tested in the input test.
#
# The results, image file contents, are compared with previously
# created image files. 50 values are compared in each of the
# continuum file and the spectral line file, about 10% of the image.
#
# Returns True if successful, and False if it has failed.
####################################################################

def fitorder_test():
    retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    casalog.post( "Starting imcontsub INPUT/OUTPUT tests.", 'NORMAL2' )

    # First step get rid of all the old test files!
    for file in os.listdir( '.' ):
        if file.startswith( 'fit_test_' ):
            shutil.rmtree( file )

    for order in [1,2]:
        contfile='fit_test_cont'+str(order)
        linefile='fit_test_line'+str(order)

        oldcontfile='g192_a2.cont.order'+str(order)
        oldlinefile='g192_a2.contfree.order'+str(order)
        try:
            results=imcontsub( 'g192_a2.image', fitorder=order, contfile=contfile, linefile=linefile )
        except Exception, err:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                +"\nError: Unable to subtract continuum with a fit order="+str(order)\
                +"\n\t REULTS: "+str(results)
        else:
            if ( not os.path.exists( contfile ) or not os.path.exists( linefile ) or not results ): 
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: output files were NOT created for fitorder="\
                   +str(order)+" test."
            else:
                # Now that we know something has been done lets check some values
                # with previously created files to see if the values are the same.
                # We randomly pick 50 points (almost 10%)
                for count in range(0,50):
                    x = random.randint(0,511)
                    y = random.randint(0,511)
                    box=str(x)+','+str(y)+','+str(x)+','+str(y)
                    chan = str(random.randint(0,39))

                    line_prev_value={}
                    line_cur_value={ 'empty':''}
                    try:
                        line_prev_value = imval( oldlinefile, box=box, chans=chan, stokes='I' )
                        line_cur_value  = imval( linefile, box=box, chans=chan, stokes='I' )
                    except:
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                          +"\nError: Unable to compare spectral line files."
                    else:
                        #print "Spec line prev value: ", line_prev_value
                        #print "spec line current value: ", line_cur_value
                        if ( line_prev_value != line_cur_value ):
                            retValue['success']    = False
                            retValue['error_msgs'] = '\nError: continuum value differs with '\
                               + "previously calculated value at: "\
                               + "\t["+str(x)+','+str(y)+','+chan+',I].'\
                               + "\tvalues are "+str(line_prev_value)+" and "+str(line_cur_value)
                    cont_prev_value={}
                    cont_cur_value={ 'empty':''}
                    try:
                        cont_prev_value = imval( oldcontfile, box=box, chans=chan, stokes='I' )
                        cont_cur_value  = imval( contfile, box=box, chans=chan, stokes='I' )
                    except:
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs']\
                          +"\nError: Unable to compare continuum files "\
                          +oldcontfile+"  and  "+contfile
                    else:
                      #print "Continuum prev value: ", cont_prev_value
                      #print "Continuum current value: ", cont_cur_value
                      if ( cont_prev_value != cont_cur_value ):
                          retValue['success']    = False
                          retValue['error_msgs'] = '\nError: continuum value differs with '\
                             + "previously calculated value at: "\
                             + "\t["+str(x)+','+str(y)+','+chan+',I].'
                             #+ "\tvalues are "+str(line_prev_value)+" and "+str(line_cur_value)

    return retValue


####################################################################
# Methods for the automated CASA testing
####################################################################

def description():
    return " Tests the correctness of the imcontsub task in CASA including:"\
           + "\n\n"\
           + "1. Incorrect input for each paramter.  Incorrect input includes"\
           + "   one input of the incorrect data type, out-of-bounds (where"\
           + "   applicable, and correct data type but non-sensical."\
           + "2. Doing a couple continuum subtractions checking the "\
           + ".  results with previous results. "\
           + "3. Doing continuum subtraction with fitorder from 1 to ?? "\
           + "   and verifying the results." \
           + "4. Doing continuum subtraiong with region selection on the sky,"\
           + "   channels, and stokes values, as well as using an input"\
           + "   region file."\
           + "\n\n"\
           + "Data used for this test includes: "\
           + "     1. g192_a2.image"

def data():
    return ['g192_a2.image', 'g192_a2.image-2.rgn', \
            'g192_a2.contfree', 'g192_a2.cont', \
            'g192_a2.contfree.order1', 'g192_a2.cont.order1', \
            'g192_a2.contfree.order2', 'g192_a2.cont.order2',
            ]


def doCopy():
    #print "\n\nIn IMCONTSUB doCopy()\n\n"
    return [1, 1, 1, 1, 1, 1, 1, 1]

def run():
    test_list = [ 'input_test()', 'continuum_test()', \
                  'region_test()', 'fitorder_test()' ]


    # This would be really, really, really, really nice to run in a loop
    # and use the eval() command to execute the various methods BUT for
    # some idiotic reason, which I do not know, when you do this. The
    # casapy function, update_params() can not be found and you get an
    # error msg but if you call each method directly then there is no issue.
    
    passed = True
    error_msgs = ""
    testResults=[]
    testResults.append( input_test() )
    testResults.append( continuum_test() )
    #testResults.append( region_test() )
    testResults.append( fitorder_test() )
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
        raise Exception, 'imcontsub test has failed!\n'+error_msgs
    
    return []
