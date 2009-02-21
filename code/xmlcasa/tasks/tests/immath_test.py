import sys
import traceback
import os
import commands

image1='./data/gula.im'
image2='./data/sgps.im'

outimage = './data/results.im'

allTestsPassed = True;



## HELPER FUNCTIONS

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

################        TEST 1          ###################
# Test to see if all the points of a single image are doubled.
# Note that we a few values from the input and output images to see
# if we truly doubled each value.
testPassed = True;
print "**** Testing the doubling of every value in an image. *** "
immath( outimage, str('"')+image1+str('" * 2' ) );

# Check all of the corners, note we really should get the coordsys
# of these images and from that get their sizes with the topixel
# function, however, we are assuming that its a 256x256x1x46 image.
points = [ [0,0,0,0], [255,255,0,45], [127,127,0,21], [12,43,0,3] ];
try: 
    for i in range( len(points) ):
        point=points[i]

        inValue = _getPixelValue( image1, point )
        outValue = _getPixelValue( outimage, point )
        
        if ( inValue*2 != outValue ):
            testPassed = False;
            print "Values are incorrect at ", point, " in ", inValue, " and out: ",  outValue
except ValueError, ve:
    testPassed = False;

    
if (testPassed):
    print "PASSED: doubling of every value in an image test\n\n"
else:
    print "FAILED: doubling of every value in an image test\n\n"
    allTestsPassed = False
    

################        TEST 2          ###################
# Test to see if we can grab a single plane out of the image.
# Then use this single plane and add it to the whole image.
# In this case we want the plane at channel 5.  NOTE: We can't
# do this with LEL at the moment, but will be able to in the
# future.  The commented out immath() calls are for the future
# support.
testPassed = True;
print "**** Testing extraction of a single plane."

# First test that we can extract a plane from the bigger image.
# Checking the size of the resulting image and a few of the
# points to make sure they are from the correct plane.
default( 'immath');
immath( outimage, str('"')+image1+str('"' ), chans='5' );
#immath( outimage, str('"')+image1+str('"[INDEXIN(4,[5])]' ) );

ia.open( outimage );
size = ia.shape();
ia.close()
if ( size[0]!=256 or size[1]!=256 or size[2]!=1 or size[3]!=1 ):
    testPassed=False;
    print "Size of subimage is incorrect: ", size, ". Expected the resulting image\n to be 256x256x1x 1 pixels."
else:
    inPoints = [ [0,0,0,5], [255,255,0,5], [127,127,0,5], [9,178,0,5] ];
    outPoints = [ [0,0,0,0], [255,255,0,0], [127,127,0,0], [9,178,0,0] ];
    diffs=[];
    
    try:
        for i in range( len(points) ):
            inValue  = _getPixelValue( image1, inPoints[i] );
            outValue = _getPixelValue( outimage, outPoints[i] );
        
            if ( inValue != outValue ):
                TestPassed=False;
                print "Values are incorrect at IN:", inPoints[i], "=", inValue, " and  OUT: ",outPoints[i], "=",  outValue
       
            diffs.append( outValue );
    except ValueError, ve:
        testPassed = False;

    
# We know how to make a subimage so now lets actually add this plane
# to the whole image.
tmpfile="./data/temp.im"
outimage="./data/results2.im"

default( 'immath')
outfile=tmpfile
expr=str('"')+image1+str('"' )
chans='5'
go()

default( 'immath')
outfile=outimage
expr=str('"')+image1+str('"' ) + str(" + " ) + str('"')+tmpfile + str('"')
#expr=str('"')+image1+str('"' ) + str(" + " ) + str('"')+image1+str('"[INDEXIN(4,[5])]' );
go()

# Now do some value checks
# of these images and from that get their sizes with the topixel
# function, however, we are assuming that its a 256x256x1x46 image.
ia.open( outfile )
size=ia.shape()
ia.close()
if ( size[0]!=256 or size[1]!=256 or size[2]!=1 or size[3]!=46 ):
        testPassed=False;
        print "Size of image is incorrect: ", size, ". Expected the resulting image\n to be 256x256x1x46 pixels."
else: 
    # Note that the first 3 axes need to match was was used for the
    # inPoints list above in order to use the planeValues gathered above.
    try :
        points = [ [0,0,0,0], [255,255,0,45], [127,127,0,12], [9,178,0,33] ];
        for i in range( len(points) ):
            point=points[i]
            inValue  = _getPixelValue( image1, point );
            outValue = _getPixelValue( outimage, point );

        # NOTE: There can be rounding errors so we don't expect it
        #       to be exact.
        if ( abs(inValue+diffs[i]-outValue) > 0.000001 ):
            testPassed = False;
            print "Values are incorrect at ", point, "\t\nIN: ", inValue, " OUT: ",  outValue, ". Expected difference was: ", diffs[i]
    except ValueError, ve:
        print _exceptionInfo()
        testPassed = False
        
if (testPassed):
    print "PASSED: extraction of a plane\n\n"
else:
    print "FAILED: extraction of a plane\n\n"
    allTestsPassed = False

################        TEST 3          ###################    
# Test using region files.  In this test we use an ImageRegion
# created with the CASA viewer to test region selection.
commands.getstatus( str('rm -rf ')+outfile )
testPassed = True;
print "**** Testing region selection from a file."
rgnFile='./data/sgps.im.rgn'

# First check some error conditions.  Each of these immath
# calls should fail.
default( 'immath' )
try:
    region=rgnFile
    box="[0,0,10,10]"
except:
    print "FAILURE immath did not fail with region and box both set"
    testPassed = False

try:
    region=rgnFile
    chans='>5'
except:
    print "FAILURE immath did not fail with region and chans both set"
    testPassed = False
print 'Done testing inputs with warnings'

# To test if immath handles a region properly, note that in this
# file we have a polygon region.  We don't do anything fancy with
# the data, just get the subimage that is created from the region.
default('immath')
outfile=outimage
expr=str('"')+image2+str('"')
go()
imstat( outfile )

# Note the statistical results are stored in a dictionary in the
# global variable xstat.
max=7.79535007477
min=-8.33498287201
sum=10942.1707248
if ( abs( float(xstat['max'][0]) - max) > 0.000001 ):
    testPassed=False
    print "Maximum in selected region is not correct. Found ", xstat['max'][0], " expected ", max
if ( abs( float(xstat['min'][0]) - min ) > 0.000001 ):
    testPassed=False
    print "Minimum in selected region is not correct. Found ", xstat['min'][0], " expected ", min    
if ( abs( float(xstat['sum'][0]) - sum ) > 0.000001 ):
    testPassed=False
    print "Sum of all points in selected region is not correct. Found ", xstat['sum'][0], " expected ", sum    
    

    

if (testPassed):
    print "PASSED: region selection test\n\n"
else:
    print "FAILED: region selection test\n\n"
    allTestsPassed = False
    

################        DONE          ###################
if ( allTestsPassed ):
    print "CONGRATULATIONS: All of the tests passed"
else:
    print "Oh so sorry!  Not all tests passed you get to do some debugging"





