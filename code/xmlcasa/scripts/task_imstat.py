import os
from taskinit import *
from imregion import *

from odict import odict

def imstat(imagename=None,region=None,box=None,chans=None,stokes=None):
        try:
            casalog.origin('imstat')
            retValue = {}
            ia.open(imagename)


            # If the user hasn't specified any region information then
            # generate statistical information for the entire image.
            if ( len(box)<1 and len(chans)<1 and len(stokes)<1 and len(region)<1):
                # Get stats for whole image
                retValue = ia.statistics( robust=True )
                ia.close()
                return retValue
            # If we've made it here then we are finding the stats
            # of a particular region in the image.
            axes=getimaxes(imagename)
            statAxes=[]
            if ( len(box)>0 ):
                statAxes.append(axes[0][0])
                statAxes.append(axes[1][0])
            if ( len(chans)>0 ):
                statAxes.append(axes[2][0])

            # Get the region information, if the user has specified
            # a region file it is given higher priority.
            reg={}
            if ( len(region)>1 ):
                if (
                    ( len(box)>1 and box!='-1' )
                    or ( len(chans)>1 and chans!='-1' )
                    or ( len(stokes)>1 and stokes !='-1' )
                ):
                    casalog.post( "Ignoring region selection\information in"\
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
            if ( len( reg.keys() ) < 1 ):
                raise Exception, 'Ill-formed region: '+str(reg)+'. can not continue.'                 
            
            # Now that we know which axes we are using, and have the region
            # selected, lets get that stats!  NOTE: if you have axes size
            # greater then 0 then the maxpos and minpos will not be displayed

            ia.open(imagename)
            retValue=ia.statistics( region=reg, robust=True  )

            # Cleanup
            ia.done()
                
        except Exception, instance:
            casalog.post('*** Error *** ' + str(instance), 'SEVERE' )
        return retValue
        

def imstat_print( values, name ) :
    print 'Statistics on ', name
    print ''
    print 'Region ---'
    print '\t --\x1B[94m bottom-left corner (pixel) [blc]:    \x1B[0m', list(values['blc'])
    print '\t --\x1B[94m top-right corner (pixel) [trc]:      \x1B[0m', list(values['trc'])
    print '\t --\x1B[94m bottom-left corner (world) [blcf]:    \x1B[0m', values['blcf']
    print '\t --\x1B[94m top-right corner( world) [trcf]:      \x1B[0m', values['trcf']
    print ''
    print 'Values --'

    if ( 'flux' in values ):
        print '\t --\x1B[94m flux [flux]:                          \x1B[0m',array2string( values['flux'])
    print '\t --\x1B[94m number of points [npts]:              \x1B[0m',array2string( values['npts'])
    print '\t --\x1B[94m maximum value [max]:                 \x1B[0m',array2string( values['max'])
    print '\t --\x1B[94m minimum value [min]:                 \x1B[0m',array2string( values['min'])

    if ( 'maxpos' in values ):
        print '\t --\x1B[94m position of max value (pixel) [maxpos]: \x1B[0m', list(values['maxpos'])
    if ( 'minpos' in values ):        
        print '\t --\x1B[94m position of min value (pixel) [minpos]: \x1B[0m', list(values['minpos'])
    if ( 'maxposf' in values ):        
        print '\t --\x1B[94m position of max value (world) [maxposf]: \x1B[0m', values['maxposf']
    if ( 'minposf' in values ):        
        print '\t --\x1B[94m position of min value (world) [minposf]: \x1B[0m', values['minposf']
    print '\t --\x1B[94m Sum of pixel values [sum]:           \x1B[0m', array2string( values['sum'])
    print '\t --\x1B[94m Sum of squared pixel values [sumsq]:   \x1B[0m', array2string( values['sumsq'])
                                                                             
    print ''
    print 'Statistics ---'
    print '\t --\x1B[94m Mean of the pixel values [mean]:      \x1B[0m', array2string( values['mean'])
    print '\t --\x1B[94m Standard deviation of the Mean [sigma]:\x1B[0m', array2string( values['sigma'])
    print '\t --\x1B[94m Root mean square [rms]:              \x1B[0m', array2string( values['rms'])
    print '\t --\x1B[94m Median of the pixel values [median]:    \x1B[0m', array2string( values['median'])
    print '\t --\x1B[94m Median of the deviations [medabsdevmed]:      \x1B[0m', array2string( values['medabsdevmed'])
    print '\t --\x1B[94m Quartile [quartile]:                      \x1B[0m', array2string( values['quartile'])
    return
