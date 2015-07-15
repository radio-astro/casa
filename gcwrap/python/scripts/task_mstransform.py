
import os, re
import shutil
import string
import copy
import math
import time
from taskinit import *
from parallel.parallel_data_helper import ParallelDataHelper
import flaghelper as fh
from update_spw import update_spwchan
import inspect
from callibrary import callibrary


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
             keepflags,
             usewtspectrum,
             combinespws,        # spw combination --> cvel
             chanaverage,        # channel averaging --> split
             chanbin,
             hanning,            # Hanning --> cvel
             regridms,           # regridding to new frame --> cvel
             mode, 
             nchan, 
             start, 
             width, 
             nspw,               # spw separation
             interpolation,
             phasecenter,
             restfreq, 
             outframe, 
             veltype,
             timeaverage,        # time averaging --> split
             timebin,
             timespan,
             maxuvwdistance,
             calibration,
             callib,
             disableparallel,    # HIDDEN parameter to create MMS in sequential
             ddistart,           # HIDDEN internal parameter for the sub-table re-indexing
             taql,               # HIDDEN internal parameter
             monolithic_processing      # HIDDEN parameter for the MMS monolithic processing
             ):

    ''' This task can replace split, cvel, partition and hanningsmooth '''
    
    casalog.origin('mstransform')
    
    ''' HEURISTICS:
            input MS  -->  output MS
            input MMS -->  output MMS
    
        user can set createmms=True to create the following:
            input MS  -->  output MMS
   
    '''
    # Initialize the helper class
    pdh = ParallelDataHelper('mstransform', locals())
    
    # When dealing with MMS, process in parallel or sequential
    # disableparallel is a hidden parameter. Only for debugging purposes!
    if disableparallel:
        pdh.bypassParallelProcessing(1)
    else:
        pdh.bypassParallelProcessing(0)
    
    # Validate input and output parameters
    try:
        pdh.setupIO()
    except Exception, instance:
        casalog.post('%s'%instance,'ERROR')
        return False

    # Process the input Multi-MS
    if ParallelDataHelper.isParallelMS(vis) == True and monolithic_processing == False:
        '''
        retval{'status': True,  'axis':''}         --> can run in parallel        
        retval{'status': False, 'axis':'value'}    --> treat MMS as monolithic MS, set new axis for output MMS
        retval{'status': False, 'axis':''}         --> treat MMS as monolithic MS, create an output MS
        '''
        
        retval = pdh.validateInputParams()
        # Cannot create an output MMS.
        if retval['status'] == False and retval['axis'] == '':
            casalog.post('Cannot process MMS with the requested transformations','WARN')
            casalog.post('Use task listpartition to see the contents of the MMS')
            casalog.post('Will create an output MS','WARN')
            createmms = False
            
        # MMS is processed as monolithic MS. 
        elif retval['status'] == False and retval['axis'] != '':
            createmms = True
            pdh.override__args('createmms', True)
            pdh.override__args('monolithic_processing', True)
            separationaxis = retval['axis']
            pdh.override__args('separationaxis', retval['axis'])
            casalog.post("Will process the input MMS as a monolithic MS",'WARN')
            casalog.post("Will create an output MMS with separation axis \'%s\'"%retval['axis'],'WARN')
            
        # MMS is processed in parallel
        else:
            createmms = False
            try:
                pdh.override__args('createmms', False)
                pdh.setupCluster('mstransform')
                pdh.go()
            except Exception, instance:
                casalog.post('%s'%instance,'ERROR')
                return False
            
            return True
                
    # Create an output Multi-MS
    if createmms == True:
        
        # Check the heuristics of separationaxis and the requested transformations
        pval = pdh.validateOutputParams()
        if pval == 0:
            raise Exception, 'Cannot create MMS using separationaxis=%s with some of the requested transformations.'\
                            %separationaxis
                             
        try:
            pdh.setupCluster('mstransform')
            pdh.go()
            monolithic_processing = False
        except Exception, instance:
            casalog.post('%s'%instance,'ERROR')
            return False
        
        return True
                    
        
    # Create a local copy of the MSTransform tool
    mtlocal = casac.mstransformer()
    mslocal = mstool()
        
    try:
                    
        # Gather all the parameters in a dictionary.
        config = {}
        
        if keepflags:
            taqlstr = ''
        else:
            taqlstr = "NOT (FLAG_ROW OR ALL(FLAG))"
        
        # MMS taql selection
        if taql != '' and taql != None:
            if not keepflags:
                taqlstr = taqlstr + " AND "+taql
            else:
                taqlstr = taql
        
        config = pdh.setupParameters(inputms=vis, outputms=outputvis, field=field, 
                    spw=spw, array=array, scan=scan, antenna=antenna, correlation=correlation,
                    uvrange=uvrange,timerange=timerange, intent=intent, observation=str(observation),
                    feed=feed, taql=taqlstr)
        
        # ddistart will be used in the tool when re-indexing the spw table
        config['ddistart'] = ddistart
        
        config['datacolumn'] = datacolumn
        dc = datacolumn.upper()            
        # Make real a virtual MODEL column in the output MS
        if "MODEL" in dc or dc == 'ALL':
            config['realmodelcol'] = realmodelcol

        config['usewtspectrum'] = usewtspectrum
        
        # Add the tile shape parameter
        if tileshape.__len__() == 1:
            # The only allowed values are 0 or 1
            if tileshape[0] != 0 and tileshape[0] != 1:
                raise ValueError, 'When tileshape has one element, it should be either 0 or 1.'
                
        elif tileshape.__len__() != 3:
            # The 3 elements are: correlations, channels, rows
            raise ValueError, 'Parameter tileshape must have 1 or 3 elements.'
            
        config['tileshape'] = tileshape                

        if combinespws:
            casalog.post('Combine spws %s into new output spw'%spw)
            config['combinespws'] = True
            
        # Only parse chanaverage if chanbin is valid
        if chanaverage and isinstance(chanbin, int) and chanbin <= 1:
            raise Exception, 'Parameter chanbin must be > 1 to do channel averaging'
            
        # Validate the case of int or list chanbin
        if chanaverage and pdh.validateChanBin():
            casalog.post('Parse channel averaging parameters')
            config['chanaverage'] = True
            
            # convert numpy types, until CAS-6493 is not fixed
            chanbin = fh.evaluateNumpyType(chanbin)
            config['chanbin'] = chanbin
            
        if hanning:
            casalog.post('Apply Hanning smoothing')
            config['hanning'] = True
            
        if regridms:
            casalog.post('Parse regridding parameters')            
            config['regridms'] = True
            # Reset the defaults depending on the mode
            # Only add non-empty string parameters to config dictionary
            start, width = pdh.defaultRegridParams()
            config['mode'] = mode
            config['nchan'] = nchan
            if start != '':
                config['start'] = start
            if width != '':
                config['width'] = width
            if nspw > 1:
                casalog.post('Separate MS into %s spws'%nspw)
            config['nspw'] = nspw
            config['interpolation'] = interpolation
            if restfreq != '':
                config['restfreq'] = restfreq
            if outframe != '':
                config['outframe'] = outframe
            if phasecenter != '':
                config['phasecenter'] = phasecenter
            config['veltype'] = veltype
            
        # Only parse timeaverage parameters when timebin > 0s
        if timeaverage:
            tb = qa.convert(qa.quantity(timebin), 's')['value']
            if not tb > 0:
                raise Exception, "Parameter timebin must be > '0s' to do time averaging"
                       
        if timeaverage:
            casalog.post('Parse time averaging parameters')
            config['timeaverage'] = True
            config['timebin'] = timebin
            config['timespan'] = timespan
            config['maxuvwdistance'] = maxuvwdistance
            
        if calibration:
            casalog.post('Parse calibration parameters')
            mycallib = callibrary()
            mycallib.read(callib)
            config['calibration'] = True
            config['callib'] = mycallib.cld
        
        # Configure the tool and all the parameters
        
        casalog.post('%s'%config, 'DEBUG')
        mtlocal.config(config)
        
        # Open the MS, select the data and configure the output
        mtlocal.open()
        
        # Run the tool
        casalog.post('Apply the transformations')
        mtlocal.run()        
            
        mtlocal.done()
                    
    except Exception, instance:
        mtlocal.done()
        casalog.post('%s'%instance,'ERROR')
        return False


    # Update the FLAG_CMD sub-table to reflect any spw/channels selection
    if ((spw != '') and (spw != '*')) or chanaverage == True:
        isopen = False
        mytb = tbtool()
        try:
            mytb.open(outputvis + '/FLAG_CMD', nomodify=False)
            isopen = True
            nflgcmds = mytb.nrows()
            
            if nflgcmds > 0:
                mademod = False
                cmds = mytb.getcol('COMMAND')
                widths = {}
                #print "width =", width
                if hasattr(chanbin, 'has_key'):
                    widths = chanbin
                else:
                    if hasattr(chanbin, '__iter__') and len(chanbin) > 1:
                        for i in xrange(len(chanbin)):
                            widths[i] = chanbin[i]
                    elif chanbin != 1:
#                        print 'using ms.msseltoindex + a scalar width'
                        numspw = len(mslocal.msseltoindex(vis=vis,
                                                     spw='*')['spw'])
                        if hasattr(chanbin, '__iter__'):
                            w = chanbin[0]
                        else:
                            w = chanbin
                        for i in xrange(numspw):
                            widths[i] = w
#                print 'widths =', widths 
                for rownum in xrange(nflgcmds):
                    # Matches a bare number or a string quoted any way.
                    spwmatch = re.search(r'spw\s*=\s*(\S+)', cmds[rownum])
                    if spwmatch:
                        sch1 = spwmatch.groups()[0]
                        sch1 = re.sub(r"[\'\"]", '', sch1)  # Dequote
                        # Provide a default in case the split selection excludes
                        # cmds[rownum].  update_spwchan() will throw an exception
                        # in that case.
                        cmd = ''
                        try:
                            #print 'sch1 =', sch1
                            sch2 = update_spwchan(vis, spw, sch1, truncate=True,
                                                  widths=widths)
                            #print 'sch2 =', sch2
                            ##print 'spwmatch.group() =', spwmatch.group()
                            if sch2:
                                repl = ''
                                if sch2 != '*':
                                    repl = "spw='" + sch2 + "'"
                                cmd = cmds[rownum].replace(spwmatch.group(), repl)
                        #except: # cmd[rownum] no longer applies.
                        except Exception, e:
                            casalog.post(
                                "Error %s updating row %d of FLAG_CMD" % (e,
                                                                          rownum),
                                         'WARN')
                            casalog.post('sch1 = ' + sch1, 'DEBUG1')
                            casalog.post('cmd = ' + cmd, 'DEBUG1')
                        if cmd != cmds[rownum]:
                            mademod = True
                            cmds[rownum] = cmd
                if mademod:
                    casalog.post('Updating FLAG_CMD', 'INFO')
                    mytb.putcol('COMMAND', cmds)

            mytb.close()
            
        except Exception, instance:
            if isopen:
                mytb.close()
            mslocal = None
            mytb = None
            casalog.post("*** Error \'%s\' updating FLAG_CMD" % (instance),
                         'SEVERE')
            return False

    mytb = None

    # Write history to output MS, not the input ms.
    try:
        param_names = mstransform.func_code.co_varnames[:mstransform.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        write_history(mslocal, outputvis, 'mstransform', param_names,
                      param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')
        return False

    mslocal = None
    
    return True
    
 
    
