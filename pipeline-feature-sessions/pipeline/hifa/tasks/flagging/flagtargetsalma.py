from __future__ import absolute_import
import os
import types
import string

import flaghelper

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks

# the logger for this module
LOG = infrastructure.get_logger(__name__)

"""
Flag ALMA target science target data.
"""

class FlagTargetsALMAInputs(basetask.StandardInputs):
    """
    FlagTargetsALMA Inputs manages the inputs for the FlagTargetALMA task.
    
    .. py:attribute:: context

        the (:class:`~pipeline.infrastructure.launcher.Context`) holding all
        pipeline state

    .. py:attribute:: vis
    
        a string or list of strings containing the MS name(s) on which to
        operate
        
    .. py:attribute:: output_dir
    
        the directory to which pipeline data should be sent

    .. py:attribute:: flagbackup
    
        a boolean indicating whether whether existing flags should be backed
        up before new flagging begins.

    .. py:attribute:: template
    
        A boolean indicating whether flagging templates are to be applied.

    .. py:attribute:: filetemplate

        The filename of the ASCII file that contains the flagging template 
    """    

    flagbackup = basetask.property_with_default('flagbackup', False)
    template = basetask.property_with_default('template', True)

    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
                 template=None, filetemplate=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


    @property
    def inpfile(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('inpfile')

        vis_root = os.path.splitext(self.vis)[0].split('_target')[0]
        return os.path.join(self.output_dir, vis_root + '.flagtargetscmds.txt')


    @property
    def filetemplate(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('filetemplate')

        if not self._filetemplate:
            vis_root = os.path.splitext(self.vis)[0].split('_target')[0]
            return vis_root + '.flagtargetstemplate.txt'

        if type(self._filetemplate) is types.ListType:
            idx = self._my_vislist.index(self.vis)
            return self._filetemplate[idx]

        return self._filetemplate

    @filetemplate.setter
    def filetemplate(self, value):
        if value in (None, ''):
            value = []
        elif type(value) is types.StringType:
            value = list(value.replace('[','').replace(']','').replace("'","").split(','))
        self._filetemplate = value
    
    def to_casa_args(self):
        """
        Translate the input parameters of this class to task parameters 
        required by the CASA task flagdata. The returned object is a 
        dictionary of flagdata arguments as keyword/value pairs.
        
        :rtype: dict        
        """
        return {'vis'        : self.vis,
                'mode'       : 'list',
                'action'     : 'apply',                     
                'inpfile'    : self.inpfile,
                'flagbackup' : self.flagbackup}


# tell the infrastructure to prefentially apply the targets
# flags to the split MS(s)
api.ImagingMeasurementSetsPreferred.register(FlagTargetsALMAInputs)

class FlagTargetsALMAResults(basetask.Results):
    def __init__(self, summaries, flagcmds):
       super(FlagTargetsALMAResults, self).__init__()
       self.summaries = summaries
       self._flagcmds = flagcmds

    def flagcmds(self):
        return self._flagcmds

    def merge_with_context(self, context):
        # nothing to do
        pass

    def __repr__(self):
        # Step through the summary list and print a few things.
        # SUBTRACT flag counts from previous agents, because the counts are
        # cumulative.
        s = 'Target flagging results:\n'
        
        for idx in range(0, len(self.summaries)):
            flagcount = int(self.summaries[idx]['flagged'])
            totalcount = int(self.summaries[idx]['total'])

            # From the second summary onwards, subtract counts from the previous
            # one
            if idx > 0:
                flagcount = flagcount - int(self.summaries[idx-1]['flagged'])

            s += '\tSummary %s (%s) :  Flagged : %s out of %s (%0.2f%%)\n' % (
                    idx, self.summaries[idx]['name'], flagcount, totalcount,
                    100.0*flagcount/totalcount)
        
        return s


class FlagTargetsALMA(basetask.StandardTaskTemplate):
    """
    FlagTargetsALMA is a class for target flagging. It can perform
    
    - Template flags
    
    """

    # link the accompanying inputs to this task 
    Inputs = FlagTargetsALMAInputs

    def prepare(self):
        """
        Prepare and execute a flagdata flagging job appropriate to the
        task inputs.

        """
        # create a local alias for inputs, so we're not saying 'self.inputs'
        # everywhere
        inputs = self.inputs
        
        # get the flagdata command string required for the results
        flag_cmds = self._get_flag_commands()
        flag_str = '\n'.join(flag_cmds)

        # write the flag commands to the file
        with open(inputs.inpfile, 'w') as stream:
            stream.writelines(flag_str)

        # to save inspecting the file, also log the flag commands
        LOG.debug('Flag commands for %s:\n%s', inputs.vis, flag_str)

        # Map the pipeline inputs to a dictionary of CASA task arguments 
        task_args = inputs.to_casa_args()

        # create and execute a flagdata job using these task arguments
        job = casa_tasks.flagdata(**task_args)
        summary_dict = self._executor.execute(job)

        agent_summaries = dict((v['name'], v) for v in summary_dict.values())
        
        ordered_agents = ['before', 'template']

        summary_reps = [agent_summaries[agent] 
                        for agent in ordered_agents 
                        if agent in agent_summaries]

        # return the results object, which will be used for the weblog
        return FlagTargetsALMAResults(summary_reps, flag_cmds)

    def analyse(self, results):
        """
        Analyse the results of the flagging operation.
        
        This method does not perform any analysis, so the results object is
        returned exactly as-is, with no data massaging or results items
        added. If additional statistics needed to be calculated based on the
        post-flagging state, this would be a good place to do it.
        """
        return results

    def _get_flag_commands(self):
        """
        Get the flagging commands as a string suitable for flagdata.
        """
        # create a local variable for the inputs associated with this instance
        inputs = self.inputs
        
        # the empty list which will hold the flagging commands
        flag_cmds = []        
        flag_cmds.append("mode='summary' name='before'")
        
        # flag template?
        if inputs.template:
            if not os.path.exists(inputs.filetemplate):
                LOG.warning('Template flag file \'%s\' for \'%s\' not found.'
                             % (inputs.filetemplate, inputs.ms.basename))
            else:
                flag_cmds.extend(self._read_flagfile(inputs.filetemplate))
            flag_cmds.append("mode='summary' name='template'")
    
        
        return flag_cmds

    def _read_flagfile(self, filename):
        if not os.path.exists(filename):
            LOG.warning('%s does not exist' % filename)
            return []

        # strip out comments and empty lines to leave the real commands.
        # This is so we can compare the number of valid commands to the number
        # of commands specified in the file and complain if they differ
        return [cmd for cmd in flaghelper.readFile(filename) 
                if not cmd.strip().startswith('#')
                and not all(c in string.whitespace for c in cmd)]
