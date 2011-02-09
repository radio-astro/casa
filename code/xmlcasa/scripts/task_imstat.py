import os
from taskinit import *
from imregion import *

from odict import odict

def imstat(imagename=None, region=None, box=None, chans=None, stokes=None, listit=None, verbose=None):
        retValue = {}
        _myia = iatool.create()
        try:
            casalog.origin('imstat')
            _myia.open(imagename)
            mycsys = _myia.coordsys()
            reg = rg.frombcs(
                mycsys.torecord(), _myia.shape(), box, chans,
                stokes, "f", region
            )
            """
            # If the user hasn't specified any region information then
            # generate statistical information for the entire image.
            if ( len(box)<1 and len(chans)<1 and len(stokes)<1 and len(region)<1):
                # Get stats for whole image
                retValue = ia.statistics( robust=True, list=listit, verbose=verbose )
                ia.close()
                #myf['xstat']=retValue
                return retValue
                #retValue = {'xstat':xstat }
                #return retValue
            
            # If we've made it here then we are finding the stats
            # of a particular region in the image.
            axes=getimaxes(imagename)
            statAxes=[]
            #if ( len(box)>0 or len(poly)>0 ):
            if ( len(box)>0 ):
                statAxes.append(axes[0][0])
                statAxes.append(axes[1][0])
            if ( len(chans)>0 ):
                statAxes.append(axes[2][0])


            # Get the region information, if the user has specified
            # a region file it is given higher priority.
            reg={}
            if ( len(region)>1 ):
                if ( ( len(box)>1 and box!='-1' )
                     or ( len(chans)>1 and chans!='-1' )
                     or ( len(stokes)>1 and stokes !='-1' ) ):
                    casalog.post( "Ignoring region selection\ninformation in"\
                                  " the box, chans, and stokes parameters."\
                                  " Using region information\nin file: " + region, 'WARN' );
                if os.path.exists( region ):
                    # We have a region file on disk!
                    reg=rg.fromfiletorecord( region );
                else:
                    # The name given is the name of a region stored
                    # with the image.
                    # Note that we accept:
                    #    'regionname'          -  assumed to be in imagename
                    #    'my.image:regionname' - in my.image
                    reg_names=region.split(':')
                    if ( len( reg_names ) == 1 ):
                        reg=rg.fromtabletorecord( imagename, region, False )
                    else:
                        reg=rg.fromtabletorecord( reg_names[0], reg_names[1], False )
            else: 
                reg=imregion( imagename, chans, stokes, box, '', '' )
            if ( len( reg .keys() ) < 1 ):
                raise Exception, 'Ill-formed region: '+str(reg)+'. can not continue.'                 
            #print "REGION from imrgion: \n"reg
            
            # Now that we know which axes we are using, and have the region
            # selected, lets get that stats!  NOTE: if you have axes size
            # greater then 0 then the maxpos and minpos will not be displayed

            #retValue=ia.statistics( axes=statAxes, region=reg, robust=True  )
            ia.open(imagename)
            """
            retValue = _myia.statistics( region=reg, robust=True, list=listit, verbose=verbose  )
            #myf['xstat']=retValue

            _myia.done()
            
            #return
            return retValue
                
        except Exception, instance:
        	_myia.done()
        	casalog.post( '*** Error ***'+str(instance), 'SEVERE' )
        	return 
        

