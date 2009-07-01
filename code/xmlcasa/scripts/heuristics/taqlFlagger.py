"""Module that enables direct flagging of data using TaQL commands.
"""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Changed timing 'brackets'.
# 30-Aug-2007 jfl Flux calibrating release.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
#  5-Dec-2007 jfl Added 'PdB Gibbs channels' rule.
#  7-Jan-2008 jfl Added 'flag channels' rule, improved HTML description.
#  9-Jan-2008 jfl Bug fix to flag channels rule; FIELD_ID now list.  
# 14-Jan-2008 jfl Bug fix to flag channels rule; was not finding channels
#                 for all SpW.  
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
# 14-Jul-2008 jfl last 4769 release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

# alma modules

from baseFlagger import *


class TaqlFlagger(BaseFlagger):
    """Class to enable direct flagging of data by TaQL commands."""

    def __init__(self, htmlLogger, msName, rules, flag_targets = []):
        """Constructor.

        Keyword arguments:
        htmlLogger   -- HTMLLogger object providing access to html log.
        msName       -- Name of MeasurementSet to be flagged.
        rules        -- List of rules to be applied.
        flag_targets -- List of field_ids to which rules are to be applied.
        """

#        print 'TaqlFlagger.__init__ called'
        BaseFlagger.__init__(self, htmlLogger, msName, rules, flag_targets) 


    def description(self):
        description = ['''TaqlFlagger - use TaQL to flag specific data.''', []]
        for rule in self._rules:
            description[-1].append(rule['rule'])
        return description    
     

    def operate(self, stageDescription, dataView):
        """Method to apply the flags to the data.

        Keyword arguments:
        stageDescription -- Dictionary giving stage 'name' and 'colour' to be
                            used in plots for data flagged here.
        dataView         -- Object used to get basic info on the 
                            MeasurementSet.
        """

#        print 'TaqlFlagger.operate called'
        self._htmlLogger.timing_start('TaqlFlagger.operate')

        data = dataView.getData()
        telescopeName = data['summary']['telescope_name']
        antenna_range = data['summary']['antenna_range']
        data_desc_range = data['summary']['data_desc_range']
        nchannels = data['summary']['nchannels']

# potentially any field_id could be flagged

        self._potentially_flagged_target_ids = range(len(
         data['summary']['field_names']))

        flags = {}
        for rule in self._rules:
            flags[rule['rule']] = []

# flag data according to each rule in turn

        for ri, rule in enumerate(self._rules):
            if rule['rule'] == 'autocorrelation':
                for antenna in antenna_range:
                    flag_description = {}
                    flag_description['rule'] = rule['rule']
                    flag_description['colour'] = rule['colour']
                    flag_description['stageName'] = stageDescription['name']
                    flag_description['ANTENNA1'] = [antenna] 
                    flag_description['ANTENNA2'] = [antenna]
  
                    flags[rule['rule']].append(flag_description)

            elif rule['rule'] == 'PdB Gibbs channels':
                if telescopeName != 'IRAM_PDB':
                    continue

                for data_desc_id in data_desc_range:
                    nchan = nchannels[data_desc_id]

# do nothing if continuum or bandwidth < 40MHz

                    if nchan == 1:
                        continue

#                    chan_freq = data['summary']['chan_freq'][0]
#                    if abs(chan_freq[-1] - chan_freq[0]) < 40.0e6:
#                        print 'data_desc_id not Gibbs flagged', data_desc_id
#                        continue

# calculate the indeces of the channels to be flagged. This should depend
# on the PdB configuration for this SpW (i.e. is the SpW the merge of two
# or more subbands) but does not at present.

                    Gibbs_half_width = 2.0
                    channels = arange(nchan)
                    centre_channel = (nchan - 1.0) / 2.0
                    channels_flagged = compress(
                     abs(centre_channel - channels) <= Gibbs_half_width,
                     channels)

                    flag_description = {}
                    flag_description['rule'] = rule['rule']
                    flag_description['colour'] = rule['colour']
                    flag_description['stageName'] = stageDescription['name']
                    flag_description['DATA_DESC_ID'] = data_desc_id
                    flag_description['CHANNELS'] = channels_flagged
  
                    flags[rule['rule']].append(flag_description)

            elif rule['rule'] == 'flag channels':

# get list of field ids to which flags are to be applied

                target_field_ids = []
                for flag_target in self._flag_targets:
                    target_ids = dataView.getFieldsOfType(flag_target)
                    for target_id in target_ids:
                        if not(target_field_ids.count(target_id)):
                            target_field_ids.append(target_id)

                if len(target_field_ids) == 0:
                    continue

                bandpassFlaggingStage = rule['stageName']

# get the flagging info for the data

                flagging,flaggingReason,flaggingApplied = dataView.getFlags()

# flag the channels for each data_desc_id

                for data_desc_id in data_desc_range:
                    nchan = nchannels[data_desc_id]
                    if nchan == 1:
                        continue

# ..get the channels to be flagged, only gets the first flagging occurrence
#   for each spw

                    flag_channels = None
                    for row,val in enumerate(flaggingReason):
                        if val['stageDescription']['name'] == \
                         bandpassFlaggingStage:
                            for flagList in flagging[row].values():
                                for flag in flagList:
                                    if data_desc_id == flag['DATA_DESC_ID']:
                                        flag_channels = flag['CHANNELS']
                                        break
                            if flag_channels != None:
                                break 
                        if flag_channels != None:
                            break 

# ..do the flagging

                    if flag_channels != None:
                        flag_description = {}
                        flag_description['rule'] = rule['rule']
                        flag_description['colour'] = rule['colour']
                        flag_description['stageName'] = stageDescription['name']
                        flag_description['FIELD_ID'] = target_field_ids
                        flag_description['DATA_DESC_ID'] = data_desc_id
                        flag_description['CHANNELS'] = flag_channels
                        flags[rule['rule']].append(flag_description)

            else:           
                raise NameError, 'bad rule: %s' % rules[ri]

# set any flags raised

        n_flags = self._sumFlags(flags)
        print '        number of flagging commands raised %s' % n_flags
        dataView.setFlags(stageDescription, self._rules, flags)

        self._htmlLogger.timing_stop('TaqlFlagger.operate')
        return


    def writeGeneralHTMLDescription(self, stageName):
        """Write a general description of the class to html.

        Keyword arguments:
        stageName  -- Name of the recipe stage using this object.
        """

        self._htmlLogger.logHTML('''
         <p>The MeasurementSet was flagged directly as follows:<ul>''')

        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])
            if rule['rule'] == 'autocorrelation':
                description = '''Rows containing autocorrelation data.'''
            elif rule['rule'] == 'PdB Gibbs channels':
                description = '''To remove 'ringing' at subband
                 boundaries in spectral windows with bandwidth > 40MHz , 
                 channels within 2 indeces of the centre were flagged.''' 
            elif rule['rule'] == 'flag channels':
                description = '''Channel flags from stage '%s' were set
                 in source types %s.''' % (rule['stageName'],
                 self._flag_targets)

            self._htmlLogger.logHTML('<li> %s' % description)
        self._htmlLogger.logHTML('</ul>')


    def writeDetailedHTMLDescription(self, stageName):
        """Write a description of the class to html.

        Keyword arguments:
        stageName -- Name of the recipe stage using this object.
        """

        self._htmlLogger.logHTML('<h3>Data Flagging</h3>')

        self._htmlLogger.logHTML('''
         Flags were set according to the following rules:<ul>''')

        for rule in self._rules:
            description = 'No description available for flagging rule: %s' % (
             rule['rule'])

            if rule['rule'] == 'autocorrelation':
                description = '''Rows containing autocorrelation data in 
                 the %s were flagged. This was done using %s to
                 read the rows with %s==%s, setting their 
                 %s, then writing them back.''' % (
                 self._htmlLogger.glossaryLink('MeasurementSet'),
                 self._htmlLogger.glossaryLink('TaQL'),
                 self._htmlLogger.glossaryLink('ANTENNA1'),
                 self._htmlLogger.glossaryLink('ANTENNA2'),
                 self._htmlLogger.glossaryLink('FLAG_ROW'))
            elif rule['rule'] == 'PdB Gibbs channels':
                description = '''Spectral SpW with bandwidth > 40MHz are 
                 the result of 2 subbands merged together. The centre of the 
                 composite spectrum lies at the noisy edges of both subbands 
                 and may exhibit 'ringing' that cannot be calibrated out. 
                 %s was used to select each such SpW in turn, then channels 
                 within 2 indeces of the centre channel had their FLAG values
                 set.''' % (self._htmlLogger.glossaryLink('TaQL'))

            elif rule['rule'] == 'flag channels':
                description = '''Channel flags derived at another stage
                 were applied directly to the data. The flagging stage
                 to be used was read from 'stageName'; and the types of
                 source to be flagged were set in 'flag_targets'. In this case
                 'stageName' was %s and 'flag_targets' %s''' % (
                 rule['stageName'], self._flag_targets)


            self._htmlLogger.logHTML('<li> %s' % description)

        self._htmlLogger.logHTML('''</ul>
        <p>The flagging was performed by Python class TaqlFlagger.''')
