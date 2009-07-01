"""Module to keep track of flags applied to MS."""

# History:
# 21-Jan-2009 jfl ut4b release. Original version.
#  7-Apr-2009 jfl mosaic release.

# package modules

from numpy import *
import pickle
import types

# alma modules


class MSFlagger:
    """
    Class that keeps track of the flagging state of the wrapped
    Measurement Set. 
    """

    def __init__(self, tools, htmlLogger, msName, msCalibrater, verbose=False):
        """Constructor.

        Keyword arguments:
        tools      -- BaseTools object, giving access to the casa tools.
        htmlLogger -- The HTMLLogger object that is writing the HTML
                      log.
        msName     -- The name of the Measurement Set whose calibration
                      state is being kept.
        msCalibrater -- The MSCalibrater encapsulating the calibration state
                        of the MS.
        verbose    -- True if verbose reports from the object are to
                      be written to the terminal. 
        """
#        print 'MSFlagger constructor called'

        htmlLogger.timingStageStart('MSFlagger')
        tools.copy(self)
        self._html = htmlLogger
        self._msName = msName
        self._msCalibrater = msCalibrater
        self._verbose = verbose

# initialise internal variables.

        self._memoryFlagVersion = None

# get some basic info on ms

        self._ms.open(self._msName)
        msInfo = self._ms.range(['data_desc_id', 'field_id'])
        self._data_desc_ids = msInfo['data_desc_id']
        self._all_field_ids = msInfo['field_id']

        self._nchannels = zeros(shape(self._data_desc_ids), int)
        for data_desc_id in self._data_desc_ids:
            self._ms.selectinit(data_desc_id)
            msInfo = self._ms.getdata(['axis_info'])
            self._nchannels[data_desc_id] = len(msInfo['axis_info']['freq_axis']
             ['chan_freq'])
        self._ms.close()

# open the ms in the flagger 

        self._flagger.open(self._msName)

# get list of field/spw combinations that actually contain data.

        self._validFieldSpw = []
        self._table.open(self._msName, nomodify=False)
        for field in self._all_field_ids:
            for spw in self._data_desc_ids:
                subTable = self._table.query(
                 'DATA_DESC_ID==%s && FIELD_ID==%s' % (spw,field))
                nrow = subTable.nrows()
                if nrow > 0:
                    self._validFieldSpw.append([field, spw])
        self._table.close()

# create FLAG column and initialise it, FLAG_MARK keyword

        self._table.open('%s/FIELD' % self._msName, nomodify=False)
        keywords = self._table.getkeywords()
        if not keywords.has_key('FLAGGING_STATE'):
            if self._verbose:
                print 'no flaggingState info in ms'
            self._flaggingState = {}

# set up a BeforeHeuristics state as record before anything has been done

            self._flagger.restoreflagversion('Original')
            self._current_flag_mark = -1
            self._current_flag_mark_col = -1 * ones(shape(self._all_field_ids),
             int)
            self._flagVersion = 'Current'
            self.saveFlagState('BeforeHeuristics')

# now set flag mark to 0 and adopt state as 'Current'. This is the real starting
# point for the flagging.

            self._current_flag_mark = 0
            self._current_flag_mark_col = zeros(shape(self._all_field_ids), int)
            self.saveFlagState('Current')
            self._table.putkeyword('FLAGGING_STATE', pickle.dumps(
             self._flaggingState))
        else:
            if self._verbose:
                print 'reading flaggingState info from ms'
            self._flaggingState = pickle.loads(self._table.getkeyword(
             'FLAGGING_STATE'))
            self._current_flag_mark = self._flaggingState['Current']['FLAG_MARK']
            self._current_flag_mark_col = self._flaggingState['Current']\
             ['FLAG_MARK_COL']
            self._flagVersion = 'Current'

        if self._verbose:
            print 'initial state:'
            print '   flagVersion          :', self._flagVersion
            print '   current_flag_mark    :', self._current_flag_mark
            print '   current_flag_mark_col:', self._current_flag_mark_col
            print 'available flagVersions:'
            for k,v in self._flaggingState.iteritems():
                print '                  ', k, v['FLAG_MARK'],\
                 v['FLAG_MARK_COL']
        

    def _flagTable(self, flags, apply):
        """
        Method to set flags in the MS.
        """
#        print 'MSFlagger._flagTable called'
    
# list of fields whose data are flagged by this call
           
        aggregate_fields_flagged = []
        selected_rows = 0

# loop through the flags and do the flagging requested

        self._table.open(self._msName, nomodify=False)

        for rule, flag_list in flags.iteritems():
            for flag_description in flag_list:
                if flag_description['rule'] == 'remove flags':
                    stageName = flag_description['stageName']
                    self._removeFlags(stageName)
                    continue

# flag each (spw,field) separately to keep a record of the flag stats

                if flag_description.has_key('DATA_DESC_ID'):
                    spw_list = [flag_description['DATA_DESC_ID']]
                    if type(spw_list) != types.ListType:
                        spw_list = [spw_list]
                else:
                    spw_list = self._data_desc_ids
                                
                if flag_description.has_key('FIELD_ID'):
                    field_list = flag_description['FIELD_ID']
                    if type(field_list) != types.ListType:
                        field_list = [field_list]
                else:
                    field_list = self._all_field_ids

                ant = ant1 = ant2 = None
                if flag_description.has_key('ANTENNA1'):
                    ant1 = list(flag_description['ANTENNA1'])
                    ant2  = list(flag_description['ANTENNA2'])
                elif flag_description.has_key('ANTENNA'):
                    ant  = list(flag_description['ANTENNA'])

                for field in field_list:
                    for spw in spw_list:
                        if self._validFieldSpw.count([field,spw]) == 0:
                            continue

# update range of fields flagged

                        if aggregate_fields_flagged.count(field) == 0:
                            aggregate_fields_flagged.append(field)

# assemble TaQL command

                        taql = '''NOT(FLAG_ROW) && DATA_DESC_ID==%s &&
                         FIELD_ID IN %s''' % (spw, field)

                        if ant1 != None:
                            taql += ''' && ((ANTENNA1 IN %s && ANTENNA2 IN %s) ||
                             (ANTENNA1 IN %s && ANTENNA2 IN %s))''' % (
                             ant1, ant2, ant2, ant1)

                        if ant != None:
                            taql += ' && (ANTENNA1 IN %s || ANTENNA2 IN %s)' % (
                             ant, ant)

# 0.1 sec added to time interval to handle rounding errors. Could be
# done properly using the INTERVAL column.

                        if flag_description.has_key('TIME'):
                            if flag_description.has_key('TIME_INTERVAL'):
                                interval = flag_description['TIME_INTERVAL']
                                taql += ' && TIME IN ['
                                for i,timestamp in enumerate(
                                 flag_description['TIME']):
                                     taql += '{%s,%s},' % (
                                      timestamp-0.1-interval[i]/2.0,
                                      timestamp+0.1+interval[i]/2.0)
                                taql = taql[0:-1] + ']'
                            else:
                                taql += ' && TIME IN ['
                                for timestamp in flag_description['TIME']:
                                    taql += '{%s,%s},' % (timestamp-0.01,
                                     timestamp+0.01)
                                taql = taql[0:-1] + ']'

# get the number of rows in table and the number flagged by this rule, ignore
# the 'apply' parameter here as in the case of bandpass edge flagging (where
# apply is set False) we still want the view recalculated.

                        selected = self._table.query(taql)
                        selected_rows += selected.nrows()

# flag

                        if flag_description.has_key('CHANNELS'):
                            channels = flag_description['CHANNELS']
                            flag = selected.getcol('FLAG')
                            flag_vals = ones([shape(flag)[2]], int)
                            for p in range(shape(flag)[0]):
                                for channel in channels:
                                    flag[p,channel,:] = flag_vals

                            if not flag_description.has_key('ROWS_SET'):
                                flag_description['ROWS_SET'] = {}

                            if apply:
                                selected.putcol('FLAG', flag)
                                flag_description['ROWS_SET'][(spw,field)] = \
                                 selected.nrows()
                            else:
                                flag_description['ROWS_SET'][(spw,field)] = 0
                        else:
                            flag_row = selected.getcol('FLAG_ROW')
                            flag_row[:] = 1

                            if not flag_description.has_key('ROWS_SET'):
                                flag_description['ROWS_SET'] = {}

                            if apply:
                                selected.putcol('FLAG_ROW', flag_row)
                                flag_description['ROWS_SET'][(spw,field)] = \
                                 selected.nrows()
                            else:
                                flag_description['ROWS_SET'][(spw,field)] = 0

        self._table.close()

        if selected_rows > 0:

# If any flags have actually been set then increment FLAG_MARK keyword and
# set this value for fields that have been flagged

            self._current_flag_mark += 1
            for field_id in aggregate_fields_flagged:
                self._current_flag_mark_col[field_id] = self._current_flag_mark


    def adoptAsCurrentFlagState(self):
        """
        Adopt the flag state we are in as the 'Current' one. Use with care
        for resetting flagging stages.
        """  
        if self._verbose:
            print 'adoptAsCurrentFlagState name=', self._flagVersion

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

        temp = self._flaggingState[self._flagVersion].copy()
        self._flaggingState['Current'] = temp
        self._flagVersion = 'Current'
        self._flagger.saveflagversion('Current')

        if self._verbose:
            print 'saveflagversion Current'
            print 'flag mark', self._flaggingState[self._flagVersion]


    def apply_bandpass_flags(self, data_desc_id, field_id, flag_channels):
        """Flag the specified channels. Use to temporarily apply flags.
           Calls to this method and remove_bandpass_flags 
           corrupt the class for other use and it should be protected by 
           bracketing such calls with rememberFlagState and recallFlagState.

        Keyword arguments:
        data_desc_id  -- The SpW to be flagged.
        field_id      -- The field(s) to be flagged.
        flag_channels -- The channels to be flagged.
        """
        if flag_channels != None:
            if self._verbose:
                print 'apply bandpass flags', data_desc_id, field_id,\
                 flag_channels
            self._table.open(self._msName, nomodify=False)
            taql = 'FIELD_ID IN %s && DATA_DESC_ID==%s' % (field_id,
             data_desc_id)
            subTable = self._table.query(taql)
            new_flag = subTable.getcol('FLAG')
            new_flag[:,flag_channels,:] = 1
            subTable.putcol('FLAG', new_flag)
            self._table.close()

# set flag version to msfCorrupt as this method changes things behind the back
# of the rest of the class.

            self._flagVersion = 'msfCorrupt'


    def deleteFlagState(self, flagState):
        """
        Remove a flag state.
        """
        if self._verbose:
            print 'deleteFlagState flagState=', flagState

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

        if self._flaggingState.has_key(flagState):
            ignore = self._flaggingState.pop(flagState)
        else:
            pass
#            print 'no work needed'


    def getFlagMarkDict(self, field_id):
        """Get the flag 'mark' for the specified field.
        """

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

        result = {}
        result[field_id] = self._current_flag_mark_col[field_id]
        result = str(result)

# replace unusual symbols to avoid problems with TaQL

        result = result.replace(' ', '')
        result = result.replace('{', '')
        result = result.replace('}', '')
        result = result.replace(':', '-')
        result = result.replace(',', '.')
        return result


    def getFlagMarkInfo(self):
        """
        Return the flag mark info for the flag state.
        """
        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

        return self._current_flag_mark, self._current_flag_mark_col


    def getFlags(self):
        """
        Read the records of heuristics flagging from the MS.
        """
#        print 'MSFlagger.getFlags called'

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

        flag_column = []
        reason_column = []
        applied_column = []

# get the COMMAND,REASON,APPLIED columns of the FLAG_CMD sub-table.

        self._table.open('%s/FLAG_CMD' % self._msName)
        command = self._table.getcol('COMMAND')
        reason = self._table.getcol('REASON')
        applied = self._table.getcol('APPLIED')
        for row,val in enumerate(command):
            flag_column.append(pickle.loads(val))
            reason_column.append(pickle.loads(reason[row]))
        applied_column = list(applied)
        self._table.close()
        return flag_column, reason_column, applied_column


    def recallFlagState(self):
        """
        Put back the remembered flag state. Pairs with rememberFlagState.
        """  
        if self._verbose:
            print 'recallFlagState'
        if self._memoryFlagVersion == None:
            raise NameError, 'no flag state in memory'

        if self._memoryFlagVersion == self._flagVersion:
            pass
#            print 'no work required'
        else:
            self._flagVersion = self._memoryFlagVersion
            self._flagger.restoreflagversion(self._flagVersion)
        
        if self._verbose:
            print 'flag version recalled', self._flagVersion
            print 'flag mark recalled', self._flaggingState[self._flagVersion]


    def rememberFlagState(self):
        """
        Remember the flag state we're in. Pairs with recallFlagState.
        """  
        if self._verbose:
            print 'rememberFlagState'

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

# if we are in 'Current' flag state, make sure that the flaggingState dictionary
# is up to date before 'remembering' it.

        if self._flagVersion == 'Current':
            self._flaggingState['Current'] = {
             'FLAG_MARK':self._current_flag_mark,
             'FLAG_MARK_COL':list(self._current_flag_mark_col)}
            self._flagger.saveflagversion('Current')

        self._memoryFlagVersion = self._flagVersion
        self._memoryFlagDetails = self._flaggingState[self._flagVersion]

        if self._verbose:
            print 'flag version remembered', self._flagVersion
            print 'flag mark remembered', self._flaggingState[self._flagVersion]


    def remove_bandpass_flags(self, data_desc_id, field_id, flag_channels):
        """Unflag the specified channels. Use for removing flags that have
           been temporarily applied. Calls to apply_bandpass_flags and this method
           corrupt the class for other use and it should be protected by 
           bracketing such calls with rememberFlagState and recallFlagState.
        
        Keyword arguments:
        data_desc_id  -- The SpW to be flagged.
        field_id      -- The field(s) to be flagged.
        flag_channels -- The channels to be flagged.
        """
        if flag_channels != None:
            if self._verbose:
                print 'removing bandpass flags', data_desc_id, field_id,\
                 flag_channels
            self._table.open(self._msName, nomodify=False)
            taql = 'FIELD_ID IN %s && DATA_DESC_ID==%s' % (field_id,
             data_desc_id)
            subTable = self._table.query(taql)
            new_flag = subTable.getcol('FLAG')
            new_flag[:,flag_channels,:] = 0
            subTable.putcol('FLAG', new_flag)
            self._table.close()

# set flag version to msfCorrupt as this method changes things behind the back
# of the rest of the class.

            self._flagVersion = 'msfCorrupt'


    def resetFlagsToStageEntry(self, stageName, firstStageName):
        """
        Set the flagging state to what it was at entry to this stage after
        a previous run.

        Keyword arguments:
        stageName      -- The name of the stage.
        firstStageName -- The name of the first stage in the recipe.
        """
        if self._verbose:
            print 'msFlagger.resetFlagsToStageEntry called'

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

# get the flags applied.

        flag_column,reason_column,applied_column = self.getFlags()

# find the entry for this stageName - special case for first stage in recipe

        if stageName == firstStageName:
            break_row = 0
        else:
            break_row = None
            for row_num, row_val in enumerate(reason_column):
                if row_val['stageDescription']['name'] == stageName:
                    break_row = row_num
                    break;

# don't really know where we are if the stageName is not recognised

        if break_row == None:
            raise NameError, "stage '%s' not found" % stageName 
        else:

# delete all FLAG_CMD rows after and including the first occurrence

            self._table.open('%s/FLAG_CMD' % self._msName, nomodify=False)
            nrows = self._table.nrows()
            removeRows = range(break_row, nrows)
            self._table.removerows(rownrs=removeRows)
            self._table.close()

# restore the flags on entry to this stage, and make this the current state

# ..if we're starting from the beginning then go back to 'BeforeHeuristics'
#   in case the state for the first stage has not yet been stored.

            if stageName == firstStageName:
                temp = 'BeforeHeuristics'
            else:

# remove characters from the flagVersion that the filesystem will not like

                temp = stageName.replace(' ', '.')
                temp = temp.replace('{', '.')
                temp = temp.replace('}', '.')
                temp = temp.replace(':', '.')
                temp = temp.replace(',', '.')
                temp = temp.replace('/', '.')

#            print 'resetting state'
            self._table.open('%s/FIELD' % self._msName, nomodify=False)
            pickledFlaggingState = self._table.getkeyword('FLAGGING_STATE')
            self._flaggingState = pickle.loads(pickledFlaggingState)
            self._table.close()

            self._current_flag_mark = self._flaggingState[temp]['FLAG_MARK']
            self._current_flag_mark_col = self._flaggingState[temp]\
             ['FLAG_MARK_COL']

            self._flagger.restoreflagversion(temp)
            if self._verbose:
                print 'restoreflagversion', temp
            self._flagVersion = temp

            self.adoptAsCurrentFlagState()

# delete all flagging states where flag_mark is greater than 'Current' -
# but not 'Current' itself, nor 'BeforeHeuristics'

            flaggingStateCopy = self._flaggingState.copy()
            for k,v in flaggingStateCopy.iteritems():
                if k != 'Current' and k != 'BeforeHeuristics' and \
                 v['FLAG_MARK'] >= self._current_flag_mark:
                    ignore = self._flaggingState.pop(k)
                    self._flagger.deleteflagversion(k)

        if self._verbose:
            print 'state after reset:'
            print '   flagVersion          :', self._flagVersion
            print '   current_flag_mark    :', self._current_flag_mark
            print '   current_flag_mark_col:', self._current_flag_mark_col
            print 'available flagVersions:'
            for k,v in self._flaggingState.iteritems():
                print '                  ', k, v['FLAG_MARK'],\
                 v['FLAG_MARK_COL']


    def saveFlagState(self, flagVersion):
        """
        Keyword arguments:
        flagVersion -- The name to be given to the saved flag version.
        """  
        if self._verbose:
            print 'saveFlagState name=', flagVersion

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

# it is an error to do this if the flagversion now is not 'Current'.

        if self._flagVersion != 'Current':
            raise NameError, 'tried to save a non-current flagversion'

# remove characters from the flagVersion that the filesystem will not like

        flagVersion = flagVersion.replace(' ', '.')
        flagVersion = flagVersion.replace('{', '.')
        flagVersion = flagVersion.replace('}', '.')
        flagVersion = flagVersion.replace(':', '.')
        flagVersion = flagVersion.replace(',', '.')
        flagVersion = flagVersion.replace('/', '.')

# check that the state has not already been saved

        saveRequired = True
        if self._flaggingState.has_key(flagVersion):
            if (self._flaggingState[flagVersion]['FLAG_MARK'] == 
             self._current_flag_mark) and all(
             self._flaggingState[flagVersion]['FLAG_MARK_COL'] ==  
             self._current_flag_mark_col):
                if self._verbose:
                    print 'no new save required'
                saveRequired = False 
 
        if saveRequired:

# save the ms flagversion

            self._flagger.saveflagversion(flagVersion)

# store this info in the flaggingState dictionary

            self._flaggingState[flagVersion] = {
             'FLAG_MARK':self._current_flag_mark,
             'FLAG_MARK_COL':list(self._current_flag_mark_col)}


    def saveFlagStateToFile(self, flagVersion):
        """
        Method to save flag state to internal variables, and write these
        variables to the MS.

        Keyword arguments:
        flagVersion -- The name to be given to the saved flag version.
        """  

# save state internally

        self.saveFlagState(flagVersion)

# and write out internal variables to MS.

        self._table.open('%s/FIELD' % self._msName, nomodify=False)
        pickledFlaggingState = pickle.dumps(self._flaggingState)
        self._table.putkeyword('FLAGGING_STATE', pickledFlaggingState)
        self._table.close()


    def setFlagState(self, newFlagVersion):
        """
        Keyword arguments:
        newFlagVersion -- The flag version you want to be set in the data.
        """  
        if self._verbose:
            print 'setFlagState new=', newFlagVersion, ' current=',\
             self._flagVersion

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

# do nothing if already in desired flag state
            
        if newFlagVersion == self._flagVersion:
            pass
#            print 'nothing to be done'
        else:

# check that the desired flag version is known

            if not self._flaggingState.has_key(newFlagVersion):
                raise NameError, 'unknown flag version: ' + newFlagVersion
            else:

# if the current flag version is 'Current' then save the state

                if self._flagVersion == 'Current':
                    if self._verbose:
                        print 'saveflagversion Current'
                    self._flagger.saveflagversion('Current')

                    self._flaggingState['Current'] = {
                     'FLAG_MARK':self._current_flag_mark,
                     'FLAG_MARK_COL':list(self._current_flag_mark_col)}

# restore flag version to ms

                self._flagger.restoreflagversion(newFlagVersion)
                if self._verbose:
                    print 'restoreflagversion', newFlagVersion

# and set FLAG_MARK info

                self._current_flag_mark = self._flaggingState[newFlagVersion]\
                 ['FLAG_MARK']
                self._current_flag_mark_col = list(self._flaggingState[
                 newFlagVersion]['FLAG_MARK_COL'])

                self._flagVersion = newFlagVersion

# and invalidate the calibration that the MSCalibrater currently believes
# to be applied, if any

                self._msCalibrater.resetCalibrationApplied()

        if self._verbose:
            print 'flag mark', self._flaggingState[self._flagVersion]


    def setFlags(self, stageDescription, rules, flags, apply=True):
        """
        Keyword arguments:
        stageDescription -- Dictionary containing a description of the
                            calling stage.
        rules            -- List of the rules applied.
        flags            -- list of flags to be applied.
        apply            -- True if flags are to be applied, otherwise
                            do not apply them but store the flags in
                            the flag table.
        """
#        print 'MSFlagger.setflags'
        if self._verbose:
            print 'MSFlagger.setFlags called'

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'
    
# it's an error if you try to flag the ms when the flag state is other than
# 'Current'

        if self._flagVersion != 'Current':
            raise NameError, "trying to flag MS in non 'Current' state"
                        
# flag the table
                        
        self._flagTable(flags, apply)
                            
# pickle the flag dictionary and for now write it to the COMMAND column
# of the FLAG_CMD sub-table.

        self._table.open('%s/FLAG_CMD' % self._msName, nomodify=False)
        pickled_flags = pickle.dumps(flags)
        command_list = list(self._table.getcol('COMMAND'))
        command_list.append(pickled_flags)
        reason = {}
        reason['stageDescription'] = stageDescription
        reason['rules'] = rules
        pickled_reason = pickle.dumps(reason)
        reason_list = list(self._table.getcol('REASON'))
        reason_list.append(pickled_reason)
        applied_list = list(self._table.getcol('APPLIED'))
        applied_list.append(apply)
        self._table.addrows(1)
        self._table.putcol('COMMAND', array(command_list))
        self._table.putcol('REASON', array(reason_list))
        self._table.putcol('APPLIED', array(applied_list))
        self._table.close()

        if self._verbose:
            print 'after flags applied', self._flagVersion,\
             self._current_flag_mark, self._current_flag_mark_col


    def getValidFieldSpw(self):
        """Method to return a list of field/spw combinations that do contain
        data"""

        result = list(self._validFieldSpw)
        return result


    def writeFlaggingStatistics(self, stageDescription, rules, flagTargetIDs):
        """Method to log to html the number of flags set by the various
        rules used in this reduction stage.

        Keyword arguments:
        stageDescription -- Dictionary containing a description of the
                            calling stage.
        rules            -- List of the rules applied.
        flagTargetIDs    -- List of IDs of fields potentially flagged.
        """

        self._html.logHTML('<h3>Flagging Statistics</h3>')

        if self._flagVersion == 'msfCorrupt':
            raise NameError, 'MSFlagger is corrupt'

# check if any flagging was done
    
        if len(rules)==0:
            self._html.logHTML('No flagging was done.')
            return
        
# open FLAG_CMD table, access flagging record for this stage

        self._table.open('%s/FLAG_CMD' % self._msName)
        reasonCol = self._table.getcol('REASON')
        commandCol = self._table.getcol('COMMAND')
        self._table.close()

# get total number of rows in table

        num_rows = {}
        self._table.open(self._msName)
        num_rows['all'] = self._table.nrows()
        for spw in self._data_desc_ids:
            for field in flagTargetIDs:
                if self._validFieldSpw.count([field,spw]) == 0:
                    continue
                subTable = self._table.query('DATA_DESC_ID==%s && FIELD_ID==%s'\
                 % (spw,field))
                num_rows[(spw,field)] = subTable.nrows()
        self._table.close()

# calculate number of rows flagged by each rule

        data_flagged = {}
        for rule in rules:
            if rule.has_key('axis'):
                ruleName = '%s - %s' % (rule['rule'], rule['axis'])
            else:
                ruleName = rule['rule']
            data_flagged[ruleName] = {}
            for spw in self._data_desc_ids:
                for field in flagTargetIDs:
                    if self._validFieldSpw.count([field,spw]) == 0:
                        continue
                    data_flagged[ruleName][(spw,field)] = {}
                    data_flagged[ruleName][(spw,field)]['FLAG_SET'] = 0
                    data_flagged[ruleName][(spw,field)]['FLAG_ROW_SET'] = 0
                    data_flagged[ruleName][(spw,field)]['NCHANNELS_SET'] = 0

        for i,pickled_reason in enumerate(reasonCol):
            reason = pickle.loads(pickled_reason)
            if reason['stageDescription']['name'] == stageDescription['name']:
                flags = pickle.loads(commandCol[i])
                for k,v in flags.iteritems():
                    for flag in v:
                        if flag.has_key('DATA_DESC_ID'):
                            spw_list = flag['DATA_DESC_ID']
                            if type(spw_list) != types.ListType:
                                spw_list = [spw_list]
                        else:
                            spw_list = self._data_desc_ids

                        if flag.has_key('FIELD_ID'):
                            field_list = flag['FIELD_ID']
                            if type(field_list) != types.ListType:
                                field_list = [field_list]
                        else:
                            field_list = flagTargetIDs

                        if flag.has_key('CHANNELS'):
                            nchannels_set = len(flag['CHANNELS'])
                        else:
                            nchannels_set = None

                        for spw in self._data_desc_ids:
                            for field in flagTargetIDs:
                                if self._validFieldSpw.count([field,spw]) == 0:
                                    continue
                                if not flag['ROWS_SET'].has_key((spw,field)):
                                    continue

                                row_set = flag['ROWS_SET'][(spw,field)]

                                if nchannels_set != None:
                                    data_flagged[flag['rule']][(spw,field)]\
                                     ['FLAG_SET'] += row_set
                                    data_flagged[flag['rule']][(spw,field)]\
                                     ['NCHANNELS_SET'] = nchannels_set
                                else:
                                    data_flagged[flag['rule']][(spw,field)]\
                                     ['FLAG_ROW_SET'] += row_set

        self._html.logHTML('''
         <table CELLPADDING="5" BORDER="1"
             <tr>
                 <th>Rule</th>
                 <th></th>
             </tr>''')
        for rule in rules:
            if rule.has_key('axis'):
                ruleName = '%s - %s' % (rule['rule'], rule['axis'])
            else:
                ruleName = rule['rule']

            self._html.logHTML('''
             <tr>
                 <td>%s</td>''' % ruleName)

            self._html.logHTML('''
                 <td>
                     <table CELLPADDING="5" BORDER="1"
                         <tr>
                             <th>SpW</th>
                             <th>Field ID</th>
                             <th># rows</th>
                             <th># channels</th>
                             <th># rows where FLAG_ROW set here</th>
                             <th>percentage FLAG_ROW set</th>
                             <th># rows where FLAG elements set here</th>
                             <th>percentage FLAG rows affected</th>
                             <th># FLAG channels set here</th>
                             <th>percentage channels flagged</th>
                         </tr>''')

            keys = data_flagged[ruleName].keys()
            keys.sort()
            for k in keys:
                percentage_flag_row = 0.0
                percentage_flag = 0.0
                percentage_channels = 0.0
                flag_row_cell_colour = ''
                flag_cell_colour = ''
                channel_cell_colour = ''

                if num_rows[k] > 0:
                    if data_flagged[ruleName][k]['FLAG_ROW_SET'] > 0:
                        percentage_flag_row = 100.0 * float(
                         data_flagged[ruleName][k]['FLAG_ROW_SET']) / \
                         num_rows[k]
                        flag_row_cell_colour = 'bgcolor="red"'

                    if data_flagged[ruleName][k]['FLAG_SET'] > 0:
                        percentage_flag = 100.0 * float(
                         data_flagged[ruleName][k]['FLAG_SET']) / num_rows[k]
                        flag_cell_colour = 'bgcolor="red"'

                if self._nchannels[k[0]] > 0:
                    if data_flagged[ruleName][k]['NCHANNELS_SET'] > 0:
                        percentage_channels = 100.0 * float(
                         data_flagged[ruleName][k]['NCHANNELS_SET']) / \
                         self._nchannels[k[0]]
                        channel_cell_colour = 'bgcolor="red"'

                self._html.logHTML('''
                         <tr>
                             <td>%s</td>
                             <td>%s</td>
                             <td>%s</td>
                             <td>%s</td>
                             <td %s>%s</td>
                             <td>%4.2f</td>
                             <td %s>%s</td>
                             <td>%4.2f</td>
                             <td %s>%s</td>
                             <td>%4.2f</td>
                         </tr>''' % (k[0], k[1], num_rows[k],
                                     self._nchannels[k[0]],
                                     flag_row_cell_colour, 
                                     data_flagged[ruleName][k]['FLAG_ROW_SET'],
                                     percentage_flag_row,
                                     flag_cell_colour,
                                     data_flagged[ruleName][k]['FLAG_SET'],
                                     percentage_flag,
                                     channel_cell_colour, 
                                     data_flagged[ruleName][k]['NCHANNELS_SET'],
                                     percentage_channels))

            self._html.logHTML('''
                     </table>
                 </td>
             </tr>''')
        self._html.logHTML('''
         </table>''')
