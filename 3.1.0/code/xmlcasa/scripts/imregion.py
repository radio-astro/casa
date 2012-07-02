#
# This file was generated using xslt from its XML file
#
# Copyright 2007, Associated Universities Inc., Washington DC
#
# TODO
# Add support for Linear coordinates
# 
import sys
import os
import string
import inspect
from taskinit import *
from odict import odict
from parameter_check import *
import re

def imregion(imagename='', spectral='', stokes='', box='', poly='', circ='', dropExtra=False):
        """Helper task for seleting regions in images
        
           The end user should never see this task, and this
           code should move to C++ once a comfortable region
           selection as been agreed upon.
        """

        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
            if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
        myf=sys._getframe(stacklevel).f_globals
        ####local params
        cs=myf['cs']
        rg=myf['rg']
        casalog=myf['casalog']

        # strip out any spaces in box, CAS-2050
        box = re.sub(' ', '', box)
        ###
        #Add type/menu/range error checking here
        arg_names=['imagename','spectral','stokes','box','poly','circ']
        arg_values=[imagename,spectral,stokes,box,poly,circ]
        arg_types=[str,(list,str),str,str,str,str,str]
        #parameter_printvalues(arg_names,arg_values,arg_types)
        try:
            parameter_checktype(arg_names,arg_values,arg_types)
        except TypeError, e:
            print "imregion -- TypeError: ", e
            return None
        except ValueError, e:
            print "imregion -- OptionError: ", e
            return None


        # Now we get to the heart of the script!!!
        retValue={}
        myia = iatool.create()
        try:
            # Find where the directional, stokes, and spectral
            # axies are.
            axes=getimaxes(imagename)
                
            # Initialize the coordinate system for the region
            # we are about to create
            csys = None
            myia.open( imagename )
            csys=myia.coordsys()

            # Find where the directional and channel axies are
            # Save the internal placement of the axies in a list
            # (which will be in the following order:
            #    direction1: RA, Longitude
            #    direction2: DEC, Lattitude
            #    spectral: chan~chan;>chan;<chan;chan,chan,chan
            #    stokes: I or V
            axisTypes=csys.axiscoordinatetypes()
            axisNames=csys.names()
            stokesValues=[]
            if ( len(axes[3][1] ) > 0  ):
                stokesValues=csys.stokes()

            # Warn user if they've given a selection for an axis
            # that doesn't exist.
            if ( len(stokes) > 0 and axes[3][0]=='' ):
                casalog.post( 'Image file '+imagename+" doesn't have a stokes axis, but stokes values given.", 'WARN' )

            if ( len(spectral) > 0 and axes[2][0]=='' ):
                casalog.post( 'Image file '+imagename+" doesn't have a spectral axis, but spectral values given.", 'WARN' )

            #TODO HANDLE a SINGLE DIRECTIONAL AXIS
                


            # Get the min/max values for each axis, in pixels
            # This will be used as the default value if the user
            # doesn't supply one.
            # TODO use world coordiates values, when wbox and wpolygon
            # work.  The min/max values will be in blcf, and trcf
            
            fullSize=myia.boundingbox(myia.setboxregion())
            mins=fullSize['blc']
            maxs=fullSize['trc']

            # When extracting the coordinates from the user input
            # that if no units were given that values given are
            # pixel values. Also note that a ! at the end of a
            # region indicates that the region is to be not selected.
            # Initialize our list of regions to start.
            regions={}

            # Loop through the list of boxes selecting the region
            # as specified by the user. ';' separates box specifiations
            # ',' separtes the coordinate positions.  Boxes are
            # specified in blc, trc coordinats.
            boxes=[];pols=[];spects=[];
            if ( len(box) > 0 ): boxes=box.split(';')

            pols=[]
            if ( len(stokes) > 0 ):
                if ( stokes.count(',') > 0 ):
                    pols=stokes.split(',');
                else:
                    # We have a string of values that we want toe
                    # make into a list.
                    pols=_make_pol_list( stokes, stokesValues )
            pols=_sort_pol_list( pols, stokesValues )
            
            tmpSpects=[]
            spects=[]
            if ( len(spectral) > 0 ):
                if ( spectral.count( ';' ) < 1 ):
                    tmpSpects=spectral.split(',')
                else:
                    tmpSpects=spectral.split(';')

                for i in range( len(tmpSpects) ):
                    chanSet=tmpSpects[i].split(',')
                    if ( len( chanSet ) == 1 ):
                        spects.append( chanSet[0] )
                    else:
                        for j in range( len( chanSet ) ):
                            spects.append( chanSet[j] )

            numNewRegions=1
            if ( len(boxes) > 0 ):
                numNewRegions*=len(boxes)
            if ( len(pols) > 0 and axes[3][0]!='' ):
                numNewRegions*=len(pols)
            if ( len(spects) > 0 and axes[2][0]!='' ) :
                numNewRegions*=len(spects)

            for regIndex in range( numNewRegions ):
                # Initialze the bottom-left and top-right coordinates and indexes
                blc=[]
                trc=[]
                for j in range( len( axes ) ):
                    if axes[j][0]=='':
                        # This axes was not found so keep don't add it
                        continue;
                    blc.append('')
                    trc.append('')

                if ( len(boxes) > 0 ):
                    boxIndex = regIndex % len(boxes)
                else:
                    boxIndex = -1;

                if ( len(spects) > 0 and axes[2][0]!=''):
                    specIndex = regIndex % len(spects)
                else:
                    specIndex = -1;

                if ( len(pols) > 0 and axes[3][0]!='' ):
                    polIndex = regIndex % len(pols)
                else:
                    polIndex = -1;
                    
                if ( boxIndex >= 0 ):
                    # Strip of any leading/trailing square braces.
                    theBox = boxes[boxIndex];
                    if ( theBox[0]=='[' ):
                        theBox=theBox[1:]
                    if  ( theBox[len(theBox)-1]==']' ):
                        theBox=theBox[0:-1]
                    coords = theBox.split(',')
                    if ( len( coords ) != 4 ) :
                        raise Exception, "Illegal box coordinates: "+box+". Expected four values representing the\n\tbottom-left corner and the top-right corner"
                        continue

                    # If the coord. is an int type then its a pixel value.
                    # and we need to add the units.
                    # TODO add a check that they are all the same units
                    # CURRENTLY ONLY SUPPORT PIXEL VALUES!
                    if ( not _imregion_units_given(coords) ):
                        #for coordIndex  in range( len( coords ) ):
                        #    coords[coordIndex]+='pix'
                        no_op='noop'
                    else:
                        raise Exception, "Sorry! Coordinates are accepted in pixel values only"

                    if ( int(coords[0]) >= mins[axes[0][0]] and
                         int(coords[0]) <= maxs[axes[0][0]] ) :
                        blc[axes[0][0]]=int(coords[0])
                    elif( int(coords[0]) == -1 ):
                        blc[axes[0][0]] = mins[axes[0][0]]
                    else:
                        raise Exception, "Out of range box pixel coordinate: " \
                             + coords[0] + ".\nExpected values in the range " \
                             + str(mins[axes[0][0]]) + " to " \
                             + str(maxs[axes[0][0]])
                            
                    if ( int(coords[1]) >= mins[axes[1][0]] and
                         int(coords[1]) <= maxs[axes[1][0]] ):
                        blc[axes[1][0]]=int(coords[1])
                    elif( int(coords[1]) == -1 ):
                        blc[axes[1][0]] = mins[axes[1][0]]
                    else:
                        raise Exception, "Out of range box pixel coordinate: " \
                             + coords[1] + ".\nExpected values in the range " \
                             + str(mins[axes[1][0]]) + " to " \
                             + str(maxs[axes[1][0]])
                            
                    if ( int(coords[2]) >= mins[axes[0][0]] and
                         int(coords[2]) <= maxs[axes[0][0]] ):
                        trc[axes[0][0]]=int(coords[2])
                    elif( int(coords[2]) == -1 ):
                        trc[axes[0][0]] = maxs[axes[0][0]]
                    else:
                        raise Exception, "Out of range box pixel coordinate: " \
                             + coords[2] + ".\nExpected values in the range " \
                             + str(mins[axes[0][0]]) + " to "\
                             + str(maxs[axes[0][0]])

                    if ( len(stokesValues) > 0 ):                            
                        if ( int(coords[3]) >= mins[axes[1][0]] and
                             int(coords[3]) <= maxs[axes[1][0]] ):
                            trc[axes[1][0]]=int(coords[3])
                        elif( int(coords[2]) == -1 ):
                            trc[axes[1][0]] = maxs[axes[1][0]]
                        else:
                            raise Exception, "Out of range box pixel coordinate: " \
                                + coords[3] + ".\nExpected values in the range " \
                                + str(mins[axes[1][0]]) + " to "\
                                + str(maxs[axes[1][0]])
                elif ( not dropExtra ):
                    blc[axes[0][0]]=mins[axes[0][0]]
                    blc[axes[1][0]]=mins[axes[1][0]]
                    trc[axes[0][0]]=maxs[axes[0][0]]
                    trc[axes[1][0]]=maxs[axes[1][0]]
                else:
                    # We don't want to keep this axis so don't add it.
                    noop='noop'                    

                if ( ( specIndex < 0 or axes[2][0]=='' )  and dropExtra):
                    # We don't want to keep this axis so don't add it.
                    noop='noop'
                elif ( specIndex >= 0 and axes[2][0]!=''):
                    #Parse the spectral values, we are
                    # expecting to get a min and max value.
                    values=__parse_spectral(spects[specIndex],str(mins[axes[2][0]]),str(maxs[axes[2][0]]) )
                    #print "SPECTRAL VALS: ", values
                    if ( values[0] == -1 ):
                        blc[axes[2][0]]=mins[axes[2][0]]
                    else:
                        blc[axes[2][0]]=values[0]

                    if ( values[1] == -1 ):
                        trc[axes[2][0]]=maxs[axes[2][0]]
                    else:
                        trc[axes[2][0]]=values[1]
                elif ( axes[2][0]!='' ):
                    blc[axes[2][0]]=mins[axes[2][0]]
                    trc[axes[2][0]]=maxs[axes[2][0]]                    

                if ( ( polIndex < 0 or axes[3][0]=='' ) and dropExtra ):
                    # We don't want to keep this axis so don't add it.
                    noop='noop'
                elif ( polIndex >= 0 and axes[2][0]!=''  ):
                    # Parse to get this into a pixel value.
                    # Note that we could do this more efficiently,
                    # we could make a region that inlcudes a range of
                    # polarization/stokes values rather then a single value.
                    polVals=_parse_stokes( stokesValues, pols[polIndex],mins[axes[3][0]],maxs[axes[3][0]] )

                    if ( len(stokesValues) > 0 and polVals[0] == -1 ):
                        blc[axes[3][0]]=mins[axes[3][0]]
                    else:
                        blc[axes[3][0]]=polVals[0]
                    if ( polVals[1] == -1 ):                        
                        trc[axes[3][0]]=maxs[axes[3][0]]
                    else:
                        trc[axes[3][0]]=polVals[1]

                elif ( len(stokesValues) > 0 and axes[3][0]!='' ):
                    blc[axes[3][0]]=mins[axes[3][0]]
                    trc[axes[3][0]]=maxs[axes[3][0]]
      
                # <KLUDGE> (but then isn't all this code?) CAS-1910. I have no idea where this is supposed
                # to go in the above, um, code so the best I can do is to put a check here and fix things
                # the best as I can. At best, it won't break anything.
                if (type(blc[1]) == int and trc[1] == ''):
                    # we missed setting the trc[1] value above so just stick it in here
                    trc[1] = coords[3]
                # </KLUDGE>
  
                # Create a strings for the wbox command.
                # For now we assume they are all pixel values, because
                # this is all we allow.
                blcStr=''
                for i in range( len(blc) ):
                    if (blc[i] > trc[i]):
                        raise Exception, "All elements of blc " + str(blc) + " must be less than or equal to corresponding elements of trc " + str(trc) + ". Element number " + str(i) + " is not."
                    if ( i > 0 ):
                        blcStr+=','
                    if ( isinstance( blc[i], str ) ):
                        blcStr+=blc[i]+"pix"
                    else:
                        blcStr+=str(blc[i])+"pix"

                trcStr=''
                for i in range( len(trc) ):
                    if ( i > 0 ):
                        trcStr+=','
                    if ( isinstance( trc[i], str ) ):
                        trcStr+=trc[i]+"pix"
                    else:
                        trcStr+=str(trc[i])+"pix"

                #print "BLC STR: ", blcStr
                #print "TRC STR: ", trcStr
                #tmpRegion=rg.box( blc=blc, trc=trc )
                tmpRegion=rg.wbox( blc=blcStr, trc=trcStr, csys=csys.torecord() )
                regions["region"+str(len(regions))]=tmpRegion
                casalog.post( "Created Box at: "+str(blc)+"  "+str(trc),'DEBUGGING','imregion')
                
            # Loop through the list of boxes selecting the region
            # as specified by the user. ';' separates box specifiations
            # ',' separtes the coordinate positions.  Boxes are
            # specified in blc, trc coordinates.
            polys=[];
            if ( len(poly) > 0 ): polys=poly.split(';')
            if ( len(polys)>0 and (len(pols)>0 or len(spects)>0 ) ):            
                casalog.post( 'Sorry! Polygon region supports selection on the directional axes only', 'WARNING', 'imregion')

            for regIndex in range( len(polys) ):
                # Loop through the list of values, breaking them into
                # a list of x and a list of y values.
                current=polys[regIndex]
                xVals=''
                yVals=''
                for coord in range( 1, len(current), 2 ):
                    if ( coord > 1 ):
                        xVals+=','
                        yVals+=','
                    if ( not current[coord].isdigit() ):
                        xVals+=current[coord]+"pix"
                    else:
                        xVals+=current[coord]
                    if ( not current[coord+1].isdigit() ):
                        yVals+=current[coord+1]+"pix" 
                    else:
                        yVals+=current[coord+1]

                tmpRegion=rg.wpolygon( xVals, yVals, csys=csys.torecord() )
                regions["region"+str(len(rgs))]=tmpRegion
                casalog.post("Created polygon region with: "+xVals+"\n\t"+YVals,
                             'DEBUGGING', 'imregion' );
                

            # Loop through the list of circles selecting the region
            # as specified by the user. ';' separates box specifiations
            # ',' separtes the coordinate positions.  Boxes are
            # specified in blc, trc coordinates.
            if ( len(circ) > 0 ):
                casalog.post( "Sorry, circle selection is not supported yet!", "WARNING", 'imregion' )

            # Cleanup time
            if ( csys != None ):
                del csys
            if ( myia.isopen() ):
                myia.close()
                
            if ( len(regions) >  1 ):
                retValue = rg.makeunion( regions )
            else:
                retValue=regions['region0']
            return retValue

        except Exception, instance:
            print '*** Error ***',instance
            myia.done()
            return         

#
#
def __parse_spectral( spectString='', min=0, max=0 ):
    retValue=[ min, max ]
    #print "min: ", min
    #print "max: ", max

    values=spectString.split('~')
    if ( len(values)==2 ):
        # We have a min and max value
        if ( values[0] == '-1' ):
            retValue[0]=min
        else:
            retValue[0]=values[0]
        if (values[1] == '-1' ):
            retValue[1]=max
        else:
            retValue[1]=values[1]
    elif ( len(values)==1 ):
        # We probably have a <, <=, > or >= sign
        if ( values[0].startswith( '<=') ):
            retValue[0]=min
            retValue[1]=values[0][2:]
        elif ( values[0].startswith( '<') ):
            retValue[0]=min
            ## NOTE when we support non-pixel values this will need
            ## to change!!! We are assuming things will fail later
            ## if the return values aren't integers, which non-pixel
            ## values won't be integers.
            retValue[0]=min
            retValue[1]=values[0][1:]
            if ( retValue[1].isdigit() ):
                retValue[1] = str( int(retValue[1])-1 )
        elif( values[0].startswith( '>=') ):
            retValue[0]=values[0][2:]
            retValue[1]=max            
        elif( values[0].startswith( '>') ):
            ## NOTE when we support non-pixel values this will need
            ## to change!!! We are assuming things will fail later
            ## if the return values aren't integers, which non-pixel
            ## values won't be integers.
            retValue[0]=values[0][1:]
            if ( retValue[0].isdigit() ):
                retValue[0] = str( int(retValue[0])+1 )
            retValue[1]=max
        elif( values[0]== '-1' ):
            retValue[0]=min
            retValue[1]=max
        else:
            # We have a single spectral value.
            try:
                value=int(values[0])
            except:
                raise Exception, "Invalid spectral specification "\
                      +str( values[0] )                          
            if ( int(values[0]) >= int(min) and int(values[0]) <= int(max) ):
                retValue[0]=retValue[1]=values[0]
            else:
                raise Exception, "Invalid spectral specification "\
                  +str( values[0] )
            return retValue                
    else:
        raise Exception, "Invalid spectral coordinate value: "+str(values)
        return retValue
    
    ## Do a sanity checks to make sure we are in the proper range.
    ## TODO add warning's when we adjust to min or max value.
    if ( int(retValue[1]) > int(max) ):
        retValue[1] = str(max);
    if ( int(retValue[0]) < int(min) ):
        retValue[0]= str(min);
    if ( int(retValue[0]) > int(max) or
         int(retValue[1]) < int(min) or
         int(min) > int(max) ):
        raise Exception, "The specified channel range is invalid: " + str(retValue[0]) + " to " + str(retValue[1]) + ".\nExpected values in the range " + str(min) + "to" + str(max)

    if ( not retValue[0].isdigit() or not retValue[1].isdigit() ):
        print "Sorry! Only pixel values supported at this time."+"\n\t Values given were: "+str(retValue)
        raise Exception, "Sorry! Only pixel values supported at this time."+"\n\t Values given were: "+str(retValue)

    if ( retValue[0].isdigit() ):
        #retValue[0]+='pix'
        retValue[0]=int(retValue[0])
    if ( retValue[1].isdigit() ):
        #retValue[1]+='pix'
        retValue[1]=int(retValue[1])

    
    # If we make it here all is good!!!
    return retValue

#
# Convert a string of Polarzation values into a list of 
def _make_pol_list( stokesStr='', stokesValues=[] ):
    retValue=[]
    #stokesValues=['I','Q','U','V','RR','RL','LR','LL','XX','XY','YX','YY','RX',
    #                  'RY','LX','LY','XR','XL','YR','YL','PP','PQ','QP','QQ',
    #                  'RCircular','LCircular','Linear','Ptotal','Plinear',
    #                  'PFtotal','PFlinear','Pangle']

          
    if ( len(stokesValues) < 0 ):
         #We have no stokes values so we have nothing to do
         return retValue
    i=0;
    stokesStr = stokesStr.strip()
    while ( i < len(stokesStr) ):
        # First try and find the 2 character long polarization value
        # in our list of stokes values, if its not found then try a
        # single character, and if that's not found raise an exception

        twoLetter=stokesStr[i:i+2]
        oneLetter=current=stokesStr[i:i+1]
        if ( oneLetter.isdigit() ) :
            retValue.append(oneLetter)
            i=i+1
        elif( twoLetter == '-1' ):
            retValue.append( '-1' )
            i=i+2
        elif ( stokesValues.count( twoLetter ) > 0 ):
            retValue.append(twoLetter)
            i=i+2
        elif ( stokesValues.count( oneLetter ) > 0 ):
            retValue.append(oneLetter)
            i=i+1
        else:
            raise Exception, "Invalid Stokes value found in: " + stokesStr

    return retValue

#
# Sort the list of polarizations based on the ordering in
# measure/implement/Measures/Stokes.h
def _sort_pol_list( polList=[], stokesValues=[] ):
    retValue=polList

    # The known stokes values in the same order as the enum in
    #   measures/implement/Measures/Stokes.h
    #stokesValues=['I','Q','U','V','RR','RL','LR','LL','XX','XY','YX','YY','RX',
    #                  'RY','LX','LY','XR','XL','YR','YL','PP','PQ','QP','QQ',
    #                  'RCircular','LCircular','Linear','Ptotal','Plinear',
    #                  'PFtotal','PFlinear','Pangle']
    
    # Need to sort the list we are returning according to the order in the
    # list of stokes values. We'll just do a bubble sort since it is likely
    # the list of stokes values returned is never greater then 4.
    numNumerics=0
    for i in range( len(retValue)-1, 0, -1 ):

        for j in range(0, i):
            if ( retValue[j].isdigit() ):
                numNumerics=numNumerics+1
                continue
            jOrdNum=stokesValues.index( retValue[j] )
            jOrdNumPlusOne=stokesValues.index( retValue[j+1] )
            if ( jOrdNum > jOrdNumPlusOne ):
                # Swap values.
                tmp = retValue[j]
                retValue[j]=retValue[j+1]
                retValue[j+1]=tmp

    # If we have a numerical list then use internal sort
    # methods.
    if ( numNumerics == len( retValue ) ):
        retValue.sort()
    return retValue

#
# Parse a Stokes/polarization value.
def _parse_stokes( stokesValues=[], stokesStr='',min=0, max=0 ):
    retValue=[min, max]
    stokesStr=stokesStr.upper()

    # The known stokes values in the same order as the enum in
    #   measures/implement/Measures/Stokes.h
    #stokesValues=['I','Q','U','V','RR','RL','LR','LL','XX','XY','YX','YY','RX',
    #                  'RY','LX','LY','XR','XL','YR','YL','PP','PQ','QP','QQ',
    #                  'RCircular','LCircular','Linear','Ptotal','Plinear',
    #                  'PFtotal','PFlinear','Pangle']

    if ( stokesStr.isdigit() ):
        retValue[0]=retValue[1]=int(stokesStr)
    elif( stokesStr == '-1' ):
            retValue[0]=min
            retValue[1]=max
    else:
        if ( stokesValues.count( stokesStr ) < 1 ):
            raise Exception ,"Invalid stokes values: " + stokesStr
        else:
            retValue[0]=retValue[1]=stokesValues.index(stokesStr)
    return retValue
    

# Check to see if there are any units given
# Returns True if there are units with at
# least one of the values.
#
# TODO verify that all values have the same unit, but
#      may not be a desired check.
def _imregion_units_given( valueList=[] ):
    retValue=False
    for index in range( len(valueList) ):
        if ( valueList[index].find('-') == 0 or valueList[index].find('+') == 0 ):
            value=valueList[index][1:]
            if (not value.isdigit() ):
                retValue=True
        elif ( not valueList[index].isdigit() ):
            retValue=True;
            
    return retValue
                

#
#
#
def _imregion_defaults(param=None):
        a=inspect.stack()
        stacklevel=0
        for k in range(len(a)):
          if (string.find(a[k][1], 'ipython console') > 0):
                stacklevel=k
                break
        myf=sys._getframe(stacklevel).f_globals
        a = odict()
        a['imagename']  = ''
        a['regions']  = ''

        a['async']=False

### This function sets the default values but also will return the list of
### parameters or the default value of a given parameter
        if(param == None):
                myf['__set_default_parameters'](a)
        elif(param == 'paramkeys'):
                return a.keys()
        else:
                if(a.has_key(param)):
                        return a[param]

#
#
def _imregion_check_params(param=None, value=None):
    a=inspect.stack() 
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'ipython console') > 0):
            stacklevel=k
            break
    myf=sys._getframe(stacklevel).f_globals

    return myf['cu'].verifyparam({param:value})
#
#
def _imregion_description(key='imregion', subkey=None):
        desc={'imregion': 'Helper task for seleting regions in images',
               'imagename': 'Name of the input image',
               'regions': 'Either the filename containing the region information,a list of region specifications.'
              }

#
# Set subfields defaults if needed
#

        if(desc.has_key(key)) :
           return desc[key]

