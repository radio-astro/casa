"""Module with Base Class for flagging classes."""

# History:
# 16-Jul-2007 jfl First version.
# 30-Aug-2007 jfl Flux calibrating release.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 13-May-2008 jfl 13 release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.

# package modules

from numpy import * 


class BaseFlagger:
    """Base Class for flagging classes.
    """

    def __init__(self, htmlLogger, msName, rules, flag_targets = []):
        """Constructor.

        Keyword arguments:
        htmlLogger   -- Route for logging to html structure.
        msName       -- Name of MeasurementSet
        rules        -- list of flagging rules.
        flag_targets -- List of field ids to be flagged.
        """

#        print 'BaseFlagger constructor called: %s %s' % (rules, flag_targets)
        self._rules = rules
        self._flag_targets = flag_targets
        self._htmlLogger = htmlLogger
        self._description = {}
        self._potentially_flagged_target_ids = []


    def _sumFlags(self, flag_dictionary):
        """Sum number of flags in dictionary
        """
        n_flags = 0
        for key in flag_dictionary.keys():
            n_flags += len(flag_dictionary[key])
        return n_flags


    def description(self):
        description = None
        return description


    def median_mad(self, data):
        """Return the median and MAD of data.
        """
        data_median = 0.0
        data_mad = 0.0
        if len(data) > 0:
            data_median = median(data)
            data_mad = median(abs(data - data_median))
        return data_median, data_mad


    def operate(self, stageDescription, dataView):
        """Public method to apply the flag rules to the data 'view'.

        Keyword arguments:
        stageDescription -- Dictionary with a description of the calling
                            reduction stage.
        dataView         -- Object providing the data 'view'.
        """


#        print 'BaseFlagger.operate called'

# descriptions of rules

        self._description['remove flags'] = '''
         <h4>Description of algorithm</h4>
         Removes flags. TBD.''' 

        self._htmlLogger.logHTML('''BaseFlagger object was run
         on the data, applying the following rules:<ul>''')
        for rule in self._rules:
            link = rule['rule']
            linkInfo = ""
            self._htmlLogger.openNode(link,
             '%s.%s' % (stageDescription['name'], rule['rule']), True,
             linkInfo)
            description = 'no description available'
            if self._description.has_key(rule['rule']):
                description = self._description[rule['rule']] 
            self._htmlLogger.logHTML(description)
            self._htmlLogger.closeNode()
        self._htmlLogger.logHTML('</ul>')
        
# if this stage has been run on the ms before, then reset the ms
# to the pre-stage flagged state

        dataView.resetFlags(stageDescription['name'])

# get list of field ids to which flags are to be applied

        flag_target_ids = []
        for flag_target in self._flag_targets:
            target_ids = dataView.getFieldsOfType(flag_target)
            for target_id in target_ids:
                if not(flag_target_ids.count(target_id)):
                    flag_target_ids.append(target_id)

# get the data before the flags are applied - flagger doesn't use this but
# it makes dataView record the 'before' state for display purposes.

        ignore = dataView.getData()

        flags = {}
        for rule in self._rules:
            flags[rule['rule']] = []

        for rule in self._rules:
            if rule['rule'] == 'remove flags':
                new_flags = [{'stageName':stageDescription['name'],
                 'rule':rule['rule'], 'stageName':rule['stageName']}]
                flags[rule['rule']] += new_flags
            elif rule['rule'] == 'transfer flags':
                new_flags = dataView.transferFlags(stageDescription, rule)
                flags[rule['rule']] += new_flags
            else:           
                raise NameError, 'bad rule: %s' % rule['rule']

# record this stage in the FLAG_CMD subTable.

        dataView.setFlags(stageDescription, self._rules, flags)


    def potentiallyFlaggedTargetIDs(self):
        """Return list of field IDs that pontentially had flags applied.
        """
        return self._potentially_flagged_target_ids


    def rules(self):
        """Return list of rules applied.
        """
        rules = list(self._rules)
        return rules


    def writeHTMLDescription(self, stageName):
        """Write a description of the class to HTML.

        Keyword arguments:
        stageName -- Name of recipe stage using this class.
        """
        self._htmlLogger.logHTML("""No flagging description available""")
