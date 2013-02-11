import os
import shutil
from taskinit import *

# Decorator function to print the arguments of a function
def dump_args(func):
    "This decorator dumps out the arguments passed to a function before calling it"
    argnames = func.func_code.co_varnames[:func.func_code.co_argcount]
    fname = func.func_name
   
    def echo_func(*args,**kwargs):
        print fname, ":", ', '.join('%s=%r' % entry for entry in zip(argnames,args) + kwargs.items())
        return func(*args, **kwargs)
   
    return echo_func

class MSTHelper():
    def __init__(self, args={}):
        '''what can we do here?'''
        # Validate the input parameters
        # Raise exception on input parameters
        # one method per functionality of the task?
        # def combspw_setup(self, **pars)
        
        # Create a copy of the original local parameters
        self.__origpars = args.copy()
        self.__selpars = {}        
            
    def setupIO(self):
        '''Validate input and output parameters'''
        
        for k,v in self.__origpars.items():
            if k == 'vis' and isinstance(v, str):
                # only one input MS
                if not os.path.exists(v):
                    raise Exception, 'Visibility data set not found - please verify the name.'
            elif k == 'vis' and isinstance(v, list):
                casalog.post('Multiple input MSs is not yet implemented.','WARN')
                raise Exception, 'Multiple input MSs is not yet implemented.'
            # LATER: multiple input
#            elif k == 'vis' and isinstance(v, list):
#                # LATER: multiple input MSs
#                for ms in v:
#                    if not os.path.exists(ms):
#                        casalog.post('Visibility data set not found - please verify the name','SEVERE')
#                        raise Exception, 'Visibility data set not found - please verify the name'
                
            elif k == 'outputvis' and isinstance(v, str):
                # only one output MS
                if v.isspace() or v.__len__() == 0:
                    casalog.post('Please specify outputvis.','SEVERE')
                    raise ValueError, 'Please specify outputvis.'
                elif os.path.exists(v):
                    casalog.post("Output MS %s already exists - will not overwrite it."%v,'SEVERE')
                    raise ValueError, "Output MS %s already exists - will not overwrite it."%v                    
#            elif k == 'outputvis' and isinstance(v, list):
#                # multiple output MSs
#                for ms in v:
#                    if ms.isspace() or ms.__len__() == 0:
#                        casalog.post('Please specify outputvis','SEVERE')
#                        raise ValueError, 'Please specify outputvis'
#                    elif os.path.exists(ms):
#                        casalog.post("Output MS %s already exists - will not overwrite it."%ms,'SEVERE')
#                        raise ValueError, "Output MS %s already exists - will not overwrite it."%ms
        
        return True 
    
#    @dump_args
    def setupParameters(self, **pars):
        '''Create a dictionary with non-empty parameters'''
        
        seldict = {}
        for k,v in pars.items():
            if v != None and v != "":
                self.__selpars[k] = v
        
        return self.__selpars
    
    @dump_args
    def makeInputLists(self):
        '''Make data selection parameters lists if
        they are not, when a list of input MSs is requested'''
        
        # input is a list of MSs
        if (isinstance(self.__origpars['vis'], list)):
            lsize = self.__origpars['vis'].__len__()
            # Check data selection parameters
            for k,v in self.__selpars.items():
                if not isinstance(v, list):                    
                    # make it a list
                    listvalue = []
                    for i in range(lsize):
                        listvalue.append(v)
                        self.__selpars[k] = listvalue
                        
        return self.__selpars
            
            
        
    @dump_args 
    def freqAvg(self, **pars):
        ''' Get the sub-parameters for freqaverage=True
            which are freqbin and useweights'''
        
        for k,v in pars.items():
            if k == "freqbin":
                fb = v 
            elif k == "useweights":
                wght = v
        
        # Validate the parameters
        
#@dump_args
def mstransform(
             vis, 
             outputvis,           # output
             createmms,           # MMS --> partition
             separationaxis, 
             numsubms,
             tileshape,          # tiling
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
             realmodelcol,
             combinespws,        # spw combination --> cvel
             freqaverage,        # frequency averaging --> split
             freqbin,
             useweights,
             hanning,            # Hanning --> cvel
             regridms,           # regridding to new frame --> cvel
             mode, 
             nchan, 
             start, 
             width, 
             interpolation,
             phasecenter,
             restfreq, 
             outframe, 
             veltype,
             separatespws,       # spw separation
             nspws,
             timeaverage,        # time averaging --> split
             timebin,
             timespan,
             quantize_c,
             minbaselines              
             ):

    ''' This task can replace applycal, concat, cvel, partition and split '''
    
    casalog.origin('mstransform')
    
    # If multiple MSs, process them in parallel
    if createmms:
        # TODO: use the cluster infrastructure
        createmms = False
        pass
   
    # Initialize the helper class
    mth = MSTHelper(locals())
    
    # Validate input and output parameters
    mth.setupIO()
    
    # LATER: Check if selection lists match input MS list
#    mth.setupParameters(field=field, spw=spw, array=array, scan=scan, 
#                                    antenna=antenna, uvrange=uvrange, time=timerange, 
#                                    intent=intent, observation=observation,feed=feed)
    
#    mth.makeInputLists()

    # Create a local copy of the MSTransform tool
    mtlocal = casac.mstransformer()
        
    try:
        # LATER: Loop through list of input MSs
                    
        # Gather all the parameters in a dictionary.
        
        # Add to dictionary only the non-empty parameters
        config = {}
        config = mth.setupParameters(inputms=vis, outputms=outputvis, field=field, 
                    spw=spw, array=array, scan=scan, antenna=antenna, uvrange=uvrange, 
                    time=timerange, intent=intent, observation=str(observation),feed=feed)
        
        config['datacolumn'] = datacolumn
        config['realmodelcol'] = realmodelcol

        if combinespws:
            casalog.post('Combine spws %s into new output spw'%spw)
            config['combinespws'] = True
        if freqaverage:
            casalog.post('Frequency averaging is not yet implemented', 'WARN')
#            config['freqaverage'] = True
#            config['freqbin'] = freqbin
#            config['useweights'] = useweights
        if hanning:
            casalog.post('Apply Hanning smoothing')
            config['hanning'] = True
        if regridms:
            casalog.post('Parse regridding parameters')
            
            config['regridms'] = True
            config['mode'] = mode
            config['nchan'] = nchan
            config['start'] = str(start)
            config['width'] = str(width)
            config['interpolation'] = interpolation
            config['phasecenter'] = phasecenter
            config['restfreq'] = restfreq
            config['outframe'] = outframe
            config['veltype'] = veltype
        if separatespws:
            casalog.post('Separation of spws is not yet implemented', 'WARN')
#            config['nspws'] = nspws # this will change
        if timeaverage:
            casalog.post('Time averaging is not yet implemented', 'WARN')
#            config['timebin'] = timebin
#            config['timespan'] = timespan
#            config['quantize_c'] = quantize_c
#            config['minbaselines'] = minbaselines
        
        # Configure the tool and all the parameters
        mtlocal.config(config)
        
        # Open the MS, select the data and configure the output
        mtlocal.open()
        
        # Run the tool
        casalog.post('Apply the transformations')
        mtlocal.run()        
            
        mtlocal.done()
                    
    except Exception, instance:
        mtlocal.done()
        raise Exception, instance
    
        
    
    
    
    
    
    
    
    
    
    
    
    
    
    
