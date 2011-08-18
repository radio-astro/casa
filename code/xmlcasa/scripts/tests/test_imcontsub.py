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
# # This test was designed to run in the CASA unit test system.
# # This example shows who to run it manually from outside casapy.
# casapy -c runUnitTest test_imcontsub
#
# or
#
# # This example shows who to run it manually from with casapy.
# runUnitTest.main(['imcontsub_test'])
#
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

#import random
import os
import shutil
import casac
from tasks import *
from taskinit import *
import unittest

# Input files
list = ['g192_a2.image', 'g192_a2.image-2.rgn', 
        'g192_a2.contfree', 'g192_a2.cont', 
        'g192_a2.contfree.order1', 'g192_a2.cont.order1', 
        'g192_a2.contfree.order2', 'g192_a2.cont.order2',
        'boxychans_cont.im', 'boxychans_line.im']

# Tests the correctness of the imcontsub task in CASA including:
#   1. Incorrect input for each paramter.  Incorrect input includes
#      one input of the incorrect data type, out-of-bounds (where
#      applicable, and correct data type but non-sensical.
#   2. Doing a couple continuum subtractions checking the 
#      results with previous results. 
#   3. Doing continuum subtraction with fitorder from 1 to ?? 
#      and verifying the results.
#   4. Doing continuum subtraiong with region selection on the sky,
#      channels, and stokes values, as well as using an input
#      region file.
#   
#   Data used for this test includes: 
#        1. g192_a2.image

class imcontsub_test(unittest.TestCase):

    def setUp(self):
        if(os.path.exists(list[0])):
            for file in list:
                os.system('rm -rf ' +file)
        
        datapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/g192redux/reference/'
        for file in list:
            os.system('cp -r ' +datapath + file +' ' + file)


    def tearDown(self):
        for file in list:
            os.system('rm -rf ' +file)
            os.system('rm -rf cont_*')
            os.system('rm -rf input_test*')
            os.system('rm -rf fit_test*')
            os.system('rm -rf line_*')
            
            if os.path.exists( 'garbage.rgn' ):
                os.remove('garbage.rgn')
        
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
    
    def test_bad_imagename(self):
        """Test bad image name throws exception"""
        
        exception = False
        try:
            imcontsub( 'g192', contfile='input_test_cont1', linefile='input_test_line1' )
            exception = True
        except:
            exception = False
        self.assertTrue(exception)
        
    def test_bad_linefile(self):
        """ Test bad line file name fails"""
        
        # FIXME shouldn't a bad linefile name throw an exception?
        filename = 'input_test_line1'
        myfile = open(filename, mode="w")
        myfile.write("x")
        myfile.close()
        results = imcontsub( 'g192_a2.image', linefile=filename )
        self.assertTrue(not results)
            
    def test_bad_contfile(self):
        """ Test bad continuum file name fails"""
        
        # FIXME shouldn't a bad linefile name throw an exception?
        filename = 'input_test_cont'
        myfile = open(filename, mode="w")
        myfile.write("x")
        myfile.close()
        results = imcontsub( 'g192_a2.image', contfile=filename )
        self.assertTrue(not results)
        
    def test_bad_fitorder(self):
        """Test bad fitorder fails"""
            
        results=imcontsub( 'g192_a2.image', fitorder=-1, contfile='moment_test' )
        self.assertTrue(not results)
        
        results=imcontsub( 'g192_a2.image', fitorder=-2.3, contfile='moment_test' )
        self.assertTrue(not results)
        
    def test_bad_region(self):
        """ Test bad region parameter fails"""
        
        results = imcontsub( 'g192_a2.image', region=7 )
        self.assertTrue(not results)
        
        filename = os.getcwd()+'/garbage.rgn'

        results = imcontsub( 'g192_a2.image', region=filename)
        self.assertTrue(not results)
        
        fp=open( filename, 'w' )
        fp.writelines('This file does NOT contain a valid CASA region specification\n')
        fp.close()
        results = imcontsub( 'g192_a2.image', region=filename)
        self.assertTrue(not results)


    def test_bad_chans(self):
        """Test bad chans parameter causes failure"""
        
        results = imcontsub( 'g192_a2.image', chans='-5' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', chans='-2' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', chans='-18' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', chans='45' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', chans='40' )
        self.assertTrue(not results)


    def test_bad_box(self):
        """Test bad box parameter causes failure"""
        
        results = imcontsub( 'g192_a2.image', box='-3,0,511,511' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', box='0,-3,511,511' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', box='-2,0,511,511' )
        self.assertTrue(not results)
        
        results = imcontsub( 'g192_a2.image', box='0,-2,511,511' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', box='0,0,512,511' )
        self.assertTrue(not results)
        
        results = imcontsub( 'g192_a2.image', box='0,0,511,512' )
        self.assertTrue(not results)
        
        results = imcontsub( 'g192_a2.image', box='0, 0,525,511' )
        self.assertTrue(not results)
        
        results = imcontsub( 'g192_a2.image', box='0,0,511,525' )
        self.assertTrue(not results)

    def test_bad_stokes(self):
        """Test bad stokes parameter causes failure"""
        results = imcontsub( 'g192_a2.image', stokes='Q' )
        self.assertTrue(not results)

        results = imcontsub( 'g192_a2.image', stokes='yellow' )
        self.assertTrue(not results)


    
    ## ####################################################################
    ## # Continuum subtraction correctness test.
    ## #
    ## # This test subtacts the continuum from the g192 data file
    ## # and compares the results (both continuum and spectral line
    ## # with subtracted continuum files) with pervious results.
    ## #
    ## # Random values are selected in the files and compared.
    ## # FIXME compare the entire arrays, not bloody random values
    ## # FIXED - replacing it with the equivalent but better test_fitorder(0)
    ## # sped the suite up from 1847s to 1194s on faraday.cv.nrao.edu.
    ## #
    ## # Returns True if successful, and False if it has failed.
    ## ####################################################################
    
    ## def test_continuum(self):
    ##     '''Imcontsub: Testing continuum sub correctness'''
    ##     retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
    ##     casalog.post( "Starting imcontsub CONTINUUM SUB CORRECTNESS tests.", 'NORMAL2' )
    
    ##     try:
    ##         results=imcontsub( 'g192_a2.image', fitorder=0, contfile='cont_test_cont1', linefile='cont_test_line1' )
    ##     except Exception, err:
    ##         retValue['success']=False
    ##         retValue['error_msgs']=retValue['error_msgs']\
    ##                  +"\nError: Unable to subtract continuum with a fit order of 0 "\
    ##                  +"\n\t REULTS: "+str(results)
    ##     else:
    ##         if ( not os.path.exists( 'cont_test_cont1' ) or not os.path.exists( 'cont_test_line1' ) or not results ): 
    ##             retValue['success']=False
    ##             retValue['error_msgs']=retValue['error_msgs']\
    ##                    +"\nError: continuum 3 output files were NOT created."
    ##         else:
    ##             # Now that we know something has been done lets check some values
    ##             # with previously created files to see if the values are the same.
    ##             # We randomly pick 50 points (almost 10%)
    ##             # FIXME lovely, yes let's pick random values to make sure any failures 
    ##             # cannot easily be reproduced, ugh
    ##             for count in range(0,50):
    ##                 x = random.randint(0,511)
    ##                 y = random.randint(0,511)
    ##                 box=str(x)+','+str(y)+','+str(x)+','+str(y)
    ##                 chan = str(random.randint(0,39))
    
    ##                 line_prev_value={}
    ##                 line_cur_value={'empty':''}
    ##                 try: 
    ##                     line_prev_value = imval( 'g192_a2.contfree', box=box, chans=chan, stokes='I' )
    ##                     line_cur_value  = imval( 'cont_test_line1', box=box, chans=chan, stokes='I' )
    ##                 except:
    ##                     retValue['success']=False
    ##                     retValue['error_msgs']=retValue['error_msgs']\
    ##                         +"\nError: Unable to compare spectral line files."
    ##                 else:
    ##                    #print "Spec line prev value: ", line_prev_value
    ##                    #print "spec line current value: ", line_cur_value  
    ##                    casalog.post( "*** line_prev_value " + str(line_prev_value), 'WARN')
    ##                    casalog.post( "*** line_cur_value " + str(line_cur_value), 'WARN')      
    ##                    if ( (line_prev_value['data'] != line_cur_value['data']).any() ):
    ##                     retValue['success']    = False
    ##                     retValue['error_msgs'] = '\nError: spectral line value differs with '\
    ##                           + "previously calculated value at: "\
    ##                           + "\t["+str(x)+','+str(y)+','+chan+',I].'\
    ##                           + "\tvalues are "+str(line_prev_value)+" and "+str(line_cur_value)
    ##                 try:
    ##                     cont_prev_value = imval( 'g192_a2.cont', box=box, chans=chan, stokes='I' )
    ##                     cont_cur_value  = imval( 'cont_test_cont1', box=box, chans=chan, stokes='I' )
    ##                 except:
    ##                     retValue['success']=False
    ##                     retValue['error_msgs']=retValue['error_msgs']\
    ##                         +"\nError: Unable to compare continuum files."
    ##                 else:
    ##                    #print "Continuum prev value: ", cont_prev_value
    ##                    #print "Continuum current value: ", cont_cur_value        
    ##                    if ( (cont_prev_value['data'] != cont_cur_value['data']).any() ):
    ##                     retValue['success']    = False
    ##                     retValue['error_msgs'] = '\nError: continuum value differs with '\
    ##                         + "previously calculated value at: "\
    ##                         + "\t["+str(x)+','+str(y)+','+chan+',I].'
    ##                         #+ "\tvalues are "+str(cont_prev_value)+" and "+str(cont_cur_value)
    
    ##     self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    
    ####################################################################
    # Region selection correction test.
    #
    # This test selects a region for continuum subtraction. Checks
    # are done to make sure only the data in the selected region
    # changes.
    #
    # Returns True if successful, and False if it has failed.
    ####################################################################
    
#    def test_region(self):
#        '''Imcontsub: Testing region parameters'''
#        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
#        casalog.post( "Starting imcontsub REGION tests.", 'NORMAL2' )
#    
#        # First step get rid of all the old test files!
#        for file in os.listdir( '.' ):
#            if file.startswith( 'rgn_test_' ):
#                shutil.rmtree( file )
#    
#    
#    
#        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    
    ####################################################################
    # Fitorder tests correctness test.
    #
    # This test subtacts the continuum from the g192 data file
    # for fitorder =1 and fitorder=2, note that fitorder=0 is
    # tested in the continuum test and valid/invalid inputs to
    # the fitorder paramter are tested in the input test.
    #
    # The results, image file contents, are compared with previously
    # created image files.
    #
    # Returns True if successful, and False if it has failed.
    ####################################################################
    
    def test_fitorder(self):
        '''Imcontsub: testing fitorder correctness'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        casalog.post( "Starting imcontsub INPUT/OUTPUT tests.", 'NORMAL2' )
    
        # First step get rid of all the old test files!
        for file in os.listdir( '.' ):
            if file.startswith( 'fit_test_' ):
                shutil.rmtree( file )
    
        # TODO I'm just fixing the tests here to do something more reasonable since they were broken
        # and in bad need of rewriting (see the diff with the previous version before I made these
        # edits). This whole test file probably needs to be reviewed but of course time is tight so
        # I cannot do that now.
    
        for order in xrange(2):
            contfile='fit_test_cont'+str(order)
            linefile='fit_test_line'+str(order)

            if order > 0:
                oldcontfile='g192_a2.cont.order'+str(order)
                oldlinefile='g192_a2.contfree.order'+str(order)
            else:
                oldcontfile = 'g192_a2.cont'
                oldlinefile = 'g192_a2.contfree'
                
            try:
                results = imcontsub('g192_a2.image', fitorder=order, contfile=contfile, linefile=linefile)
            except Exception, err:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                    +"\nError: Unable to subtract continuum with a fit order="+str(order)\
                    +"\n\t REULTS: "+str(results)
            else:
                if os.path.isdir(contfile) and os.path.isdir(linefile) and results:
                    retValue = self.cmp_images(linefile, oldlinefile,
                                               "Order " + str(order) + " line image",
                                               retValue)
                    retValue = self.cmp_images(contfile, oldcontfile,
                                               "Order " + str(order) + " continuum image",
                                               retValue)
                else:  
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: output files were NOT created for fitorder="\
                       +str(order)+" test."

        self.assertTrue(retValue['success'],retValue['error_msgs'])

    def cmp_images(self, newimg, oldimg, errmsg, retval, tol=1.0e-8):
        """
        Check that newimg matches oldimg to within tol.
        errmsg is a descriptive label in case there is an error.
        retval is updated with the results and returned.
        """
        try:
            subtract_expr = '(\"' + newimg + '\"-\"' + oldimg + '\")'
            output_image = newimg + '.diff'
            immath(mode='evalexpr', expr=subtract_expr, outfile=output_image)
            ia.open(output_image)
            stats = ia.statistics()
            ia.close()
            absmax = max(abs(stats['min']), abs(stats['max']))
            # in an infinite precision utopia, the difference image would be 0, but
            # alas, we do not live in such a world yet.
            if (absmax > tol):
                retval['success'] = False
                retval['error_msgs'] += errmsg + ' error: Max residual is ' + str(absmax)
        except Exception, err:
            retval['success'] = False
            retval['error_msgs'] += errmsg + " error: Exception thrown: " + str(err)
        return retval
        

    def test_box_and_chans(self):
        '''Imcontsub: Testing box and chans'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        #casalog.post("Starting imcontsub box and chans tests.", 'NORMAL2')

        oldcfil = 'boxychans_cont.im'
        cfil = 'test_' + oldcfil
        oldlfil = 'boxychans_line.im'
        lfil = 'test_' + oldlfil
        bx   = [400, 420, 490, 470]
    
        try:
            results = imcontsub('g192_a2.image', fitorder=0, contfile=cfil,
                                linefile=lfil, box=bx,
                                chans='32~37')  # Purposely one-sided.  
        except Exception, err:
            retValue['success']=False
            retValue['error_msgs'] += "\nError: Unable to subtract continuum with box and chans "\
                                      +"\n\t RESULTS: "+str(results)
        else:
            if os.path.isdir(cfil) and os.path.isdir(lfil) and results:
                retValue = self.cmp_images(lfil, oldlfil,
                                           "box and chans line image", retValue)
                retValue = self.cmp_images(cfil, oldcfil,
                                           "box and chans continuum image",
                                           retValue)
            else:
                retValue['success']=False
                retValue['error_msgs'] += "\nError: box and chans output files were NOT created."

        self.assertTrue(retValue['success'],retValue['error_msgs'])
        
def suite():
    return [imcontsub_test]

