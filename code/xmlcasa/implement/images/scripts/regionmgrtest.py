##############################################################################
#                                                                            #
# Test Name:                                                                 #
#    regionmgrtest_regression.py                                             #
# Rationale for Inclusion:                                                   #
#    This is a Python script designed to test the functionality and          #
#    accuracy of the CASA region manager tool.                               #
# Features Tested:                                                           #
#    absreltype, box, complement, concatenation, difference, done,           #
#    fromfiletorecord, fromrecordtotable, fromtabletorecord, intersction     #
#    ispixelregion, isworldregion, makeunion, namesintable, setcoordinates   #
#    wbox, wpolygon                                                          #
#                                                                            #
#    Note that some of the image analysis tools are used in the testing      #
#    of the region managers functionality.                                   #
# Features NOT Tested:                                                       #
#    copyregions, deletefromtable, dflt, extension, extractsimpleregions     #
#    fromglobaltotable, pixeltoworldregion, quarter, wmask                   #
# Success/failure criteria:                                                  #
#    Internally tests each method for pass/fail.  Throws an uncaught         #
#    exception ending test when an unexpected failure occurs.                #
#    All tests pass if script runs to completion.                            #
# Required Data:                                                             #
#    None, random data is generated when needed.  All temporary files will   #
#    be found in ./temp_data                                                 #
# Todo:                                                                      #
#    Look in script for the text TODO                                        #
#    In general there needs to be more tests for failure conditions          #
#    Complete the namesintable, filetorec, rectotbl, tbltorec test
##############################################################################
#                                                                            #
# Created by  SJaeger 2008-08-28 some tests from imagetest.py                #
#                                                                            #
##############################################################################

import os
import shutil
import time
import math
import random
from numpy import array

#
# regionmgrtest.py: test casapy region manager tool
#

##############################################################################
#
# Messaging utils
#
##############################################################################
def note(message, priority="NORMAL", origin="regionmgrtest", ntime=None, postcli='F'):
    #if not ntime:  #if (ntime==None):
    #    ntime=time.asctime()
    #print ntime, priority, origin, message
    print message
    casalog.postLocally(message, priority, origin)

def info(message):
    #note(message,origin='regionmgrtest')
    print message
    casalog.postLocally(message, priority="NORMAL", origin='regionmgrtest')

def fail(message=""):
    casalog.postLocally(message, priority="SEVERE", origin='regionmgrtest')
    raise RuntimeError, message


##############################################################################
#
# Array content utils
#
##############################################################################


##############################################################################
# all() returns T if every element of x is either T (if x's
# type is boolean) or non-zero (otherwise). It returns F if any
# element of x is either F or zero. For example,
#
# all(y > 3)
#
# returns T if-and-only-if every element of y is greater than 3.
# If x is not numeric an error is generated and an undefined value
# is returned.
def all(x,y):
    if len(x) != len(y):
        return false
    for i in range(len(x)):
        if not (x[i]==y[i]):
            print "x["+str(i)+"]=", x[i]
            print "y["+str(i)+"]=", y[i]
            return false
    return true

##############################################################################
def allbooleq(x,y):
    if len(x) != len(y):
        return false
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (x[i]==y[i]):
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (x[i][j]==x[i][j]):
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (x[i][j][k]==y[i][j][k]):
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (x[i][j][k][l]==y[i][j][k][l]):
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in all')
    return true

##############################################################################
def alltrue(x):
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (x[i]):
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (x[i][j]):
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (x[i][j][k]):
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (x[i][j][k][l]):
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in alltrue')
    return true


##############################################################################
def alleq(x,y,tolerance=0):
    if x.size != y.size:
        print "x.size=", x.size
        print "y.size=", y.size
        return false
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (abs(x[i]-y[i]) < tolerance):
                print "x[",i,"]=", x[i]
                print "y[",i,"]=", y[i]
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (abs(x[i][j]-y[i][j]) < tolerance):
                    print "x[",i,"][",j,"]=", x[i][j]
                    print "y[",i,"][",j,"]=", y[i][j]
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (abs(x[i][j][k]-y[i][j][k]) < tolerance):
                        print "x[",i,"][",j,"][",k,"]=", x[i][j][k]
                        print "y[",i,"][",j,"][",k,"]=", y[i][j][k]
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (abs(x[i][j][k][l]-y[i][j][k][l]) < tolerance):
                            print "x[",i,"][",j,"][",k,"][",l,"]=", x[i][j][k][l]
                            print "y[",i,"][",j,"][",k,"][",l,"]=", y[i][j][k][l]
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in alleq')
    return true

##############################################################################
def alleqnum(x,num,tolerance=0):
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (abs(x[i]-num) < tolerance):
                print "x[",i,"]=", x[i]
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (abs(x[i][j]-num) < tolerance):
                    print "x[",i,"][",j,"]=", x[i][j]
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (abs(x[i][j][k]-num) < tolerance):
                        print "x[",i,"][",j,"][",k,"]=", x[i][j][k]
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (abs(x[i][j][k][l]-num) < tolerance):
                            print "x[",i,"][",j,"][",k,"][",l,"]=", x[i][j][k]
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in alleq')
    return true

##############################################################################
def datamax(x):
    if len(x.shape)==1:
        dmax = x[0]
        for i in range(x.shape[0]):
            if dmax < x[i]: dmax = x[i]
        return dmax
    if len(x.shape)==2:
        dmax = x[0][0]
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if dmax < x[i][j]: dmax = x[i][j]
        return dmax
    if len(x.shape)==3:
        dmax = x[0][0][0]
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if dmax < x[i][j][k]: dmax = x[i][j][k]
        return dmax
    if len(x.shape)==4:
        dmax = x[0][0][0][0]
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if dmax < x[i][j][k][l]: dmax = x[i][j][k][l]
        return dmax
    if len(x.shape)>4:
        stop('unhandled array shape in max')

##############################################################################
#
# Region record utils
#
##############################################################################

def is_region_record(record={}):
    ret_value = True
    
    # Make sure the record has the minimal keywords
    if ( not record.has_key('isRegion') ):
        ret_value = False
    if ( not record.has_key('name') ):
        ret_value = False                                
    if ( not record.has_key('comment') ):
        ret_value = False
    if ( not record.has_key('regions') and not record.has_key('oneRel') ):
        ret_value = False

    # If we have a box, check for the box keys.
    # TODO add check on expect blc/trc values.
    if ( record['name'].lower()=='wcbox' or record['name'].lower()=='lcslicer' ):
        if ( not record.has_key('blc') ):
            ret_value = False
        if ( not record.has_key('trc') ):
            ret_value = False

    # If we have a polygon, check for the polygon keys.
    # TODO, add a check for the expected x,y, and pixelAxes values
    if ( record['name'].lower()=='wcpolygon' ):
        if ( not record.has_key('x') ):
            ret_value = False
        if ( not record.has_key('y') ):
            ret_value = False
        if ( not record.has_key('pixelAxes') ):
            ret_value = False        

    return ret_value

##############################################################################
#
# Data (image) creation and deletion utils
#
##############################################################################

def cleanup(dir):
    if (os.path.isdir(dir)):
        info("Cleaning up directory "+dir)
        def errFunc(raiser, problemPath, excInfo):
            note(raiser.__name__+'failed on'+problemPath,"SEVERE")
            raise RuntimeError, "Cleanup of " + dir + " fails!"
        shutil.rmtree(dir,0,errFunc)
    return true


##############################################################################
def make_test_data(file_name='testimage.im'):
    if ( not os.path.exists(datadir) ):
        try:
            os.mkdir(datadir,0770)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "Failed to make directory " + datadir

    file_path = datadir+"/"+file_name    
    if ( os.path.exists(file_path) ):
        if (os.path.isdir(file_path) ):
            def errFunc(raiser, problemPath, excInfo):
                note(raiser.__name__+'failed on'+problemPath,"SEVERE")
                raise RuntimeError, "Cleanup of " + dir + " fails!"
            shutil.rmtree(file_path,0,errFunc)
        else:
            os.remove( file_path )

    nx = 50; ny = 100; nz = 5
    data = ia.makearray(0, [nx,ny,nz])

    ia.fromarray(file_path,data)
    note('Created image='+ ia.name(), origin="regmgrtest")

    random.seed(time.time())
    for k in range(0,nz):
        slice = ia.getchunk(blc=[0,0,k], trc=[nx-1,ny-1,k], list=F)
        for i in range(0,nx):
            slice[i]=list(slice[i])
            for j in range(0,ny):
                #slice[i][j] = k+1
                slice[i][j] = random.uniform(-0.0000009,k+1.0000001)
                slice[i]=tuple(slice[i])
        ok = ia.putchunk(pixels=slice, blc=[0,0,k], list=F)
        if not ok: fail('putchunk failed')
        #note('Set plane ' + str(k) + ' to ' + str(k), origin='maketestdata()')

    ia.done()
    return file_path


##############################################################################
#
# The Region Manager Test
# Your joy is your sorrow unmasked.
##############################################################################
        
def regmgrtest():
    
    #########################################################################
    def absreltype_test():
        ret_value=True

        # Values between 1-4 are valid as follows:
        #     1  = abs
        #     2  = relref
        #     3  = relcen
        #     4  = reldir

        if ( rg.absreltype( 1 ).lower() != 'abs' ):
            ret_value=False
            note("absreltype failed with value 1", "WARN" )

        if ( rg.absreltype( 2 ).lower() != 'relref' ):
            ret_value=False
            note("absreltype failed with value 2", "WARN" )

        if ( rg.absreltype( 3 ).lower() != 'relcen' ):
            ret_value=False
            note("absreltype failed with value 3", "WARN" )

        # For now CASA does not support this, the uncomment
        # the test once the support has been added.
        #if ( rg.absreltype( 4 ).lower() != 'reldir' ):
        #    ret_value=False
        #    note("absreltype failed with value 4", "WARN" )

        # Now test some invalid types
        info ('Expect some warning for invalid absreltype values')
        if ( rg.absreltype( 0 ).lower() != 'unknown' ):
            ret_value=False
            note("absreltype did NOT failed with value 0", "WARN" )

        if ( rg.absreltype( 5 ).lower() != 'unknown' ):
            ret_value=False
            note("absreltype did NOT failed with value 5", "WARN" )

        if ( rg.absreltype( 2000 ).lower() != 'unknown' ):
            ret_value=False
            note("absreltype did NOT failed with value 200", "WARN" )

        try :
            rg.absreltype( "absolute" );
            ret_value=False
            note("absreltype did NOT failed with value 0", "WARN" )
        except:
            # an exception is a good thing in this case.
            noop='noop'

        rg.done()
        return ret_value

    #########################################################################
    def namesintable_test():
        ret_value=True

        rg.done()
        return ret_value

    #########################################################################
    def filetorec_test():
        ret_value=True

        rg.done()
        return ret_value
        
    #########################################################################
    def rectotbl_test():
        ret_value=True

        rg.done()
        return ret_value

    #########################################################################
    def tbltorec_test():
        ret_value=True

        rg.done()
        return ret_value

    #########################################################################
    def box_test():
        ret_value=True

        im_file=make_test_data("Your")
        ok=ia.open(im_file)
        if not ok:
            stop( String('Failed to open image ')+String(im_file) )

        # SIMPLE PIXEL BOX.
        #
        # TODO Check the blc and trc values to make sure they are
        # the desired ones, this can likely be added to is_region_record
        # which then should be renamed to check_region_record.
        region1 = rg.box(blc=[5,25,1],trc=[15,95,2])
        if ( not is_region_record( region1 ) ):
            ret_value=False
            note("failed to make a simple pixel region box", "WARN" )


        # Make a 2nd region with wbox, the two regions should have the
        # same shape and pixel values -- but the region records will have
        # some different dictionary keys&values
        ok = rg.setcoordinates(ia.coordsys().torecord())
        if not ok:
            ret_value=False
            stop( String("Failed to set region mgr's coordinate system.") )
        region2 = rg.wbox(blc='5pix,25pix,1pix',trc='15pix,95pix,2pix')
        if ( not is_region_record( region2 ) ):
            ret_value=False
            note("failed to make a simple world region box", "WARN" )

        pixels1 = ia.getregion(region=region1)
        pixels2 = ia.getregion(region=region2)        
        if ( not alleq( pixels1, pixels2, 0.000001 ) ):
            ret_value=False
            note("world and pixel region box select different pixels", "WARN" )
        rg.done()

        # FRACTIONAL PIXEL BOX.
        # Since the original image is 50x100x5, we expect the fractional
        # image to be 25x50x5.  So we check for this.
        #
        # TODO ia.boundingbox() should be used to get the actual size
        #      of the image.
        region1 = rg.box(blc=[.25,.25],trc=[.75,.75],frac=true)
        pixels = ia.getregion(region=region1)
        if ( not is_region_record( region1 ) ):
            ret_value=False
            note("failed to make a fractional region box", "WARN" )
        elif ( len(pixels)!=25 or len(pixels[0])!=50 or len(pixels[0][0])!=5 ):
            ret_value=False

        rg.done()
        ia.done()
        return ret_value
    
    #########################################################################
    def polygon_test():
        ret_value=True

        im_file=make_test_data("joy")
        ok=ia.open(im_file)
        if not ok:
            stop( String('Failed to open image ')+String(im_file) )

        try:
            # We expect an exception here complaining about the
            # coordinate system not being set.
            info(' ')
            note( "EXPECT an exception when the next polygon region created" )
            info(' ')
            region1 = rg.wpolygon(x=['3pix','6pix','7pix','8pix'], y=['11pix','5pix','10pix','7.5pix'], pixelaxes=[0,1])
            ret_value=False
            note( "wpolygon did NOT fail as expected. Expected an error stating that the coordinate system was not set", "WARN" )
        except StandardError, e:
            noop='noop'

        region1 = rg.wpolygon(x=['3pix','6pix','7pix','8pix'],
                              y=['11pix','5pix','10pix','7.5pix'],
                              pixelaxes=[0,1], csys=ia.coordsys().torecord() )
        if ( not is_region_record( region1 ) ):
            ret_value=False
            note("failed to make a polygon region with world coordinate", "WARN" )        

        rg.done()
        return ret_value
        

    #########################################################################
    def intersection_test():
        ret_value=True

        im_file=make_test_data("is")
        ok=ia.open(im_file)
        if not ok:
            stop( String('Failed to open image ')+String(im_file) )

        # Create two box regions as follows:
        #    box1: blc=5,30  trc=20,40
        #    box2: blc=15,5  trc=47,38
        # intersection then is a box with blc=15,30 and trc=20,38

        try:
            ok=rg.setcoordinates(ia.coordsys().torecord())
            if not ok:
                ret_value=False
                stop( String("Failed to set region mgr's coordinate system.") )

            region1 = rg.wbox(blc='5pix,30pix',trc='20pix,40pix', csys=ia.coordsys().torecord())
            region2 = rg.wbox(blc='15pix,5pix',trc='47pix,38pix', csys=ia.coordsys().torecord())
            

            if ( not is_region_record( region1 ) or not is_region_record( region2 ) ):
            	ret_value=False
                note("failed to make region boxes with a world coord system.", "WARN" )

            regions={'region1':region1, 'region2':region2}
            region3 = rg.intersection( regions=regions )
            if ( not is_region_record( region3 ) ):
            	ret_value=False
                note("failed to make intersected region.", "WARN" )
            else:
                # Test what the region manager has created for the intersection
                stats=ia.statistics( region=region3 )
                blc=stats['blc']
                trc=stats['trc']
                if ( blc[0] != 15 or blc[1] != 30 ):
                    ret_value=False
                    note( "Intersecting region has wrong blc, "+str(blc)+". expected [15,30]", "WARN" )
                if ( trc[0] != 20 or trc[1] != 38 ):
                    ret_value=False
                    note( "Intersecting region has wrong trc, "+str(trc)+". expected [20,38]", "WARN" )
              
        except 3:
            fail( "An exception has occured. The INTERSECTION test has failed.\n"+e )
            ret_value=False
        
        ia.done()
        rg.done()
        return ret_value
        
    #########################################################################
    def union_test():
        ret_value=True

        im_file=make_test_data("your")
        ok=ia.open(im_file)
        if not ok:
            stop( String('Failed to open image ')+String(im_file) )

        # Create two box regions as follows:
        #    box1: blc=5,30  trc=20,40
        #    box2: blc=15,5  trc=47,38
        # union then is a box with blc=5,5 and trc=47,40

        try:
            ok=rg.setcoordinates(ia.coordsys().torecord())
            if not ok:
                ret_value=False
                stop( String("Failed to set region mgr's coordinate system.") )

            region1 = rg.wbox(blc='5pix,30pix',trc='20pix,40pix', csys=ia.coordsys().torecord())
            region2 = rg.wbox(blc='15pix,5pix',trc='47pix,38pix', csys=ia.coordsys().torecord())
            

            if ( not is_region_record( region1 ) or not is_region_record( region2 ) ):
            	ret_value=False
                note("failed to make region boxes with a world coord system.", "WARN" )

            regions={'region1':region1, 'region2':region2}
            region3 = rg.makeunion( regions=regions )
            if ( not is_region_record( region3 ) ):
            	ret_value=False
                note("failed to make intersected region.", "WARN" )
            else:
                # Test what the region manager has created for the intersection
                stats=ia.statistics( region=region3 )
                blc=stats['blc']
                trc=stats['trc']
                if ( blc[0] != 5 or blc[1] != 5 ):
                    ret_value=False
                    note( "Union of regions has wrong blc, "+str(blc)+". expected [5,5]", "WARN" )
                if ( trc[0] != 47 or trc[1] != 40 ):
                    ret_value=False
                    note( "Union of regions has wrong trc, "+str(trc)+". expected [47,40]", "WARN" )
            
              
        except 3:
            fail( "An exception has occured, \n\t"+e )
            ret_value=False
        
        ia.done()
        rg.done()
        return ret_value
        
    #########################################################################
    # NOTE: The Image analysis tool is unable to handle complement
    #       regions (WCComplement) properly.  The tests using the
    #       statistics function have been commented out until 
    def complement_test():
        ret_value=True

        im_file=make_test_data("sorrow")
        ok=ia.open(im_file)
        if not ok:
            stop( String('Failed to open image ')+String(im_file) )

        # Create box regions as follows:
        #    box1: blc=0,0  trc=13,100
        # complement then is a box with blc=14,0 and trc=50,100

        try:
            ok=rg.setcoordinates(ia.coordsys().torecord())
            if not ok:
                ret_value=False
                stop( String("Failed to set region mgr's coordinate system.") )

            region1 = rg.wbox(blc='0pix,0pix',trc='13pix,100pix', csys=ia.coordsys().torecord())

            if ( not is_region_record( region1 ) ):
            	ret_value=False
                note("failed to make region boxes with a world coord system.", "WARN" )
            results = rg.complement( region=region1, comment="right-side of image" )
            if ( not is_region_record( results ) ):
            	ret_value=False
                note("failed to make intersected region.", "WARN" )
            else:
                # Test what the region manager has created for the intersection
                # TODO -- ia.statistics doesn't handle complement regions.
                #         when this is fixed uncomment the tests below.
                stats=ia.statistics( region=results )
                blc=stats['blc']
                trc=stats['trc']
                #print "COMPLEMENT BLC ", blc
            	#if ( blc[0] != 14 or blc[1] != 50 ):
                #    ret_value=False
                #    note( "Complement of regions has wrong blc, "+str(blc)+". expected ", "WARN" )
            	#if ( trc[0] != 0 or trc[1] != 100 ):
                #    ret_value=False
                #    note( "Complement of regions has wrong trc, "+str(trc)+". expected ", "WARN" )
            
              
        except 3:
            fail( "An exception has occured, \n\t"+e )
            ret_value=False
        
        ia.done()
        rg.done()
        return ret_value
        
    #########################################################################
    # NOTE: The Image analysis tool is unable to handle difference
    #       regions (WCDIFFERENCE) properly.  The tests using the
    #       statistics function have been commented out until
    def difference_test():
        ret_value=True

        im_file=make_test_data("unmasked.")
        ok=ia.open(im_file)
        if not ok:
            stop( String('Failed to open image ')+String(im_file) )

        # Create two box regions as follows:
        #    box1: blc=5,10  trc=32,40
        #    box2: blc=7,15  trc=25,35
        # difference then is a box with a hole in the middle.

        try:
            ok=rg.setcoordinates(ia.coordsys().torecord())
            if not ok:
                ret_value=False
                stop( String("Failed to set region mgr's coordinate system.") )

            region1 = rg.wbox(blc='5pix,10pix',trc='32pix,40pix', csys=ia.coordsys().torecord())
            region2 = rg.wbox(blc='7pix,15pix',trc='25pix,35pix', csys=ia.coordsys().torecord())
            

            if ( not is_region_record( region1 ) or not is_region_record( region2 ) ):
            	ret_value=False
                note("failed to make region boxes with a world coord system.", "WARN" )

            region3 = rg.difference( region1=region1, region2=region2, comment="A box with a hole" )
            if ( not is_region_record( region3 ) ):
            	ret_value=False
                note("failed to make intersected region.", "WARN" )
            else:
                # Test what the region manager has created.
                # TODO check some of the values.
                stats=ia.statistics( region=region3 )
                blc=stats['blc']
                trc=stats['trc']
                if ( blc[0] != 5 or blc[1] != 10  ):
                    ret_value=False
                    note( "Difference of regions has wrong blc, "+str(blc)+". expected ", "WARN" )
                if ( trc[0] !=  32 or trc[1] != 40 ):
                    ret_value=False
                    note( "Difference of regions has wrong trc, "+str(trc)+". expected ", "WARN" )
            
              
        except 3:
            fail( "An exception has occured, \n\t"+e )
            ret_value=False
        
        ia.done()
        rg.done()
        return ret_value
        
    #########################################################################
    def concat_test():        
        ret_value=True

        im_file=make_test_data("some_garbage")
        ok=ia.open(im_file)
        if not ok:
            stop( String('Failed to open image ')+String(im_file) )

        
        try:
            ok=rg.setcoordinates(ia.coordsys().torecord())
            if not ok:
                ret_value=False
                stop( String("Failed to set region mgr's coordinate system.") )

            # Create following regions:
            #    1. Slice of the spectral axes where we plan to examine areas.
            #    2. a polygon and two box regions to be applied to
            #      the spectral region
            box=rg.wbox(blc='15pix',trc='18pix', pixelaxes=[2],csys=ia.coordsys().torecord())
            if ( not is_region_record( box ) ):
            	ret_value=False
                note("failed to make box region with a world coord system.", "WARN" )
            region1 = rg.wpolygon(x=['3pix','6pix','7pix','8pix'], y=['11pix','5pix','10pix','7.5pix'], pixelaxes=[0,1])
            region2 = rg.wbox(blc='35pix,74pix',trc='41pix,81pix', csys=ia.coordsys().torecord())
            region3 = rg.wbox(blc='7pix,15pix',trc='25pix,35pix', csys=ia.coordsys().torecord())
            if ( not is_region_record( region1 ) or not is_region_record( region2 ) or not is_region_record( region3 ) ):
            	ret_value=False
                note("failed to make regions for concat test.", "WARN" )            

            regions={'region1':region1,'region2':region2,'region3':region3}
            casalog.filter('DEBUG2')
            results = rg.concatenation( box, regions, comment="Concatenated region" )
            if ( not is_region_record( results ) ):
                ret_value=False
                note("failed to create concatenated region.", "WARN" )
            else:
                # TODO -- ia.statistics can not handle returned results.
                # Test what the region manager has created
                #stats=ia.statistics( region=results, axes=[0,1] )
                #print stats
                print ''
        except 3:
            fail( "An exception has occured, \n\t"+e )
            ret_value=False


        rg.done()
        ia.done()
        return ret_value


    #########################################################################
    #  Call each of our test!
    #########################################################################
    test_names=['absreltype_test',
                'namesintable_test',
                'filetorec_test',
                'rectotbl_test',
                'tbltorec_test',
                'box_test',
                'polygon_test',
                'intersection_test',
                'union_test',
                'complement_test',
                'difference_test',
                'concat_test' ]

    for i in range( len(test_names) ):
        info(' ')
        print "STARTING test: ", test_names[i]
        ok = eval( test_names[i]+"()" )

        if ( not ok ):
            fail( "Test "+test_names[i]+ " has FAILED" )
        else:
            info( "Test "+test_names[i]+ " was SUCCESSFUL" )

## MAIN

# Clean and prepare the test data
datadir ="./tmp_data"
if not cleanup(datadir): fail()


regmgrtest()
print "End of regionmgrtest.py"
cleanup(datadir)
#exit()
