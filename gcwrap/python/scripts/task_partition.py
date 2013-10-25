import os
import shutil
import string
import copy
import math
from taskinit import *
from task_mstransform import MSTHelper
import flaghelper as fh

def partition(vis,
           outputvis,
           createmms,
           separationaxis,
           numsubms,
           parallel,
           flagbackup,      # only for MMS
           ddistart,        # to be used by mstransform internally. Hidden parameter!
           datacolumn,
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
           feed
           ):
    """Create a multi visibility set from an existing visibility set:

    Keyword arguments:
    vis -- Name of input visibility file (MS)
            default: none; example: vis='ngc5921.ms'
    outputvis -- Name of output visibility file (MS)
                  default: none; example: outputvis='ngc5921_src.ms'
    createmms -- Boolean flag if we're creating Multi MS
                  default: True
        separationaxis -- what axis do we intend to split on.
                   default = 'both'
                   Options: 'scan','spw','both'
        numsubms -- Number of sub-MSs to create.
                    default: 64
        parallel -- Run in parallel or sequential.
                   default: True
        flagbackup -- Backup the FLAG column of the output MMS
                   default: True
                   
    datacolumn -- Which data column to split out
                  default='corrected'; example: datacolumn='data'
                  Options: 'data', 'corrected', 'model', 'all',
                  'float_data', 'lag_data', 'float_data,data', and
                  'lag_data,data'.
                  note: 'all' = whichever of the above that are present.
    field -- Field name
              default: field = '' means  use all sources
              field = 1 # will get field_id=1 (if you give it an
                          integer, it will retrieve the source with that index)
              field = '1328+307' specifies source '1328+307'.
                 Minimum match can be used, egs  field = '13*' will
                 retrieve '1328+307' if it is unique or exists.
                 Source names with imbedded blanks cannot be included.
    spw -- Spectral window index identifier
            default=-1 (all); example: spw=1
    antenna -- antenna names
               default '' (all),
               antenna = '3 & 7' gives one baseline with antennaid = 3,7.
    timerange -- Time range
                 default='' means all times.  examples:
                 timerange = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
                 timerange='< YYYY/MM/DD/HH:MM:SS.sss'
                 timerange='> YYYY/MM/DD/HH:MM:SS.sss'
                 timerange='< ddd/HH:MM:SS.sss'
                 timerange='> ddd/HH:MM:SS.sss'
    scan -- Scan numbers to select.
            default '' (all).
    intent -- Select based on the scan intent.
                  default '' (all)
    array -- (Sub)array IDs to select.     
             default '' (all).
    uvrange -- uv distance range to select.
               default '' (all).
    observation -- observation ID(s) to select.
                   default '' (all).
    """

    casalog.origin('partition')
    
    # Initiate the helper class    
    msth = MSTHelper(locals()) 
    
    # Validate input and output parameters
    try:
        msth.setupIO()
    except Exception, instance:
        casalog.post('%s'%instance,'ERROR')
        return False

    if createmms:   
        
        # The user decides to run in parallel or sequential
        if not parallel:
            casalog.post('Will process the MS in sequential')
            msth.bypassParallelProcessing(1)
        else:
            msth.bypassParallelProcessing(0)
            casalog.post('Will process the MS in parallel')

        # Get a cluster
        msth.setupCluster(thistask='partition')
        
        # Execute the jobs using simple_cluster
        try:
            msth.go()
        except Exception, instance:
            casalog.post('%s'%instance,'ERROR')
            return False
                    
        # Create a backup of the flags that are in the MMS
        casalog.origin('partition')
        if flagbackup and os.path.exists(outputvis):
            casalog.post('Create a backup of the flags that are in the MMS')
            fh.backupFlags(aflocal=None, msfile=outputvis, prename='partition')    

        return True


    # Create local copies of the MSTransform and ms tools
    mtlocal = casac.mstransformer()
    mslocal = mstool()
        
    try:
                    
        # Gather all the parameters in a dictionary.        
        config = {}
        config = msth.setupParameters(inputms=vis, outputms=outputvis, field=field, 
                    spw=spw, array=array, scan=scan, antenna=antenna, correlation=correlation,
                    uvrange=uvrange,timerange=timerange, intent=intent, observation=str(observation),
                    feed=feed)
        
        # ddistart will be used in the tool when re-indexing the spw table
        config['ddistart'] = ddistart
        
        config['datacolumn'] = datacolumn

        # Configure the tool and all the parameters
        
        casalog.post('%s'%config, 'DEBUG')
        mtlocal.config(config)
        
        # Open the MS, select the data and configure the output
        mtlocal.open()
        
        # Run the tool
        casalog.post('Run the tool to partition the MS')
        mtlocal.run()        
            
        mtlocal.done()
                    
    except Exception, instance:
        mtlocal.done()
        casalog.post('%s'%instance,'ERROR')
        return False


    # Write history to output MS, not the input ms.
    try:
        param_names = partition.func_code.co_varnames[:partition.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        casalog.post('Updating the history of the output MS','DEBUG1')
        write_history(mslocal, outputvis, 'partition', param_names,
                      param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')
        return False

    return True
