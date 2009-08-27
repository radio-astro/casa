# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 17-Dec-2007 jfl Add calibration method info.
# 20-Mar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release. writeHTMLDescription got working, 
#                 ..TestAmplitude added.
# 25-Jun-2008 jfl regression release.
# 14-Jul-2008 jfl last 4769 release.
# 15-Jul-2008 jfl solve2 modified for 5653 release.
# 10-Sep-2008 jfl msCalibrater release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
# 29-Jan-2009 jfl G_t default changed from 3600 to 60.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

import types

# package modules

from numpy import *

# alma modules

from baseData import *


class BandpassCalibration(BaseData):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, methodForEachSpw=None, defaultSourceType='*BANDPASS*',
     dataType='complex', polAverage=False, bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools             -- BaseTools object.
        bookKeeper        -- BookKeeper object.
        msCalibrater      -- MSCalibrater object.
        msFlagger         -- MSFlagger object.
        htmlLogger        -- The HTMLLogger object that is writing the HTML
                             log of this reduction run.
        msName            -- The name of the MeasurementSet being reduced.
        stageName         -- The name of the stage using the object.
        methodForEachSpw  -- Dictionary with method to use for each spw.
        defaultSourceType -- Source to use if not specified in methodForEachSpw.
        dataType          -- The type of data to be returned by getdata;
                             'amplitude' or 'phase'.
        polAverage        -- True if gains are averaged over polarization.
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels at edge of band.
        """

#        print 'BandpassCalibration.__init__ called %s' % methodForEachSpw
        BaseData.__init__(self, tools, bookKeeper, msCalibrater, msFlagger,
         htmlLogger, msName, stageName)
        self._bandpassFlaggingStage = bandpassFlaggingStage
        data_desc_ids = self._results['summary']['data_desc_range'] 

# set default method to use if none specified

        if methodForEachSpw==None:
            methodForEachSpw = {} 
            for data_desc_id in data_desc_ids:

# do nothing if spw is continuum or field/spw has no data

                if self._results['summary']['nchannels'][data_desc_id] == 1:
                    continue

                field = self.getFieldsOfType(defaultSourceType)[0] 
                if self._valid_field_spw.count([field,data_desc_id]) == 0:
                    print 'no data', field, data_desc_id
                    continue

                methodForEachSpw[data_desc_id] = {}
                methodForEachSpw[data_desc_id]['G_t'] = 60.0
                methodForEachSpw[data_desc_id]['field_id'] = field
                methodForEachSpw[data_desc_id]['method'] = {'mode':'CHANNEL'} 
                methodForEachSpw[data_desc_id]['bandpass_flagging_stage'] = None
        self._methodForEachSpw = methodForEachSpw
        self._defaultSourceType = defaultSourceType
        self._dataType = dataType
        self._polAverage = polAverage


    def _calculate_channel_B(self, btab, data_desc_id, field_id, command, 
     gtab=None):
        """
        """

#        print 'BandpassCalibration._calculate_channel_B called %s' % btab

        self._htmlLogger.timing_start(
         'BandpassCalibration._calculate_channel_B')

# set the calibrations to be applied beforehand

        if self._results['summary']['telescope_name'] == 'VLA':
            self._msCalibrater.setapply(type='GAINCURVE')
        self._msCalibrater.setapply(type='G', table=gtab)

# solve

        new_commands,error = self._msCalibrater.solve(field=field_id,
         spw=data_desc_id,
         type='B', t='inf', combine='scan',
         table=btab, append=False, apmode='AP', solnorm=True, minsnr=0.0)

        command += new_commands

        self._htmlLogger.timing_stop('BandpassCalibration._calculate_channel_B')


    def _calculate_polynomial_B(self, btab, data_desc_id, field_id, degamp,
     degphase, command, gtab=None):
        """
        """

#        print 'BandpassCalibration._calculate_polynomial_B called'

        self._htmlLogger.timing_start(
         'BandpassCalibration._calculate_polynomial_B')

# set the calibrations to be applied beforehand

        if self._results['summary']['telescope_name'] == 'VLA':
            self._msCalibrater.setapply(type='GAINCURVE')
        self._msCalibrater.setapply(type='G', table=gtab)

# solve

        new_commmands,error = self._msCalibrater.solvebandpoly(field=field_id,
         spw=data_desc_id, table=btab, append=False, 
         degamp=degamp, degphase=degphase, visnorm=False, solnorm=True,
         maskcenter=0, maskedge=0)

        command += new_commands

        self._htmlLogger.timing_stop(
         'BandpassCalibration._calculate_polynomial_B')


    def calculate(self):
        """
        """

#        print 'BandpassCalibration.calculate called - %s' % \
#         self._methodForEachSpw
        self._htmlLogger.timing_start('BandpassCalibration.calculate')

        data_desc_ids = self._results['summary']['data_desc_range'] 

# are the data already available

        inputs = self.inputs()

        entry,parameters = self._bookKeeper.available(
         objectType=inputs['objectType'], sourceType=inputs['sourceType'],
         furtherInput=inputs['furtherInput'],
         outputFiles=[])

        if entry == None:
            self._htmlLogger.timing_start('BandpassCalibration.calculateNew')

# remember the current flag state so that it can be re-applied after the
# calibration has been calculated; there is some setting/unsetting of
# bandpass edge flags during the calibration process, so this ensures that
# the ms state is not corrupted.

            self._msFlagger.rememberFlagState()
#            command.append('...save input flag state')

# get the current flagging info

            flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
            parameters = {'history':self._fullStageName,
                          'data':{},
                          'command':{},
                          'method':self._methodForEachSpw.copy(),
                          'dependencies':{}}

            for data_desc_id in self._methodForEachSpw.keys():
                parameters['data'][data_desc_id] = {}
                parameters['command'][data_desc_id] = []

# use an alias for brevity

                command = parameters['command'][data_desc_id]

# get parameters to use in calculating the bp calibration

                t = self._methodForEachSpw[data_desc_id]['G_t']
                field_id = self._methodForEachSpw[data_desc_id]['field_id']
                method = self._methodForEachSpw[data_desc_id]['method']
                mode = method['mode']
                bandpassFlaggingStage = self._methodForEachSpw[data_desc_id]\
                 ['bandpass_flagging_stage']

# find how many channels there are in this SpW. How many unflagged data rows
# are there.

                nchannels = 0
                self._ms.open(thems=self._msName)
                self._ms.selectinit(0, True)
                self._ms.selectinit(data_desc_id, False)
                msInfo = self._ms.getdata(['axis_info'], ifraxis=True)
                chan_freq = msInfo['axis_info']['freq_axis']['chan_freq']
                nchannels = len(chan_freq)

                query = """FIELD_ID==%s AND (ANTENNA1!=ANTENNA2) AND 
                 NOT(FLAG_ROW) AND NOT(ALL(FLAG))""" % (field_id)
                rtn = self._ms.selecttaql(msselect = query)
                nrows = self._ms.nrow(selected=True)
                self._ms.close()

# skip if continuum

                if nchannels == 1:
                    self._log('....spectral_window_id %s has continuum data' % 
                     data_desc_id)
                    parameters['data'][data_desc_id]['mode'] = 'continuum'
                    continue

# recall pure flag state - flagging channels in the previous loop may have
# corrupted it.

                self._msFlagger.recallFlagState()

# get filenames for calibrations

                flag_marks = self._msFlagger.getFlagMarkDict(field_id)
                gtab = 'bandpass.phaseup.f%s.spw%s.t%s.fm%s' % (field_id,
                 data_desc_id, t, flag_marks)
                btab = 'bandpass.%s.spw%s.t%s.fm%s' % (self._base_msName,
                 data_desc_id, t, flag_marks)

                parameters['data'][data_desc_id]['gtab'] = gtab
                parameters['data'][data_desc_id]['mode'] = 'failed'
                parameters['data'][data_desc_id]['table'] = btab
                parameters['data'][data_desc_id]['error'] = ''

# skip if no data

                if nrows == 0:
                    self._log('....spectral_window_id %s has no unflagged data'
                     % data_desc_id)
                    parameters['data'][data_desc_id]['error'] = 'no data'
                    continue

# get the channels to be flagged for polynomial bandpass fitting or for
# G calibration

                flag_channels = None
                for row,val in enumerate(flaggingReason):
                    if val['stageDescription']['name'] == bandpassFlaggingStage:
                        for flagList in flagging[row].values():
                            for flag in flagList:
                                if data_desc_id == flag['DATA_DESC_ID']:
                                    flag_channels = flag['CHANNELS']
                                    break
                        break

# calculate the G calibration - phase only to improve the correlation of the
# bandpass data

# apply any bandpass flagging to improve the S/N of the G calculation

                self._msFlagger.apply_bandpass_flags(data_desc_id, field_id, 
                 flag_channels)
                command.append('...apply channel flags %s' % flag_channels)

# calculate G

                if self._results['summary']['telescope_name'] == 'VLA':
                    self._msCalibrater.setapply(type='GAINCURVE')

                new_commands,error = self._msCalibrater.solve(field=field_id,
                 spw=data_desc_id, type='G', t=t, combine='scan',
                 apmode='P', table=gtab, append=False)

                command += new_commands
                parameters['data'][data_desc_id]['error'] = error
                if error != None:
                    continue

# check that derivation of G has not failed silently

                if not os.path.exists(gtab):
                    parameters['data'][data_desc_id]['error'] = \
                     'failed to phase-up BANDPASS calibration data'
                    command.append('...failed to calculate G')
                    continue

# calculate B

                if mode == 'CHANNEL':
                    parameters['data'][data_desc_id]['mode'] = 'B'
                    self._msFlagger.remove_bandpass_flags(data_desc_id,
                     field_id, flag_channels)
                    command.append('...remove channel flags %s' % flag_channels)
                    try:
                        self._calculate_channel_B(btab, data_desc_id, field_id,
                         command, gtab=gtab)
                    except KeyboardInterrupt:
                        raise
                    except:
                        error_report = self._htmlLogger.openNode('exception',
                         '%s.bandpass_calc_exception' % (self._stageName),
                         True, stringOutput = True)

                        self._htmlLogger.logHTML('Exception details<pre>')
                        traceback.print_exc()
                        traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                        self._htmlLogger.logHTML('</pre>')
                        self._htmlLogger.closeNode()

                        error_report += ' while calculating BANDPASS calibration'

                        parameters['data'][data_desc_id]['error'] += \
                         error_report

                elif mode == 'POLYNOMIAL':
                    parameters['data'][data_desc_id]['mode'] = 'BPOLY'

# apply the bandpass flagging to remove noisy or curved areas of the bandpass

                    self._msFlagger.apply_bandpass_flags(data_desc_id,
                     field_id, flag_channels)
                    command.append('...apply channel flags %s' % flag_channels)
                    try:
                        self._calculate_polynomial_B(btab, data_desc_id,
                         field_id, method['ampdeg'], method['phasedeg'],
                         command, gtab=gtab)
                    except KeyboardInterrupt:
                        raise
                    except:
                        error_report = self._htmlLogger.openNode('exception',
                         '%s.bandpass_calc_exception' % (self._stageName),
                         True, stringOutput = True)

                        self._htmlLogger.logHTML('Exception details<pre>')
                        traceback.print_exc()
                        traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                        self._htmlLogger.logHTML('</pre>')
                        self._htmlLogger.closeNode()

                        error_report += ' while calculating BANDPASS calibration'

                        parameters['data'][data_desc_id]['error'] += \
                         error_report

                else:
                    raise NameError, 'bad mode: %s' % mode

# restore flag state on entry

            self._msFlagger.recallFlagState()
#            command.append('...restore input flag state')

# store the object info in the BookKeeper

            self._bookKeeper.enter(
             objectType=inputs['objectType'],
             sourceType=inputs['sourceType'],
             furtherInput=inputs['furtherInput'],
             outputFiles=[],
             outputParameters=parameters)
            self._htmlLogger.timing_stop('BandpassCalibration.calculateNew')

        self._parameters = parameters
        self._htmlLogger.timing_stop('BandpassCalibration.calculate')

        return parameters


    def getData(self):
        """Public method to return the B calibration as a 'view' of the data.

        Keyword arguments:
        topLevel -- True if this is the data 'view' to be displayed directly.
        """

#        print 'BandpassCalibration.getData called'

        self._htmlLogger.timing_start('BandpassCalibration.getdata')

# calculate the G gains for 'BeforeHeuristics', 'StageEntry' and 'Current' -
# 'BeforeHeuristics' assumes no bandpassFlaggingStage

        self._msFlagger.setFlagState('BeforeHeuristics')
        originalBP = self.calculate()
        self._msFlagger.setFlagState('StageEntry')
        stageEntryBP = self.calculate()
        self._msFlagger.setFlagState('Current')
        currentBP = self.calculate()

# read info on flagging history from the MS

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()

# now read the results from the table and export them

        antenna_range = self._results['summary']['antenna_range']
        results = {}

        for data_desc_id in originalBP['data'].keys():
            nchannels = self._results['summary']['nchannels'][data_desc_id]

# check for continuum spw

            if nchannels == 1:
                continue

# check table exists

            if not os.path.exists(currentBP['data'][data_desc_id]['table']):
                continue

            field_id = currentBP['method'][data_desc_id]['field_id']

# open the CAL_DESC sub-table and read the column that maps CAL_DESC_ID
# to DATA_DESC_ID

            self._table.open('%s/CAL_DESC' % currentBP['data'][data_desc_id]
             ['table'])
            cal_desc_2_data_desc = self._table.getcol('SPECTRAL_WINDOW_ID')[0]
            self._table.close()

            cal_desc_id = compress(cal_desc_2_data_desc==data_desc_id,
             arange(len(cal_desc_2_data_desc)))[0]

# read the results themselves. Limiting the length of the gain and
# cal_flag arrays is to correct for the fact that the calibration
# table arrays all have the number of channels of the largest
# spw. This should change in the future.

            self._table.open(currentBP['data'][data_desc_id]['table'])
            taql = 'CAL_DESC_ID==%s && FIELD_ID==%s' % (cal_desc_id, field_id)
            subTable = self._table.query(query=taql)
            antenna1 = subTable.getcol('ANTENNA1')
            gain = subTable.getcol('GAIN')[:,:nchannels,:]
            cal_flag = subTable.getcol('FLAG')[:,:nchannels,:]
            field_id_column = subTable.getcol('FIELD_ID')
            self._table.close()

# and 'StageEentry' info

            self._table.open(stageEntryBP['data'][data_desc_id]['table'])
            subTable= self._table.query(query=taql)
            cal_flag_stage_entry = subTable.getcol('FLAG')[:,:nchannels,:]
            self._table.close()

# and 'BeforeHeuristics' info

            self._table.open(originalBP['data'][data_desc_id]['table'])
            subTable= self._table.query(query=taql)
            cal_flag_original = subTable.getcol('FLAG')[:,:nchannels,:]
            self._table.close()

            if shape(cal_flag) != shape(cal_flag_original):
                print 'calibration flag', shape(cal_flag)
                print 'calibration flag original', shape(cal_flag_original)
                continue

            if shape(cal_flag) != shape(cal_flag_stage_entry):
                print 'calibration flag', shape(cal_flag)
                print 'calibration flag on stage entry', shape(
                 cal_flag_stage_entry)
                continue

            npol = 2
            if alltrue(cal_flag[1,:,:]):
                npol = 1

# ..and get bandpass 'flagged' channels; only gets the first flagging occurrence
# for each spw
         
            edge_channels = []
            if self._bandpassFlaggingStage != None:
                for row,val in enumerate(flaggingReason):
                    if val['stageDescription']['name'] == \
                     self._bandpassFlaggingStage:
                        for flagList in flagging[row].values():
                            for flag in flagList:
                                if data_desc_id == flag['DATA_DESC_ID']:
                                    edge_channels = list(
                                     flag['CHANNELS'])
                                    break
                        if edge_channels != []:
                            break
                    if edge_channels != []:
                        break

            if self._polAverage:

# average the polarization results

                if self._dataType == 'amplitude':
                    data = zeros([max(antenna_range)+1, nchannels], float)
                else:
                    raise NameError, 'bad dataType: %s' % self._dataType
                flag = [zeros([max(antenna_range)+1, nchannels], int),
                 zeros([max(antenna_range)+1, nchannels], int),
                 zeros([max(antenna_range)+1, nchannels], int)]

                for i in range(shape(gain)[2]):
                    antenna = antenna1[i]
                    if antenna not in antenna_range:
                        print \
                         'antenna1 %s from file %s is outside antenna range' % (
                         antenna, currentBP['data'][data_desc_id]['table'])
                        continue
                    for p in range(npol):
                        data[antenna,:] += sqrt(
                         gain[p,:,i].imag**2 + gain[p,:,i].real**2)
                        flag[2][antenna,:] = logical_or(flag[2][antenna,:],
                         cal_flag[p,:,i])
                        flag[1][antenna,:] = logical_or(flag[1][antenna,:],
                         cal_flag_stage_entry[p,:,i])
                        flag[0][antenna,:] = logical_or(flag[0][antenna,:],
                         cal_flag_original[p,:,i])
                    data[antenna,:] /= float(npol)

                description = {}
                description['DATA_DESC_ID'] = int(data_desc_id)
                description['FIELD_ID'] = int(field_id)
                if self._dataType =='complex':
                    description['TITLE'] = \
                     'Field:%s SpW:%s Pol:Average - complex bandpass calibration' % (
                     self._fieldName[int(field_id)], self._pad(data_desc_id))
                else:
                    description['TITLE'] = \
                     'Field:%s SpW:%s Pol:Average - bandpass calibration %s' % (
                     self._fieldName[int(field_id)], self._pad(data_desc_id),
                     self._dataType)
                result = {}
                result['dataType'] = 'bandpass calibration %s' % self._dataType
                result['xtitle'] = 'CHANNEL'
                result['x'] = arange(nchannels)
                result['ytitle'] = 'ANTENNA'
                result['y'] = arange(max(antenna_range)+1)
                result['data'] = data
                result['mad_floor'] = zeros([max(antenna_range)+1,
                 nchannels], float)
                result['dataType'] = 'bp cal amplitude'
                result['dataUnits'] = ''
                result['flagVersions'] = ['BeforeHeuristics', 'StageEntry',
                 'Current']
                result['flag'] = flag
                result['chunks'] = []
                if self._bandpassFlaggingStage != None:
                    result['noisyChannels'] = edge_channels
                    result['bandpassFlaggingStage'] = self._bandpassFlaggingStage

                pickled_description = pickle.dumps(description)
                results[pickled_description] = result
            else:

# separate results for pols

                for p in range(npol):
                    if (self._dataType == 'amplitude') or \
                     (self._dataType == 'phase'):
                        data = zeros([max(antenna_range)+1, nchannels], float)
                    elif (self._dataType == 'complex'):
                        data = zeros([max(antenna_range)+1, nchannels], complex)
                    else:
                        raise NameError, 'bad dataType: %s' % self._dataType
                    flag = [ones([max(antenna_range)+1, nchannels], int),
                     ones([max(antenna_range)+1, nchannels], int),
                     ones([max(antenna_range)+1, nchannels], int)]

                    for i in range(shape(gain)[2]):
                        antenna = antenna1[i]
                        if antenna not in antenna_range:
                            print \
                             'antenna1 %s from file %s is outside antenna range' % (
                             antenna, currentBP['data'][data_desc_id]['table'])
                            continue

                        if self._dataType == 'amplitude':
                            data[antenna,:] = sqrt(
                             gain[p,:,i].imag**2 + gain[p,:,i].real**2)
                        elif self._dataType == 'phase':
                            data[antenna,:] = arctan2(
                             gain[p,:,i].imag, gain[p,:,i].real)
                        elif self._dataType == 'complex':
                            data[antenna,:] = gain[p,:,i]

                        flag[2][antenna,:] = cal_flag[p,:,i]
                        flag[1][antenna,:] = cal_flag_stage_entry[p,:,i]
                        flag[0][antenna,:] = cal_flag_original[p,:,i]

                    description = {}
                    description['DATA_DESC_ID'] = int(data_desc_id)
                    description['FIELD_ID'] = int(field_id)
                    description['POLARIZATION_ID'] = int(p)
                    if self._dataType =='complex':
                        description['TITLE'] = \
                         'Field:%s SpW:%s Pol:%s - complex bandpass calibration' % (
                         self._fieldName[int(field_id)], self._pad(data_desc_id), p)
                    else:
                        description['TITLE'] = \
                         'Field:%s SpW:%s Pol:%s - bandpass calibration %s' % (
                         self._fieldName[int(field_id)], self._pad(data_desc_id), p,
                         self._dataType)
                    result = {}
                    result['dataType'] = 'bandpass calibration %s' % self._dataType
                    result['xtitle'] = 'CHANNEL'
                    result['x'] = arange(nchannels)
                    result['ytitle'] = 'ANTENNA'
                    result['y'] = arange(max(antenna_range)+1)
                    result['data'] = data
                    result['mad_floor'] = zeros([max(antenna_range)+1,
                     nchannels], float)
                    result['dataType'] = 'bp cal %s' % self._dataType
                    result['dataUnits'] = ''
                    result['flagVersions'] = ['BeforeHeuristics', 'StageEntry', 
                     'Current']
                    result['flag'] = flag
                    result['chunks'] = []
                    if self._bandpassFlaggingStage != None:
                        result['noisyChannels'] = edge_channels
                        result['bandpassFlaggingStage'] = self._bandpassFlaggingStage

                    pickled_description = pickle.dumps(description)
                    results[pickled_description] = result

# now add the latest results to the returned structure
        
        self._results['flagging'].append(flagging)
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

# copy history and dependency info

        self._results['parameters'] = currentBP
        self._results['parameters']['dependencies'] = {'originalBP':
         originalBP}
        self._results['parameters']['dependencies'] = {'StageEntryBP':
         stageEntryBP}

        for k in results.keys():
            if self._results['data'].has_key(k):
                self._results['data'][k].append(results[k])
            else:
                self._results['data'][k] = [results[k]]

        temp = python_copy.deepcopy(self._results)

        self._htmlLogger.timing_stop('BandpassCalibration.getdata')
        return temp


    def inputs(self):
        """Method to return the interface parameters of this object.
        """
        data_desc_ids = self._results['summary']['data_desc_range'] 

        result = {}
        result['objectType'] = 'BandpassCalibration'
        result['sourceType'] = self._defaultSourceType
        result['furtherInput'] = {'methodForEachSpw':self._methodForEachSpw,
         'bandpassFlaggingStage':self._bandpassFlaggingStage}
        result['outputFiles'] = []
        result['dependencies'] = []

        flag_marks = {}
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        for data_desc_id in data_desc_ids:
            if self._methodForEachSpw.has_key(data_desc_id):
                field_id = self._methodForEachSpw[data_desc_id]['field_id']
                flag_marks[field_id] = flag_mark_col[field_id]

        flag_marks = str(flag_marks)

# replace unusual symbols to avoid problems with TaQL

        flag_marks = flag_marks.replace(' ', '')
        flag_marks = flag_marks.replace('{', '')
        flag_marks = flag_marks.replace('}', '')
        flag_marks = flag_marks.replace(':', '-')
        flag_marks = flag_marks.replace(',', '.')

        result['flag_marks'] = flag_marks

        return result


    def setapply(self, spw, field):
        """Method to arrange for the current bandpass calibration to be
        applied to the specified field and spw when a correct or solve is done.
        """
        nchannels = self._results['summary']['nchannels']

#        print 'BandpassCalibration.setApply', spw, field
        if self._results['summary']['telescope_name'] == 'VLA':
            self._msCalibrater.setapply('GAINCURVE')
 
        if nchannels[spw] != 1:
            self._msCalibrater.setapply(type=self._parameters
             ['data'][spw]['mode'], table=self._parameters
             ['data'][spw]['table'])


    def createGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = """<p>The normalised bandpass calibration was calculated."""

        bandpassDescription = {'bandpass calibration':description}
        return bandpassDescription


    def createDetailedHTMLDescription(self, stageName, parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        if parameters == None:
             parameters = self._parameters

# handle case where all spw were continuum

        allContinuum = True
        for v in parameters['data'].values():
           if v['mode'] != 'continuum':
               allContinuum = False

        if allContinuum:
            description = """
             <p>No bandpass calibration was performed because all spectral
              windows contain continuum data."""

        elif parameters.has_key('separate node'):
            description = parameters['separate node']

        else:
            description = """
             <p>The normalised bandpass calibration was calculated using 
              the parameters listed in the following table:

             <table CELLPADDING="5" BORDER="1"
             <tr>
              <th>Spw</th>
              <th>Field</th>
              <th>G_t</th>
              <th>Mode</th>
              <th>Channel Flags</th>
              <th>Apply Table</th>
              <th>Apply Type</th>
              <th>Casapy Calls</th>
              <th>Error?</th>
             </tr>"""

            keys = parameters['data'].keys()
            keys.sort()

            for i in keys:
                entry = parameters['data'][i]
                if entry['mode'] == 'continuum':
                    description += """
                     <tr>
                     <td>%s continuum</td>
                     </tr>""" % i
                    continue

# solve parameters

                description += """
                 <tr>
                  <td>%s</td>
                  <td>%s</td>
                  <td>%s</td>
                  <td>%s</td>""" % (
                  i,
                  parameters['method'][i]['field_id'],
                  parameters['method'][i]['G_t'], 
                  parameters['method'][i]['method']['mode'])

                description += '<td>%s</td>' % (
                 parameters['method'][i]['bandpass_flagging_stage'])

# apply parameters 

                if entry.has_key('table'):
                    description += """
                     <td>%s</td>
                     <td>%s</td>""" % (entry['table'], entry['mode'])

# casapy calls, in a separate node to save space.
             
                description += '<td>'
                description += self._htmlLogger.openNode('casapy calls',
                 '%s.spw%s.%s' % (stageName, i, 'bpCal_casapy_calls'), True,
                 stringOutput=True)
                for line in parameters['command'][i]:
                    self._htmlLogger.logHTML('<br>%s' % line)
                self._htmlLogger.closeNode()
                description += "</td>"

# use &nbsp; to make empty cells look good

                if entry.has_key('error'):
                    if entry['error'] == '':
                        description += """<td>&nbsp;</td>"""
                    else:
                        description += """<td>%s</td>""" % entry['error']
                else:
                    description += """<td>&nbsp;</td>"""
                description += "</tr>"
            description += """
             </table>
 
             where:
             <ul>
              <li>G_t is the value of t used to calculate
               a phase-only G calibration of the bandpass data, which is 
               applied to phase up that data before the bandpass calibration
               itself is calculated.
              <li>Mode is the type of bandpass solution obtained; 'CHANNEL', 
               or 'POLYNOMIAL'.
              <li>'Channel Flags' gives the name of the stage that specifies the
               channels to be flagged, if necessary, during the calculation
               of the bandpass calibration. These channels are flagged
               before the phase-only G calibration is performed, and before
               a POLYNOMIAL bandpass solution is calculated. They are
               unflagged before a CHANNEL bandpass solution is calculated.
              <li>'Apply Table' is the name of the file containing the bandpass
               solution.
              <li>'Apply Type' gives the casapy type of the bandpass solution.
             </ul>"""

#             <p>The bandpass calibration was calculated by the Python class 
#             BandpassCalibration."""

        bandpassDescription = {'bandpass calibration':description}
        return bandpassDescription


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = self.createGeneralHTMLDescription(stageName)

        self._htmlLogger.logHTML("""
         <p>The data view is a 2-d array with axes 'Channel' and
         'ANTENNA'. Each pixel is the bandpass calibration
         %s for that channel/baseline.""" % self._dataType)

        self._htmlLogger.logHTML(description['bandpass calibration'])


    def writeDetailedHTMLDescription(self, stageName, topLevel,
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        """

        description = self.createDetailedHTMLDescription(stageName)

        if topLevel:
            self._htmlLogger.logHTML("""<h3>Data View</h3>""")

        self._htmlLogger.logHTML("""
         <p>The data view is a 2-d array with axes 'Channel' and
         'ANTENNA'. Each pixel is the bandpass calibration
         %s for that channel/baseline.""" % self._dataType)

        self._htmlLogger.logHTML(description['bandpass calibration'])


class BandpassTestCalibratedAmplitude(BandpassCalibration):
    """
    """

    def _addResults(self, field, spw, antenna_range, nchannels, method,
     results, error=False):
        """
        """

# Use TaQL query to get subset of data for these fields.

        self._ms.open(thems=self._msName)
        self._ms.selectinit(spw, False)
        query = 'FIELD_ID IN %s' % field
        rtn = self._ms.selecttaql(query)

# get the current corrected data

        flagVersions = ['BeforeHeuristics', 'Current']
        corr_axis, chan_freq, times, chunks, antenna1,\
         antenna2, ifr_real, ifr_imag, ifr_flag, ifr_flag_row = \
         self._getBaselineData(field, spw, antenna_range, 'corrected',
         flagVersions)

# declare arrays

        ncorr = len(corr_axis)
        data_n = zeros([ncorr, max(antenna_range)+1, max(antenna_range)+1,
         nchannels], int)
        data_n_on_entry = zeros([ncorr, max(antenna_range)+1,
         max(antenna_range)+1, nchannels], int)
        no_data_flag = ones([ncorr, max(antenna_range)+1, max(antenna_range)+1,
         nchannels], int)
        amp_mean = zeros([ncorr, max(antenna_range)+1, max(antenna_range)+1,
         nchannels], float)
        amp_stddev = zeros([ncorr, max(antenna_range)+1,
         max(antenna_range)+1, nchannels], float)
        phase_mean = zeros([ncorr, max(antenna_range)+1,
         max(antenna_range)+1, nchannels], float)
        phase_stddev = zeros([ncorr, max(antenna_range)+1,
         max(antenna_range)+1, nchannels], float)

# find flags on entry

        for antenna in antenna_range:
            ifr_range = arange(len(antenna1))
            ifr_range = compress(antenna1==antenna, ifr_range)

            for corr in range(ncorr):
                for ifr in ifr_range:
                    other_ant = antenna2[ifr]

                    no_data_flag[corr, antenna, other_ant,:] = \
                     no_data_flag[corr, other_ant, antenna,:] = False

                    for channel in range(nchannels):
                        real_time_series = compress(logical_not(
                         logical_or(ifr_flag[0][corr,channel,ifr,:],
                         ifr_flag_row[0][ifr,:])), ifr_real[corr,channel,ifr,:])
                        data_n_on_entry[corr,antenna,other_ant,channel] = \
                         data_n_on_entry[corr,other_ant,antenna,channel] = \
                         len(real_time_series)

# now the real data - if an error has occurred then leave all data bad,
# otherwise loop through antennas and for each build up 'baseline v channels'
# image

        if not error:
            for antenna in antenna_range:
#               self._writeOnLine('building image for antenna %s' % antenna)
                ifr_range = arange(len(antenna1))
                ifr_range = compress(antenna1==antenna, ifr_range)

                for corr in range(ncorr):
                    for ifr in ifr_range:
                        other_ant = antenna2[ifr]

# calculate the mean, median and standard deviation of the good data
# points for each channel.

                        for channel in range(nchannels):
                            real_time_series = compress(logical_not(
                             logical_or(ifr_flag[-1][corr,channel,ifr,:],
                             ifr_flag_row[-1][ifr,:])),
                             ifr_real[corr,channel,ifr,:])
                            imag_time_series = compress(logical_not(
                             logical_or(ifr_flag[-1][corr,channel,ifr,:],
                             ifr_flag_row[-1][ifr,:])),
                             ifr_imag[corr,channel,ifr,:])
                            amp_time_series = sqrt(
                             pow(real_time_series,2) +
                             pow(imag_time_series,2))

                            n = data_n[corr,antenna,other_ant,channel] = \
                             data_n[corr,other_ant,antenna,channel] = \
                             len(real_time_series)

                            if n > 0:
                                real_mean = mean(real_time_series)
                                imag_mean = mean(imag_time_series)
                                amp_mean[corr,antenna,other_ant,channel] = \
                                 amp_mean[corr,other_ant,antenna,channel] = \
                                 sqrt(pow(real_mean,2) + pow(imag_mean,2))
                                phase_mean[corr,antenna,other_ant,channel] = \
                                 phase_mean[corr,other_ant,antenna,channel] = \
                                 arctan2(imag_mean, real_mean)

                            if n > 1:
                                amp_stddev[corr,antenna,other_ant,channel] = \
                                 amp_stddev[corr,other_ant,antenna,channel] = \
                                 sqrt(sum(pow(amp_time_series -
                                 amp_mean[corr,antenna,other_ant,channel],
                                 2)) / (n * (n - 1)))

# phase stddev more compicated than amp. To eliminate phase wrapping
# calculate the phases relative to the complex mean.

                                rotated_real = real_time_series * cos(
                                 phase_mean[corr,antenna,other_ant,channel]) + \
                                 imag_time_series * \
                                 sin(phase_mean[corr,antenna,other_ant,channel])
                                rotated_imag = -real_time_series * sin(
                                 phase_mean[corr,antenna,other_ant,channel]) + \
                                 imag_time_series * \
                                 cos(phase_mean[corr,antenna,other_ant,channel])
                                phase_stddev[corr,antenna,other_ant,channel] = \
                                 phase_stddev[corr,other_ant,antenna,channel] = \
                                 sqrt(sum(pow(
                                 arctan2(rotated_imag, rotated_real),2) /
                                 (n * (n - 1))))

        for antenna in antenna_range:
            for corr in range(ncorr):
                description = {}
                description['DATA_DESC_ID'] = int(spw)
                description['FIELD_ID'] = int(field)
                description['CORR'] = corr_axis[corr]
                description['ANTENNA1'] = int(antenna)
                description['method'] = method
                description['TITLE'] = 'Field:%s Spw:%s Ant1:%s Pol:%s calibrated bandpass amplitude' % (
                 field, self._pad(spw), self._pad(antenna), corr_axis[corr])
                result = {}
                result['dataType'] = 'calibrated bandpass amplitude'
                result['dataUnits'] = ''
                result['xtitle'] = 'Channel'
                result['x'] = arange(nchannels)
                result['ytitle'] = 'ANTENNA2'
                result['y'] = arange(max(antenna_range)+1)
                result['data'] = amp_mean[corr,antenna]
                result['mad_floor'] = amp_stddev[corr,antenna]
                result['flag'] = [no_data_flag[corr,antenna],
                 array(data_n_on_entry[corr,antenna]==0, int),
                 array(data_n[corr,antenna]==0, int)]
                result['flagVersions'] = ['NoData'] + flagVersions
                result['chunks'] = []

                pickled_description = pickle.dumps(description)
                results[pickled_description] = result

        self._ms.close()


    def description (self):
        description = ['BandpassTestCalibratedAmplitude']
        return description



    def getData(self, topLevel=False):
        """
        """
        self._htmlLogger.timing_start('BandpassTestCalibratedAmplitude.getdata')

# ensure ms flag state is 'Current' and save it for restoration afterward
# - calibration of the data can flag it implicitly and we want to avoid
# carrying forward 'hidden' flagging like that.

        self._msFlagger.setFlagState('Current')
        self._msFlagger.saveFlagState('getDataEntry')

# get a list of valid field/spw combinations - i.e. those that have
# some data

        valid_field_spw = self._msFlagger.getValidFieldSpw()

# calculate the B gains

        bpCal = BandpassCalibration.calculate(self)

# now calculate the results from the tables and export them

        results = {'parameters':{'history':'',
                                 'dependencies':{}},
                   'data':{}}

        results['parameters']['history'] = self._fullStageName
        results['parameters']['dependencies']['bpCal'] = bpCal

        antenna_range = self._results['summary']['antenna_range']

# apply the B calibration

        for data_desc_id in bpCal['data'].keys():
            nchannels = self._results['summary']['nchannels'][data_desc_id]
            if nchannels == 1:
                continue

# apply B to the test field

            method = bpCal['method'][data_desc_id]
            test_field_id = method['test_field_id']

# but don't bother if the field/spw is a combination with no data

            if valid_field_spw.count([test_field_id, data_desc_id]) == 0:
                continue

            error = False
            command = []
            try:
                self.setapply(spw=data_desc_id, field=test_field_id)
                command += self._msCalibrater.correct(spw=data_desc_id,
                 field=test_field_id)
            except KeyboardInterrupt:
                raise
            except:
                print 'no bp calibration for spw %s' % data_desc_id
                error = True

# calculate the amplitudes and add them to the result structure

            self._addResults(test_field_id, data_desc_id, antenna_range,
             nchannels, method, results['data'], error=error)

# now add the latest results to the returned structure

        flagging,flaggingReason,flaggingApplied = self._msFlagger.getFlags()
        self._results['flagging'].append(flagging)
        self._results['flaggingReason'].append(flaggingReason)
        self._results['flaggingApplied'].append(flaggingApplied)

        for k in results['data'].keys():
            if self._results['data'].has_key(k):
                self._results['data'][k].append(results['data'][k])
            else:
                self._results['data'][k] = [results['data'][k]]

# add history and parameters info

        self._results['parameters'] = results['parameters']

        temp = python_copy.deepcopy(self._results)

# restore the flag state on entry and adopt is as 'Current'

        self._msFlagger.setFlagState('getDataEntry')
        self._msFlagger.adoptAsCurrentFlagState()

        self._htmlLogger.timing_stop('BandpassTestCalibratedAmplitude.getdata')
        return temp


    def createGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        description = {'bandpass calibration':'''
         The data are the amplitudes in each channel of the 'test' field 
         calibrater data after they have been bandpass calibrated.'''}

        return description


    def createDetailedHTMLDescription(self, stageName, parameters=None):
        """Write a detailed description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        parameters -- Dictionary that holds the descriptive information.
        """

        if parameters == None:
             parameters = self._parameters

        description = '''
         The data are the amplitudes in each channel of the 'test' field 
         calibrater data after they have been bandpass calibrated.'''

        description += BandpassCalibration.createDetailedHTMLDescription(
         self, stageName, parameters=parameters['dependencies']['bpCal'])[
         'bandpass calibration']

        description = {'bandpass calibration':description}

        return description


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """
        
        description = self.createHTMLDescription(stageName)
        self._htmlLogger.logHTML(description)


    def writeDetailedHTMLDescription(self, stageName, topLevel, 
     parameters=None):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        topLevel  -- True if this data 'view' is to be displayed directly,
                     not passing through a data modifier object.
        parameters -- Dictionary that holds the descriptive information.
        """
        
        if parameters == None:
             parameters = self._results['parameters']

        description = self.createDetailedHTMLDescription(stageName,
         parameters=parameters)

        if topLevel:
            self._htmlLogger.logHTML('<h3>Data View</h3>')
        self._htmlLogger.logHTML(description)


class BandpassCalibrationAmplitude(BandpassCalibration):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, methodForEachSpw=None, defaultSourceType='*BANDPASS*',
     bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools             -- BaseTools object.
        bookKeeper        -- BookKeeper object.
        msCalibrater      -- MSCalibrater object.
        msFlagger         -- MSFlagger object.
        htmlLogger        -- The HTMLLogger object that is writing the HTML
                             log of this reduction run.
        msName            -- The name of the MeasurementSet being reduced.
        stageName         -- The name of the stage using this object.
        methodForEachSpw  -- Dictionary with method to use for each spw.
        defaultSourceType -- Source to use if not specified in methodForEachSpw.
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels at edge of band.
        """

#        print 'BandpassCalibrationAmplitude.__init__ called'
        BandpassCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, methodForEachSpw,
         defaultSourceType, 'amplitude',
         bandpassFlaggingStage=bandpassFlaggingStage)


class BandpassCalibrationPhase(BandpassCalibration):

    def __init__(self, tools, bookKeeper, msCalibrater, msFlagger, htmlLogger,
     msName, stageName, methodForEachSpw=None, defaultSourceType='*BANDPASS*',
     bandpassFlaggingStage=None):
        """Constructor.

        Keyword arguments:
        tools             -- BaseTools object.
        bookKeeper        -- BookKeeper object.
        msCalibrater      -- MSCalibrater object.
        msFlagger         -- MSFlagger object.
        htmlLogger        -- The HTMLLogger object that is writing the HTML
                             log of this reduction run.
        msName            -- The name of the MeasurementSet being reduced.
        stageName         -- The name of the stage using this object.
        methodForEachSpw  -- Dictionary with method to use for each spw.
        defaultSourceType -- Source to use if not specified in methodForEachSpw.
        bandpassFlaggingStage -- Name of stage whose channel flags specify
                              -- channels at edge of band.
        """

#        print 'BandpassCalibrationPhase.__init__ called'
        BandpassCalibration.__init__(self, tools, bookKeeper, msCalibrater,
         msFlagger, htmlLogger, msName, stageName, methodForEachSpw,
         defaultSourceType, 'phase',
         bandpassFlaggingStage=bandpassFlaggingStage)
