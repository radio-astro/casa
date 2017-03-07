
import os
import shutil
from taskinit import *
from parallel.parallel_data_helper import ParallelDataHelper


def cvel2(
    vis,
    outputvis,
    keepmms,
    passall,    # hidden parameter for backwards compatibiliy
    field,
    spw,
    scan,
    antenna,
    correlation,
    timerange,
    intent,
    array,
    uvrange,
    observation,
    feed,
    datacolumn,
    mode,
    nchan,
    start,
    width,
    interpolation,
    phasecenter,
    restfreq,
    outframe,
    veltype,
    hanning,
    ):
    
    """ This task used the MSTransform framework. It needs to use the ParallelDataHelper
        class, implemented in parallel.parallel_data_helper.py. 
    """

    def check_do_preaveraging(vis, spw, field, mode, nchan, start, width, phasecenter,
                              restfreq, outframe, veltype):
        """Returns whether this task is going to perform pre-averaging before
        regridding. Introduced to quick-fix CAS-9798. The aim is to do
        the same checks done in task_cvel. All parameters as specified in the task.
        """

        # Reproduce as closely as possible the checks done in task_cevel. But note
        # there is not 'newphasecenter' here as MSTransformManager seems to handle
        # that issue in a different way.
        dopreaverage=False
        
        ms = casac.ms()
        ms.open(vis)
        thespwsel = ms.msseltoindex(vis=vis, spw=spw)['spw']
        thefieldsel = ms.msseltoindex(vis=vis, field=field)['field']
        outgrid =  ms.cvelfreqs(spwids=thespwsel, fieldids=thefieldsel,
                                mode=mode, nchan=nchan, start=start, width=width,
                                phasec=phasecenter, restfreq=restfreq,
                                outframe=outframe, veltype=veltype, verbose=False)
        if(len(outgrid)>1):
            tmpavwidth = []
            for thespw in thespwsel:
                outgridw1 =  ms.cvelfreqs(spwids=[thespw], fieldids=thefieldsel,
                                          mode='channel', nchan=-1, start=0, width=1,
                                          # native width
                                          phasec=phasecenter, restfreq=restfreq,
                                          outframe=outframe, veltype=veltype,
                                          verbose=False)
                if(len(outgridw1)>1):
                    widthratio = abs((outgrid[1]-outgrid[0])/(outgridw1[1]-outgridw1[0]))                    
                    if(widthratio>=2.0): # do preaverage
                        tmpavwidth.append( int(widthratio+0.001) )
                        dopreaverage=True
                else:
                    tmpavwidth.append(1)

                if dopreaverage:
                    preavwidth = tmpavwidth
                
        ms.close()

        if(dopreaverage and (mode=='channel' or mode=='channel_b')):
            if(max(preavwidth)==1):
                dopreaverage = False

        return dopreaverage


    # Initialize the helper class  
    pdh = ParallelDataHelper("cvel2", locals()) 

    casalog.origin('cvel2')
        
    # Validate input and output parameters
    try:
        pdh.setupIO()
    except Exception, instance:
        casalog.post('%s'%instance,'ERROR')
        return False

    # Input vis is an MMS
    if pdh.isParallelMS(vis) and keepmms:
        
        status = True   
        
        # Work the heuristics of combinespws=True and the separationaxis of the input             
        retval = pdh.validateInputParams()
        if not retval['status']:
            raise Exception, 'Unable to continue with MMS processing'
                        
        pdh.setupCluster('cvel2')

        # Execute the jobs
        try:
            status = pdh.go()
        except Exception, instance:
            casalog.post('%s'%instance,'ERROR')
            return status
                           
        return status


    # Create local copy of the MSTransform tool
    mtlocal = mttool()

    tblocal = tbtool()

    try:
        # Gather all the parameters in a dictionary.        
        config = {}
        
        config = pdh.setupParameters(inputms=vis, outputms=outputvis, field=field, 
                    spw=spw, array=array, scan=scan, antenna=antenna, correlation=correlation,
                    uvrange=uvrange,timerange=timerange, intent=intent, observation=observation,
                    feed=feed)

        # Ad-hoc fix for CAS-9798
        if check_do_preaveraging(vis, spw, field, mode, nchan, start, width, phasecenter,
                                 restfreq, outframe, veltype):
            raise RuntimeError('ERROR: cvel2 (and cvel) do not regrid properly for channel '
                               'widths > or = 2 x the native channel width, please use '
                               'clean  or tclean for larger regridding. A fix is expected '
                               'for CASA 5.0, all earlier versions also have this issue.')

        config['datacolumn'] = datacolumn
        casalog.post('Will work on datacolumn = %s'%datacolumn.upper())
        
        # In cvel the spws are always combined
        config['combinespws'] = True
        
        # Hanning smoothing
        config['hanning'] = hanning
        
        # Set the regridms parameter in mstransform
        config['regridms'] = True

        if passall == True:
            casalog.post('Parameter passall=True is not supported in cvel2','WARN')
        
        # Reset the defaults depending on the mode
        # Only add non-empty string parameters to config dictionary
        start, width = pdh.defaultRegridParams()
        config['mode'] = mode
        config['nchan'] = nchan
        if start != '':
            config['start'] = start
        if width != '':
            config['width'] = width

        config['interpolation'] = interpolation
        if restfreq != '':
            config['restfreq'] = restfreq
        if outframe != '':
            config['outframe'] = outframe
        if phasecenter != '':
            config['phasecenter'] = phasecenter
        config['veltype'] = veltype
        config['nspw'] = 1
        config['taql'] = "NOT (FLAG_ROW OR ALL(FLAG))"
        
        # Configure the tool and all the parameters        
        casalog.post('%s'%config, 'DEBUG')
        mtlocal.config(config)
        
        # Open the MS, select the data and configure the output
        mtlocal.open()
        
        # Run the tool
        mtlocal.run()        
            
        mtlocal.done()

    except Exception, instance:
        mtlocal.done()
        casalog.post('%s'%instance,'ERROR')
        return False

    # Write history to output MS, not the input ms.
    try:
        mslocal = mstool()
        param_names = cvel2.func_code.co_varnames[:cvel2.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        write_history(mslocal, outputvis, 'cvel2', param_names,
                      param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')
        return False

    mslocal = None
    
    return True
        
