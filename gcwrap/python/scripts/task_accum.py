import sys
import os
from taskinit import *

def accum(vis=None,tablein=None,incrtable=None,caltable=None,field=None,calfield=None,interp=None,accumtime=None,spwmap=None):

       """ Accumulate incremental calibration solutions into a calibration table:

       Accum will interpolate and extrapolate a temporal calibration
       table onto a new table that has a regularly-space time grid.

       The first run of accum defines the time grid and fills this
       table with the results from the input table.

       Subsequent use of accum will combine additional calibration
       tables onto the same grid of the initial accum table to obtain
       an output accum table.  See below for a concrete example.

       Keyword arguments:
       vis --   Name of input visibility file
                default: none; example: vis='ngc5921.ms'
       tablein -- Input cumulative calibration table
                ''=> first
                  in accumtime.  Then use output table from previous runs.
       accumtime -- Time sampling (sec) of the output table.
                Used only if tablein=''.
                default: 1.0
                example: accumtime=40.0
                accumtime ~ data sampling time is recommended
       incrtable -- incremental calibration table name
                default: none; example: incrtable='ngc5921_phase.gcal'
                  on first use of accum, incrtable will be
                  resampled onto caltable with the timescale given
                  by accumtime
       caltable -- Output cumulative calibration table
                default: none; example: caltable='ngc5921.Cgcal'
                  caltable = complex multiplication of incrtable with tablein
                  tablein and caltable have the same time grid;
                  incrtable time stamps are arbitrary.
       field -- List of field names from tablein to process
                default: ''==>all; (most likely)
                  example: field='0957+561 3C286'
                  usual syntax for field selection
       calfield -- List of field names in incremental table to use.
                default: ''==>all; example: calfield='0957*,3C2*'
                  usual syntax for field selection
       interp -- Interpolation mode to use on incremental solutions
                 default: 'linear'; example: interp='nearest'
                 'linear' linearly interpolates the amplitude and
                   phase in incrtable to add to tablein.  RECOMMENDED
                 'nearest' chooses the nearest value in incrtable to
                   add to tablein
                 'aipsline' is similar to linear, but does interpolated
                   real and imaginary parts.  NOT RECOMMENDED
       spwmap -- Spectral windows combinations to form
                 default: [-1];
                 default means process spws independently
                    example: spwmap=[0,0,1,1] means apply results from
                      spw = 0,0,1,1 to spw 0,1,2,3
                      defaults is equivalent to spwmap = [0,1,2,3]
       Example use:  Doing a phase cal with short solution interval and then
                     an amplitude solution on a longer integration time.

          First, generate the calibration tables for phase and flux cal

          taskname = 'gaincal'
          default
          cal1 = 'J1335-331,3C286'  # phase and flux calibrator
          taskname = 'gaincal'
          default
          vis = '3C219.ms'
          gaintype = 'G'
          calmode = 'p'             # do a phase cal on raw data
          caltable = 'phase.cal'    # output phase calibration table
          field = cal1              # calibrator names
          solint = -1               # calibrate each point
          refant = 'VA04'           # reference antenna
          go

          calmode = 'ap'            # do an amp/phase cal removing first phase cal
          caltable = 'amp.cal'      # output amplitude calibration table
          solint = 0                # solution for each scan
          gaintable = 'phase.cal'   # First, apply previous phase calibration on the fly
          go

          taskname = 'fluxscale'    # Determine the flux of phase calibrator
          default
          vis = '3C219.ms'
          caltable = 'amp.cal'      # amp cal input
          fluxtable = 'flux.cal'    # calibration table with phase cal flux adjusted
          reference = '3C286'       # Sources to use as amplitude standard.
                                           # Should have used setjy on these sources
          transfer = 'J1335-331'    # Sources to be scaled to flux density
          go

          taskname = 'smoothcal'    # Smooth amplitude and phases
          vis = '3C219.ms'
          tablein = 'flux.cal'
          caltable = 'smooth.cal'   # smooths amplitude and phase
          field = 'J1334-331'       # Only smooth phase calibrator
          smoothtype = 'median'
          smoothtime = 600.0        # Desired smoothing time.  Usually contains
                                        # several calibration points


                                    # Define accum table from first phase only cal
          taskname = 'accum'
          default
          vis = '3C219.ms'
          tablein = ''              # No input table on first run
          accumtime = -1            # grid time defaults to sampling time
          incrtable = 'phase.cal'   # Input phase cal
          caltable = 'accum1.cal'   # Output accum table with phase cal
          field = ''                # All sources in phase.cal
          interp = 'linear'
          go

          tablein = accum1.cal'     # input is output from above accum run
          incrtable = 'amp.cal'     # Input amplitude cal
          caltable = 'accum2.cal'   # Vector produce of tablein and incrtable
          field = ''                # Determine cal values for all field
          calfield = 'J1334-331'    # Only interpolate from phase cal
          interp = 'linear'
          go

#         accum2.cal contains the calibration of the raw data.  The amplitudes have
#           been smooths to 600 seconds, the phases have not.




       """

       #Python script
       try:
              if(tablein != '') :
                     accumtime=-1.0

              if ((type(vis)==str) & (os.path.exists(vis))):
                     cb.open(filename=vis,compress=False,addcorr=False,addmodel=False)
              else:
                     raise Exception, 'Visibility data set not found - please verify the name'

              cb.accumulate(tablein=tablein,incrtable=incrtable,tableout=caltable,
                            field=field,calfield=calfield,interp=interp,t=accumtime,spwmap=spwmap)
              cb.close()

       except Exception, instance:
              print '*** Error ***',instance
              cb.close()
              raise Exception, instance

