import os
from taskinit import *

def fluxscale(vis=None,caltable=None,fluxtable=None,reference=None,transfer=None,listfile=None,append=None,refspwmap=None,gainthreshold=None,antenna=None,timerange=None,scan=None,incremental=None,fitorder=None,display=None):
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

       gainthreshold -- % deviation threshold from the median gain to be used flux scaling derivation

       antenna -- Select antennas to be used to derive flux scaling 

       timerange -- Select timerage to be used to derive flux scaling with given antenna selection

       scan -- Select scans to be used to derived flux scaling with given antenna selection

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
                              timerange=timerange,scan=scan,
                              incremental=incremental,fitorder=fitorder,display=display)
               mycb.close()

               #write history
               try:
                      param_names = fluxscale.func_code.co_varnames[:fluxscale.func_code.co_argcount]
                      param_vals = [eval(p) for p in param_names]
                      write_history(mstool(), vis, 'fluxscale', param_names,
                                    param_vals, casalog)
                      writeResultsHistory(mstool(), vis, casalog, output)
               except Exception, instance:
                      casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                                   'WARN')

       except Exception, instance:
               print '*** Error ***',instance
               mycb.close()
               raise Exception, instance

       return output


def writeResultsHistory(myms, vis, mycasalog, indict):
    """                                                 
      write returned output of fluxscale to HISTORY subtable of the parent ms
    """                                                                      
    isOpen = False                                                           
    try:                                                                     
        myms.open(vis)                                                       
        isOpen = True                                                        

        mainkeys = indict.keys()

        spwids = indict['spwID']
        freqs = indict['freq']  
        msg0 = "Fluxscale results *****"
        myms.writehistory(message=msg0, origin='fluxscale')
        for ky in mainkeys:                                
            try:                                           
                fieldid = int(ky)                          
            except:                                        
                fieldid = None                             

            if fieldid!=None:
                fdict = indict[ky]
                fname = fdict['fieldName']   
                fitF = fdict['fitFluxd']     
                fitFerr = fdict['fitFluxdErr']
                fitRefFreq = fdict['fitRefFreq']
                spix = fdict['spidx']           
                spixerr = fdict['spidxerr']     
                msg1 = fname+"(field id="+ky+") " 
                myms.writehistory(message=msg1, origin='fluxscale')
                for ispw in spwids:                                
                    strspw = str(ispw)                             
                    spwfdict = fdict[strspw]                       
                    flux = spwfdict['fluxd']                       
                    fluxerr = spwfdict['fluxdErr']                 
                    freq = freqs[ispw]                             
                    fvalbase = 1.0                                 
                    funit=''                                       
                    if freq>1.e9:
                        fvalbase = 1.e9
                        funit = 'GHz'
                    elif freq>1.e6:
                        fvalbase = 1.e6
                        funit = 'MHz'
                    elif freq>1.e3:
                        fvalbase = 1.e3
                        funit = 'kHz'
                    else:
                        if freq > 0.0:
                            funit = 'Hz'
                    freq = freq/fvalbase
                    if funit!='':
                        funitlast=funit
                        fvalbaselast=fvalbase
                    if freq  < 0.0:
                      msg2 = "  Spw "+strspw+" insufficient data, flux density is not determined."
                    else:
                      msg2 = "  Spw "+strspw+"(freq = {:7.3f}".format(freq)+funit+") Flux density = {:10.6f}".format(flux[0])+\
                             "+/-{:10.6f}".format(fluxerr[0])+" Jy"
                    myms.writehistory(message=msg2, origin='fluxscale')
                if len(spwids) > 1:
                    msg3 = "  Spectral index fitting coefficients [zero-point, alpha, beta] with errors :"
                    msg3a = "["
                    msg3b = "["
                    nspix = len(spix)
                    for ispix in range(nspix):
                        msg3a += "{:5.3f}".format(spix[ispix])
                        msg3b += "{:5.3f}".format(spixerr[ispix])
                        if ispix!=nspix-1:
                           msg3a += ","
                           msg3b += ","
                    msg3a += "]"
                    msg3b += "]"
                    msg3 += msg3a + "+/-" + msg3b
                    fitreffreq = fitRefFreq/fvalbaselast
                    msg4 = "  Fitted flux density = {:10.6f}".format(fitF)+"+/-{:10.6f}".format(fitFerr)+\
                           " Jy (reference freq = {:7.3f}".format(fitreffreq)+funitlast+")"
                    myms.writehistory(message=msg3, origin='fluxscale')
                    myms.writehistory(message=msg4, origin='fluxscale')
    except Exception, instance:
        mycasalog.post("*** Error \'%s\' updating fluxscale results in HISTORY of %s" % (instance, vis), 'WARN' )

    finally:
        if isOpen:
            myms.close()

