from asap.scantable import scantable
from asap._asap import _edgemarker
import numpy
import math
        
class edgemarker:
    """
    The edgemarker is a helper tool to calibrate OTF observation
    without explicit OFF scans. According to a few user-specified
    options, the class automatically detects an edge region of the
    map and mark integrations within this region as OFF.

    The edgemarker supports raster pattern as well as other generic 
    ones (e.g. lissajous, double circle). The constructor takes
    one boolean parameter to specify whether scan pattern is raster
    or not. This is because that edge detection algorithms for raster
    and others are different.

    Current limitation of this class is that it cannot handle some
    complicated observed area. Typical case is that the area has
    clear 'dent' (e.g. a composite area consisting of two diamond-
    shaped areas that slightly overlap). In such case, the class
    will fail to detect such feature.

    Note that the class takes a copy of input data so that input
    data will not be overwritten. Result will be provided as a
    separate data whose contents are essentially the same as input
    except for that some integrations are marked as OFF.

    Here is typical usage:

       s = scantable( 'otf.asap', average=False )
       marker = edgemarker( israster=False )
       marker.setdata( s )
       marker.setoption( fraction='15%', width=0.5 )
       marker.mark()

       # get result as scantable instance
       s2 = marker.getresult()

       # save result on disk
       marker.save( 'otfwithoff.asap', overwrite=True )
    """
    def __init__( self, israster=False ):
        """
        Constructor.

        israster -- Whether scan pattern is raster or not. Set True
                    if scan pattern is raster. Default is False.
        """
        self.israster = israster
        self.marker = _edgemarker( self.israster )
        self.st = None

    def setdata( self, st ):
        """
        Set data to be processed.

        st -- Data as scantable instance.
        """
        self.st = st
        self.marker._setdata( self.st, False )
        self.marker._examine()

    def setoption( self, *args, **kwargs ):
        """
        Set options for edge detection. Valid options depend on
        whether scan pattern is raster or not (i.e. constructor
        is called with israster=True or False).

        === for raster (israster=True) ===
        fraction -- Fraction of OFF integration in each raster
                    row. Either numerical value (<1.0) or string
                    is accepted. For string, its value should be
                    'auto' or format 'xx%'. For example, '10%'
                    is same as 0.1. The 'auto' option estimates
                    number of OFFs based on t_OFF = sqrt(N) t_ON.
                    Default is 0.1.
        npts -- Number of OFF integration in each raster row.
                Default is -1 (use fraction).

        Note that number of integrations specified by the above
        parameters will be marked as OFF from both ends. So, twice
        of specified number/fraction will be marked as OFF. For
        example, if you specify fraction='10%', resultant fraction
        of OFF integrations will be 20%.
        
        Note also that, if both fraction and npts are specified,
        specification by npts will come before. 

        === for non-raster (israster=False) ===
        fraction -- Fraction of edge area with respect to whole
                    observed area. Either numerical value (<1.0)
                    or string is accepted. For string, its value
                    should be in 'xx%' format. For example, '10%'
                    is same as 0.1. Default is 0.1.
        width -- Pixel width for edge detection. It should be given
                 as a fraction of the median spatial separation
                 between neighboring integrations in time. Default
                 is 0.5. In the most case, default value will be fine.
                 Larger value will cause worse result. Smaller value
                 may improve result. However, if too small value is
                 set (e.g. 1.0e-5), the algorithm may not work.
        elongated -- Set True only if observed area is extremely
                     elongated in one direction. Default is False.
                     In most cases, default value will be fine.
        """
        option = {}
        if self.israster:
            keys = [ 'fraction', 'npts' ]
        else:
            keys = [ 'fraction', 'width', 'elongated' ]
        for key in keys:
            if kwargs.has_key( key ):
                option[key] = kwargs[key]

        if len(option) > 0:
            self.marker._setoption( option )

    def mark( self ):
        """
        Process data. Edge region is detected according to detection
        parameters given by setoption(). Then, integrations within
        edge region will be marked as OFF.
        """
        self.marker._detect()
        self.marker._mark()

    def getresult( self ):
        """
        Get result as scantable instance. Returned scantable is
        copy of input scantable except for that some data are
        marked as OFF as a result of edge detection and marking.
        """
        return scantable( self.marker._get() )

    def save( self, name, overwrite=False ):
        """
        Save result as scantable.

        name -- Name of the scantable.
        overwrite -- Overwrite existing data if True. Default is
                     False.
        """
        s = self.getresult()
        s.save( name, overwrite=overwrite )

    def plot( self ):
        """
        """
        from matplotlib import pylab as pl
        from asap import selector
        from asap._asap import srctype as st
        pl.clf()

        # result as a scantable
        s = self.getresult()

        # ON scan
        sel = selector()
        sel.set_types( int(st.pson) )
        s.set_selection( sel )
        diron = numpy.array( s.get_directionval() ).transpose()
        diron[0] = rotate( diron[0] )
        s.set_selection()
        sel.reset()

        # OFF scan
        sel.set_types( int(st.psoff) )
        s.set_selection( sel )
        diroff = numpy.array( s.get_directionval() ).transpose()
        diroff[0] = rotate( diroff[0] )
        s.set_selection()
        sel.reset()
        del s
        del sel

        # plot
        pl.ioff()
        ax=pl.axes()
        ax.set_aspect(1.0)
        pl.plot( diron[0], diron[1], '.', color='blue', label='ON' )
        pl.plot( diroff[0], diroff[1], '.', color='green', label='OFF' )
        [xmin,xmax,ymin,ymax] = pl.axis()
        pl.axis([xmax,xmin,ymin,ymax])
        pl.legend(loc='best',prop={'size':'small'},numpoints=1)
        pl.xlabel( 'R.A. [rad]' )
        pl.ylabel( 'Declination [rad]' )
        pl.title( 'edgemarker result' )
        pl.ion()
        pl.draw()

def _0to2pi( v ):
    return v % (2.0*math.pi)

def quadrant( v ):
    vl = _0to2pi( v )
    base = 0.5 * math.pi
    return int( vl / base )

def quadrantList( a ):
    n = len(a)
    nquad = numpy.zeros( 4, dtype=int )
    for i in xrange(n):
        v = quadrant( a[i] )
        nquad[v] += 1
    #print nquad
    return nquad

def rotate( v ):
    a = numpy.zeros( len(v), dtype=float )
    for i in xrange(len(v)):
        a[i] = _0to2pi( v[i] )
    nquad = quadrantList( a )
    quadList = [[],[],[],[]]
    rot = numpy.zeros( 4, dtype=bool )
    if all( nquad==0 ):
        print 'no data'
    elif all( nquad>0 ):
        #print 'extends in all quadrants'
        pass
    elif nquad[0]>0 and nquad[3]>0:
        #print 'need rotation'
        rot[3] = True
        rot[2] = nquad[1]==0 and nquad[2]>0
    #print rot

    for i in xrange(len(a)):
        if rot[quadrant(a[i])]:
            a[i] -= 2*math.pi
    return a
