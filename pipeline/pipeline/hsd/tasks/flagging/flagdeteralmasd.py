
# Imports
# -------

from __future__ import absolute_import

import collections
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
from pipeline.domain import DataTable
from pipeline.h.tasks.flagging import flagdeterbase
from pipeline.infrastructure.displays import pointing


# ------------------------------------------------------------------------------

# Initialize a logger
# -------------------

LOG = infrastructure.get_logger(__name__)

# ------------------------------------------------------------------------------

class FlagDeterALMASingleDishInputs(flagdeterbase.FlagDeterBaseInputs):


    edgespw = basetask.property_with_default('edgespw', True)
    fracspw = basetask.property_with_default('fracspw', 0.0625)
    template = basetask.property_with_default('template', True)

    #New property for QA0 flags
    qa0 = basetask.property_with_default('qa0', True)
    qa2 = basetask.property_with_default('qa2', True)

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, output_dir=None, flagbackup=None,
                 autocorr=None, shadow=None, scan=None, scannumber=None,
                 intents=None, edgespw=None, fracspw=None, fracspwfps=None, online=None,
                 fileonline=None, template=None, filetemplate=None, hm_tbuff=None, tbuff=None, 
                 qa0=None, qa2=None):

        # Initialize the public member variables of the inherited class
        # FlagDeterBaseInputs()

        super(FlagDeterALMASingleDishInputs, self).__init__(
            context, vis=vis, output_dir=output_dir, flagbackup=flagbackup,
            autocorr=autocorr, shadow=shadow, scan=scan, scannumber=scannumber,
            intents=intents, edgespw=edgespw, fracspw=fracspw,
            fracspwfps=fracspwfps, online=online, fileonline=fileonline,
            template=template, filetemplate=filetemplate, hm_tbuff=hm_tbuff,
            tbuff=tbuff)

        self.fracspwfps = fracspwfps
        self.qa0 = qa0
        self.qa2 = qa2

    # autocorr parameter must be overridden since its default 
    # value must be False
    @property
    def autocorr(self):
        return self._autocorr
    
    @autocorr.setter
    def autocorr(self, value):
        if value is None:
            value = False
        self._autocorr = value

    # flacspw must be overridden since its default value 
    # must be '1.875GHz'
    @property
    def fracspw(self):
        return self._fracspw
    
    @fracspw.setter
    def fracspw(self, value):
        if value is None:
            value = '1.875GHz'
        self._fracspw = value

    @property
    def fracspwfps(self):
        return self._fracspwfps

    @fracspwfps.setter
    def fracspwfps(self, value):
        if value is None:
            value = 0.048387
        self._fracspwfps = value

    @property
    def intents(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('intents')

        if self._intents is not None:
            return self._intents

        # return just the unwanted intents that are present in the MS
        intents_to_flag = set(['POINTING','FOCUS','ATMOSPHERE','SIDEBAND','UNKNOWN', 'SYSTEM_CONFIGURATION', 'CHECK'])
        return ','.join(self.ms.intents.intersection(intents_to_flag))
    
    @intents.setter
    def intents(self, value):
        self._intents = value

    def to_casa_args(self):

        # Initialize the arguments from the inherited
        # FlagDeterBaseInputs() class

        task_args = super(FlagDeterALMASingleDishInputs, self).to_casa_args()


        # Return the tflagdata task arguments

        return task_args



class FlagDeterALMASingleDishResults(flagdeterbase.FlagDeterBaseResults):
    def merge_with_context(self, context):
        # call parent's method
        super(FlagDeterALMASingleDishResults, self).merge_with_context(context)
        
        # update datatable
        datatable = DataTable(name=context.observing_run.ms_datatable_name, readonly=False)
        # this task uses _handle_multiple_vis framework 
        msobj = context.observing_run.get_ms(self.inputs['vis'])
        datatable._update_flag(context, msobj.name)
        datatable.exportdata(minimal=False)
        
        # regenerate pointing plots
        if not basetask.DISABLE_WEBLOG:
            LOG.info('Regenerate pointing plots to update flag information')
            for antenna in msobj.antennas:
                for (target, reference) in msobj.calibration_strategy['field_strategy'].iteritems():
                    LOG.debug('target field id %s / reference field id %s'%(target,reference))
                    task = pointing.SingleDishPointingChart(context, msobj, antenna, 
                                                                target_field_id=target,
                                                                reference_field_id=reference,
                                                                target_only=True)
                    task.plot(revise_plot=True)
                    task = pointing.SingleDishPointingChart(context, msobj, antenna, 
                                                                target_field_id=target,
                                                                reference_field_id=reference,
                                                                target_only=False)
                    task.plot(revise_plot=True)
            


class FlagDeterALMASingleDish(flagdeterbase.FlagDeterBase):

# ------------------------------------------------------------------------------

    # Make the member functions of the FlagDeterALMASingleDishInputs() class member
    # functions of this class

    Inputs = FlagDeterALMASingleDishInputs
    
    # Flag edge channels if bandwidth exceeds bandwidth_limit
    # Currently, default bandwidth limit is set to 1.875GHz but it is 
    # controllable via parameter 'fracspw' 
    @property
    def bandwidth_limit(self):
        if isinstance(self.inputs.fracspw, str):
            return casatools.quanta.convert(self.inputs.fracspw, 'Hz')['value']
        else:
            return 1.875e9 # 1.875GHz
    
    def prepare(self):
        results = super(FlagDeterALMASingleDish, self).prepare()
        return FlagDeterALMASingleDishResults(results.summaries, results.flagcmds())
        
    def _yield_edge_spw_cmds(self):
        inputs = self.inputs
        # loop over the spectral windows, generate a flagging command for each
        # spw in the ms. Calling get_spectral_windows() with no arguments
        # returns just the science windows, which is exactly what we want.
        for spw in inputs.ms.get_spectral_windows():
            try:
                # test that this spw should be flagged by assessing number of
                # correlations, TDM/FDM mode etc.
                self.verify_spw(spw)
            except ValueError as e:
                # this spw should not be or is incapable of being flagged
                LOG.debug(e.message)
                continue

            # get fraction of spw to flag from template function
            fracspw_org = inputs.fracspw
            try:
                fracspw_list = []
                for _frac in fracspw_org:
                    inputs.fracspw = _frac
                    fracspw_list.append(self.get_fracspw(spw))
            finally:
                inputs.fracspw = fracspw_org
            if len(fracspw_list) == 0:
                continue
            elif len(fracspw_list) == 1:
                fracspw_list.append(fracspw_list[0])

            # If the twice the number of flagged channels is greater than the
            # number of channels for a given spectral window, skip it.
            #frac_chan = int(round(fracspw * spw.num_channels + 0.5))
            # Make rounding less agressive
            frac_chan_list = map(lambda x: int(round(x * spw.num_channels)), fracspw_list)[:2]
            if sum(frac_chan_list) >= spw.num_channels:
                LOG.debug('Too many flagged channels %s for spw %s '
                          '' % (spw.num_channels, spw.id))
                continue

            # calculate the channel ranges to flag. No need to calculate the
            # left minimum as it is always channel 0.
            l_max = frac_chan_list[0] - 1
            #r_min = spw.num_channels - frac_chan - 1
            # Fix asymmetry 
            r_min = spw.num_channels - frac_chan_list[1]
            r_max = spw.num_channels - 1

            # state the spw and channels to flag in flagdata format, adding
            # the statement to the list of flag commands
            def yield_channel_ranges():
                if l_max >= 0:
                    yield '0~{0}'.format(l_max)
                if r_max >= r_min:
                    yield '{0}~{1}'.format(r_min, r_max)
            channel_ranges = list(yield_channel_ranges())

            if len(channel_ranges) == 0:
                continue
            
            cmd = '{0}:{1}'.format(spw.id, ';'.join(channel_ranges))
            
            LOG.debug('list type edge fraction specification for spw %s' % spw.id)
            LOG.debug('cmd=\'%s\'' % cmd)

            yield cmd

    def _get_edgespw_cmds(self):
        inputs = self.inputs
        
        if isinstance(inputs.fracspw, float) or isinstance(inputs.fracspw, str):
            to_flag = super(FlagDeterALMASingleDish, self)._get_edgespw_cmds()
        elif isinstance(inputs.fracspw, collections.Iterable):
            # inputs.fracspw is iterable indicating that the user want to flag 
            # edge channels with different fractions/number of channels for 
            # left and right edges
            

            # to_flag is the list to which flagging commands will be appended
            to_flag = list(self._yield_edge_spw_cmds())
                        
        return to_flag
    
    def get_fracspw(self, spw):    
        # override the default fracspw getter with our ACA-aware code
        #if spw.num_channels in (62, 124, 248):
        #    return self.inputs.fracspwfps
        #else:
        #    return self.inputs.fracspw
        if isinstance(self.inputs.fracspw, float):
            return self.inputs.fracspw
        elif isinstance(self.inputs.fracspw, str):
            LOG.debug('bandwidth limited edge flagging for spw %s' % spw.id)
            bandwidth_limit = self.bandwidth_limit
            bandwidth = float(spw.bandwidth.value)
            fracspw = 0.5 * (bandwidth - bandwidth_limit) / bandwidth
            LOG.debug('fraction is %s' % fracspw)
            return max(0.0, fracspw)

    def verify_spw(self, spw):
        # override the default verifier, adding bandwidth check
        super(FlagDeterALMASingleDish, self).verify_spw(spw)

        # Skip if TDM mode where TDM modes are defined to be modes with 
        # <= 256 channels per correlation
        #dd = self.inputs.ms.get_data_description(spw=spw)
        #ncorr = len(dd.corr_axis)
        #if ncorr * spw.num_channels > 256:
        #    raise ValueError('Skipping edge flagging for FDM spw %s' % spw.id)

        # Skip if edge channel flagging is based on bandwidth limit, and 
        # bandwidth is less than bandwidth limit
        if isinstance(self.inputs.fracspw, str) and spw.bandwidth.value <= self.bandwidth_limit:
            raise ValueError('Skipping edge flagging for spw %s' % spw.id)

    def _get_flag_commands(self):
        """
        Edit flag commands so that all summaries are based on target data instead of total.
        """
        flag_cmds = super(FlagDeterALMASingleDish, self)._get_flag_commands()
        for i in xrange(len(flag_cmds)):
            if flag_cmds[i].startswith("mode='summary'"):
                flag_cmds[i] += " intent='OBSERVE_TARGET#ON_SOURCE'"
        
        return flag_cmds
