import os
from taskinit import *
from odict import odict
import numpy

# The function that handles the imval task.
def imval(imagename, region, box, chans, stokes):
    myia = iatool()
    mycsys = cstool()
    try:
        # Blank return value.
        retValue = { 'blc':[], 'trc':[], 'unit': '', 'data': [], 'mask': []}
        casalog.origin('imval')
    
        try:
            axes=getimaxes(imagename)
        except:
            raise Exception, "Unable to determine the axes of image: "+imagename
        
    
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
            try:
                myia.open(imagename)
    
                # Get the default  pixelvalue() at the referencepixel pos.
                csys=myia.coordsys()
                ref_values = csys.referencepixel()['numeric']
                point=[]
                for val in ref_values.tolist():
                    point.append( int(round(val) ) )
                casalog.post( 'Getting data value at point '+str(point), 'NORMAL' )
                results = myia.pixelvalue(point)
    
                retValue = _imval_process_pixel( results, point )
                retValue['axes']=axes
                casalog.post( 'imval task complete for point'+str(point), 'NORMAL1' )
                return retValue
            except Exception, instance:
                raise Exception, instance
            finally:
                myia.done()
    
        # If the box parameter only has two value then we copy
        # them.  
        if ( box.count(',') == 1 ):
            box = box + ','+ box
    
        # If we are finding the value at a single point this
        # is a special case and we use ia.getpixelvalue()
    
        singlePt = _imval_get_single( box, chans, stokes, axes )
        if ( len( singlePt ) == 4 and singlePt.count( -1 ) < 1 ):
            try:
                casalog.post( 'Getting data value at point '+str(singlePt), 'NORMAL' )
                myia.open( imagename )
                results = myia.pixelvalue( singlePt )
                retValue = _imval_process_pixel( results, singlePt )
                retValue['axes']=axes
                casalog.post( 'imval task complete for point '+str(singlePt), 'NORMAL1' )
                return retValue
            except Exception, instance:
                raise Exception, instance
            finally:
                myia.done()
            
            
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
        myia.open(imagename)
        mycsys = myia.coordsys()  

        if ( len(box) == 0 and len(region) == 0): 
            ctypes = mycsys.axiscoordinatetypes()
            ndir = 0
            nlin = 0
            for ctype in ctypes:
                if ctype == 'Direction':
                    ndir += 1
                elif ctype == 'Linear':
                    nlin += 1
            if ndir == 2 or nlin == 2:
                try:
                    ref_values = mycsys.referencepixel()['numeric']
                    values = ref_values.tolist()
                    box = str(int(round(values[axes[0][0]])))+','\
                        + str(int(round(values[axes[1][0]])))+',' \
                        + str(int(round(values[axes[0][0]])))+','\
                        +str(int(round(values[axes[1][0]])))
                except:
                    raise Exception, "Unable to find the size of the input image."
            
        # Because the help file says -1 is valid, apparently that's supported functionality, good grief
        
        if box.startswith("-1"):
            box = ""
        if chans.startswith("-1"):
            chans = ""
        if stokes.startswith("-1"):
            stokes = ""
        reg = rg.frombcs(
            mycsys.torecord(), myia.shape(), box, chans,
            stokes, "a", region
        )
        
    
        # Now that we know which axes we are using, and have the region
        # selected, lets get that stats!  NOTE: if you have axes size
        # greater then 0 then the maxpos and minpos will not be displayed
        if ( reg.has_key( 'regions' ) ):
            casalog.post( "Complex region found, only processing the first"\
                          " SIMPLE region found", "WARN" )
            reg=reg['regions']['*1']
        retValue = _imval_getregion( imagename, reg )
        retValue['axes']=axes
    
        casalog.post( 'imval task complete for region bound by blc='+str(retValue['blc'])+' and trc='+str(retValue['trc']), 'NORMAL1' )
        return retValue
    except Exception, instance:
        casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        raise
    finally:
        myia.done()    
        mycsys.done() 
                
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
    
    retValue={
        'blc':point, 'trc': point, 'unit':results['value']['unit'],
        'data': numpy.array([results['value']['value']]),
        'mask': numpy.array([results['mask']])
    }
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
def _imval_getregion( imagename, region):
    retvalue= {}
    myia = iatool()
    try:
        # Open the image for processing!
        myia.open(imagename)
        # Find the array of data and mask values.
        data_results=myia.getregion( region=region, dropdeg=True, getmask=False )
        mask_results=myia.getregion( region=region, dropdeg=True, getmask=True )
                
        # Find the bounding box of the region so we can report it back to
        # the user.
        bbox = myia.boundingbox( region=region )
        
        if ( not bbox.has_key( 'blc' ) ):
            casalog.post( "ia.boundingbox() has returned unexpected results, blc value absent.", "SEVERE" )
            myia.done()
            return retvalue
        if ( not bbox.has_key( 'trc' ) ):
            casalog.post( "ia.boundingbox() has returned unexpected results, trc value absent.", "SEVERE" )
            myia.done()
            return retvalue
        
        # get the pixel coords
        mycsys = myia.coordsys()
        myarrays = _imval_iterate(bbox['blc'], bbox['trc'])
        mycoords = mycsys.toworldmany(myarrays)
        outcoords = _imval_redo(data_results.shape, mycoords['numeric'])
        
        avalue = myia.pixelvalue( bbox['blc'].tolist() )
        if ( not avalue.has_key('value') or not avalue['value'].has_key('unit') ):
            casalog.post( "ia.pixelvalue() has returned unexpected results, data value absent or ill-formed.", "SEVERE" )
            myia.done()
            return retvalue

        retvalue={
            'blc':bbox['blc'].tolist(),'trc':bbox['trc'].tolist(),
            'unit':avalue['value']['unit'], 'data':data_results,
            'mask': mask_results, 'coords': outcoords
        }
    except Exception, instance:
        raise instance
    finally:
        myia.done()
    return retvalue

def _imval_iterate(begins, ends, arrays=None, place=None, depth=0, count=None):
    if (depth == 0):
        count = [0]
        mylist = []
        diff = numpy.array(ends) - numpy.array(begins) + 1
        prod = diff.prod()
        for i in range(len(begins)):
            mylist.append(numpy.zeros([prod]))
        arrays = numpy.array(mylist)
    for i in range(begins[depth], ends[depth] + 1):
        if (depth == 0):
            tmpplace = []
            for j in range(len(begins)):
                tmpplace.append(0)
            place = numpy.array(tmpplace)
        place[depth] = i
        if (depth == len(begins) - 1):
            for k in range(depth + 1):
                arrays[k][count[0]] = place[k]
            count[0] = count[0] + 1
        else:
            mydepth = depth + 1
            _imval_iterate(begins, ends, arrays, place, mydepth, count)
    return arrays

def _imval_redo(shape, arrays):
    list_of_arrays = []
    for x in range(arrays[0].size):
        mylist = []
        for arr in arrays:
            mylist.append(arr[x])
        list_of_arrays.append(numpy.array(mylist))
    array_of_arrays = numpy.array(list_of_arrays)
    # because shape is an immutable tuple
    newshape = list(shape)
    newshape.append(array_of_arrays.shape[1])
    return array_of_arrays.reshape(newshape)

