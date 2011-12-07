import numpy
from asap.scantable import scantable
from asap._asap import stgrid
import pylab as pl

class asapgrid:
    def __init__( self, infile ):
        self.infile = infile
        self.outfile = None
        self.gridder = stgrid( self.infile )

    def setData( self, infile ):
        self.gridder._setin( infile )

    def setIF( self, ifno ):
        self.gridder._setif( ifno )

    def setPolList( self, pollist ):
        self.gridder._setpollist( pollist )

    def defineImage( self, nx=-1, ny=-1, cellx='', celly='', center='' ):
        self.gridder._defineimage( nx, ny, cellx, celly, center )

    def setOption( self, convType='box', width=-1 ):
        self.gridder._setoption( convType, width )

    def setWeight( self, weightType='uniform' ):
        self.gridder._setweight( weightType ) 

    def grid( self ):
        self.gridder._grid()

    def save( self, outfile='' ):
        self.outfile = self.gridder._save( outfile ) 

    def plot( self, plotchan=-1, plotpol=-1 ):
        plotter = _SDGridPlotter( self.infile, self.outfile )
        plotter.plot( chan=plotchan, pol=plotpol )
        
class _SDGridPlotter:
    def __init__( self, infile, outfile=None ):
        self.infile = infile
        self.outfile = outfile
        if self.outfile is None:
            self.outfile = self.infile.rstrip('/')+'.grid'
        self.grid = None
        self.pointing = None
        self.data = None
        self.nx = -1
        self.ny = -1
        self.nchan = 0
        self.npol = 0
        self.pollist = []
        self.cellx = 0.0
        self.celly = 0.0
        self.center = [0.0,0.0]
        self.nonzero = [[0.0],[0.0]]
        self.get()

    def get( self ):
        s = scantable( self.infile, average=False )
        self.pointing = numpy.array( s.get_directionval() ).transpose()
        spectra = []
        for i in xrange(s.nrow()):
            spectra.append( s._getspectrum( i ) )
        spectra = numpy.array( spectra ).transpose()
        self.nchan = spectra.shape[0]
        del s

        s = scantable( self.outfile, average=False )
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
        dirstring = numpy.array(s.get_direction()).take(range(0,nrow,self.npol))
        self.grid = numpy.array( s.get_directionval() ).take(range(0,nrow,self.npol),axis=0).transpose()
        spectra = numpy.zeros( (self.npol,self.nchan,nrow/self.npol), dtype=float )
        irow = 0 
        for i in xrange(nrow/self.npol):
            for ip in xrange(self.npol):
                spectra[ip,:,i] = s._getspectrum( irow )
                irow += 1

        idx = 0
        d0 = dirstring[0].split()[-1]
        while ( dirstring[idx].split()[-1] == d0 ):  
            idx += 1
        self.ny = idx
        self.nx = nrow / (self.npol * idx )
        #print 'nx,ny=',self.nx,self.ny
        
        self.cellx = abs( self.grid[0][0] - self.grid[0][1] )
        self.celly = abs( self.grid[1][0] - self.grid[1][self.ny] )
        #print 'cellx,celly=',self.cellx,self.celly

        self.data = spectra.reshape( (self.npol,self.nchan,self.nx,self.ny) )

    def plot( self, chan=-1, pol=-1 ):
        if pol < 0:
            data = self.data.mean(axis=0)
            opt = 'averaged over pol'
        else:
            idx = self.pollist.tolist().index( pol )
            #print 'idx=',idx
            data = self.data[idx]
            opt = 'pol %s'%(pol)
        if chan < 0:
            data = data.mean(axis=0)
            opt += ', averaged over channel'
        else:
            data = data[chan]
            opt += ', channel %s'%(chan)
        title = 'Gridded Image (%s)'%(opt)
        pl.figure(10)
        pl.clf()
        pl.plot(self.grid[0],self.grid[1],'.',color='blue')
        pl.plot(self.pointing[0],self.pointing[1],'.',color='red')
        extent=[self.grid[0].min()-0.5*self.cellx,
                self.grid[0].max()+0.5*self.cellx,
                self.grid[1].min()-0.5*self.celly,
                self.grid[1].max()+0.5*self.celly]
        pl.imshow(data,extent=extent,origin='lower',interpolation='nearest')
        pl.colorbar()
        pl.xlabel('R.A. [rad]')
        pl.ylabel('Dec. [rad]')
        pl.title( title )
