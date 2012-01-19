import os
import pylab as pl
from taskinit import *

def plotants(vis=None,figfile=''):
       """Plot the antenna distribution in the local reference frame:

       The location of the antennas in the MS will be plotted with
       X-toward local east; Y-toward local north.

       Keyword arguments:
       vis -- Name of input visibility file.
               default: none. example: vis='ngc5921.ms'

       figfile -- Save the plotted figure in this file.
               default: ''. example: figfile='myFigure.png'

               The name of each antenna (egs. vla=antenna number) is
                  shown next to its respective location.

               DO NOT use the buttons on the Mark Region line.  These are
                  not implemented yet and might abort CASA.

               You can zoom in by pressing the magnifier button (bottom,
                  third from left) and making a rectangular region with
                  the mouse.  Press the home button (left most button) to
                  remove zoom.

               A hard-copy of this plot can be obtained by pressing the
                  button on the right at the bottom of the display.  This
                  produces a png format file.
       """
       try:
              if type(vis) == str and os.path.isdir(vis):
                     mp.open(vis)
              else:
                     raise Exception, 'Visibility data set not found - please verify the name'
              pl.ion() # countering tableplot's inadequacies with poor practice
              pl.clf()
              mp.plotoptions(plotsymbol='ro');
              mp.plot(type='array')
              mp.reset()
              pl.axis('equal')
              pl.axis('scaled')
              #pl.ylabel('Y (m)')          # Replace mp's "X (m)".
              pl.title(vis)
              if ( len(figfile) > 0 ) :
                   mp.savefig( figfile )

       except Exception, instance:
              print '*** Error ***',instance

       mp.close()
