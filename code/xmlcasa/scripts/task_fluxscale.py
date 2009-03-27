import os
from taskinit import *

def fluxscale(vis=None,caltable=None,fluxtable=None,reference=None,transfer=None,append=None,refspwmap=None):
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

       """

       try:

               casalog.origin('fluxscale')

               cb.open(vis)
               cb.fluxscale(tablein=caltable,tableout=fluxtable,reference=reference,
               transfer=transfer,append=append,refspwmap=refspwmap)
               cb.close()


               #write history
               ms.open(vis,nomodify=False)
               ms.writehistory(message='taskname = fluxscale',origin='fluxscale')
               ms.writehistory(message='vis         = "'+str(vis)+'"',origin='fluxscale')
               ms.writehistory(message='caltable    = "'+str(caltable)+'"',origin='fluxscale')
               ms.writehistory(message='fluxtable   = "'+str(fluxtable)+'"',origin='fluxscale')
               ms.writehistory(message='reference   = '+str(reference),origin='fluxscale')
               ms.writehistory(message='transfer    = '+str(transfer),origin='fluxscale')
               ms.writehistory(message='append      = '+str(append),origin='fluxscale')
               ms.writehistory(message='refspwmap   = '+str(refspwmap),origin='fluxscale')
               ms.close()

       except Exception, instance:
               print '*** Error ***',instance
               cb.close()
               raise Exception, instance
