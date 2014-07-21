import os
import shutil
import string
import copy
import math
from taskinit import *
from parallel.parallel_data_helper import ParallelDataHelper
import testhelper as th

def hanningsmooth2(vis=None, 
                   outputvis=None,
                   keepmms=None,
                   field=None,
                   spw=None, 
                   scan=None, 
                   antenna=None, 
                   correlation=None,
                   timerange=None, 
                   intent=None,
                   array=None,
                   uvrange=None,
                   observation=None,
                   feed=None,
                   datacolumn=None, 
                   ):

    """Hanning smooth frequency channel data to remove Gibbs ringing

    """

    casalog.origin('hanningsmooth2')
    
    
    # Initiate the helper class    
    pdh = ParallelDataHelper("hanningsmooth2", locals()) 

    # Validate input and output parameters
    try:
        pdh.setupIO()
    except Exception, instance:
        casalog.post('%s'%instance,'ERROR')
        return False

    # Input vis is an MMS
    if pdh.isParallelMS(vis) and keepmms:
        
        if not pdh.validateInputParams():        
            raise Exception, 'Unable to continue with MMS processing'
                        
        pdh.setupCluster('hanningsmooth2')

        # Execute the jobs
        try:
            pdh.go()
        except Exception, instance:
            casalog.post('%s'%instance,'ERROR')
            return False
                    
        return True


    # Actual task code starts here

    # Create local copies of the MSTransform and ms tools
    mtlocal = mttool()
    mslocal = mstool()

    try:
                    
        # Gather all the parameters in a dictionary.        
        config = {}
        
        config = pdh.setupParameters(inputms=vis, outputms=outputvis, field=field, 
                    spw=spw, array=array, scan=scan, antenna=antenna, correlation=correlation,
                    uvrange=uvrange,timerange=timerange, intent=intent, observation=observation,
                    feed=feed)
        
        
        # Check if CORRECTED column exists, when requested
        datacolumn = datacolumn.upper()
        if datacolumn == 'CORRECTED':
            dc = 'CORRECTED_DATA'
            if th.getColDesc(vis,dc) == {}:
                casalog.post('Input CORRECTED_DATA does not exist. Will use DATA','WARN')
                datacolumn = 'DATA'
             
        casalog.post('Will use datacolumn = %s'%datacolumn, 'DEBUG')
        config['datacolumn'] = datacolumn
        
        # Call MSTransform framework with hanning=True
        config['hanning'] = True

        # Configure the tool 
        casalog.post('%s'%config, 'DEBUG1')
        mtlocal.config(config)
        
        # Open the MS, select the data and configure the output
        mtlocal.open()
        
        # Run the tool
        casalog.post('Apply Hanning smoothing on data')
        mtlocal.run()        
            
        mtlocal.done()
                    
    except Exception, instance:
        mtlocal.done()
        casalog.post('%s'%instance,'ERROR')
        return False

    # Write history to output MS, not the input ms.
    try:
        param_names = hanningsmooth2.func_code.co_varnames[:hanningsmooth2.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        casalog.post('Updating the history in the output', 'DEBUG1')
        write_history(mslocal, outputvis, 'hanningsmooth2', param_names,
                      param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')
        return False

    mslocal = None
    
    return True
 
 