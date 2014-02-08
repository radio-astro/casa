import os
from taskinit import *

def fluxscale(vis=None,caltable=None,fluxtable=None,reference=None,transfer=None,listfile=None,append=None,refspwmap=None,gainthreshold=None,antenna=None,incremental=None,fitorder=None,display=None):
       """Bootstrap the flux density scale from standard calibrators:

       After running gaincal on standard flux density calibrators (with or
       without a model), and other calibrators with unknown flux densities,
       fluxscale will determine the flux density of the unknowns calibrators
       that are most consistent with the standard calibrator antenna gains.

       Keyword arguments:
       vis -- Name of input visibility file
               default: none; example: vis='ngc5921.ms'
       caltable -- Name of input calibration table
               default: none; example: caltable='ngc5921.gcal'
               This cal table was obtained from task gaincal.
       fluxtable -- Name of output, flux-scaled calibration table
               default: none; example: fluxtable='ngc5921.gcal2'
               The gains in this table have been adjusted by the
               derived flux density each calibrator.  The MODEL_DATA
               column has NOT been updated for the flux density of the
               calibrator.  Use setjy to do this if it is a point source.
       reference -- Reference field name(s)
               The names of the fields with a known flux densities or
                  visibilties that have been placed in the MODEL column
                  by setjy or ft for a model not in the CASA system.
               The syntax is similar to field.  Hence field index or
                  names can be used.
               default: none; example: reference='1328+307'
       transfer -- Transfer field name(s)
               The names of the fields with unknown flux densities.
                  These should be point-like calibrator sources
               The syntax is similar to field.  Hence source index or
                 names can be used.
               default: '' = all sources in caltable that are not specified
                  as reference sources.  Do not include unknown target sources
               example: transfer='1445+099, 3C84'; transfer = '0,4'

               NOTE: All fields in reference and transfer must have solutions
               in the caltable.

       listfile -- Fit listfile name
               The list file contains the flux density, flux density error,
                 S/N, and number of solutions (all antennas and feeds) for each
                 spectral window.  NOTE: The nominal spectral window frequencies
                 will be included in the future.
               default: '' = no fit listfile will be created.

       append -- Append fluxscaled solutions to the fluxtable.
               default: False; (will overwrite if already existing)
               example: append=True
       refspwmap -- Vector of spectral windows enablings scaling across
               spectral windows
               default: [-1]==> none.
               Example with 4 spectral windows:
               if the reference fields were observed only in spw=1 & 3,
               and the transfer fields were observed in all 4 spws (0,1,2,3),
               specify refspwmap=[1,1,3,3].
               This will ensure that transfer fields observed in spws 0,1,2,3
               will be referenced to reference field solutions only in
               spw 1 or 3.

       incremental -- Create an incremental caltable containing only the gain correction 
                    factors. 
               default: False; (older behavior, produces flux scaled gain table)

       fitorder -- the order of spectral fitting when solutions for multiple spws are available
               default: 1

       display -- display statistics (histogram)  of derived correction factors
               default: false
       """

       try:
               casalog.origin('fluxscale')

               mycb = cbtool()
               mycb.open(filename=vis,compress=False,addcorr=False,addmodel=False)
               output = mycb.fluxscale(tablein=caltable,tableout=fluxtable,reference=reference,
                              transfer=transfer,listfile=listfile,append=append,
			      refspwmap=refspwmap,gainthreshold=gainthreshold,antenna=antenna,
                              incremental=incremental,fitorder=fitorder,display=display)
               mycb.close()

               #write history
               try:
                      param_names = fluxscale.func_code.co_varnames[:fluxscale.func_code.co_argcount]
                      param_vals = [eval(p) for p in param_names]
                      write_history(mstool(), vis, 'fluxscale', param_names,
                                    param_vals, casalog)
               except Exception, instance:
                      casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                                   'WARN')

       except Exception, instance:
               print '*** Error ***',instance
               mycb.close()
               raise Exception, instance

       return output
