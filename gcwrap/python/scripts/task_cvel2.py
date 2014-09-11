
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
        class, implemented in parallel.parallel_data_helper.py. The differences of cvel2
        and cvel and the following:
    
    ....* cvel2 only implements the passall = False behavior
    ....* it calls the MSTransform framework always with combinespws = True
    ....* add a datacolumn parameter
    ....
    """
    
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
        
#         casalog.origin('cvel')
# 
#         if not (type(vis) == str) & os.path.exists(vis):
#             raise Exception, \
#                 'Visibility data set not found - please verify the name'
# 
#         if outputvis == '':
#             raise Exception, \
#                 'Must provide output data set name in parameter outputvis.'
# 
#         if os.path.exists(outputvis):
#             raise Exception, \
#                 'Output MS %s already exists - will not overwrite.' \
#                 % outputvis
# 
#         if os.path.exists(outputvis + '.flagversions'):
#             raise Exception, \
#                 'The flagversions "%s.flagversions" for the output MS already exist. Please delete.' \
#                 % outputvis

        # Handle selectdata explicitly
        #  (avoid hidden globals)
#         if selectdata == False:
#             timerange = ''
#             array = ''
#             antenna = ''
#             scan = ''
# 
#         if type(antenna) == list:
#             antenna = ', '.join([str(ant) for ant in antenna])
# 
#         if field == '':
#             field = '*'
# 
#         if spw == '':
#             spw = '*'

#         if passall and spw == '*' and field == '*':
#             # all spws and fields selected, nothing to pass through
#             passall = False
# 
#         doselection = True
#         if field == '*' and spw == '*' and (not selectdata
#                 or selectdata and antenna == '' and timerange == ''
#                 and scan == '' and array == ''):
#             doselection = False

        # open input MS
#        ms.open(vis)

#         if type(phasecenter) == str:
#             # ## blank means take field 0
#             if phasecenter == '':
#                 phasecenter = ms.msseltoindex(vis, field=field)['field'][0]
#             else:
#                 tmppc = phasecenter
#                 try:
#                     if len(ms.msseltoindex(vis,
#                            field=phasecenter)['field']) > 0:
#                         tmppc = ms.msseltoindex(vis,
#                                 field=phasecenter)['field'][0]
#                 except Exception, instance:
#                         # #succesful must be string like '0' or 'NGC*'
#                     # #failed must be a string 'J2000 18h00m00 10d00m00'
#                     tmppc = phasecenter
#                 phasecenter = tmppc
# 
#         newphasecenter = phasecenter
#         phasecentername = phasecenter
#         if not type(phasecenter) == str:
#             # first check that this field will be in the output MS
#             if not phasecenter in ms.msseltoindex(vis,
#                     field=field)['field']:
#                 message = 'Field id ' + str(phasecenter)
#                 message += \
#                     ' was selected as phasecenter but is not among the fields selected for output: '
#                 message += str(ms.msseltoindex(vis, field=field)['field'
#                                ])
#                 ms.close()
#                 raise Exception, message
# 
#             tb.open(vis + '/FIELD')
#             try:
#                 # get the name for later display
#                 phasecentername = tb.getcell('NAME', phasecenter) \
#                     + ' (original field ' + str(phasecenter)
#                 tb.close()
#                 # phase center id will change if we select on fields,
#                 # the name column is only optionally filled and not necessarily unique.
#                 # But ms.msseltoindex(vis,field=field)['field'] returns the old field ids
#                 # in the order in which they will occur in the new field table.
#                 # => need to get index from there as new phase center ID
#                 newphasecenter = ms.msseltoindex(vis,
#                         field=field)['field'
#                         ].tolist().index(phasecenter)
#                 phasecentername += ', new field ' + str(newphasecenter) \
#                     + ')'
#             except:
#                 tb.close()
#                 message = 'phasecenter field id ' + str(phasecenter) \
#                     + ' is not valid'
#                 ms.close()
#                 raise Exception, message

#         if mode == 'frequency':
#              # reset the default values
#             if start == 0:
#                 start = ''
#             if width == 1:
#                 width = ''
#              #check that start and width have units if they are non-empty
#             if not start == '':
#                 if qa.quantity(start)['unit'].find('Hz') < 0:
#                     ms.close()
#                     raise TypeError, \
#                         'start parameter is not a valid frequency quantity ' \
#                         % start
#             if not width == '' and qa.quantity(width)['unit'].find('Hz'
#                     ) < 0:
#                 ms.close()
#                 raise TypeError, \
#                     'width parameter %s is not a valid frequency quantity ' \
#                     % width
#         elif mode == 'velocity':
# 
#             # # reset the default values
#             if start == 0:
#                 start = ''
#             if width == 1:
#                 width = ''
#             # #check that start and width have units if they are non-empty
#             if not start == '':
#                 if qa.quantity(start)['unit'].find('m/s') < 0:
#                     ms.close()
#                     raise TypeError, \
#                         'start parameter %s is not a valid velocity quantity ' \
#                         % start
#             if not width == '' and qa.quantity(width)['unit'].find('m/s'
#                     ) < 0:
#                 ms.close()
#                 raise TypeError, \
#                     'width parameter %s is not a valid velocity quantity ' \
#                     % width
#         elif mode == 'channel' or mode == 'channel_b':
# 
#             if type(width) != int or type(start) != int:
#                 ms.close()
#                 raise TypeError, \
#                     'start and width have to be integers with mode = %s' \
#                     % mode

        # # check if preaveraging is necessary
#         dopreaverage = False
#         preavwidth = [1]
# 
#         thespwsel = ms.msseltoindex(vis=vis, spw=spw)['spw']
#         thefieldsel = ms.msseltoindex(vis=vis, field=field)['field']
#         outgrid = ms.cvelfreqs(
#             spwids=thespwsel,
#             fieldids=thefieldsel,
#             mode=mode,
#             nchan=nchan,
#             start=start,
#             width=width,
#             phasec=newphasecenter,
#             restfreq=restfreq,
#             outframe=outframe,
#             veltype=veltype,
#             verbose=False,
#             )
#         if len(outgrid) > 1:
#             tmpavwidth = []
#             for thespw in thespwsel:
#                 outgridw1 = ms.cvelfreqs(  # native width
#                     spwids=[thespw],
#                     fieldids=thefieldsel,
#                     mode='channel',
#                     nchan=-1,
#                     start=0,
#                     width=1,
#                     phasec=newphasecenter,
#                     restfreq=restfreq,
#                     outframe=outframe,
#                     veltype=veltype,
#                     verbose=False,
#                     )
#                 if len(outgridw1) > 1:
#                     widthratio = abs((outgrid[1] - outgrid[0])
#                             / (outgridw1[1] - outgridw1[0]))
#                     if widthratio >= 2.0:  # do preaverage
#                         tmpavwidth.append(int(widthratio + 0.001))
#                         dopreaverage = True
#                 else:
#                     tmpavwidth.append(1)
# 
#             if dopreaverage:
#                 preavwidth = tmpavwidth
# 
#         ms.close()

#         # if in channel mode and preaveraging,
#         if dopreaverage and (mode == 'channel' or mode == 'channel_b'):
#             if max(preavwidth) == 1:
#                 dopreaverage = False
#             else:
#                 # switch to frequency mode
#                 mode = 'frequency'
#                 if width > 0:
#                     start = str(outgrid[0] / 1E6) + 'MHz'
#                     width = str(outgrid[1] - outgrid[0]) + 'Hz'
#                 else:
#                     start = str(outgrid[len(outgrid) - 1] / 1E6) + 'MHz'
#                     width = str(-(outgrid[1] - outgrid[0])) + 'Hz'
#                 casalog.post('After pre-averaging, will switch to frequency mode with start='
#                               + start + ', width = ' + width, 'INFO')
# 
#         if dopreaverage and hanning and max(preavwidth) > 2:
#             casalog.post('NOTE: You have requested Hanning smoothing and at the same time you have chosen\n'
# 
#                          + 'a large width parameter which makes pre-averaging necessary.\n'
# 
#                          + 'The Hanning-smoothing may be redundant in this context.\n'
#                          , 'WARN')

#         # determine parameter "datacolumn"
#         tb.open(vis)
#         allcols = tb.colnames()
#         tb.close()
#         dpresent = 'DATA' in allcols
#         mpresent = 'MODEL_DATA' in allcols
#         cpresent = 'CORRECTED_DATA' in allcols
#         if dpresent and cpresent:
#             datacolumn = 'all'
#         elif dpresent and not cpresent:
#             datacolumn = 'data'
#         elif cpresent and not dpresent:
#             datacolumn = 'corrected_data'
#         elif mpresent and not cpresent and not dpresent:
#             datacolumn = 'model_data'
#         else:
#             raise Exception, \
#                 'Neither DATA nor CORRECTED_DATA nor MODEL_DATA column present. Cannot proceed.'

#         if doselection and not dopreaverage:
#             casalog.post('Creating selected SubMS ...', 'INFO')
#             ms.open(vis)
#             ms.split(
#                 outputms=outputvis,
#                 field=field,
#                 spw=spw,
#                 step=[1],
#                 baseline=antenna,
#                 subarray=array,
#                 timebin='-1s',
#                 time=timerange,
#                 whichcol=datacolumn,
#                 scan=scan,
#                 uvrange='',
#                 )
#             ms.close()
#         elif dopreaverage and not doselection:
# 
#             if hanning:
#                 casalog.post('Creating working copy for Hanning-smoothing ...'
#                              , 'INFO')
#                 shutil.rmtree(outputvis + 'TMP', ignore_errors=True)
#                 shutil.copytree(vis, outputvis + 'TMP')
#                 ms.open(outputvis + 'TMP', nomodify=False)
#                 ms.hanningsmooth(datacolumn=datacolumn)
#                 ms.close()
#                 hanning = False
#                 ms.open(outputvis + 'TMP')
#             else:
#                 ms.open(vis)
# 
#             casalog.post('Creating preaveraged SubMS using widths '
#                          + str(preavwidth), 'INFO')
#             ms.split(outputms=outputvis, whichcol=datacolumn,
#                      step=preavwidth)
#             ms.close()
#         elif doselection and dopreaverage:
# 
#             if hanning:
#                 casalog.post('Creating selected working copy for Hanning-smoothing ...'
#                              , 'INFO')
#                 shutil.rmtree(outputvis + 'TMP', ignore_errors=True)
#                 ms.open(vis)
#                 ms.split(
#                     outputms=outputvis + 'TMP',
#                     field=field,
#                     spw=spw,
#                     step=[1],
#                     baseline=antenna,
#                     subarray=array,
#                     timebin='-1s',
#                     time=timerange,
#                     whichcol=datacolumn,
#                     scan=scan,
#                     uvrange='',
#                     )
#                 ms.close()
#                 ms.open(outputvis + 'TMP', nomodify=False)
#                 ms.hanningsmooth(datacolumn=datacolumn)
#                 ms.close()
#                 hanning = False
#                 ms.open(outputvis + 'TMP')
#                 casalog.post('Creating preaveraged SubMS using widths '
#                              + str(preavwidth), 'INFO')
#                 ms.split(outputms=outputvis, whichcol=datacolumn,
#                          step=preavwidth)
#                 ms.close()
#             else:
#                 casalog.post('Creating selected, preaveraged SubMS using widths '
#                               + str(preavwidth), 'INFO')
#                 ms.open(vis)
#                 ms.split(
#                     outputms=outputvis,
#                     field=field,
#                     spw=spw,
#                     step=preavwidth,
#                     baseline=antenna,
#                     subarray=array,
#                     timebin='-1s',
#                     time=timerange,
#                     whichcol=datacolumn,
#                     scan=scan,
#                     uvrange='',
#                     )
#                 ms.close()
#         else:
# 
#             # no selection or preaveraging necessary, just copy
#             casalog.post('Creating working copy ...', 'INFO')
#             shutil.rmtree(outputvis, ignore_errors=True)
#             shutil.copytree(vis, outputvis)

        # Combine and if necessary regrid it
#         ms.open(outputvis, nomodify=False)
# 
#         message = 'Using ' + phasecentername \
#             + ' as common direction for the output reference frame.'
#         casalog.post(message, 'INFO')
# 
#         if not ms.cvel(
#             mode=mode,
#             nchan=nchan,
#             start=start,
#             width=width,
#             interp=interpolation,
#             phasec=newphasecenter,
#             restfreq=restfreq,
#             outframe=outframe,
#             veltype=veltype,
#             hanning=hanning,
#             ):
#             ms.close()
#             raise Exception, 'Error in regridding step ...'
#         ms.close()

        # deal with the passall option
#         temp_suffix = '.deselected'
#         if passall:
#             # determine range of fields
#             fieldsel = ms.msseltoindex(vis=vis, field=field)['field']
#             tb.open(vis + '/FIELD')
#             nfields = tb.nrows()
#             tb.close()
#             fielddesel = ''
#             for i in xrange(0, nfields):
#                 if not i in fieldsel:
#                     if not fielddesel == '':
#                         fielddesel += ','
#                     fielddesel += str(i)
# 
#             # determine range of SPWs
#             spwsel = ms.msseltoindex(vis=vis, spw=spw)['spw']
#             tb.open(vis + '/SPECTRAL_WINDOW')
#             nspws = tb.nrows()
#             tb.close()
#             spwdesel = ''
#             for i in xrange(0, nspws):
#                 if not i in spwsel:
#                     if not spwdesel == '':
#                         spwdesel += ','
#                     spwdesel += str(i)
# 
#             if not (fielddesel == '' and spwdesel == ''):
#                 # split out the data not selected by the conditions on field and spw
#                 # from the original MS and join it to the output MS
# 
#                 # need to do this in two steps
#                 # I) field == "*" and deselected spws
#                 if not spwdesel == '':
#                     ms.open(vis)
#                     casalog.post('Passing through data with', 'INFO')
#                     casalog.post('      spws:   ' + spwdesel, 'INFO')
# 
#                     ms.split(
#                         outputms=outputvis + temp_suffix,
#                         field='*',
#                         spw=spwdesel,
#                         step=[1],
#                         baseline=antenna,
#                         subarray=array,
#                         timebin='-1s',
#                         time=timerange,
#                         whichcol=datacolumn,
#                         scan=scan,
#                         uvrange='',
#                         )
#                     ms.close()
# 
#                     # join with the deselected part
#                     ms.open(outputvis, nomodify=False)
#                     rval = ms.concatenate(msfile=outputvis
#                             + temp_suffix)
#                     ms.close()
#                     shutil.rmtree(outputvis + temp_suffix)
#                     if not rval:
#                         raise Exception, \
#                             'Error in attaching passed-through data ...'
# 
#                 # II) deselected fields and selected spws
#                 if not fielddesel == '':
#                     ms.open(vis)
#                     casalog.post('Passing through data with', 'INFO')
#                     casalog.post('    fields: ' + fielddesel, 'INFO')
#                     casalog.post('      spws: ' + spw, 'INFO')
# 
#                     ms.split(
#                         outputms=outputvis + temp_suffix,
#                         field=fielddesel,
#                         spw=spw,
#                         step=[1],
#                         baseline=antenna,
#                         subarray=array,
#                         timebin='-1s',
#                         time=timerange,
#                         whichcol=datacolumn,
#                         scan=scan,
#                         uvrange='',
#                         )
#                     ms.close()
# 
#                     # join with the deselected part
#                     ms.open(outputvis, nomodify=False)
#                     rval = ms.concatenate(msfile=outputvis
#                             + temp_suffix)
#                     ms.close()
#                     shutil.rmtree(outputvis + temp_suffix)
#                     if not rval:
#                         raise Exception, \
#                             'Error in attaching passed-through data ...'

    # Write history to output MS
#         ms.open(outputvis, nomodify=False)
#         ms.writehistory(message='taskname=cvel', origin='cvel')
#         ms.writehistory(message='vis         = "' + str(vis) + '"',
#                         origin='cvel')
#         ms.writehistory(message='outputvis   = "' + str(outputvis) + '"'
#                         , origin='cvel')
#         ms.writehistory(message='passall     = "' + str(passall) + '"',
#                         origin='cvel')
#         ms.writehistory(message='field       = "' + str(field) + '"',
#                         origin='cvel')
#         ms.writehistory(message='spw         = ' + str(spw),
#                         origin='cvel')
#         ms.writehistory(message='antenna     = "' + str(antenna) + '"',
#                         origin='cvel')
#         ms.writehistory(message='timerange   = "' + str(timerange) + '"'
#                         , origin='cvel')
#         ms.writehistory(message='mode        = ' + str(mode),
#                         origin='cvel')
#         ms.writehistory(message='nchan       = ' + str(nchan),
#                         origin='cvel')
#         ms.writehistory(message='start       = ' + str(start),
#                         origin='cvel')
#         ms.writehistory(message='width       = ' + str(width),
#                         origin='cvel')
#         ms.writehistory(message='interpolation = '
#                         + str(interpolation), origin='cvel')
#         ms.writehistory(message='outframe    = "' + str(outframe) + '"'
#                         , origin='cvel')
#         ms.writehistory(message='phasecenter = "'
#                         + str(phasecentername) + '"', origin='cvel')
#         ms.writehistory(message='hanning   = "' + str(hanning) + '"',
#                         origin='cvel')
#         ms.close()
# 
#         return True
#     except Exception, instance:
# 
#         print '*** Error *** ', instance
#         # delete temp output (comment out for debugging)
#         if os.path.exists(outputvis + '.spwCombined'):
#             casalog.post('Deleting temporary output files ...', 'INFO')
#             shutil.rmtree(outputvis + '.spwCombined')
#         raise Exception, instance


