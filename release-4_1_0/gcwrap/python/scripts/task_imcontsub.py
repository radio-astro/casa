import os
import re
from taskinit import *

from odict import odict

def imcontsub(imagename=None,linefile=None,contfile=None,fitorder=None,region=None,box=None,chans=None,stokes=None,xstat=None):

    casalog.origin('imcontsub')
    filesExist=False
    if ( len( linefile ) > 0 ):
        if ( os.path.exists( linefile ) ):
            casalog.post('Error: file ' + linefile
                          +' already exists, please delete before continuing.',\
                          'SEVERE' )
            filesExist=True
    else:
        casalog.post("The linefile parameter is empty, consequently the"
                      +" spectral line image will NOT be\nsaved on disk.", \
                      'WARN')
            
    if ( len( contfile ) > 0 ):
            if ( os.path.exists( contfile ) ):
                casalog.post( 'Error: Unable to continue file '+contfile\
                              +' already exists, please delete before continuing.',\
                          'SEVERE' )
                filesExist=True
    else:
        casalog.post("The contfile parameter is empty, consequently the"
                      +" continuum image will NOT be\nsaved on disk.", \
                      'WARN')
    if ( filesExist ):
        return False
    
    _myia = iatool()
    _myia.open(imagename)
    mycsys = _myia.coordsys()

    if isinstance(box, list):
        box = ', '.join([str(b) for b in box])

    # Don't mix chans up with reg!  reg selects a subset for output, and chans
    # selects a subset to define the line-free channels.
    myrg = rgtool()
    reg = myrg.frombcs(csys=mycsys.torecord(), shape=_myia.shape(),
                     box=box, stokes=stokes, stokescontrol="f",
                     region=region)
    channels = []
    if chans != None and len(chans) > 0:
        channels = myrg.selectedchannels(chans, _myia.shape())
    
    try:
        # Now do the continuum subtraction.
        _myia.continuumsub(outline=linefile, outcont=contfile, region=reg,
                           channels=channels, fitorder=fitorder, overwrite=False)
        return True
                
    except Exception, err:
        casalog.post( 'Error: Unable to perform continuum subtraction'+str(err), 'SEVERE' )
        raise
    finally:
        _myia.done()
        if ( reg != None ):
            del reg
        
    return True

#
"""
#TODO add a try/catch block or type checking to make
#sure all channel values are ints.
def _parse_chans( chanString='', min=0, max=0 ):
    retValue=[]
    startChan=min;
    endChan=max;

    values=chanString.split('~')
    if ( len(values)==2 ):
        # We have a min and max value
        startChan=int(values[0])
        endChan=int(values[1])
    elif ( len(values)==1 ):
        # We probably have a <, <=, > or >= sign
        if ( values[0].startswith( '<=') ):
            endChan=int(values[0][2:])
        elif ( values[0].startswith( '<') ):
            endChan=int(values[0][1:])-1
        elif( values[0].startswith( '>=') ):
            startChan=int(values[0][2:])
        elif( values[0].startswith( '>') ):
            startChan=int(values[0][1:])+1
        elif( values[0]== '-1' ):
            startChan = min
            endChan   = max
        elif ( values.count( ',' ) > 0 ):
            # We have a list of specific channel numbers
            startChan=endChan=-1
            tmpList = chanString.split( ',' )
            for j in range( len( tmpList ) ):
                retValue.append( int( tmpList[j] ) )
        else:
            # We have a single value
            startChan=int(values[0])
            endChan=int(values[0])

    if ( startChan >= 0 and endChan >= 0 and \
         startChan <= int(max) and endChan <= int(max) ):
        for i in range( startChan, endChan+1 ):
            retValue.append( i )
    else:
        raise Exception, "Invalid channel specification: "+str(values)
    
    return retValue
"""
