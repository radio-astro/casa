import numpy
from asap import rcParams
from asap.scantable import scantable
from asap.selector import selector
from asap._asap import stgrid, stgrid2
import pylab as pl
from logging import asaplog

class asapgrid_base(object):
    def __init__( self ):
        self.outfile = None
        self.ifno = None
        self.gridder = None
        self.infile = None
        self.scantab = None

    def setData( self, infile ):
        raise NotImplementedError('setData is not implemented')
    
    def setIF( self, ifno ):
        """
        Set IFNO to be processed. Currently, asapgrid allows to process
        only one IFNO for one gridding run even if the data contains
        multiple IFs. If you didn't specify IFNO, default value, which
        is IFNO in the first spectrum, will be processed.

        ifno -- IFNO to be processed.
        """
        self.ifno = ifno
        self.gridder._setif( self.ifno )
        
    def setPolList( self, pollist ):
        """
        Set list of polarization components you want to process.
        If not specified, all POLNOs will be processed.

        pollist -- list of POLNOs.
        """
        self.gridder._setpollist( pollist )

    def setScanList( self, scanlist ):
        """
        Set list of scans you want to process. If not specified, all
        scans will be processed.

        scanlist -- list of SCANNOs.
        """
        self.gridder._setscanlist( scanlist )

    def defineImage( self, nx=-1, ny=-1, cellx='', celly='', center='' ):
        """
        Define spatial grid.

        First two parameters, nx and ny, define number of pixels of
        the grid. If which of those is not specified, it will be set
        to the same value as the other. If none of them are specified,
        it will be determined from map extent and cell size.

        Next two parameters, cellx and celly, define size of pixel.
        You should set those parameters as string, which is constructed
        numerical value and unit, e.g. '0.5arcmin', or numerical value.
        If those values are specified as numerical value, their units
        will be assumed to 'arcsec'. If which of those is not specified,
        it will be set to the same value as the other. If none of them
        are specified, it will be determined from map extent and number
        of pixels, or set to '1arcmin' if neither nx nor ny is set.

        The last parameter, center, define the central coordinate of
        the grid. You should specify its value as a string, like,

           'J2000 05h08m50s -16d23m30s'

        or 

           'J2000 05:08:50 -16.23.30'

        You can omit equinox when you specify center coordinate. In that
        case, J2000 is assumed. If center is not specified, it will be
        determined from the observed positions of input data.

        nx -- number of pixels along x (R.A.) direction.
        ny -- number of pixels along y (Dec.) direction.
        cellx -- size of pixel in x (R.A.) direction.
        celly -- size of pixel in y (Dec.) direction.
        center -- central position of the grid.
        """
        if not isinstance( cellx, str ):
            cellx = '%sarcsec'%(cellx)
        if not isinstance( celly, str ):
            celly = '%sarcsec'%(celly)
        self.gridder._defineimage( nx, ny, cellx, celly, center )

    def setFunc( self, func='box', convsupport=-1, truncate="-1", gwidth="-1", jwidth="-1" ):
        """
        Set convolution function. Possible options are 'box' (Box-car,
        default), 'sf' (prolate spheroidal), 'gauss' (Gaussian), and 
        'gjinc' (Gaussian * Jinc).
        Width of convolution function can be set using several parameters.
        For 'box' and 'sf', we have one parameter, convsupport, that
        specifies a cut-off radius of the convlolution function. By default
        (-1), convsupport is automatically set depending on each convolution
        function. Default values for convsupport are:

           'box': 1 pixel
           'sf': 3 pixels

        For 'gauss', we have two parameters for convolution function,
        truncate and gwidth. The truncate is similar to convsupport
        except that truncate allows to specify its value as float or
        string consisting of numeric and unit (e.g. '10arcsec' or
        '3pixel'). Available units are angular units ('arcsec', 'arcmin',
        'deg', etc.) and 'pixel'. Default unit is 'pixel' so that if
        you specify numerical value or string without unit to gwidth,
        the value will be interpreted as 'pixel'. gwidth is an HWHM of
        gaussian. It also allows string value. Interpretation of the
        value for gwidth is same as truncate. Default value for 'gauss'
        is

              gwidth: '-1' ---> sqrt(log(2.0)) pixel
            truncate: '-1' ---> 3*gwidth pixel

        For 'gjinc', there is an additional parameter jwidth that
        specifies a width of the jinc function whose functional form is

            jinc(x) = J_1(pi*x/jwidth) / (pi*x/jwidth)

        Default values for 'gjinc' is

              gwidth: '-1' ---> 2.52*sqrt(log(2.0)) pixel
              jwidth: '-1' ---> 1.55
            truncate: '-1' ---> automatically truncate at first null

        Default values for gwidth and jwidth are taken from Mangum et al.
        (2007).

        func -- Function type ('box', 'sf', 'gauss', 'gjinc').
        convsupport -- Width of convolution function. Default (-1) is
                       to choose pre-defined value for each convolution
                       function. Effective only for 'box' and 'sf'.
        truncate -- Truncation radius of the convolution function.
                    Acceptable value is an integer or a float in units of
                    pixel, or a string consisting of numeric plus unit.
                    Default unit for the string is 'pixel'. Default (-1)
                    is to choose pre-defined value for each convolution
                    function. Effective only for 'gauss' and 'gjinc'.
        gwidth -- The HWHM of the gaussian. Acceptable value is an integer
                  or a float in units of pixel, or a string consisting of
                  numeric plus unit. Default unit for the string is 'pixel'.
                  Default (-1) is to choose pre-defined value for each
                  convolution function. Effective only for 'gauss' and
                  'gjinc'.
        jwidth -- The width of the jinc function. Acceptable value is an
                  integer or a float in units of pixel, or a string
                  consisting of numeric plus unit. Default unit for the
                  string is 'pixel'. Default (-1) is to choose pre-defined
                  value for each convolution function. Effective only for
                  'gjinc'.
        """
        self.gridder._setfunc(func,
                              convsupport=convsupport,
                              truncate=truncate,
                              gwidth=gwidth,
                              jwidth=jwidth)

    def setWeight( self, weightType='uniform' ):
        """
        Set weight type. Possible options are 'uniform' (default),
        'tint' (weight by integration time), 'tsys' (weight by
        Tsys: 1/Tsys**2), and 'tintsys' (weight by integration time
        as well as Tsys: tint/Tsys**2).

        weightType -- weight type ('uniform', 'tint', 'tsys', 'tintsys')
        """
        self.gridder._setweight( weightType )

    def enableClip( self ):
        """
        Enable min/max clipping.

        By default, min/max clipping is disabled so that you should
        call this method before actual gridding if you want to do
        clipping.
        """
        self.gridder._enableclip()

    def disableClip( self ):
        """
        Disable min/max clipping.
        """
        self.gridder._disableclip()

    def grid( self ):
        """
        Actual gridding which will be done based on several user inputs. 
        """
        self.gridder._grid()
        
    def plotFunc(self, clear=True):
        """
        Support function to see the shape of current grid function.

        clear -- clear panel if True. Default is True.
        """
        pl.figure(11)
        if clear:
            pl.clf()
        f = self.gridder._getfunc()
        convsampling = 100
        a = numpy.arange(0,len(f)/convsampling,1./convsampling,dtype=float)
        pl.plot(a,f,'.-')
        pl.xlabel('pixel')
        pl.ylabel('convFunc')

    def save( self, outfile='' ):
        raise NotImplementedError('save is not implemented')

    def plot( self, plotchan=-1, plotpol=-1, plotobs=False, plotgrid=False ):
        raise NotImplementedError('plot is not implemented')

    def getResult( self ):
        raise NotImplementedError('getResult is not implemented')

class asapgrid(asapgrid_base):
    """
    The asapgrid class is defined to convolve data onto regular
    spatial grid. Typical usage is as follows:

       # create asapgrid instance with two input data
       g = asapgrid( ['testimage1.asap','testimage2.asap'] )
       # set IFNO if necessary
       g.setIF( 0 )
       # set POLNOs if necessary
       g.setPolList( [0,1] )
       # set SCANNOs if necessary
       g.setScanList( [22,23,24] )
       # define image with full specification
       # you can skip some parameters (see help for defineImage)
       g.defineImage( nx=12, ny=12, cellx='10arcsec', celly='10arcsec',
                      center='J2000 10h10m10s -5d05m05s' )
       # set convolution function
       g.setFunc( func='sf', convsupport=3 )
       # enable min/max clipping
       g.enableClip()
       # or, disable min/max clipping
       #g.disableClip()
       # actual gridding
       g.grid()
       # save result
       g.save( outfile='grid.asap' )
       # plot result
       g.plot( plotchan=1246, plotpol=-1, plotgrid=True, plotobs=True )
    """
    def __init__( self, infile ):
        """
        Create asapgrid instance.

        infile -- input data as a string or string list if you want
                  to grid more than one data at once.  
        """
        super(asapgrid,self).__init__()
        self.gridder = stgrid()
        self.infile=infile
        self.setData(infile)

    def setData( self, infile ):
        """
        Set data to be processed.

        infile -- input data as a string or string list if you want
                  to grid more than one data at once.  
        """
        if isinstance( infile, str ):
            self.gridder._setin( infile )
        else:
            self.gridder._setfiles( infile )
        self.infile = infile 

    def save( self, outfile='' ):
        """
        Save result. By default, output data name will be constructed
        from first element of input data name list (e.g. 'input.asap.grid').

        outfile -- output data name. 
        """
        self.outfile = self.gridder._save( outfile ) 

    def plot( self, plotchan=-1, plotpol=-1, plotobs=False, plotgrid=False ):
        """
        Plot gridded data.

        plotchan -- Which channel you want to plot. Default (-1) is
                    to average all the channels.
        plotpol -- Which polarization component you want to plot.
                   Default (-1) is to average all the polarization
                   components.
        plotobs -- Also plot observed position if True. Default
                   is False. Setting True for large amount of spectra
                   might be time consuming.
        plotgrid -- Also plot grid center if True. Default is False.
                    Setting True for large number of grids might be
                    time consuming.
        """
        import time
        t0=time.time()
        # to load scantable on disk
        storg = rcParams['scantable.storage']
        rcParams['scantable.storage'] = 'disk'
        [nx,ny] = self.gridder._get_resultant_map_size()
        [cellx,celly] = self.gridder._get_resultant_cell_size()
        plotter = _SDGridPlotter( self.infile, self.outfile, self.ifno,
                                  nx=nx, ny=ny, cellx=cellx, celly=celly )
        plotter.plot( chan=plotchan, pol=plotpol, plotobs=plotobs, plotgrid=plotgrid )
        # back to original setup
        rcParams['scantable.storage'] = storg
        t1=time.time()
        asaplog.push('plot: elapsed time %s sec'%(t1-t0))
        asaplog.post('DEBUG','asapgrid.plot')
        
class asapgrid2(asapgrid_base):
    """
    The asapgrid class is defined to convolve data onto regular
    spatial grid. Typical usage is as follows:

       # create asapgrid instance with input scantable
       s = scantable( 'testimage1.asap', average=False )
       g = asapgrid( s )
       # set IFNO if necessary
       g.setIF( 0 )
       # set POLNOs if necessary
       g.setPolList( [0,1] )
       # set SCANNOs if necessary
       g.setScanList( [22,23,24] )
       # define image with full specification
       # you can skip some parameters (see help for defineImage)
       g.defineImage( nx=12, ny=12, cellx='10arcsec', celly='10arcsec',
                      center='J2000 10h10m10s -5d05m05s' )
       # set convolution function
       g.setFunc( func='sf', width=3 )
       # enable min/max clipping
       g.enableClip()
       # or, disable min/max clipping
       #g.disableClip()
       # actual gridding
       g.grid()
       # get result as scantable
       sg = g.getResult()
    """
    def __init__( self, scantab ):
        """
        Create asapgrid instance.

        scantab -- input data as a scantable or a list of scantables
                   to grid more than one data at once.  
        """
        super(asapgrid2,self).__init__()
        self.gridder = stgrid2()
        self.scantab = scantab
        self.setData( scantab )

    def setData( self, scantab ):
        """
        Set data to be processed.

        scantab -- input data as a scantable or a list of scantables
                   to grid more than one data at once.  
        """
        if isinstance( scantab, scantable ):
            self.gridder._setin( scantab )
        else:
            self.gridder._setfiles( scantab )
        self.scantab = scantab

    def getResult( self ):
        """
        Return gridded data as a scantable.
        """
        tp = 0 if rcParams['scantable.storage']=='memory' else 1
        return scantable( self.gridder._get( tp ), average=False )    

class _SDGridPlotter:
    def __init__( self, infile, outfile=None, ifno=-1, nx=-1, ny=-1, cellx=0.0, celly=0.0 ):
        if isinstance( infile, str ):
            self.infile = [infile]
        else:
            self.infile = infile
        self.outfile = outfile
        if self.outfile is None:
            self.outfile = self.infile[0].rstrip('/')+'.grid'
        self.nx = nx
        self.ny = ny if ny > 0 else nx
        self.nchan = 0
        self.npol = 0
        self.pollist = []
        self.cellx = cellx
        self.celly = celly if celly > 0.0 else cellx
        self.center = [0.0,0.0]
        self.nonzero = [[0.0],[0.0]]
        self.ifno = ifno
        self.tablein = None
        self.nrow = 0
        self.blc = None
        self.trc = None
        self.get()

    def get( self ):
        s = scantable( self.outfile, average=False )
        self.nchan = len(s._getspectrum(0))
        nrow = s.nrow()
        pols = numpy.ones( nrow, dtype=int )
        for i in xrange(nrow):
            pols[i] = s.getpol(i)
        self.pollist, indices = numpy.unique( pols, return_inverse=True )
        self.npol = len(self.pollist)
        self.pollist = self.pollist[indices[:self.npol]]
        #print 'pollist=',self.pollist
        #print 'npol=',self.npol
        #print 'nrow=',nrow

        if self.nx <= 0 or self.ny <= 0:
            idx = 1
            d0 = s.get_direction( 0 ).split()[-2]
            d = s.get_direction(self.npol*idx)
            while( d is not None \
                   and d.split()[-2] != d0):
                idx += 1
                d = s.get_direction(self.npol*idx)
        
            self.nx = idx
            self.ny = nrow / (self.npol * idx )
            #print 'nx,ny=',self.nx,self.ny

        self.blc = s.get_directionval( 0 )
        self.trc = s.get_directionval( nrow-self.npol )
        #print self.blc
        #print self.trc

        if self.cellx <= 0.0 or self.celly <= 0.0:
            if nrow > 1:
                incrx = s.get_directionval( self.npol )
                incry = s.get_directionval( self.nx*self.npol )
            else:
                incrx = [0.0,0.0]
                incry = [0.0,0.0]
            self.cellx = abs( self.blc[0] - incrx[0] )
            self.celly = abs( self.blc[1] - incry[1] )
            #print 'cellx,celly=',self.cellx,self.celly

    def plot( self, chan=-1, pol=-1, plotobs=False, plotgrid=False ):
        if pol < 0:
            opt = 'averaged over pol'
        else:
            opt = 'pol %s'%(pol)
        if type(chan) is list:
            opt += ', averaged over channel %s-%s'%(chan[0],chan[1])
        elif chan < 0:
            opt += ', averaged over channel'
        else:
            opt += ', channel %s'%(chan)
        data = self.getData( chan, pol )
        #data = numpy.fliplr( data )
        title = 'Gridded Image (%s)'%(opt)
        pl.figure(10)
        pl.clf()
        # plot grid position
        if plotgrid:
            x = numpy.arange(self.blc[0],self.trc[0]+0.5*self.cellx,self.cellx,dtype=float)
            #print 'len(x)=',len(x)
            #print 'x=',x
            ybase = numpy.ones(self.nx,dtype=float)*self.blc[1]
            #print 'len(ybase)=',len(ybase)
            incr = self.celly 
            for iy in xrange(self.ny):
                y = ybase + iy * incr
                #print y
                pl.plot(x,y,',',color='blue')
        # plot observed position
        if plotobs:
            for i in xrange(len(self.infile)):
                self.createTableIn( self.infile[i] )
                irow = 0 
                while ( irow < self.nrow ):
                    chunk = self.getPointingChunk( irow )
                    #print chunk
                    pl.plot(chunk[0],chunk[1],',',color='green')
                    irow += chunk.shape[1]
                    #print irow
        # show image
        extent=[self.blc[0]-0.5*self.cellx,
                self.trc[0]+0.5*self.cellx,
                self.blc[1]-0.5*self.celly,
                self.trc[1]+0.5*self.celly]
        deccorr = 1.0/numpy.cos(0.5*(self.blc[1]+self.trc[1]))
        pl.imshow(data,extent=extent,origin='lower',interpolation='nearest')
        pl.colorbar()
        pl.xlabel('R.A. [rad]')
        pl.ylabel('Dec. [rad]')
        ax = pl.axes()
        ax.set_aspect(deccorr)
        pl.title( title )

    def createTableIn( self, tab ):
        del self.tablein
        self.tablein = scantable( tab, average=False )
        if self.ifno < 0:
            ifno = self.tablein.getif(0)
            #print 'ifno=',ifno
        else:
            ifno = self.ifno
        sel = selector()
        sel.set_ifs( ifno )
        self.tablein.set_selection( sel )
        self.nchan = len(self.tablein._getspectrum(0))
        self.nrow = self.tablein.nrow()
        del sel
        

    def getPointingChunk( self, irow ):
        numchunk = 1000
        nrow = min( self.nrow-irow, numchunk )
        #print 'nrow=',nrow
        v = numpy.zeros( (2,nrow), dtype=float )
        idx = 0
        for i in xrange(irow,irow+nrow):
            d = self.tablein.get_directionval( i )
            v[0,idx] = d[0]
            v[1,idx] = d[1]
            idx += 1
        return v

    def getData( self, chan=-1, pol=-1 ):
        if type(chan) == list:
            spectra = self.__chanAverage(start=chan[0],end=chan[1])
        elif chan == -1:
            spectra = self.__chanAverage()
        else:
            spectra = self.__chanIndex( chan )
        data = spectra.reshape( (self.npol,self.ny,self.nx) )
        if pol == -1:
            retval = data.mean(axis=0)
        else:
            retval = data[pol]
        return retval

    def __chanAverage( self, start=-1, end=-1 ):
        s = scantable( self.outfile, average=False )
        nrow = s.nrow() 
        spectra = numpy.zeros( (self.npol,nrow/self.npol), dtype=float )
        irow = 0
        sp = [0 for i in xrange(self.nchan)]
        if start < 0:
            start = 0
        if end < 0:
            end = self.nchan
        for i in xrange(nrow/self.npol):
            for ip in xrange(self.npol):
                sp = s._getspectrum( irow )[start:end]
                spectra[ip,i] = numpy.mean( sp )
                irow += 1
            
        return spectra

    def __chanIndex( self, idx ):
        s = scantable( self.outfile, average=False )
        nrow = s.nrow()
        spectra = numpy.zeros( (self.npol,nrow/self.npol), dtype=float )
        irow = 0
        sp = [0 for i in xrange(self.nchan)]
        for i in xrange(nrow/self.npol):
            for ip in xrange(self.npol):
                sp = s._getspectrum( irow )
                spectra[ip,i] = sp[idx]
                irow += 1
        return spectra
        
            
