import numpy
import os
import shutil

#src_file='/export/chipmunk1/CASA_TEST/TESTDATA/images/ALPHA/ngc5921.im'
#input_file='ngc5921.im'

src_file='/export/chipmunk1/CASA_TEST/TESTDATA/images/ALPHA/gula.im'
input_file='gula.im'


##########  PREPARATION ############
if ( os.path.isdir( input_file ) ):
    def errFunc( raiser, problemPath, execInfo ):
        casalog.post( raiser.__name__+' failed on '+problemPath, "SEVERE" )
        raise RuntimeError, "Unable to prepare test files"
    shutil.rmtree( input_file, 0, errFunc )
shutil.copytree( src_file, input_file )


##########  START TESTS ############
# Test the listing of data modes.
print "***********       LIST      ******************"
print "EXECUTING: imhead( ", input_file, ", 'list' ) "
orig_hdr=imhead( input_file, 'list' )


print " "
print "***********     SUMMARY    ******************"
imhead( input_file, 'summary' )
print " "

print " "
print "***********      FITS      ******************"
imhead( input_file, 'fits' )
print " "


print " "
print "***********     HISTORY    ******************"
imhead( input_file, 'history' )
print " "


# Test the set/get value routines.  All of them
print "*******   SET/GET  (object)  ***************"
#######  OBJECT  #############
val=imhead( input_file, 'get', 'object' )
imhead( input_file, 'put', 'object', 'theCoolObject' )
newval=imhead( input_file, 'get', 'object' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set object failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'theCoolObject'
imhead( input_file, 'put', 'object', val['value'] )

#######  IMTYPE  #############
print "*******   SET/GET  (imtype)  ***************"
val=imhead( input_file, 'get', 'imtype' )
imhead( input_file, 'put', 'imtype', '' )
newval=imhead( input_file, 'get', 'imtype' )
if ( val == newval ) :
    print "FAILED TEST: Del/Add imtype failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", ''
imhead( input_file, 'put', 'imtype', val['value'] )

#######  TELESCOPE  #############
print "*******   SET/GET  (telescope)  ***************"
val=imhead( input_file, 'get', 'telescope' )
imhead( input_file, 'put', 'telescope', 'myTelescope' )
newval=imhead( input_file, 'get', 'telescope' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set telescope failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'myTelescope'
imhead( input_file, 'put', 'telescope', val['value'] )
     
#######  OBSERVER  #############
print "*******   SET/GET  (observer)  ***************"
val=imhead( input_file, 'get', 'observer' )
imhead( input_file, 'put', 'observer', 'me' )
newval=imhead( input_file, 'get', 'observer' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set observer failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'me'
imhead( input_file, 'put', 'observer', val['value'] )

#######  DATE-OBS  #############
print "*******   SET/GET  (date-obs)  ***************"
val=imhead( input_file, 'get', 'date-obs' )
imhead( input_file, 'put', 'date-obs', '2007/06/20/00:00:00' )
newval=imhead( input_file, 'get', 'date-obs' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set date-obs failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '2007/06/20/00:00:00'
imhead( input_file, 'put', 'date-obs', str(val['value'])+str(val['unit']) )

#######  EQUINOX  #############
print "*******   SET/GET  (equinox)  ***************"
val=imhead( input_file, 'get', 'equinox' )
imhead( input_file, 'put', 'equinox', 'B1950' )
newval=imhead( input_file, 'get', 'equinox' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set equinox failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'B1950'
imhead( input_file, 'put', 'equinox', val['value'] )
       
#######  RESTFREQUENCY  #############
print "*******   SET/GET  (restfrequency)  ***************"
val=imhead( input_file, 'get', 'restfreq' )
imhead( input_file, 'put', 'restfreq', '1.421GHz' )
imhead( input_file, 'put', 'restfreq', '15.272GHz, 1.67GHz' )
newval=imhead( input_file, 'get', 'restfreq' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set restfrequency failed."
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '1.421GHz, 115.272GHz'
#imhead( input_file, 'put', 'restfreq', str(val['value'])+str(val['unit']) )
imhead( input_file, 'put', 'restfreq', val['value'])

#######  PROJECTION  #############
print "*******   SET/GET  (projection)  ***************"
val=imhead( input_file, 'get', 'projection' )
imhead( input_file, 'put', 'projection', 'TAN' )
newval=imhead( input_file, 'get', 'projection' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set projection failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'TAN'
imhead( input_file, 'put', 'projection', val['value'] )

#######  BEAM MAJOR  #############
print "*******   SET/GET  (beam major)  ***************"
val=imhead( input_file, 'get', 'beammajor' )
imhead( input_file, 'put', 'beammajor', '55arcsec')
newval=imhead( input_file, 'get', 'beammajor' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set beam failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '55arcsec'
imhead( input_file, 'put', 'beammajor', val )

#######  BEAM  MINOR #############
print "*******   SET/GET  (beam minor axis)  ***************"
val=imhead( input_file, 'get', 'beamminor' )
imhead( input_file, 'put', 'beamminor', '24arcsec' )
newval=imhead( input_file, 'get', 'beamminor' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set beam failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '24arcsec'
imhead( input_file, 'put', 'beamminor', str(val['value'])+str(val['unit']) )

#######  BEAM  #############
print "*******   SET/GET  (beam position angle)  ***************"
val=imhead( input_file, 'get', 'beampa' )
imhead( input_file, 'put', 'beampa', '0deg' )
newval=imhead( input_file, 'get', 'beampa' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set beam failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '0deg'
imhead( input_file, 'put', 'beampa', str(val['value'])+str(val['unit']) )
       
#######  BUNIT  #############
print "*******   SET/GET  (bunit)  ***************"
val=imhead( input_file, 'get', 'bunit' )
imhead( input_file, 'put', 'bunit', 'kJy/beam' )
newval=imhead( input_file, 'get', 'bunit' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set bunit failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'kJy/beam'
imhead( input_file, 'put', 'bunit', str(val['value'])+str(val['unit']) )

#######  CTYPE3  #############
print "*******   SET/GET  (ctype3)  ***************"
val=imhead( input_file, 'get', 'ctype3' )
imhead( input_file, 'put', 'ctype3', 'Declination' )
newval=imhead( input_file, 'get', 'ctype3' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set ctype3 failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'Declination'
imhead( input_file, 'put', 'ctype3', val['value'] )

#######  CRPIX2  #############
print "*******   SET/GET  (crpix2)  ***************"
val=imhead( input_file, 'get', 'crpix2' )
imhead( input_file, 'put', 'crpix2', '64.0' )
newval=imhead( input_file, 'get', 'crpix2' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set crpix2 failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '64.0'
imhead( input_file, 'put', 'crpix2', val['value'] )

#######  CRVAL4  #############
print "*******   SET/GET  (crval4)  ***************"
val=imhead( input_file, 'get', 'crval4' )
imhead( input_file, 'put', 'crval4', '1.6' )
newval=imhead( input_file, 'get', 'crval4' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set crval4 failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '1.6'
imhead( input_file, 'put', 'crval4', val['value'] )

#######  CDELT4  #############
print "*******   SET/GET  (cdelt4)  ***************"
val=imhead( input_file, 'get', 'cdelt4' )
imhead( input_file, 'put', 'cdelt4', '2.5'  )
newval=imhead( input_file, 'get', 'cdelt4' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set cdelt4 failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", '2.5'
imhead( input_file, 'put', 'cdelt4', val['value'] )


#######  CUNIT1  #############
print "*******   SET/GET  (cunit1)  ***************"
val=imhead( input_file, 'get', 'cunit1' )
imhead( input_file, 'put', 'cunit1', 'deg' )
newval=imhead( input_file, 'get', 'cunit1' )
if ( val == newval ) :
    print "FAILED TEST: Get/Set cunit1 failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'deg'
imhead( input_file, 'put', 'cunit1', val['value'] )

#######  BEAM  #############
print "*******   DEL/ADD  (beam)  ***************"
val_maj=imhead( input_file, 'get', 'beammajor' )
val_min=imhead( input_file, 'get', 'beamminor' )
val_pa=imhead( input_file, 'get', 'beampa' )
imhead( input_file, 'del', 'beammajor' )
try:
    newval=imhead( input_file, 'get', 'beammajor' )
except:
    no_op='no_op'
else:
    if ( newval != None ):
        print "FAILED TEST: Del/add beam failed"
        print "             Original value: ", val
        print "             Curent value:   ", newval
        print "             Expected value: ", 'Not Known, Not Known, Not Known'
imhead( input_file, 'add', 'beammajor', str(val_maj['value'])+str(val_maj['unit']) )
imhead( input_file, 'add', 'beamminor', str(val_min['value'])+str(val_min['unit']) )
imhead( input_file, 'add', 'beampa', str(val_pa['value'])+str(val_pa['unit']) )

#######  OBJECT  #############
print "*******   DEL/ADD  (object)  ***************"
val=imhead( input_file, 'get', 'object' )
imhead( input_file, 'del', 'object' )
newval=imhead( input_file, 'get', 'object' )
if ( val == newval ) :
    print "FAILED TEST: Del/Add object failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", 'Not Known'
imhead( input_file, 'add', 'object', val['value'])

#######  IMTYPE  #############
print "*******   DEL/ADD  (imtype)  ***************"
val=imhead( input_file, 'get', 'imtype' )
imhead( input_file, 'del', 'imtype' )
newval=imhead( input_file, 'get', 'imtype' )
if ( val == newval ) :
    print "FAILED TEST: Del/Add imtype failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", "None"
imhead( input_file, 'add', 'imtype', val['value'] )


#######  USER DEFINED KEYWORD #############
###
### Add, get, put, and del a user defined
### keyword to the header.
print "*******   USER DEFINED KEYWORD  *********"

initVal="Testing the addition of user defined keyword"
imhead( input_file, 'add', 'test', initVal )
val=imhead( input_file, 'get', 'test', val )
if ( initVal != val['value'] ) :
    print "FAILED TEST: add/get user defined keayword failed"
    print "             Curent value:   ", val
    print "             Expected value: ", initVal

val2='Updated user keyword'
imhead( input_file, 'put', 'test', val2 )
newval=imhead( input_file, 'get', 'test' )
if ( val['value'] == newval['value'] ) :
    print "FAILED TEST: put/get user defined keyword failed"
    print "             Original value: ", val
    print "             Curent value:   ", newval
    print "             Expected value: ", val2

print 'Warning: The next operation will cause an exception.'
imhead( input_file, 'del', 'test')
newval=imhead( input_file, 'get', 'test' )
if ( newval != None ):
    print "FAILED TEST: del user defined keyword failed"
    print "             Curent value:   ", newval    
    print "             Expected:       exception to be thrown" 


##########  CHECK ORIG VALUES ############
# Check to see if we have the original
# values again.  All of the tests should
# change the values back to what they
# were intially.
print "***********       LIST      ******************"
print "EXECUTING: imhead( ", input_file, ", 'list' ) "
print ""
print "This test will verify that we have the image file back in"
print "its original stat."
print ""
cur_hdr=imhead( input_file, 'list' )

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
            results = orig_hdr[key] == orig_hdr[key]
            if ( results.all() ):
                match = True
        else:
            if ( cur_hdr[key]==orig_hdr[key] ):
                match = True

    if ( not match ):
        print "FAILED TEST: Header unchanged test -- current header and original header differ."
        print "             Curent value for key "+str(key)+" is:   ", cur_hdr[key]
        print "             Expected:                               ", orig_hdr[key]

## LIST values again.  They should be the same as at the start of the test
#imhead( input_file, 'list' )
#print "\nCheck that the values listed are the same as at the"
#print "beginning of the test"


##########  CLEAN UP ############
if ( os.path.isdir( input_file ) ):
    def errFunc( raiser, problemPath, execInfo ):
        casalog.post( raiser.__name__+' failed on '+problemPath, "SEVERE" )
        raise RuntimeError, "Unable to prepare test files"
    shutil.rmtree( input_file, 0, errFunc )

