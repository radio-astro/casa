# sd task for imaging
import os
import numpy
import pylab as pl
import asap as sd
from taskinit import * 

def sdgrid(infile, antenna, scanlist, ifno, pollist, gridfunction, width, weight, clipminmax, outfile, overwrite, npix, cell, center, plot):

        casalog.origin('sdgrid')
        try:
            summary =   'Input Parameter Summary:\n' \
                      + '   infile = %s\n'%(infile) \
                      + '   antenna = %s\n'%(antenna) \
                      + '   scanlist = %s\n'%(scanlist) \
                      + '   ifno = %s\n'%(ifno) \
                      + '   pollist = %s\n'%(pollist) \
                      + '   gridfunction = %s\n'%(gridfunction) \
                      + '   width = %s\n'%(width) \
                      + '   weight = %s\n'%(weight) \
                      + '   clipminmax = %s\n'%(clipminmax) \
                      + '   outfile = %s\n'%(outfile) \
                      + '   overwrite = %s\n'%(overwrite) \
                      + '   npix = %s\n'%(npix) \
                      + '   cell = %s\n'%(cell) \
                      + '   center = %s\n'%(center) \
                      + '   plot = %s'%(plot)
            casalog.post( summary, 'DEBUG' )
            
            # infile
            if isinstance( infile, str ):
                infile = [infile]

            # scanlist
            if isinstance(scanlist,list) or isinstance(scanlist,numpy.ndarray):
                scans = list(scanlist)
            else:
                scans = [scanlist]

            # pollist
            if isinstance(pollist,list) or isinstance(pollist,numpy.ndarray):
                pols = list(pollist)
            else:
                pols = [pollist]

            # gridfunction and width
            if gridfunction.upper() == 'PB':
                msg='Sorry. PB gridding is not implemented yet.'
                raise Exception, msg
            elif gridfunction.upper() == 'BOX':
                width=-1
                

            # outfile
            outname=outfile
            if len(outname) == 0:
                outname=infile[0].rstrip('/')+'.grid'
            if os.path.exists(outname):
                if overwrite:
                    casalog.post( 'Overwrite existing file %s'%(outname), 'INFO' )
                    os.system( 'rm -rf %s'%(outname) )
                else:
                    msg='file %s exists' % (outname)
                    raise Exception, msg

            # npix
            nx=-1
            ny=-1
            if isinstance(npix,list) or isinstance(npix,numpy.ndarray):
                if len(npix)==1:
                    nx=npix[0]
                    ny=npix[0]
                else:
                    nx=npix[0]
                    ny=npix[1]
            else:
                nx=npix
                ny=npix

            # cell
            cellx=''
            celly=''
            if type(cell)==str:
                cellx=cell
                celly=cell
            elif isinstance(cell,list) or isinstance(cell,numpy.ndarray):
                if len(cell)==1:
                    if type(cell[0])==str:
                        cellx=cell[0]
                    else:
                        cellx='%sarcmin'%(cell[0])
                    celly=cell[0]
                else:
                    if type(cell[0])==str:
                        cellx=cell[0]
                    else:
                        cellx='%sarcmin'%(cell[0])
                    if type(cell[1])==str:
                        celly=cell[1]
                    else:
                        celly='%sarcmin'%(cell[1])
            else:
                cellx='%sarcmin'%(cell)
                celly=cellx

            # center
            centerstr=''
            if isinstance(center,str):
                centerstr=center
            else:
                # two-element list is assumed
                l=['J2000']
                for i in xrange(2):
                    if isinstance(center[i],str):
                        l.append(center[i])
                    else:
                        l.append('%srad'%(center[i]))
                centerstr=string.join(l)

            ############
            # Gridding #
            ############
            casalog.post('Start gridding...', "INFO")
            summary =   'Grid Parameter Summary:\n' \
                      + '   infile = %s\n'%(infile) \
                      + '   ifno = %s\n'%(ifno) \
                      + '   scans = %s\n'%(scans) \
                      + '   pols = %s\n'%(pols) \
                      + '   gridfunction = %s\n'%(gridfunction) \
                      + '   width = %s\n'%(width) \
                      + '   weight = %s\n'%(weight) \
                      + '   clipminmax = %s\n'%(clipminmax) \
                      + '   outname = %s\n'%(outname) \
                      + '   nx = %s\n'%(nx) \
                      + '   ny = %s\n'%(ny) \
                      + '   cellx = %s\n'%(cellx) \
                      + '   celly = %s\n'%(celly) \
                      + '   centerstr = %s\n'%(centerstr) \
                      + '   plot = %s'%(plot)
            casalog.post( summary, 'DEBUG' )
            gridder = sd.asapgrid( infile=infile )
            gridder.setPolList( pols )
            gridder.setScanList( scans )
            if ( ifno >= 0 ):
                gridder.setIF( ifno )
            if ( clipminmax ):
                gridder.enableClip()
            else:
                gridder.disableClip()
            gridder.setWeight( weight ) 
            gridder.defineImage( nx=nx, ny=ny,
                                 cellx=cellx, celly=celly,
                                 center=centerstr )
            gridder.setFunc( func=gridfunction,
                             width=width )
            gridder.grid()
            gridder.save( outfile=outname )
            if plot:
                gridder.plot()
            del gridder
            
        except Exception, instance:
            #print '***Error***',instance
            casalog.post( str(instance), priority = 'ERROR' )
            return
