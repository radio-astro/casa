import os
from taskinit import *
from imregion import *
from odict import odict
import numpy

# The function that handles the imval task.
def imval(imagename=None,region=None,box=None,chans=None,stokes=None):
    # Blank return value.f
    retValue = { 'blc':[], 'trc':[], 'unit': '', 'data': [], 'mask': []}

    try:
	casalog.origin('imval')
	ia.open(imagename)
	axes=getimaxes(imagename)

        # Get rid of any white space in the parameters
        region = region.replace(' ', '' )
        box    = box.replace( ' ', '' )
        chans  = chans.replace( ' ', '' )
        stokes = stokes.replace( ' ','' )

        # Default for the chans and stokes parameter is all when the
        # aren't given.  The easy way is to set them to -1, and let
        # the imageregion.py code handle it.
        if ( len(chans) < 1 ):
            chans='-1'
            
        if ( len(stokes) < 1 ):
            stokes='-1'
            

	# If the user hasn't specified any region information then
	# find the very last data point -- what ia.getpixelvalue        
	# defaults too.
	if ( len(box)<1 and len(chans)<1 and len(stokes)<1 and len(region)<1 ):
	    # Get the default  pixelvalue() at the referencepixel pos.
            csys=ia.coordsys()
            ref_values = csys.referencepixel()['numeric']
            point=[]
            for val in ref_values.tolist():
                point.append( int(round(val) ) )
            casalog.post( 'Getting data value at point '+str(point), 'NORMAL' )
	    results = ia.pixelvalue(point)
	    retValue = _imval_process_pixel( results, point )
	    ia.done()
	    retValue['axes']=axes
            casalog.post( 'imval task complete for point'+str(point), 'NORMAL1' )
	    return retValue

	# If the box parameter only has two value then we copy
        # them.  The imregion and other code expects for.  We
        # assume that if there are only 2 that it contains an
        # x (RA) and y (DEC) value only.
        if ( box.count(',') == 1 ):
            box = box + ','+ box

	# If we are finding the value at a single point this
	# is a special case and we use ia.getpixelvalue()
	#print "BOX: ", box
	#print "CHANS: ", chans
	#print "STOKES: ", stokes
	singlePt = _imval_get_single( box, chans, stokes, axes )
	#print "Single point is: ", singlePt, "   ", len(singlePt)
	if ( len( singlePt ) == 4 and singlePt.count( -1 ) < 1 ):
            casalog.post( 'Getting data value at point '+str(singlePt), 'NORMAL' )            
            
	    results = ia.pixelvalue( singlePt )
	    retValue = _imval_process_pixel( results, singlePt )
	    ia.done()
	    retValue['axes']=axes
            casalog.post( 'imval task complete for point '+str(singlePt), 'NORMAL1' )
	    return retValue

	# If we've made it here then we are finding the stats
	# of a region, which could be a set of single points.
	axes=getimaxes(imagename)
	statAxes=[]
	if ( len(box)>0 ):
	    statAxes.append(axes[0][0])
	    statAxes.append(axes[1][0])
	if ( len(chans)>0 ):
	    statAxes.append(axes[2][0])

        # If we get to this point and find that nothing was
        # given for the box parameter we use the reference
        # pixel values.
        if ( len(box) < 1 ):
            csys=ia.coordsys()
            ref_values = csys.referencepixel()['numeric']
            values = ref_values.tolist()
            box = str(int(round(values[axes[0][0]])))+','\
                  + str(int(round(values[axes[1][0]])))+',' \
                  + str(int(round(values[axes[0][0]])))+','\
                   +str(int(round(values[axes[1][0]])))

	# Get the region information, if the user has specified
	# a region file it is given higher priority.
	reg={}
	if ( len(region)>1 ):
	    if ( len(box)<1 or len(chans)<1 or len(stokes)<1 ):
		casalog.post( "Ignoring region selection\ninformation in"\
			      " the box, chans, and stokes parameters."\
			      " Using region information\nin file: " + region, 'WARN' );
	    reg=rg.fromfiletorecord( region );
	    #print "REGION from file: \n", reg
	else:
	    # Need to evaluate wheather we want dropdeg=True of False
	    # here.  Depends if users want the default to be the first
	    # slice/value on a partiuclar axis, or all values along that
	    # axis.
	    reg=imregion( imagename, chans, stokes, box, '', '', False )
	    #print "REGION from imregion: \n", reg
	    
	# Now that we know which axes we are using, and have the region
	# selected, lets get that stats!  NOTE: if you have axes size
	# greater then 0 then the maxpos and minpos will not be displayed

        # Make sure they are sorted first.
        reg['blc'].keys().sort()
        reg['trc'].keys().sort()        

        zeroblc=[]
        blc_keys=reg['blc'].keys()
        blc_keys.sort()        
        for key in blc_keys:
            value=reg['blc'][key]['value']
            if ( reg['oneRel'] ):
                #We need to conver to 0 relative!
                value=value-1
            if ( len( zeroblc ) > 0 ):
                zeroblc.append(str(value)+str(reg['blc'][key]['unit'])+',' )
            else:
                zeroblc.append(str(value)+str(reg['blc'][key]['unit']) )

        zerotrc=[]
        trc_keys=reg['trc'].keys()
        trc_keys.sort()
        for key in trc_keys:
            value=reg['trc'][key]['value']
            if ( reg['oneRel'] ):
                #We need to conver to 0 relative!
                value=value-1
            if ( len( zerotrc ) > 0 ):
                zerotrc.append(str(value)+str(reg['trc'][key]['unit'])+',' )
            else:
                zerotrc.append(str(value)+str(reg['trc'][key]['unit']) )

        casalog.post( 'Getting data values in region in blc='+str(zeroblc)+' trc='+str(zerotrc), 'NORMAL' )
        casalog.post( "Getting data values for region: "+str(reg), 'NORMAL2' )
	retValue = _imval_getregion( reg )
	retValue['axes']=axes
	#imval_print( retValue, imagename )


	# Cleanup
	del axes, reg
	ia.done()
	casalog.post( 'imval task complete for region bound by blc='+str(retValue['blc'])+' and trc='+str(retValue['trc']), 'NORMAL1' )
        return retValue
		
    except Exception, instance:
	casalog.post( str('*** Error ***'+str(instance), 'SEVERE' ))
	return retValue

    return retValue

#
# Print out the returned results.
# TODO -- finish this function
def _imval_print( values, name ) :
    print 'Results of imval task on ', name
    print ''
    print 'Region ---'
    print '\t --\x1B[94m bottom-left corner (pixel) [blc]:    \x1B[0m', list(values['blc'])
    print '\t --\x1B[94m top-right corner (pixel) [trc]:      \x1B[0m', list(values['trc'])

#
# Take the results from the ia.pixelvalue() function and
# the position given to the function and turn the results
# into the desired values; blc, trc, data, and mask
#
def _imval_process_pixel( results, point ):
    retvalue={}
    # First check that the results are a dictionary type and that
    # it contains the key/value pairs we expect.
    if ( not isinstance( results, dict ) ):
	casalog.post( "ia.pixelvalue() has returned erroneous data, Python dictionary type expectd.", "WARN" )
	casalog.post( "Value returned is: "+str(results), "SEVERE" )
	return retvalue
    
    if ( not results.has_key('mask') ):
	casalog.post( "ia.pixelvalue() has returned unexpected results, no mask value present.", "SEVERE" )
	return retvalue

    if ( not results.has_key('value') or not results['value'].has_key('unit') or not results['value'].has_key('value') ):
	casalog.post( "ia.pixelvalue() has returned unexpected results, data value absent or ill-formed.", "SEVERE" )
	return retvalue
    
    if ( results['mask'] ):
        retValue={'blc':point, 'trc': point, 'unit':results['value']['unit'],
                  'data': numpy.array([results['value']['value']]),
                  'mask': numpy.array([True]) }
    else:
        retValue={'blc':point, 'trc': point, 'unit':results['value']['unit'],
                  'data': numpy.array([results['value']['value']]),
                  'mask': numpy.array([False]) }

    return retValue

#
# Give the box, channel, and stokes values find out
# if we are getting the data from a single point in the
# image, if so then return that point.
def _imval_get_single( box, chans, stokes, axes ):
    # If we have more then one point then return an empty list.
    try: 
	junk=int(chans)
	junk=int(stokes)
    except:
        return []
    if ( box.count(';')>0 ):
        return []	    

    # If the channel wasn't specified use the first one only.
    if ( len( chans ) < 1 ):
        #chans=0
        return []

    # If the stokes values weren't specified use the first one only.
    if ( len( stokes ) < 1 ):
        #stokes=0
        return[]

    # The two x values and two y values need to be the same if its
    # a single point.  There may be only two value if its a single
    # point too.
    x=-1
    y=-1
    box=box.split(',')
    if ( len(box) == 2 ):
       x=int(box[0])
       y=int(box[1])
    elif ( len(box) == 4 and box[0]==box[2] and box[1]==box[3]):
        x=int(box[0])
        y=int(box[1])
    else:
        # We have more then one point, return empty list.
        return []

    retvalue=[-1,-1,-1,-1]

    retvalue[axes[0][0]]=x
    retvalue[axes[1][0]]=y
    retvalue[axes[2][0]]=int(chans)
    retvalue[axes[3][0]]=int(stokes)
    return retvalue

#
# Use the ia.getregion() function to construct the requested data.
def _imval_getregion( region={} ):
    try:
	retvalue= {}
	# Find the array of data and mask values.
	data_results=ia.getregion( region=region, dropdeg=True, getmask=False )
	mask_results=ia.getregion( region=region, dropdeg=True, getmask=True )
	
	# Find the bounding box of the region so we can report it back to
	# the user.
	bbox=ia.boundingbox( region=region )
	if ( not bbox.has_key( 'blc' ) ):
	    casalog.post( "ia.bounding() has returned unexpected results, blc value absent.", "SEVERE" )
	    return retvalue
	if ( not bbox.has_key( 'blc' ) ):
	    casalog.post( "ia.bounding() has returned unexpected results, blc value absent.", "SEVERE" )
	    return retvalue
	
	# Find what units the data values are stored in.
	avalue=ia.pixelvalue( bbox['blc'].tolist() )
	#print "A VALUE IS: ", avalue
	if ( not avalue.has_key('value') or not avalue['value'].has_key('unit') ):
	    casalog.post( "ia.pixelvalue() has returned unexpected results, data value absent or ill-formed.", "SEVERE" )
	    return retvalue
	
	retvalue={'blc':bbox['blc'].tolist(), 'trc':bbox['trc'].tolist(), 'unit':avalue['value']['unit'], 'data':data_results, 'mask': mask_results}
    except Exception, instance:
	casalog.post( str('*** Error ***'+str(instance), 'SEVERE' ))
	return retvalue

    return retvalue
