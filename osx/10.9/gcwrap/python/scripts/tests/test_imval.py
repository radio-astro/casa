###########################################################################
# imval_test.py
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
# Test suite for the CASA imval Task
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="imval.py:description">imval</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# imval_test stands for image value test
# </etymology>
#
# <synopsis>
# imval_test.py is a Python script that tests the correctness
# of the imval task in CASA.
#
# Regression Test for the imval task.
#
# input/output tests.  Valid and invalid inputs are given
#         for the following paramaters:
#           a) No imagename 
#           b) Bad imagename given
#           c) Incorrect data type, not a string, to imagename parameter
#           d) Out of range errors for, box, chans, & stokes parameters
#           e) incorrect data type to box, chans, & stokes parameters
#           f) Bad file name to region parameter
#           g) Incorrect data type, not a string, to region parameter
#           h) File name that does not contain a region to the region param.
# Value at a single point tests.
#           a) Value at bottom-left corner
#           b) Value at bottom-right corner
#           c) Value at top-left corner
#           d) Value at top-right corner
#           e) Value at 3 points within the image.
# An array of values
#           a) A slice of the directional plane
#           b) Two slices of the directional plane
#           c) A cube RA,Dec,and Spectral axes
#           d) Two cubes RA,Dec,and Spectral axes
#           e) A 4D blob: RA,Dec, Spetral, & Stokes.
# </synopsis> 
#
# <example>
# # This test was designed to run in the automated CASA test system.
# # This example shows who to run it manually from within casapy.
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
# To provide a test standard to the imval task to try and ensure
# coding changes do not break the task.
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
#----------------------------

###########################################################################
import time
import os
import shutil
import glob
import numpy
import casac
from tasks import *
from taskinit import *
import unittest


# Input files
image_file = 'n4826_bima.im'
good_rgn_file   =  'n4826_bima_test.rgn'


###########################################################################
# NAME: info
#
# SHORT DESCRIPTION: Display information.
#
# DESCRIPTION: Write information to the local logger.
#
############################################################################

def info(message):
    #note(message,origin='regionmgrtest')
    print message
    casalog.postLocally(message, priority="NORMAL", origin='regionmgrtest')


###########################################################################
# NAME: note
#
# SHORT DESCRIPTION: Display information.
#
# DESCRIPTION: Write information to the local logger with the given priority
#
############################################################################

def note(message, priority="NORMAL", origin="imval_test"):
    print message
    casalog.postLocally(message, priority, origin)


###########################################################################
# NAME: input_test
#
# SHORT DESCRIPTION: Make sure invalid input is detected.
#
# DESCRIPTION: Test input that is the wrong type, to no values given.
#              We expect lots of exceptions for this test!
#              More precisely
#           a) No imagename given
#           b) Bad imagename given
#           c) Incorrect data type, not a string, to imagename parameter
#           d) Out of range errors for, box, chans, & stokes parameters
#           e) incorrect data type to box, chans, & stokes parameters
#           f) Bad file name to region parameter
#           g) Incorrect data type, not a string, to region parameter
#           h) File name that does not contain a region to the region param.
#
############################################################################

class imval_test(unittest.TestCase):
    
    def setUp(self):
        if (os.path.exists(image_file)):
            os.system('rm -rf ' +image_file+ ' ' +good_rgn_file)
            
        datapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/ATST3/NGC4826/'
        os.system('cp -r ' +datapath + image_file +' ' + image_file)
        os.system('cp -r ' +datapath + good_rgn_file +' ' + good_rgn_file)

    def tearDown(self):
            os.system('rm -rf ' +image_file+ ' ' +good_rgn_file)
        
    def test_input(self):
        '''Imval: Input/output tests'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        note( "Starting imval INPUT/OUTPUT tests.", 'NORMAL2' )
    
        ###########################################################
        # Image name tests
        info( 'Performing input/output tests on imagename, errors WILL occur.' )
        results=None
        try:
            results = imval( imagename='' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Empty imagename parameter not detected."
    
        results = None
        try:
            results = imval( imagename=good_rgn_file )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                              +"\nError: Invalid image file name not detected."
    
        results=None
        try:
            results = imval( imagename=2.3 )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: Invalid image file name, 2.3,  not detected."
    
        results=None
        try:
            results = imval( imagename='n4826_bima.im' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                         +"\nError: imval failed with valid file name, n4826_bima.im."
        if ( results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Valid imagename, n4826_bima.im, test failed."
        del results        
            
        ###################################################################
        # Testing out of range errors.
        # BLC=0,0,0,0  and TRC= 255,255,0,29   for n4826_bima.im
        info( 'Performing input/output tests on "box", errors WILL occur.' )
        results=None
        try:
            results = imval( imagename=image_file, box='-3,0,-3,3' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                             +'\nInvalid box parameter, x=-3, values not detected.'
            
        results=None
        try:
            results = imval( imagename=image_file, box='200,0,262,3' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +'Invalid box parameter values,262, not detected.'
    
        results=None
        try:
            results = imval( imagename=image_file, box='0,-3,0,3' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                       + 'Invalid box parameter value, y=-3, not detected.'
    
        results=None
        try:
            results = imval( imagename=image_file, box='0,270,0,3' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                          + 'Invalid box parameter value, y=270, not detected.'
    
        results=None
        try:
            results = imval( imagename=image_file, box='0,110,0,10' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                          + 'Invalid box parameter value, y[1]>y[0], not detected.'
    
        results=None
        try:
            results = imval( imagename=image_file, box="1,2,3,4" )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs'] \
                       +'Valid box parameter values caused an error.'
    
        if ( results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Valid box test, box=[1,2,3,4], failed."
        del results        
    
        ##############################################################
        # CHANS parameter testing
        info( 'Performing input/output tests on "chans", errors WILL occur.' )
        result=None
        try:
            results = imval( imagename=image_file, chans='-3' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs'] \
                               +'Invalid chans parameter value,-3, not detected.'
    
        resutls=None
        try:
            results = imval( imagename=image_file, chans='50' )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                        retValue['success']=False
                        retValue['error_msgs']=retValue['error_msgs'] \
                            +'Invalid chans parameter value,50, not detected.'
    
        results=None
        try:
            results = imval( imagename=image_file, chans="10" )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs'] \
                  +'Valid chans parameter value caused an error.'
    
        if ( results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Valid channel test, chans='10', failed."
        del results
    
    
        ###############################################################
        # STOKES parameter testing
        info( 'Performing input/output tests on "stokes", errors WILL occur.' )
        results=None
        try:
            results = imval( imagename=image_file, stokes='Q' )        
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs'] \
                           +'Invalid stokes value, Q,  not detected.'
    
        results=None
        try:
            results = imval( imagename=image_file, stokes=0 )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs'] \
                           +'Invalid stokes value, 0,  not detected.'
        results=None
        try:
            results = imval( imagename=image_file, stokes='I' )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs'] \
                           +'Valid stokes value, I, caused errors.'
    
        if ( results == None ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Valid stokes, 'I', test failed on file ."\
                     +image_file+"\nRESULTS: "+str(results)
        del results
            
        ########################################
        # REGION parameter tests
        info( 'Performing input/output tests on "region", errors WILL occur.' )
        results=None
        try:
            results = imval( imagename=image_file, region=[1,3] )
        except:
            pass
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                   +"\nError: Bad region, '[1, 3]', was not reported."
                
                   
        # First make sure the region file does not exist.
        garbage_rgn_file = os.getcwd()+'/garbage.rgn'
        if ( os.path.exists( garbage_rgn_file )):
            os.remove( garbage_rgn_file )
        
        try:
            results = imval( imagename=image_file, \
                             region=garbage_rgn_file )
        except:
            #We want this to fail
            no_op = 'noop'
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                       + "\nError: Bad region file, 'garbage.rgn', was NOT "\
                       + "reported as missing."
                       
        try:
            rgn_file = os.getcwd()+'garbage.rgn'
            fp=open( rgn_file, 'w' )
            fp.writelines('This file does NOT contain a valid CASA region specification\n')
            fp.close()
        except Exception, err:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Unable to create bad region file.\n\t"
            raise Exception, err
    
            
    
        try:
            results = imval( imagename=image_file, region=rgn_file )
        except:
            no_op='noop'
        else:
            if ( results!=None \
                 and ( (isinstance(results,bool) and results==True )\
                 or (isinstance(results,dict) and results!={} ) ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                              + "\nError: Bad region file, 'garbage.rgn',"\
                              + " was not reported as bad."
    
        
        results=None
        try:
            results=imval( imagename=image_file, region=good_rgn_file )
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                       +"\nError: Unable to get image values in region "\
                       +" specified by file, "+good_rgn_file
        if ( results == None or results==False ):
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Valid region file, "+good_rgn_file\
                     +" tset has failed."\
                     +"\nRESULTS: "+str(results)
        del results
                
        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    ###########################################################################
    # NAME: single_point
    #
    # SHORT DESCRIPTION: Do tests to find the value at a single point
    #
    # DESCRIPTION:
    #           a) Value at bottom-left corner
    #           b) Value at bottom-right corner
    #           c) Value at top-left corner
    #           d) Value at top-right corner
    #           e) Value at 3 points within the image.
    #
    ############################################################################
    
    def test_single_point(self):
        '''Imval: Single point tests'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        note( "Starting SINGLE POINT tests.", 'NORMAL2' )
    
        # Find the min/max points of the image.
        bbox={}
        try: 
            ia.open( image_file )
            bbox=ia.boundingbox()
            ia.done()
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Unable to find size of image "+image_name
    
        dir_blc=[]
        dir_trc=[]
        min_chan=max_chan=-1
        min_stokes=max_stokes=-1    
        if ( len(bbox) > 0 and bbox.has_key('blc') and bbox.has_key('trc') ):
            blc=bbox['blc']
            trc=bbox['trc']
    
            dir_blc=[blc[0], blc[1]]
            dir_trc=[trc[0], trc[1]]
            min_chan=blc[3]
            max_chan=trc[3]
            min_Stokes=blc[2]
            max_stokes=trc[2]
    
        error_margin=0.00001
    
        
        #############################################################
        # Bottom-left
        tbox=str(dir_blc[0])+','+str(dir_blc[1])+','+str(dir_blc[0])+','\
              +str(dir_blc[1])
        msg="Bottom left corner value was Not Found"
        results=None
        try:
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan), \
                           stokes=str(min_stokes) )
        except Exception:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to get the value in the bottom left"\
                     +" corner, "+tbox+"."
        else:
            if ( results!=None and results.has_key( 'blc') \
                 and results.has_key('data') and results.has_key('unit')\
                 and results.has_key('mask') ):
                msg='Bottom left corner valus is, '+str(results['blc'])\
                     +', value is: '+str(results['data'])+str(results['unit'])\
                     +' with mask '+str(results['mask'])
            if ( results==None or not results.has_key('data') \
                 or not results.has_key('data') or \
               ( results['data']+1.035184e-09>error_margin or not results['mask']) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Expected 1.035184e-09Jy/Beam with mask=True."\
                     +"\n\t"+msg
    
        #############################################################
        # Bottom-right
        tbox=str(dir_trc[0])+','+str(dir_blc[1])+','+str(dir_trc[0])+','\
              +str(dir_blc[1])
        msg="Bottom right corner value was Not Found"
        results=None
        try:
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan),\
                           stokes=str(min_stokes) )
        except Exception:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to get the value in the bottom right"\
                     +" corner. "+tbox+"."                
        else:
            if ( results!=None and results.has_key( 'blc') \
                 and results.has_key('data') and results.has_key('unit')\
                 and results.has_key('mask') ):
                msg='Bottom right corner, '+str(results['blc'])+', value is: '\
                     +str(results['data'])+str(results['unit'])\
                     +' with mask '+str(results['mask'])
            if ( results==None or not results.has_key('data') \
                 or not results.has_key('data') or \
                ( results['data']+1.172165e-09 > 0.00001 or not results['mask'])):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                         +'\nError: Expected value of -1.172165e-09 and mask=True'\
                         +'\n\t'+msg
    
        ######################################################3
        # Top-left
        tbox=str(dir_blc[0])+','+str(dir_trc[1])+','+str(dir_blc[0])+','\
              +str(dir_trc[1])
        msg="Top left corner value was Not Found"
        results=None
    
        try:
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan),
                           stokes=str(min_stokes) )
        except Exception:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to get the value in the top left"\
                     +" corner, "+tbox+"."
        else:
            if ( results!=None and results.has_key( 'blc') \
                 and results.has_key('data') and results.has_key('unit')\
                 and results.has_key('mask') ):
                msg='Top left corner, '+str(results['blc'])+', value is: '\
                     +str(results['data'])+str(results['unit'])\
                     +' with mask '+str(results['mask'])
            if ( results==None or not results.has_key('data') \
                 or not results.has_key('data') or \
                 ( results['data']+4.2731923e-09>error_margin or not results['mask'])):
                retValue['success']=False
                retValue['error_msgs'] = retValue['error_msgs'] + "\nError: Expected value of -4.273192e-09, and mask=True"  + "\n\t" + msg
                
        #############################################################
        # Top-right
        tbox=str(dir_trc[0])+','+str(dir_trc[1])+','+str(dir_trc[0])+','\
              +str(dir_trc[1])
        msg="Top right corner value was Not Found"
        results=None
        try:
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan),\
                           stokes=str(min_stokes) )
        except Exception:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to get the value in the top right"\
                     +" corner. "+tbox+"."
        else:
            if ( results!=None and results.has_key( 'blc') \
                 and results.has_key('data') and results.has_key('unit')\
                 and results.has_key('mask') ):
                msg='Top right corner, '+str(results['blc'])+', value is: '\
                     +str(results['data'])+str(results['unit'])\
                     +' with mask '+str(results['mask'])
            if ( results==None or not results.has_key('data') \
                 or not results.has_key('data') or \
                 (results['data']+3.647830e-09>error_margin or not results['mask'])):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                    +'\nError: Expected value -3.647830e-09Jy/Beam and mask=True'\
                    +'\n\t'+msg
    
        #########################################################3
        # Last channel and stokes
        tbox=str(dir_trc[0])+','+str(dir_trc[1])+','+str(dir_trc[0])+','+\
              str(dir_trc[1])
        msg="Value NOT found when looking at last chanel and last stokes"
        results=None
    
        try:
            results=imval( imagename=image_file, box=tbox, chans=str(max_chan), \
                           stokes=str(max_stokes) )
        except Exception:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Failed to get the value at the last channel "\
                     +" and last stokes, "+tbox+"."
        else:
            if ( results!=None and results.has_key( 'blc') \
                 and results.has_key('data') and results.has_key('unit')\
                 and results.has_key('mask') ):
                msg='Value found at'+str(results['blc'])+' is: '\
                     +str(results['data'])+str(results['unit'])\
                     +'. with mask '+str(results['mask'])
                if ( results==None or not results.has_key('data') or \
                     ( results['data']-3.55266e-10 > error_margin ) ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                            +'\nError: Expected value -3.647830e-09Jy/Beam and'\
                            +' mask=True \n\t'+msg
    
            #######################################################
            # A couple of not so random points
            tbox=str(int(dir_trc[0]*2/3))+','+str(int(dir_trc[1]*2/3))+','\
                  +str(int(dir_trc[0]*2/3))+','+str(int(dir_trc[1]*2/3))
            msg="Value NOT found when looking at first random point,"+tbox+"."
            results=None
    
            try:
                results=imval( imagename=image_file, box=tbox, \
                chans=str(int(max_chan*2/3)), stokes=str(max_stokes) )
            except Exception:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: "+msg
            else:
                if ( results!=None and results.has_key( 'blc') \
                     and results.has_key('data') and results.has_key('unit')\
                     and results.has_key('mask') ):
                    msg='Value found at'+str(results['blc'])+' is: '\
                         +str(results['data'])+str(results['unit'])\
                         +'. with mask '+str(results['mask'])
                if ( results==None or not results.has_key('data') or \
                     ( results['data']-0.062294 > error_margin ) ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                    +'\nError: Expected value of 0.062294Jy/Beam and mask=True'\
                       +'\n\t'+msg
    
            # Second random point
            tbox=str(int(dir_trc[0]*1/6))+','+str(int(dir_trc[1]*2/6))+','\
                  +str(int(dir_trc[0]*1/6))+','+str(int(dir_trc[1]*2/6))
            msg="Value NOT found when looking at second random point,"+tbox+"."
            results=None
    
            try:
                results=imval( imagename=image_file, box=tbox, \
                            chans=str(int(max_chan*5/6)), stokes=str(max_stokes) )
            except Exception:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: "+msg
            else:
                if ( results!=None and results.has_key( 'blc') \
                     and results.has_key('data') and results.has_key('unit')\
                     and results.has_key('mask') ):
                     msg='Value found at'+str(results['blc'])+' is: '\
                     +str(results['data'])+str(results['unit'])\
                     +'. with mask '+str(results['mask'])
    
                if ( results==None or not results.has_key('data') or \
                     ( results['data']+0.070744 > error_margin ) ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                          +'Error: Expected value of -0.070744Jy/Beam and '\
                          +'mask=True'+'\n\t'+msg
    
        self.assertTrue(retValue['success'],retValue['error_msgs'])
    
    ###########################################################################
    # NAME: arrays 
    #
    # SHORT DESCRIPTION: Do tests to find the value at a single point
    #
    # DESCRIPTION:
    #           a) A slice of the directional plane
    #           b) Two slices of the directional plane
    #           c) A cube RA,Dec,and Spectral axes
    #           d) Two cubes RA,Dec,and Spectral axes
    #           e) A 4D blob: RA,Dec, Spetral, & Stokes.
    #
    # Note for the image we are using the axes are: RA, Dec, Stokes, Spectral
    #
    # TODO - check shape value on spectral value
    #        tests d and e.
    #      - This could be done in a loop, or some of it pulled out to
    #        a separate method instead of repeating code!
    ############################################################################
    
    def test_array(self):
        '''Imval: array values'''
        retValue = {'success': True, 'msgs': "", 'error_msgs': '' }
        note( "Starting ARRAY RESULTS tests.", 'NORMAL2' )
        error_margin = 0.00001
    
        # Find the min/max points of the image.
        bbox={}
        try: 
            ia.open( image_file )
            bbox=ia.boundingbox()
            ia.done()
        except:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Unable to find size of input image "+image_name
        
        dir_blc=dir_trc=[]
        min_chan=max_chan=min_stokes=max_stokes=-2
        if ( len(bbox) > 0 and bbox.has_key('blc') and bbox.has_key('trc') ):
            blc=bbox['blc']
            trc=bbox['trc']
            
            dir_blc=[blc[0], blc[1]]
            dir_trc=[trc[0], trc[1]]
            min_chan=blc[3]
            max_chan=trc[3]
            min_stokes=blc[2]
            max_stokes=trc[2]
        else:
            retValue['success']=False
            retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Unable to find corners of input image "+image_name
            return retValue
    
        #
        # We want to find an array of values for the following areas:
        #    1. inner quarter, for channel 5
        #    2. inner quarter, for channesl 5 and 10
        #    3. inner third, for channels 15to17
        #
        # Setup a few arrays with the input values so that we can do
        # these tests in a beautiful loop.
    
        # Input values
        testnames= [ 'inner 1/2 with 1 channel', 'inner 1/2 multi-channel',\
                     'inner 1/3rd multi-channel' ]
        boxes=[]
        boxes.append( str(int(dir_trc[0]*1/4))+','+str(int(dir_trc[1]*1/4))+\
                      ','+str(int(dir_trc[0]*3/4))+','+str(int(dir_trc[1]*3/4)))
        boxes.append( str(int(dir_trc[0]*1/4))+','+str(int(dir_trc[1]*1/4))+\
                      ','+str(int(dir_trc[0]*3/4))+','+str(int(dir_trc[1]*3/4)))
                      
        boxes.append(str(int(dir_trc[0]*1/3))+','+str(int(dir_trc[1]*1/3))+\
                     ','+str(int(dir_trc[0]*5/6))+','+str(int(dir_trc[1]*5/6)))
                    
        chans  = [ '5', '5,10', '15~17']
        stokes = [ '0', '0', '0'  ]
    
        # Expected results.
        shapes = [ [129,129,1,1], [129,129,2,1], [128,128,3,1] ]
        mins   = [ 0.417753, -0.417753, 0.4758411 ]
        maxs   = [ 1.69093, 1.537767, 0.999663 ]
        means  = [ 0.003042, 1000.0, 10000.0 ]
        
        for index in range(0,len(boxes)):
            results=None
            tbox=boxes[index]
    
            try:
                results=imval( imagename=image_file, box=tbox, \
                               chans=chans[index], stokes=stokes[index] )
            except Exception, e:
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                                        +"\nError: Failed " + testnames[index]\
                                        + " test, region is: "\
                                        + tbox+".\n\t"+str(e)
                return retValue
                                        
            msg=''
            if ( results!=None and results.has_key( 'blc') \
                 and results.has_key( 'trc') ):
                msg='Data array bounded by: , '+str(results['blc'])\
                     +' and '+str(results['trc'])
                
            data_array=[]
            if ( results!=None and results.has_key( 'data') ):
                data_array=results['data']
    
            mask_array=[]
            if ( results!=None and results.has_key( 'mask') ):
                mask_array=results['mask']
    
            if ( len( mask_array ) < 1 or \
                 ( mask_array.min()!=True and mask_array.max() != True ) ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                     +"\nError: Either no mask found, or False values were"\
                     +' found in the mask, expected all True values.'
    
            # Expect shape of 
            if ( len( data_array ) < 0 ):
                retValue['success']=False
                retValue['error_msgs']=retValue['error_msgs']\
                                        +"\nError: Empty data array found. "+msg
            else:
                msg=msg+'\nwith shape of '+ str(numpy.shape(data_array))
                # Note that the data and mask arrays are 2-D only, the
                # degenerative axes are dropped.  However, our expected
                # shape information contains 4 axes, so we can't do a
                # direct comparison.
                #print "DATA SHAPE: ", numpy.shape(data_array)
                #print "MASK SHAPE: ", numpy.shape(mask_array)
                #print "EXPECTED SHAPES: ", shapes[index]
                if ( numpy.shape(data_array)[0] != shapes[index][0] \
                     or numpy.shape(data_array)[1] != shapes[index][1] \
                     or numpy.shape(data_array) != numpy.shape(mask_array) ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                            +"\nError: Incorrect data or mask array size for "\
                            +"\n"+msg+"\nexpected shape to be "\
                            +str(shapes[index][0])\
                            +"X"+str(shapes[index][1])
                    
                dmin=data_array.min()
                dmax=data_array.max()
                dmean=data_array.mean()
                # CHECK THAT THESE ARE CORRECT
                # What if the min is < 0?
                if ( dmin+mins[index] > error_margin ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                           +"\nError: Expected minimum value of, "\
                           + str(mins[index])\
                           +" but found minimum of "+str(dmin)+"."
                if ( dmax-maxs[index] > error_margin ):
                    retValue['success']=False
                    retValue['error2_msgs']=retValue['error_msgs']\
                           +"\nError: Expected maximum value of, "\
                           + str(maxes[index])\
                           +" but found maximum of "+str(dmax)+"."
                if ( dmean-means[index] > error_margin ):
                    retValue['success']=False
                    retValue['error_msgs']=retValue['error_msgs']\
                      +"\nError: Expected mean of, "\
                      + str(means[index])\
                      +" but found mean of "+str(dmax)+"."
    
        self.assertTrue(retValue['success'],retValue['error_msgs'])     
        
    def test_coord_return(self):
        """Test returned coordinates CAS-2651"""
        myimval = imval(imagename=image_file, box="40,40,50,50", chans="5")
        myia = iatool()
        myia.open(image_file)
        mycsys = myia.coordsys()
        myia.done()
        expected = mycsys.toworld([45,45,0,5])['numeric']
        got = myimval["coords"][5,5]
        diff = got - expected
        # not 0 because of 32 bit precision issues
        self.assertTrue(max(abs(diff)) < 1e-16)
        
    def test_non_rect_region(self):
        """ verify imval works on non-rectangular regions, CAS-5734"""
        myia = iatool()
        imagename = "xxyy.im"
        myia.fromshape(imagename, [20,20,4,10])
        myia.done()
        ret = imval(
            imagename, region="circle[[0:0:20,-0.05.00],4arcmin]"
        )
        self.assertTrue(ret['data'].shape == (9, 9, 4, 10))
        
        
    
def suite():
    return [imval_test]

    
