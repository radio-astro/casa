###########################################################################
#
# Regression Test for the imval task.
#
# Test descriptions
# --------------------
# Test 1: Prepare data file(s) for testing
# Test 2: Invalid input tests
#           a) No imagename given
#           b) Bad imagename given
#           c) Incorrect data type, not a string, to imagename parameter
#           d) Out of range errors for, box, chans, & stokes parameters
#           e) incorrect data type to box, chans, & stokes parameters
#           f) Bad file name to region parameter
#           g) Incorrect data type, not a string, to region parameter
#           h) File name that does not contain a region to the region param.
# Test 3: Value at a single point tests.
#           a) Value at bottom-left corner
#           b) Value at bottom-right corner
#           c) Value at top-left corner
#           d) Value at top-right corner
#           e) Value at 3 points within the image.
# Test 4: An array of values
#           a) A slice of the directional plane
#           b) Two slices of the directional plane
#           c) A cube RA,Dec,and Spectral axes
#           d) Two cubes RA,Dec,and Spectral axes
#           e) A 4D blob: RA,Dec, Spetral, & Stokes.
#
# Test 5: Cleanup & report
#           Cleanup the staging area, and report
#           success/failure of the tests.
#     
#
# SDJ Sep. 8, 2008 Created.
#
#
#----------------------------
# TODO: produce summary data
#       make sure failure_msgs is used everywere, and display them
###########################################################################
import time
import os
import shutil
import glob
import numpy


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
# NAME: cleanup
#
# SHORT DESCRIPTION: Cleanup the data created for the test.
#
# DESCRIPTION: Delete an files that were created and copy the
#              casapy.log to the cwd for the tester.
#
############################################################################
def cleanup( cwd, test_dir ):
    if ( not os.path.exists( test_dir ) ):
        return True

    info ( 'Cleaning up files in '+test_dir )
    if ( not os.path.isdir( test_dir ) ):
        os.remove( test_dir )
    else:
        for file_name in glob.glob( test_dir+'/*.log*' ):
            short_name=file_name[file_name.rfind('/')+1:]
            os.rename( file_name, cwd+'/'+short_name )
        def errfunc( raiser, problemPath, execInfo ):
            note(raiser.__name__+'faile on'+problemPath, "SEVERE" )
            raise RuntimeError, "Cleanup of " + test_dir + "failed "
        shutil.rmtree(test_dir, 0, errfunc )
        
    return True

###########################################################################
# NAME: setup (TEST 1)
#
# SHORT DESCRIPTION: Prepare data
#
# DESCRIPTION: Prepare the data staging area for the remainder of the
#              test.  This requires copying ??? from the regression
#              test data area to a local directory.
#
############################################################################

def setup( src_file, test_dir ):
    passed=True

    try:
        os.mkdir( test_dir )
        os.chdir( test_dir )
        name=src_file[src_file.rfind('/')+1:]
        shutil.copytree( src_file, './'+name )
    except Exception, e: 
        note( 'Exception reported: '+str(e), 'WARN' )
        passed=False

    return passed


###########################################################################
# NAME: bad_input (TEST 2)
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

def bad_input( image_file ):
    passed = True

    info( 'This test will create lots of EXCEPTIONS, this is good.' )

    # Image name tests
    try:
        imval( imagename='' )
        failure_msgs['bad_input1']='Empty imagename parameter not detected.'
        passed=False
    except:
        noop='noop'

    try:
        imval( imagename='garbage.im' )
        failure_msgs['bad_input2']='Invalid image file name not detected.'
        passed=False
    except:
        noop='noop'
        
    try:
        imval( imagename=2.3 )
        failure_msgs['bad_input3']='Invalid image file name not detected.'
        passed=False
    except:
        noop='noop'
        
    # Testing out of range errors.
    # BLC=0,0,0,0  and TRC= 255,255,0,29   for n4826_bima.im

    try:
        imval( imagename=image_file, box='-3,0,-3,3' )
        failure_msgs['bad_input4']='Invalid box parameter values not detected.'
        passed=False
    except:
        noop='noop'
        
    try:
        imval( imagename=image_file, box='200,0,262,3' )
        failure_msgs['bad_input5']='Invalid box parameter values not detected.'
        passed=False
    except:
        noop='noop'

    try:
        imval( imagename=image_file, box='0,-3,0,3' )
        failure_msgs['bad_input6']='Invalid box parameter values not detected.'
        passed=False
    except:
        noop='noop'
        
    try:
        imval( imagename=image_file, box='0,270,0,3' )
        failure_msgs['bad_input7']='Invalid box parameter values not detected.'
        passed=False
    except:
        noop='noop'                        

    try:
        imval( imagename=image_file, box=[1,2,3,4] )
        failure_msgs['bad_input8']='Invalid box parameter values not detected.'
        passed=False
    except:
        noop='noop'                        

    try:
        imval( imagename=image_file, chans='-3' )
        failure_msgs['bad_input9']='Invalid chans parameter values not detected.'
        passed=False
    except:
        noop='noop'                        

    try:
        imval( imagename=image_file, chans='50' )
        failure_msgs['bad_input10']='Invalid chans parameter values not detected.'
        passed=False
    except:
        noop='noop'

    try:
        imval( imagename=image_file, chans=10 )
        failure_msgs['bad_input11']='Invalid chans parameter values not detected.'
        passed=False
    except:
        noop='noop'        

    try:
        imval( imagename=image_file, stokes='Q' )
        failure_msgs['bad_input12']='Invalid stokes parameter values not detected.'
        passed=False
    except:
        noop='noop'                                
        
    try:
        imval( imagename=image_file, stokes=0 )
        failure_msgs['bad_input13']='Invalid stokes parameter values not detected.'
        passed=False
    except:
        noop='noop'

    #
    # TODO add region parameter tests

    return passed

###########################################################################
# NAME: single_point (TEST 3)
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

def single_point( image_file ):
    passed = True

    # Find the min/max points of the image.
    bbox={}
    try: 
        ia.open( image_file )
        bbox=ia.boundingbox()
        ia.done()
    except:
        failure_msgs['single_point1']='Unable to find size of image '+image_name
        passed= False

    try:
        if ( len(bbox) > 0 and bbox.has_key('blc') and bbox.has_key('trc') ):
            blc=bbox['blc']
            trc=bbox['trc']

            dir_blc=[blc[0], blc[1]]
            dir_trc=[trc[0], trc[1]]
            min_chan=blc[3]
            max_chan=trc[3]
            min_stokes=blc[2]
            max_stokes=trc[2]

            # Bottom-left
            tbox=str(dir_blc[0])+','+str(dir_blc[1])+','+str(dir_blc[0])+','+str(dir_blc[1])
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan), stokes=str(min_stokes) )
            msg='Bottom left corner, '+str(results['blc'])+', value is: '
            msg=msg+str(results['data'])+str(results['unit'])
            msg=msg+'\n\twith mask '+str(results['mask'])
            if ( results['data']+1.035184e-09 > 0.00001 or not results['mask']):
                passed=False
                note(msg+'\n\tExpected 1.035184e-09Jy/Beam with mask=True', 'WARN' )
            else:
                info(msg)

            # Bottom-right
            tbox=str(dir_trc[0])+','+str(dir_blc[1])+','+str(dir_trc[0])+','+str(dir_blc[1])
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan), stokes=str(min_stokes) )
            msg='Bottom right corner, '+str(results['blc'])+', value is: '
            msg=msg+str(results['data'])+str(results['unit'])
            msg=msg+'\n\twith mask '+str(results['mask'])
            if ( results['data']+1.172165e-09 > 0.00001 or not results['mask']):
                passed=False
                note(msg+'\n\tExpected value of -1.172165e-09 and mask=True', 'WARN' )
            else:
                info(msg)


            # Top-left
            tbox=str(dir_blc[0])+','+str(dir_trc[1])+','+str(dir_blc[0])+','+str(dir_trc[1])
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan), stokes=str(min_stokes) )
            msg='Top left corner, '+str(results['blc'])+', value is: '
            msg=msg+str(results['data'])+str(results['unit'])
            msg=msg+'\n\twith mask '+str(results['mask'])            
            if ( results['data']+4.2731923e-09 > 0.00001 or not results['mask']):
                passed=False
                note(msg+'\n\tExpected value of -4.273192e-09, and mask=True', 'WARN' )
            else:
                info(msg)


            # Top-right
            tbox=str(dir_trc[0])+','+str(dir_trc[1])+','+str(dir_trc[0])+','+str(dir_trc[1])
            results=imval( imagename=image_file, box=tbox, chans=str(min_chan), stokes=str(min_stokes) )
            msg='Top right corner, '+str(results['blc'])+', value is: '
            msg=msg+str(results['data'])+str(results['unit'])
            msg=msg+'\n\twith mask '+str(results['mask'])
            if ( results['data']+3.647830e-09 > 0.00001 or not results['mask'] ):
                passed=False
                note(msg+'\n\tExpected value -3.647830e-09Jy/Beam and mask=True', 'WARN' )
            else:
                info(msg)

            # Last channel and stokes
            tbox=str(dir_trc[0])+','+str(dir_trc[1])+','+str(dir_trc[0])+','+str(dir_trc[1])
            results=imval( imagename=image_file, box=tbox, chans=str(max_chan), stokes=str(max_stokes) )
            msg='Value found at'+str(results['blc'])+' is: '
            msg=msg+str(results['data'])+str(results['unit'])
            msg=msg+'.\n\t with mask '+str(results['mask'])
            if ( results['data']-3.55266e-10 > 0.00001 ):
                passed=False
                note(msg+'\n\tExpected value -3.647830e-09Jy/Beam and mask=True ', 'WARN' )
            else:
                info(msg)


            # A couple of not so random points
            tbox=str(int(dir_trc[0]*2/3))+','+str(int(dir_trc[1]*2/3))+','+str(int(dir_trc[0]*2/3))+','+str(int(dir_trc[1]*2/3))
            results=imval( imagename=image_file, box=tbox, chans=str(int(max_chan*2/3)), stokes=str(max_stokes) )
            msg='Value found at'+str(results['blc'])+' is: '
            msg=msg+str(results['data'])+str(results['unit'])
            msg=msg+'.\n\t with mask '+str(results['mask'])            
            if ( results['data']-0.062294 > 0.00001 ):
                passed=False
                note(msg+'\n\tExpected value of 0.062294Jy/Beam and mask=True', 'WARN' )
            else:
                info(msg)

            tbox=str(int(dir_trc[0]*1/6))+','+str(int(dir_trc[1]*2/6))+','+str(int(dir_trc[0]*1/6))+','+str(int(dir_trc[1]*2/6))
            results=imval( imagename=image_file, box=tbox, chans=str(int(max_chan*5/6)), stokes=str(max_stokes) )
            msg='Value found at'+str(results['blc'])+' is: '
            msg=msg+str(results['data'])+str(results['unit'])
            msg=msg+'.\n\t with mask '+str(results['mask'])            
            if ( results['data']+0.070744 > 0.00001 ):
                passed=False
                note(msg+'\n\tExpected value of -0.070744Jy/Beam and mask=True', 'WARN' )
            else:
                info(msg)
                        
    except Exception, e:
        failure_msgs['single_point2']='Unable to get value at a point '+str(e)
        passed= False

    return passed

###########################################################################
# NAME: arrays (TEST 4)
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
############################################################################

def arrays( image_file ):
    passed = True

    # Find the min/max points of the image.
    bbox={}
    try: 
        ia.open( image_file )
        bbox=ia.boundingbox()
        ia.done()
    except:
        failure_msgs['arrays1']='Unable to find size of image '+image_name
        passed= False
    
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
        failure_msgs['arrays2']='Unable to find size of image '+image_name
        passed= False
        return passed

    try:
        # Get the inner quarter of the image for channel 5.
        tbox=str(int(dir_trc[0]*1/4))+','+str(int(dir_trc[1]*1/4))+','+str(int(dir_trc[0]*3/4))+','+str(int(dir_trc[1]*3/4))
        results=imval( imagename=image_file, box=tbox, chans='5', stokes='0' )
        data_array=results['data']
        mask_array=results['mask']
        if ( mask_array.min()!=True and mask_array.max() != True ):
            failure_msgs['array3']='Found False values in mask, expected all True values.'
            passed=False

        # Expect shape of 129x129
        msg='Found inner image data -- blc='+str(results['blc'])
        msg=msg+' to trc='+str(results['trc'])
        msg=msg+'-- to have shape of '+ str(numpy.shape(data_array))
        note( msg, 'NORMAL2' )
        if ( numpy.shape(data_array)[0] != 129 or numpy.shape(data_array)[1] != 129 or numpy.shape(mask_array)[0] != 129 or numpy.shape(mask_array)[1] != 129 ):
            failure_msgs['array4']='Incorrec data or mask array size, expected 129x129.'
            passed=False
        else:
            info(msg)

        dmin=data_array.min()
        dmax=data_array.max()
        dmean=data_array.mean()
        msg='Found for 129x129 image max/min/mean values to be: '
        msg=msg+str(dmin)+'/'+str(dmax)+'/'+str(dmean)
        note( msg, 'NORMAL2' )
        if ( dmin+0.417753 > 0.00001 or dmax-1.169093 > 0.00001 or dmean-0.003042 > 0.00001):
            failure_msgs['array5']='Unexpected min, max or mean value found ('+str(dmin)+","+str(dmax)+')'
            passed=False
        else:
            info(msg)            
    except Exception, e:
        failure_msgs['arrays2']='Unable to do inner 1/2 slice '+str(e)
        passed= False        
    note( "" )
    note( "" )    
    
    try:
        # Get the inner quarter of the image for channel 5 and 10.
        tbox=str(int(dir_trc[0]*1/4))+','+str(int(dir_trc[1]*1/4))+','+str(int(dir_trc[0]*3/4))+','+str(int(dir_trc[1]*3/4))
        results=imval( imagename=image_file, box=tbox, chans='5~10', stokes='0' )
        data_array=results['data']
        mask_array=results['mask']
        if ( mask_array.min()!=True and mask_array.max() != True ):
            failure_msgs['array6']='Found False values in mask, expected all True values.'
            passed=False
        # Expect shape of 129x129x2
        msg='Fourth array image test data is -- blc='+str(results['blc'])
        msg=msg+' and trc='+str(results['trc'])
        msg=msg+'-- to have shape of '+ str(numpy.shape(data_array))
        note( msg, 'NORMAL2' )
        if ( numpy.shape(data_array)[0] != 129 or numpy.shape(data_array)[1] != 129 or numpy.shape(mask_array)[0] != 129 or numpy.shape(mask_array)[1] != 129 ):
            failure_msgs['array7']='Incorrec data or mask array size, expected 129x129.'
            passed=False
        else:
            info(msg)

        dmin=data_array.min()
        dmax=data_array.max()
        dmean=data_array.mean()
        msg='Fourth image test data is  max/min/mean values to be: '
        msg=msg+str(dmin)+'/'+str(dmax)+'/'+str(dmean)
        note( msg, 'NORMAL2' )
        if ( dmin+0.6257515 > 0.00001 or dmax-1.537767 > 0.00001 ):
            failure_msgs['array8']='Unexpected min, max or mean value found ('+str(dmin)+","+str(dmax)+')'
            passed=False
        else:
            info(msg)
                    
    except Exception, e:
        failure_msgs['arrays9']='Unable to do inner 1/2 slice on two channels '+str(e)
        passed= False
    note( "" )
    note( "" )    
    
        
    try:
        # Get the inner quarter of the image for channel 15 to 17.
        tbox=str(int(dir_trc[0]*1/3))+','+str(int(dir_trc[1]*1/3))+','+str(int(dir_trc[0]*5/6))+','+str(int(dir_trc[1]*5/6))
        print "imval( imagename=image_file, box=tbox, chans='15~17', stokes='0' )"
        results=imval( imagename=image_file, box=tbox, chans='15~17', stokes='0' )
        data_array=results['data']
        mask_array=results['mask']
        if ( mask_array.min()!=True and mask_array.max() != True ):
            failure_msgs['array6']='Found False values in mask, expected all True values.'
            passed=False
        # Expect shape of 129x129x2
        msg='Found inner image data -- blc='+str(results['blc'])
        msg=msg+' to trc='+str(results['trc'])
        msg=msg+'-- to have shape of '+ str(numpy.shape(data_array))
        note( msg, 'NORMAL2' )
        if ( numpy.shape(data_array)[0] != 128 or numpy.shape(data_array)[1] != 128 or numpy.shape(mask_array)[0] != 128 or numpy.shape(mask_array)[1] != 128 ):
            failure_msgs['array7']='Incorrect data or mask array size, expected 129x129.'
            passed=False
        else:
            info(msg)

        dmin=data_array.min()
        dmax=data_array.max()
        dmean=data_array.mean()
        msg='For inner 1/2 of image found the max/min/mean values to be: '
        msg=msg+str(dmin)+'/'+str(dmax)+'/'+str(dmean)
        note( msg, 'NORMAL2' )
        if ( dmin+0.4758411 > 0.00001 or dmax-0.999663 > 0.00001 or dmean-0.0010576 > 0.00001 ):
            failure_msgs['array8']='Unexpected min, max or mean value found..'
            passed=False
        else:
            info(msg)
                    
    except Exception, e:
        failure_msgs['arrays9']='Unable to do inner 1/2 slice on two channels '+str(e)
        passed= False                

    if ( not passed ):
        print "FAILURES: ", failure_msgs
    return passed
    

###########################################################################
# MAIN 
#
# SHORT DESCRIPTION: Main routine !
#
# DESCRIPTION: This is what gets called when this script is executed.
#              For the C programmers out there, this is  main()
#
############################################################################

#
# A few global variables to keep track of stuff
#
ngc4826_src_file='/export/chipmunk1/CASA_TEST/TESTDATA/regression/ATST3/NGC4826/n4826_bima.im'
num_tests=0
num_failed=0
num_passed=0
failure_msgs={}
cwd=os.getcwd()
test_dir=cwd+'/imval_tests'
test_file=test_dir+'/n4826_bima.im'


test_names=['setup(ngc4826_src_file, test_dir)',
            'bad_input(test_file)',
            'single_point(test_file)',
            'arrays(test_file)']

# Before getting started lets make sure we are
# starting fresh
if ( cleanup(cwd, test_dir ) ):

    for test_name in test_names:
        info( ' ' )
        info( 'STARTING test: '+test_name )
        num_tests=num_tests+1
        stime=time.time()
        ok = eval( test_name )
        etime=time.time()

        info ( 'Time to run test '+test_name+' was: '+str(etime-stime) )
        if ( ok ):
            num_passed=num_passed+1
            info( 'Test: '+test_name+" has PASSED." )
        else:
            num_failed=num_failed+1
            info( 'Test: '+test_name+" has FAILED." )
        info( 'DONE test: '+test_name )
            
        #summary()
        info( '' )
        info( '' )
        info( '--------------- SUMMARY -----------' )
        info("Number of SUCCESSFUL tests: "+str(num_passed) )
        info("Number of FAILED tests:     "+str(num_failed) )

    # Put user back where they started and cleanup files
    cleanup(cwd, test_dir )
    os.chdir( cwd )

