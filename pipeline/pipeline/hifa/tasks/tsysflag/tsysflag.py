from __future__ import absolute_import

import re
import collections
import types

import numpy as np 

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils

from pipeline.hif.tasks.flagging.flagdatasetter import FlagdataSetter
from pipeline.hif.tasks.common import commonresultobjects
from pipeline.hif.tasks.common import calibrationtableaccess as caltableaccess
from pipeline.hif.tasks.common import commonhelpermethods
from pipeline.hif.tasks.common import viewflaggers

from .resultobjects import TsysflagResults, TsysflagspectraResults, TsysflagDataResults, TsysflagViewResults

LOG = infrastructure.get_logger(__name__)


class TsysflagInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
      flag_nmedian=None, fnm_limit=None, fnm_byfield=None,
      flag_derivative=None, fd_max_limit=None,
      flag_edgechans=None, fe_edge_limit=None,
      flag_fieldshape=None, ff_refintent=None, ff_max_limit=None,
      flag_birdies=None, fb_sharps_limit=None, 
      flag_toomany=None, tmf1_limit=None, tmef1_limit=None,
      metric_order=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def caltable(self):
        if self._caltable is None:
            caltables = self.context.callibrary.active.get_caltable(
              caltypes='tsys')

            # return just the tsys table that matches the vis being handled
            result = None
            for name in caltables:
                # Get the tsys table name
                tsystable_vis = \
                  caltableaccess.CalibrationTableDataFiller._readvis(name)
                if tsystable_vis in self.vis:
                    result = name
                    break

            return result

        return self._caltable

    @caltable.setter
    def caltable(self, value):
        self._caltable = value

    @property
    def flag_nmedian(self):
        return self._flag_nmedian

    @flag_nmedian.setter
    def flag_nmedian(self, value):
        if value is None:
            value = True
        self._flag_nmedian = value
 
    @property
    def fnm_limit(self):
        return self._fnm_limit

    @fnm_limit.setter
    def fnm_limit(self, value):
        if value is None:
            value = 2.0
        self._fnm_limit = value

    @property
    def fnm_byfield(self):
        return self._fnm_byfield

    @fnm_byfield.setter
    def fnm_byfield(self, value):
        if value is None:
            value = False
        self._fnm_byfield = value

    @property
    def flag_derivative(self):
        return self._flag_derivative

    @flag_derivative.setter
    def flag_derivative(self, value):
        if value is None:
            value = True
        self._flag_derivative = value

    @property
    def fd_max_limit(self):
        return self._fd_max_limit

    @fd_max_limit.setter
    def fd_max_limit(self, value):
        if value is None:
            value = 5
        self._fd_max_limit = value

    @property
    def flag_edgechans(self):
        return self._flag_edgechans

    @flag_edgechans.setter
    def flag_edgechans(self, value):
        if value is None:
            value = True
        self._flag_edgechans = value

    @property
    def fe_edge_limit(self):
        return self._fe_edge_limit

    @fe_edge_limit.setter
    def fe_edge_limit(self, value):
        if value is None:
            value = 3.0
        self._fe_edge_limit = value

    @property
    def flag_fieldshape(self):
        return self._flag_fieldshape

    @flag_fieldshape.setter
    def flag_fieldshape(self, value):
        if value is None:
            value = True
        self._flag_fieldshape = value

    @property
    def ff_refintent(self):
        return self._ff_refintent

    @ff_refintent.setter
    def ff_refintent(self, value):
        if value is None:
            value = 'BANDPASS'
        self._ff_refintent = value

    @property
    def ff_max_limit(self):
        return self._ff_max_limit

    @ff_max_limit.setter
    def ff_max_limit(self, value):
        if value is None:
            value = 5
        self._ff_max_limit = value

    @property
    def flag_birdies(self):
        return self._flag_birdies

    @flag_birdies.setter
    def flag_birdies(self, value):
        if value is None:
            value = True
        self._flag_birdies = value

    @property
    def fb_sharps_limit(self):
        return self._fb_sharps_limit

    @fb_sharps_limit.setter
    def fb_sharps_limit(self, value):
        if value is None:
            value = 0.05
        self._fb_sharps_limit = value

    @property
    def flag_toomany(self):
        return self._flag_birdies

    @flag_toomany.setter
    def flag_toomany(self, value):
        if value is None:
            value = True
        self._flag_toomany = value

    @property
    def tmf1_limit(self):
        return self._tmf1_limit

    @tmf1_limit.setter
    def tmf1_limit(self, value):
        if value is None:
            value = 0.666
        self._tmf1_limit = value

    @property
    def tmef1_limit(self):
        return self._tmef1_limit

    @tmef1_limit.setter
    def tmef1_limit(self, value):
        if value is None:
            value = 0.666
        self._tmef1_limit = value

    @property
    def metric_order(self):
        return self._metric_order

    @metric_order.setter
    def metric_order(self, value):
        if value is None:
            value = 'nmedian, derivative, edgechans, fieldshape, birdies, toomany'
        self._metric_order = value


class Tsysflag(basetask.StandardTaskTemplate):
    Inputs = TsysflagInputs

    def prepare(self):
        
        inputs = self.inputs
        
        # Initialize the final result and store caltable in result
        result = TsysflagResults()
        result.caltable = inputs.caltable

        # Load the input tsys caltable and store its vis in the result
        tsystable = caltableaccess.CalibrationTableDataFiller.getcal(inputs.caltable)        
        result.vis = tsystable.vis

        # Collect requested flag metrics from inputs into a dictionary.
        # NOTE: each key in the dictionary below should have been added to 
        # the default value for the "metric_order" property in TsysflagInputs,
        # or otherwise the Tsysflag task will always end prematurely for 
        # automatic pipeline runs.
        metrics_from_inputs = {'nmedian': inputs.flag_nmedian,
                               'derivative': inputs.flag_derivative,
                               'edgechans': inputs.flag_edgechans,
                               'fieldshape': inputs.flag_fieldshape,
                               'birdies': inputs.flag_birdies,
                               'toomany': inputs.flag_toomany}

        # Convert metric order string to list of strings
        metric_order_as_list = [metric.strip() for metric in inputs.metric_order.split(',')]
        
        # If the input metric order list contains illegal values, then log
        # an error and stop further evaluation of Tsysflag.
        for metric in metric_order_as_list:
            if metric not in metrics_from_inputs.keys():
                errmsg = "Input parameter 'metric_order' contains illegal value: '{0}'. Accepted values are: {1}.".format(metric, ', '.join(metrics_from_inputs.keys()))
                LOG.error(errmsg)
                result.task_incomplete_reason = errmsg
                return result
        
        # If any of the requested metrics are not defined in the metric order,
        # then log an error and stop further evaluation of Tsysflag.
        for metric_name, metric_enabled in metrics_from_inputs.items():
            if metric_enabled and metric_name not in metric_order_as_list:
                errmsg = "Flagging metric '{0}' is enabled, but not specified in 'metric_order'.".format(metric_name)
                LOG.error(errmsg)
                result.task_incomplete_reason = errmsg
                return result

        # Initialize ordered list of metrics to evaluate
        ordered_list_metrics_to_evaluate = []
        
        # Convert requested flagging metrics to ordered list.
        for metric in metric_order_as_list:
            if metrics_from_inputs[metric]:
                ordered_list_metrics_to_evaluate.append(metric)
                
        # Store order of metrics in result
        result.metric_order = ordered_list_metrics_to_evaluate

        # Run flagger for each metric.
        for metric in ordered_list_metrics_to_evaluate:
            result.add(metric, self._run_flagger(metric))

        # Extract before and after flagging summaries from individual results:
        stats_before = result.components[ordered_list_metrics_to_evaluate[0]].summaries[0]
        stats_after = result.components[ordered_list_metrics_to_evaluate[-1]].summaries[-1]

        # Add the "before" and "after" flagging summaries to the final result
        result.summaries = [stats_before, stats_after]

        return result


    def analyse(self, result):
        """
        Analyses the Tsysflag result:

        Identifies, for each spw, which antennas have been entirely flagged for all timestamps,
        and raises a warning about these. Identifies antennas that are entirely flagged
        in all Tsys spws.
        """
                
        # If the task ended prematurely, then no need to analyse the result
        if result.task_incomplete_reason:
            return result

        # Define the 2D metrics based on which we can determine if any antennas were entirely flagged.
        testable_metrics = ['nmedian', 'derivative', 'fieldshape', 'toomany']

        # Identify which of the testable metrics were evaluated:
        try:
            testable_metrics_completed = [metric for metric in result.metric_order 
              if metric in testable_metrics and metric in result.components.keys()]
        except AttributeError:
            testable_metrics_completed = []

        # Get the MS object
        ms = self.inputs.context.observing_run.get_ms(name=self.inputs.vis)

        # If any of the testable metrics were completed...
        if testable_metrics_completed:
            
            # Pick the testable metric that ran last.
            metric_to_test = testable_metrics_completed[-1]
            LOG.trace("Using metric '{0}' to evaluate if any antennas were entirely flagged.".format(metric_to_test))

            # Identify fields present in Tsys table:
            # Read in the Tsys table, and extract the field ids.
            tsystable = caltableaccess.CalibrationTableDataFiller.getcal(self.inputs.caltable)
            fields = {}
            for row in tsystable.rows:
                time = row.get('TIME')
                if time not in fields.keys():
                    fields[time] = row.get('FIELD_ID')
            
            # Get the science spw ids from the MS.
            science_spw_ids = [spw.id for spw in ms.get_spectral_windows(science_windows_only=True)]

            # Using the CalTo object in the TsysflagspectraResults from the
            # last-ran testable metric, identify which are the Tsys spectral
            # windows.
            calto = result.components[metric_to_test].final[0]
            tsys_spw_ids = [tsys for (spw,tsys) in enumerate(calto.spwmap) 
              if spw in science_spw_ids and spw not in result.unmappedspws]
            
            # Create translation dictionary, reject empty antenna name strings
            antenna_id_to_name = {ant.id: ant.name for ant in ms.antennas if ant.name.strip()}

            # Perform test separately for each of these intents.
            intents_of_interest = ['BANDPASS', 'AMPLITUDE', 'PHASE']

            # Create translation of field to intent for intents of interest.
            intents_for_field = dict((field.id, field.intents) for field in ms.fields)

            # Create translation of field ID to field name
            field_name_for_id = dict((field.id, field.name) for field in ms.fields)

            # Initialize set of antennas that are fully flagged for all spws, for any intent
            ants_fully_flagged_in_all_spws_any_intent = set()
            
            # Initialize flagging state
            flagging_state = collections.defaultdict(list)
            
            # Go through each view product for the specified metric.
            for description in result.components[metric_to_test].descriptions():
                
                # Get final view.
                view = result.components[metric_to_test].last(description)
                
                # Get spw
                spwid = view.spw
                
                # Go through each scan within view.
                for iscan, time in enumerate(view.axes[1].data):
                    
                    # Get flags per antenna for current scan
                    flag_per_scan = view.flag[:,iscan]
                    
                    # Get field ID and intent(s) for current scan
                    field_scan = fields[time]
                    intents_scan = intents_for_field[field_scan]
                    
                    # For each intent that this scan belongs to:
                    for intent in intents_scan:
                        # If this scan is for an intent of interest, then store scan
                        # accordingly.
                        if intent in intents_of_interest:
                            flagging_state[(intent,field_scan,spwid)].append(list(flag_per_scan))
            
            # After summarizing all available views, check if any antennas
            # are fully flagged for all available timestamps for one or more 
            # fields belonging to one or more of the intents of interest in 
            # one or more of the spws.
            ants_fully_flagged = collections.defaultdict(set)
            intents_found = set([key[0] for key in flagging_state.keys()])
            for intent in intents_found:
                fields_found = set([key[1] for key in flagging_state.keys() if key[0] == intent])
                for field in fields_found:
                    spws_found = set([key[2] for key in flagging_state.keys() if key[0:2] == (intent,field)])
                    for spwid in spws_found:
                        flags_per_ant = np.array(flagging_state[(intent,field,spwid)]).T
                        for iant, flag_for_ant in enumerate(flags_per_ant):
                            if flag_for_ant.all():
                                ants_fully_flagged[(intent,field,spwid)].update([iant])   
            
            # Check if any antennas were found to be fully flagged in all 
            # scans (timestamps) for a given intent, field, and spwid:
            sorted_keys = sorted(sorted(ants_fully_flagged.keys(), key=lambda key: key[2]),
              key=lambda key: key[0])
            for (intent, field, spwid) in sorted_keys:
                
                ants_flagged = ants_fully_flagged[(intent, field, spwid)]
                
                # Convert antenna IDs to names and create a string.
                ants_str = ", ".join(map(str, [antenna_id_to_name[iant] for iant in ants_flagged])) 
                
                # Log a warning.
                LOG.warning("%s - for intent %s (field %s: %s) and spw %s, the following antennas are fully flagged: %s" % 
                    (ms.basename, intent, field, field_name_for_id[field], spwid, ants_str))
            
            # Check if any antennas were found to be fully flagged in all
            # scans (timestamps) and all spws, for any intent.
            #
            # NOTE: The following test checks for antennas fully flagged 
            # in all spws declared by CalTo (and assumes that flagging views were
            # available for each of those spw to be able to test it). 
            # Conversely, this test does not check for antennas fully flagged in 
            # all spws within the subset of spws for which flagging views were available. 
            # If a flagging view was not available for some spw and a given intent, 
            # then no antenna will be found to be fully flagged in all spws for that intent.
            
            # Identify the field and intent combinations for which fully flagged 
            # antennas were found.
            intent_field_found = set([key[0:2] for key in ants_fully_flagged.keys()])
            for (intent,field) in intent_field_found:
                
                # Identify the spws for which fully flagged antennas were found (for current
                # intent and field).
                spws_found = set([key[2] for key in ants_fully_flagged.keys() if key[0:2] == (intent,field)])
                
                # Only proceed if the set of spws for which flagged antennas were found is
                # the same as the set of spws declared by CalTo, i.e. flagged antennas
                # were found in all spws.
                if spws_found == set(tsys_spw_ids):
                    
                    # Select the fully flagged antennas for current intent and field.
                    ants_fully_flagged_for_intent_field = [ants_fully_flagged[key] for key in 
                        ants_fully_flagged.keys() if key[0:2] == (intent,field)]
                    
                    # Identify which antennas are fully flagged in all spws, for 
                    # current intent and field, and store these for later warning 
                    # and/or updating of refant.
                    ants_fully_flagged_in_all_spws_any_intent.update(
                      set.intersection(*ants_fully_flagged_for_intent_field))
               
        else:
            LOG.trace("Cannot test if any antennas were entirely flagged "
                      "since none of the following flagging metrics were evaluated: "
                      "{0}".format(', '.join(testable_metrics)))
        
        # Store the set of antennas that are fully flagged for all Tsys spws
        # in any of the intents in the result as a list of antenna names.
        bad_antennas = [antenna_id_to_name[iant] for iant in ants_fully_flagged_in_all_spws_any_intent]

        # Determine a list of reference antennas that are found to be 
        # fully flagged. These antennas should be removed from the 
        # refant list upon merging the result into the context.
        if bad_antennas and hasattr(ms, 'reference_antenna') and \
          type(ms.reference_antenna) == types.StringType:

            # Create list of current refants
            refant = ms.reference_antenna.split(',')
                
            # Find intersection between refants and fully flagged antennas
            # and store in result.
            result.refants_to_remove = set(bad_antennas).intersection(refant)

            # If refants were found to be flagged and in need of removal...
            if result.refants_to_remove:

                # Create string for log message.            
                ant_msg = utils.commafy(result.refants_to_remove, quotes=False)
    
                # Check if removal of refants would result in an empty refant list,
                # in which case the refant update is skipped.
                if result.refants_to_remove == set(refant):
                    
                    # Log warning that refant list should have been updated, but 
                    # will not be updated so as to avoid an empty refant list.
                    LOG.warning('%s - the following antennas are fully flagged '
                      'in all Tsys spws in the "BANDPASS", "PHASE", and/or '
                      '"AMPLITUDE" intents, but are *NOT* removed from the '
                      'refant list because doing so would result in an '
                      'empty refant list: %s' % (ms.basename, ant_msg))
                    
                    # Reset the refant removal list in the result to be empty.
                    result.refants_to_remove = []
                else:
                    # Log a warning if any antennas are to be removed from 
                    # the refant list.
                        # Log warning
                        LOG.warning('%s - the following antennas are removed from '
                          'the refant list because they are fully flagged in all '
                          'Tsys spws in the "BANDPASS", "PHASE", and/or "AMPLITUDE" '
                          'intents: %s' % (ms.basename, ant_msg))
        
        return result
    
    
    def _run_flagger(self, metric):
        """
        Evaluates the Tsys spectra for a specified flagging metric.
        
        Keyword arguments:
        metric      -- string : represents the flagging metric to evaluate. 
                       Valid values: 'nmedian', 'derivative', 'edgechans', 
                       'fieldshape', 'birdies', 'toomany'.
        
        Returns:
        TsysflagspectraResults object containing the flagging views and flagging
        results for the requested metric.
        """

        LOG.info('flag '+metric)
        inputs = self.inputs

        # Initialize results object
        result = TsysflagspectraResults()
        result.view_by_field = False
        
        # Load the input tsys caltable
        tsystable = caltableaccess.CalibrationTableDataFiller.getcal(inputs.caltable)        

        # Store the vis from the tsystable in the result
        result.vis = tsystable.vis

        # Get the Tsys spw map by retrieving it from the first tsys CalFrom 
        # that is present in the callibrary. 
        spwmap = utils.get_calfroms(inputs, 'tsys')[0].spwmap

        # Construct a callibrary entry, and store in result. This entry was already
        # created and merged into the context by tsyscal, but we recreate it here
        # since it is needed by the weblog renderer to create a Tsys summary chart.
        calto = callibrary.CalTo(vis=tsystable.vis)
        calfrom = callibrary.CalFrom(inputs.caltable, caltype='tsys', spwmap=spwmap)
        calapp = callibrary.CalApplication(calto, calfrom)
        result.final = [calapp]
                
        # Construct the task that will read the data and create the
        # view of the data that is the basis for flagging.
        datainputs = TsysflagDataInputs(context=inputs.context, vis=inputs.vis,
          caltable=inputs.caltable)
        datatask = TsysflagData(datainputs)

        # Construct the generator that will create the view of the data
        # that is the basis for flagging. Update result to indicate whether
        # separate views are created per field.
        if metric == 'fieldshape':
            viewtask = TsysflagView(context=inputs.context,
              vis=inputs.vis, metric=metric, refintent=inputs.ff_refintent)
        elif metric == 'nmedian':
            viewtask = TsysflagView(context=inputs.context,
              vis=inputs.vis, metric=metric, split_by_field=inputs.fnm_byfield)  
            result.view_by_field = inputs.fnm_byfield
        else:
            viewtask = TsysflagView(context=inputs.context,
              vis=inputs.vis, metric=metric)

        # Construct the task that will set any flags raised in the
        # underlying data.
        flagsetterinputs = FlagdataSetter.Inputs(context=inputs.context,
          vis=inputs.vis, table=inputs.caltable, inpfile=[])
        flagsettertask = FlagdataSetter(flagsetterinputs)

        # Depending on the flagging metric, select the appropriate type of 
        # flagger task (Vector vs. Matrix)
        if metric in ['nmedian','derivative','fieldshape','toomany']:
            flagger = viewflaggers.NewMatrixFlagger
        elif metric in ['birdies', 'edgechans']:
            flagger = viewflaggers.NewVectorFlagger
        
        # Depending on the flagging metric, translate the appropriate input
        # flagging parameters to a more compact list of flagging rules.
        if metric == 'nmedian':
            rules = flagger.make_flag_rules (
              flag_nmedian=True, fnm_hi_limit=inputs.fnm_limit, fnm_lo_limit=0.0)
        elif metric == 'derivative':
            rules = flagger.make_flag_rules (
              flag_maxabs=True, fmax_limit=inputs.fd_max_limit)
        elif metric == 'fieldshape':
            rules = flagger.make_flag_rules (
              flag_maxabs=True, fmax_limit=inputs.ff_max_limit)
        elif metric == 'birdies':
            rules = flagger.make_flag_rules(
              flag_sharps=True, sharps_limit=inputs.fb_sharps_limit)
        elif metric == 'edgechans':
            rules = flagger.make_flag_rules(
              flag_edges=True, edge_limit=inputs.fe_edge_limit)
        elif metric == 'toomany':
            rules = flagger.make_flag_rules(
              flag_tmf1=True, tmf1_axis='Antenna1', tmf1_limit=inputs.tmf1_limit,
              flag_tmef1=True, tmef1_axis='Antenna1', tmef1_limit=inputs.tmef1_limit)

        # Construct the flagger task around the data view task and the
        # flagsetter task.
        flaggerinputs = flagger.Inputs(
          context=inputs.context, output_dir=inputs.output_dir, vis=inputs.vis,
          datatask=datatask, viewtask=viewtask, flagsettertask=flagsettertask, 
          rules=rules, niter=1, prepend='flag {0} - '.format(metric))
        flaggertask = flagger(flaggerinputs)

        # Execute the flagger task.
        flaggerresult = self._executor.execute(flaggertask)
        
        # Import views, flags, and "measurement set or caltable to flag"
        # into final result.
        result.importfrom(flaggerresult)

        # Save a snapshot of the inputs of the datatask.
        result.inputs = datainputs.as_dict()
        result.caltable = inputs.caltable
        result.stage_number = inputs.context.task_counter
        
        # Copy flagging summaries to final result.
        result.summaries = flaggerresult.summaries

        return result


class TsysflagDataInputs(basetask.StandardInputs):
    
    def __init__(self, context, vis=None, caltable=None):
        self._init_properties(vars())


class TsysflagData(basetask.StandardTaskTemplate):
    Inputs = TsysflagDataInputs

    def __init__(self, inputs):
        super(TsysflagData, self).__init__(inputs)
    
    
    def prepare(self):
        result = TsysflagDataResults()
        result.table = self.inputs.caltable
        return result


    def analyse(self, result):
        return result


class TsysflagView(object):

    def __init__(self, context, vis=None, metric=None, refintent=None, 
                 split_by_field=False):
        """
        Creates an TsysflagView instance for specified metric.

        Keyword arguments:
        metric         -- the name of the view metric:
                            'nmedian' gives an image where each pixel is the
                              tsys median for that antenna/scan.
                            'derivative' gives an image where each pixel is
                              the MAD of the channel to channel derivative
                              across the spectrum for that antenna/scan.
                            'fieldshape' gives an image where each pixel is a
                              measure of the departure of the shape for that 
                              antenna/scan from the median over all
                              scans for that antenna in the selected fields
                              in the SpW.
                            'edgechans' gives median Tsys spectra for each
                              spw.
                            'birdies' gives difference spectra between
                              the antenna median spectra and spw median.
        refintent      -- data with this intent will be used to
                          calculate the 'reference' Tsys shape to which
                          other data will be compared, in some views.
        split_by_field -- if True and if the 'metric' supports it, then create 
                          separate flagging views for each field.
        """
        self.context = context
        self.vis = vis
        self.metric = metric
        self.refintent = refintent
        self.split_by_field = split_by_field

        # Set group of intents-of-interest based on metric:
        if self.metric == 'edgechans':
            self.intentgroup = ['ATMOSPHERE','BANDPASS','AMPLITUDE']
        else:
            self.intentgroup = ['ATMOSPHERE']

    
    def __call__(self, data):
        """
        When called, the TsysflagView object calculates flagging views
        for the TsysflagDataResults that are provided, based on the 
        metric that TsysflagView was initialized for.

        data     -- TsysflagDataResults object giving access to the 
                    tsys caltable.
                    
        Returns:
        TsysflagViewResults object containing the flagging view.
        """
        # Initialize result structure
        self.result = TsysflagViewResults()

        if data.table:
            self.calculate_views(data.table)

        return self.result


    def intent_ids(self, intent, ms):
        """
        Get the fieldids associated with the given intents.
        """  
        # translate intents to regex form, i.e. '*PHASE*+*TARGET*' or
        # '*PHASE*,*TARGET*' to '.*PHASE.*|.*TARGET.*'
        re_intent = intent.replace(' ', '')
        re_intent = re_intent.replace('*', '.*')
        re_intent = re_intent.replace('+', '|')
        re_intent = re_intent.replace(',', '|')
        re_intent = re_intent.replace("'", "")
    
        # translate intents to fieldids - have to be careful that
        # PHASE ids have PHASE intent and no other
        ids = []
        if re.search(pattern=re_intent, string='AMPLITUDE'):
            ids += [field.id for field in ms.fields if 'AMPLITUDE'
              in field.intents]
        if re.search(pattern=re_intent, string='BANDPASS'):
            ids += [field.id for field in ms.fields if 'BANDPASS'
              in field.intents]
        if re.search(pattern=re_intent, string='PHASE'):
            ids += [field.id for field in ms.fields if 
              'PHASE' in field.intents and
              'BANDPASS' not in field.intents and
              'AMPLITUDE' not in field.intents]
        if re.search(pattern=re_intent, string='TARGET'):
            ids += [field.id for field in ms.fields if 'TARGET'
              in field.intents]
        if re.search(pattern=re_intent, string='ATMOSPHERE'):
            ids += [field.id for field in ms.fields if 'ATMOSPHERE'
              in field.intents]
    
        return ids
    
    
    def calculate_views(self, table):
        """
        Calculates a flagging view for the specified table, based on 
        metric that TsysflagView was initialized with.
        
        table     -- CalibrationTableData object giving access to the tsys
                     caltable.
        """
        
        # Load the tsystable from file into memory, store vis in result
        tsystable = caltableaccess.CalibrationTableDataFiller.getcal(table)        
        self.result.vis = tsystable.vis

        # Get the MS object from the context
        self.ms = self.context.observing_run.get_ms(name=self.vis)

        # Get the spws from the tsystable.
        tsysspws = set()
        for row in tsystable.rows:
            tsysspws.update([row.get('SPECTRAL_WINDOW_ID')])
       
        # Get ids of fields for intents of interest
        fieldids = {}
        for intent in self.intentgroup:
            fieldids_for_intent = self.intent_ids(intent, self.ms) 
            fieldids[intent] = fieldids_for_intent

        # Compute the flagging view for every spw and every intent
        LOG.info ('Computing flagging metrics for caltable %s ' % (table))
        for tsysspwid in tsysspws:
            for intent in self.intentgroup:
                if self.metric in ['nmedian', 'toomany']:
                    self.calculate_median_spectra_view(tsystable, tsysspwid, intent, fieldids[intent],
                      split_by_field=self.split_by_field)
                elif self.metric == 'derivative':
                    self.calculate_derivative_view(tsystable, tsysspwid, intent, fieldids[intent])
                elif self.metric == 'fieldshape':
                    self.calculate_fieldshape_view(tsystable, tsysspwid, intent, fieldids[intent], self.refintent)
                elif self.metric == 'birdies':
                    self.calculate_antenna_diff_channel_view(tsystable, tsysspwid, intent, fieldids[intent])
                elif self.metric == 'edgechans':
                    self.calculate_median_channel_view(tsystable, tsysspwid, intent, fieldids[intent])   


    def get_tsystable_data(self, tsystable, spwid, fieldids, antenna_names, corr_type, normalise=None):
        """
        Reads in specified Tsys table, selects data for the specified 
        spw and field(s) and returns the Tsys spectra data, timestamps, 
        and number of channels per spectra.
        
        Keyword arguments:
        tsystable      -- Tsys table to read data from.
        spwid          -- Data is selected for this Spectral window id.
        fieldids       -- Data are selected for these field ids.
        antenna_names  -- List of strings containing the names of the antennas,
                          which are used in the SpectrumResult.
        corr_type      -- List containing the names of the polarizations,
                          which are used in the SpectrumResult.
        normalise      -- bool : if set to True, the SpectrumResult objects are normalised.
        
        Returns:
        Tuple containing 2 elements:
          tsysspectra: dictionary of TsysflagspectraResults
          times: set of unique timestamps
        """


        # Initialize a dictionary of Tsys spectra results and corresponding times
        tsysspectra = collections.defaultdict(TsysflagspectraResults)
        times = set()

        if normalise:
            datatype = 'Normalised Tsys'
        else:
            datatype = 'Tsys'

        pols = range(len(corr_type))
        
        # Select rows from tsystable that match the specified spw and fields,
        # store a Tsys spectrum for each polarisation in the tsysspectra results
        # and store the corresponding time.
        for row in tsystable.rows:
            if row.get('SPECTRAL_WINDOW_ID') == spwid and \
              row.get('FIELD_ID') in fieldids:

                for pol in pols:
                    tsysspectrum = commonresultobjects.SpectrumResult(
                      data=row.get('FPARAM')[pol,:,0],
                      flag=row.get('FLAG')[pol,:,0],
                      datatype=datatype, filename=tsystable.name,
                      field_id=row.get('FIELD_ID'),
                      spw=row.get('SPECTRAL_WINDOW_ID'),
                      ant=(row.get('ANTENNA1'),
                      antenna_names[row.get('ANTENNA1')]), units='K',
                      pol=corr_type[pol][0],
                      time=row.get('TIME'), normalise=normalise)

                    tsysspectra[pol].addview(tsysspectrum.description,
                      tsysspectrum)
                    times.update([row.get('TIME')])

        return tsysspectra, times

    
    def calculate_median_spectra_view(self, tsystable, spwid, intent, fieldids, split_by_field=False):
        """
        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. Two data 'views' will be
        created, one for each polarization. Each 'view' is a matrix with
        axes antenna_id v time. Each point in the matrix is the median
        value of the tsys spectrum for that antenna/time. 
        
        Results are added to self.result.

        Keyword arguments:
        tsystable      -- CalibrationTableData object giving access to the tsys
                          caltable.
        spwid          -- view will be calculated using data for this spw id.
        intent         -- view will be calculated using data for this intent.
        fieldids       -- view will be calculated using data for all field_ids in
                          this list.
        split_by_field -- if True, separate views will be calculated for each
                          field_id.
        """

        # Get antenna names, ids
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(self.ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(self.ms, spwid)
        pols = range(len(corr_type))

        # If splitting views by field...
        if split_by_field:
            
            # Create translation of field ID to field name
            field_name_for_id = dict((field.id, field.name) for field in self.ms.fields)
            
            # Create separate flagging views for each field id.
            for fieldid in fieldids:
                
                # Select Tsysspectra and corresponding times for specified spwid and fieldids
                tsysspectra, times = self.get_tsystable_data(tsystable,
                  spwid, [fieldid], antenna_names, corr_type, normalise=True)
                
                # Create separate flagging views for each polarisation
                for pol in pols:
                    
                    # Initialize the median Tsys spectra results, and spectrum stack
                    tsysmedians = TsysflagspectraResults()
                    spectrumstack = None
                    
                    # Create a stack of all Tsys spectra for specified spw, pol, and field id.
                    for description in tsysspectra[pol].descriptions():
                        tsysspectrum = tsysspectra[pol].last(description)
                        if tsysspectrum.pol==corr_type[pol][0]:
                            if spectrumstack is None:
                                spectrumstack = np.vstack((tsysspectrum.data,))
                                flagstack = np.vstack((tsysspectrum.flag,))
                            else:
                                spectrumstack = np.vstack((tsysspectrum.data,
                                  spectrumstack))
                                flagstack = np.vstack((tsysspectrum.flag,
                                  flagstack))
                      
                    # From the stack of all Tsys spectra for the specified spw and pol,
                    # create a median Tsys spectrum and corresponding flagging state,
                    # convert to a SpectrumResult, and store this as a view in tsysmedians:
                    if spectrumstack is not None:
                        
                        # Initialize median spectrum and corresponding flag list
                        # and populate with valid data
                        stackmedian = np.zeros(np.shape(spectrumstack)[1])
                        stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)
                        for j in range(np.shape(spectrumstack)[1]):
                            valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                            if len(valid_data):
                                stackmedian[j] = np.median(valid_data)
                                stackmedianflag[j] = False
         
                        tsysmedian = commonresultobjects.SpectrumResult(
                          data=stackmedian, 
                          datatype='Median Normalised Tsys',
                          filename=tsystable.name, spw=spwid, 
                          pol=corr_type[pol][0], field_id=fieldid,
                          intent=intent)
                        tsysmedians.addview(tsysmedian.description, tsysmedian)
                    
                    # Initialize the data and flagging state for the flagging view,
                    # and get values for the 'times' axis.
                    times = np.sort(list(times))
                    data = np.zeros([antenna_ids[-1]+1, len(times)])
                    flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)
                    
                    # Populate the flagging view based on the flagging metric
                    for description in tsysspectra[pol].descriptions():
                        tsysspectrum = tsysspectra[pol].last(description)
                        metric = tsysspectrum.median
                        metricflag = np.all(tsysspectrum.flag)
                        
                        ant = tsysspectrum.ant
                        caltime = tsysspectrum.time
                        
                        data[ant[0], caltime==times] = metric
                        flag[ant[0], caltime==times] = metricflag
                    
                    # Create axes for flagging view
                    axes = [commonresultobjects.ResultAxis(name='Antenna1',
                      units='id', data=np.arange(antenna_ids[-1]+1)),
                      commonresultobjects.ResultAxis(name='Time', units='',
                      data=times)]
                    
                    # Convert flagging view into an ImageResult
                    viewresult = commonresultobjects.ImageResult(
                      filename=tsystable.name, data=data,
                      flag=flag, axes=axes, datatype='Median Tsys',
                      spw=spwid, intent=intent, pol=corr_type[pol][0],
                      field_id=fieldid, field_name=field_name_for_id[fieldid])
                    
                    # Store the spectra contributing to this view as 'children'
                    viewresult.children['tsysmedians'] = tsysmedians
                    viewresult.children['tsysspectra'] = tsysspectra[pol]
                    
                    # Add the view results to the class result structure
                    self.result.addview(viewresult.description, viewresult)
        
        # If not splitting by field...
        else:
            # Select Tsysspectra and corresponding times for specified spwid and fieldids
            tsysspectra, times = self.get_tsystable_data(tsystable,
              spwid, fieldids, antenna_names, corr_type, normalise=True)
            
            # Create separate flagging views for each polarisation
            for pol in pols:
                
                # Initialize the median Tsys spectra results, and spectrum stack
                tsysmedians = TsysflagspectraResults()
                spectrumstack = None
                
                # Create a stack of all Tsys spectra for specified spw and pol;
                # this will stack together spectra from all "fieldids".
                for description in tsysspectra[pol].descriptions():
                    tsysspectrum = tsysspectra[pol].last(description)
                    if tsysspectrum.pol==corr_type[pol][0]:
                        if spectrumstack is None:
                            spectrumstack = np.vstack((tsysspectrum.data,))
                            flagstack = np.vstack((tsysspectrum.flag,))
                        else:
                            spectrumstack = np.vstack((tsysspectrum.data,
                              spectrumstack))
                            flagstack = np.vstack((tsysspectrum.flag,
                              flagstack))
                
                # From the stack of all Tsys spectra for the specified spw and pol,
                # create a median Tsys spectrum and corresponding flagging state,
                # convert to a SpectrumResult, and store this as a view in tsysmedians:
                if spectrumstack is not None:
                    
                    # Initialize median spectrum and corresponding flag list
                    # and populate with valid data
                    stackmedian = np.zeros(np.shape(spectrumstack)[1])
                    stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)
                    for j in range(np.shape(spectrumstack)[1]):
                        valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                        if len(valid_data):
                            stackmedian[j] = np.median(valid_data)
                            stackmedianflag[j] = False
                    
                    tsysmedian = commonresultobjects.SpectrumResult(
                      data=stackmedian, 
                      datatype='Median Normalised Tsys',
                      filename=tsystable.name, spw=spwid, 
                      pol=corr_type[pol][0],
                      intent=intent)
                    tsysmedians.addview(tsysmedian.description, tsysmedian)
                
                # Initialize the data and flagging state for the flagging view,
                # and get values for the 'times' axis.
                times = np.sort(list(times))
                data = np.zeros([antenna_ids[-1]+1, len(times)])
                flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)
                
                # Populate the flagging view based on the flagging metric
                for description in tsysspectra[pol].descriptions():
                    tsysspectrum = tsysspectra[pol].last(description)
                    metric = tsysspectrum.median
                    metricflag = np.all(tsysspectrum.flag)
                    
                    ant = tsysspectrum.ant
                    caltime = tsysspectrum.time
                    
                    data[ant[0], caltime==times] = metric
                    flag[ant[0], caltime==times] = metricflag
                
                # Create axes for flagging view
                axes = [commonresultobjects.ResultAxis(name='Antenna1',
                  units='id', data=np.arange(antenna_ids[-1]+1)),
                  commonresultobjects.ResultAxis(name='Time', units='',
                  data=times)]
                
                # Convert flagging view into an ImageResult
                viewresult = commonresultobjects.ImageResult(
                  filename=tsystable.name, data=data,
                  flag=flag, axes=axes, datatype='Median Tsys',
                  spw=spwid, intent=intent, pol=corr_type[pol][0])
                
                # Store the spectra contributing to this view as 'children'
                viewresult.children['tsysmedians'] = tsysmedians
                viewresult.children['tsysspectra'] = tsysspectra[pol]
                
                # Add the view results to the class result structure
                self.result.addview(viewresult.description, viewresult)
            

    def calculate_derivative_view(self, tsystable, spwid, intent, fieldids):
        """
        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. Two data 'views' will be
        created, one for each polarization. Each 'view' is a matrix with
        axes antenna_id v time. Each point in the matrix is the median
        absolute deviation (MAD) of the channel to channel derivative
        across the Tsys spectrum for that antenna/time.

        Results are added to self.result.

        Keyword arguments:
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        intent    -- view will be calculated using data for this intent.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.
        """

        # Get antenna names, ids
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(self.ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(self.ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra and corresponding times for specified spwid and fieldids
        tsysspectra, times = self.get_tsystable_data(tsystable,
          spwid, fieldids, antenna_names, corr_type, normalise=True)

        # Create separate flagging views for each polarisation
        for pol in pols:
            
            # Initialize the median Tsys spectra results, and spectrum stack
            tsysmedians = TsysflagspectraResults()
            spectrumstack = None

            # Create a stack of all Tsys spectra for specified spw and pol;
            # this will stack together spectra from all "fieldids".
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if tsysspectrum.pol==corr_type[pol][0]:
                    if spectrumstack is None:
                        spectrumstack = np.vstack((tsysspectrum.data,))
                        flagstack = np.vstack((tsysspectrum.flag,))
                    else:
                        spectrumstack = np.vstack((tsysspectrum.data,
                          spectrumstack))
                        flagstack = np.vstack((tsysspectrum.flag,
                          flagstack))

            # From the stack of all Tsys spectra for the specified spw and pol,
            # create a median Tsys spectrum and corresponding flagging state,
            # convert to a SpectrumResult, and store this as a view in tsysmedians:
            if spectrumstack is not None:

                # Initialize median spectrum and corresponding flag list
                # and populate with valid data
                stackmedian = np.zeros(np.shape(spectrumstack)[1])
                stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)
                for j in range(np.shape(spectrumstack)[1]):
                    valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                    if len(valid_data):
                        stackmedian[j] = np.median(valid_data)
                        stackmedianflag[j] = False

                tsysmedian = commonresultobjects.SpectrumResult(
                  data=stackmedian, 
                  datatype='Median Normalised Tsys',
                  filename=tsystable.name, spw=spwid, pol=corr_type[pol][0],
                  intent=intent)
                tsysmedians.addview(tsysmedian.description, tsysmedian)

            # Initialize the data and flagging state for the flagging view,
            # and get values for the 'times' axis.
            times = np.sort(list(times))
            data = np.zeros([antenna_ids[-1]+1, len(times)])
            flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)

            # Populate the flagging view based on the flagging metric:
            # Get MAD of channel by channel derivative of Tsys
            # for each antenna, time/scan.
            for description in tsysspectra[pol].descriptions():

                tsysspectrum = tsysspectra[pol].last(description)

                tsysdata = tsysspectrum.data
                tsysflag = tsysspectrum.flag

                deriv = tsysdata[1:] - tsysdata[:-1]
                deriv_flag = np.logical_or(tsysflag[1:], tsysflag[:-1])
                
                valid = deriv[np.logical_not(deriv_flag)]
                if len(valid):
                    metric = np.median(np.abs(valid - np.median(valid))) * 100
                    
                    ant = tsysspectrum.ant
                    caltime = tsysspectrum.time
                    data[ant[0], caltime==times] = metric
                    flag[ant[0], caltime==times] = 0
            
            # Create axes for flagging view
            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times)]
            
            # Convert flagging view into an ImageResult
            viewresult = commonresultobjects.ImageResult(
              filename=tsystable.name, data=data,
              flag=flag, axes=axes, datatype='100 * MAD of normalised derivative',
              spw=spwid, intent=intent, pol=corr_type[pol][0])

            # Store the spectra contributing to this view as 'children'
            viewresult.children['tsysmedians'] = tsysmedians
            viewresult.children['tsysspectra'] = tsysspectra[pol]

            # Add the view results to the class result structure
            self.result.addview(viewresult.description, viewresult)


    def calculate_fieldshape_view(self, tsystable, spwid, intent, fieldids, refintent):
        """
        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. Two data 'views' will be
        created, one for each polarization. Each 'view' is a matrix with
        axes antenna_id v time. Each point in the matrix is a measure of
        the difference of the tsys spectrum there from the median of all
        the tsys spectra for that antenna/spw in the 'reference' fields
        (those with refintent). The shape metric is calculated
        using the formula:

        metric = 100 * mean(abs(normalised tsys - reference normalised tsys))

        where a 'normalised' array is defined as:

                         array
        normalised = -------------
                     median(array)

        Results are added to self.result.

        Keyword arguments:
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        intent    -- view will be calculated using data for this intent.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.
        refintent -- data with this intent will be used to
                     calculate the 'reference' Tsys shape to which
                     other data will be compared.
        """

        # Get antenna names, ids
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(self.ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(self.ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra and corresponding times for specified spwid and fieldids
        tsysspectra, times = self.get_tsystable_data(tsystable,
          spwid, fieldids, antenna_names, corr_type, normalise=True)

        # Get ids of fields for reference spectra
        referencefieldids = self.intent_ids(refintent, self.ms)

        # Create separate flagging views for each polarisation
        for pol in pols:
            
            # Initialize results object to hold the reference Tsys spectra
            tsysrefs = TsysflagspectraResults()

            # For each antenna, create a "reference" Tsys spectrum, 
            # by creating a stack of Tsys spectra that belong to the 
            # reference field ids, and calculating the median Tsys
            # spectrum for this spectrum stack:

            for antenna_id in antenna_ids:
                
                # Create a single stack of spectra, and corresponding flags, 
                # for all fields that are listed as reference field.
                spectrumstack = None
                for description in tsysspectra[pol].descriptions():
                    tsysspectrum = tsysspectra[pol].last(description)
                    if tsysspectrum.pol==corr_type[pol][0] and \
                      tsysspectrum.ant[0]==antenna_id and \
                      tsysspectrum.field_id in referencefieldids:
                        if spectrumstack is None:
                            spectrumstack = tsysspectrum.data
                            flagstack = tsysspectrum.flag
                        else:
                            spectrumstack = np.vstack((tsysspectrum.data,
                              spectrumstack))
                            flagstack = np.vstack((tsysspectrum.flag,
                              flagstack))

                if spectrumstack is not None:
                    
                    # From the stack of Tsys spectra, derive a median Tsys "reference"
                    # spectrum and corresponding flag list.
                    
                    # Ensure that the spectrum stack is 2D
                    if np.ndim(spectrumstack) == 1:
                        # need to reshape to 2d array
                        dim = np.shape(spectrumstack)[0]
                        spectrumstack = np.reshape(spectrumstack, (1,dim))
                        flagstack = np.reshape(flagstack, (1,dim))

                    # Initialize median spectrum and corresponding flag list
                    stackmedian = np.zeros(np.shape(spectrumstack)[1])
                    stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)

                    # Calculate median Tsys spectrum from spectrum stack
                    for j in range(np.shape(spectrumstack)[1]):
                        valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                        if len(valid_data):
                            stackmedian[j] = np.median(valid_data)
                            stackmedianflag[j] = False

                    # In the median Tsys reference spectrum, look for channels
                    # that may cover lines, and flag these channels:

                    LOG.debug('looking for atmospheric lines')
                    
                    # Calculate first derivative and propagate flags
                    diff = abs(stackmedian[1:] - stackmedian[:-1])
                    diff_flag = np.logical_or(stackmedianflag[1:], stackmedianflag[:-1])

                    # Determine where first derivative is larger 0.05
                    # and not flagged, and create a new list of flags 
                    # that include these channels
                    newflag = (diff>0.05) & np.logical_not(diff_flag)
                    flag_chan = np.zeros([len(newflag)+1], np.bool)
                    flag_chan[:-1] = newflag
                    flag_chan[1:] = np.logical_or(flag_chan[1:], newflag)
                    channels_flagged = np.arange(len(newflag)+1)[flag_chan]

                    # Flag lines in the median Tsys reference spectrum
                    if len(flag_chan) > 0:
                        LOG.debug('possible lines flagged in channels: %s', 
                          channels_flagged)
                        stackmedianflag[flag_chan] = True
           
                    # Create a SpectrumResult from the median Tsys reference
                    # spectrum, and add it as a view to tsysrefs
                    tsysref = commonresultobjects.SpectrumResult(
                      data=stackmedian,
                      flag=stackmedianflag,  
                      datatype='Median Normalised Tsys',
                      filename=tsystable.name, spw=spwid,
                      pol=corr_type[pol][0],
                      ant=(antenna_id, antenna_names[antenna_id]),
                      intent=intent)
                    tsysrefs.addview(tsysref.description, tsysref)

            # Initialize the data and flagging state for the flagging view,
            # and get values for the 'times' axis.
            times = np.sort(list(times))
            data = np.zeros([antenna_ids[-1]+1, len(times)])
            flag = np.ones([antenna_ids[-1]+1, len(times)], np.bool)

            # Populate the flagging view based on the flagging metric
            for description in tsysspectra[pol].descriptions():

                # Get Tsys spectrum
                tsysspectrum = tsysspectra[pol].last(description)

                # Get the 'reference' median Tsys spectrum for this antenna
                for ref_description in tsysrefs.descriptions():
                    tsysref = tsysrefs.last(ref_description)
                    if tsysref.ant[0] != tsysspectrum.ant[0]:
                        continue

                    # Calculate the metric
                    # (100 * mean absolute deviation from reference)
                    diff = abs(tsysspectrum.data - tsysref.data)
                    diff_flag = (tsysspectrum.flag | tsysref.flag)
                    if not np.all(diff_flag):
                        metric = 100.0 * np.mean(diff[diff_flag==0])
                        metricflag = 0
                    else:
                        metric = 0.0
                        metricflag = 1

                    ant = tsysspectrum.ant
                    caltime = tsysspectrum.time
                    data[ant[0], caltime==times] = metric
                    flag[ant[0], caltime==times] = metricflag

            # Create axes for flagging view
            axes = [commonresultobjects.ResultAxis(name='Antenna1',
              units='id', data=np.arange(antenna_ids[-1]+1)),
              commonresultobjects.ResultAxis(name='Time', units='',
              data=times)]

            # Convert flagging view into an ImageResult
            viewresult = commonresultobjects.ImageResult(
              filename=tsystable.name, data=data,
              flag=flag, axes=axes, datatype='Shape Metric * 100',
              spw=spwid, intent=intent, pol=corr_type[pol][0])

            # Store the spectra contributing to this view as 'children'
            viewresult.children['tsysmedians'] = tsysrefs
            viewresult.children['tsysspectra'] = tsysspectra[pol]

            # Add the view results to the class result structure
            self.result.addview(viewresult.description, viewresult)


    def calculate_median_channel_view(self, tsystable, spwid, intent, fieldids):
        """
        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. From all this one data 'view' 
        is created; a 'median' Tsys spectrum where for each channel the 
        value is the median of all the tsys spectra selected.

        Results are added to self.result.

        Keyword arguments:
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        intent    -- view will be calculated using data for this intent.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.
        """

        # Get antenna names
        antenna_names, _ = commonhelpermethods.get_antenna_names(self.ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(self.ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra for specified spwid and fieldids
        tsysspectra, _ = self.get_tsystable_data(tsystable, spwid, fieldids, 
          antenna_names, corr_type, normalise=True)

        # Initialize a stack of all Tsys spectra
        spectrumstack = None

        # Create a stack of all Tsys spectra for the specified spwid; this will
        # stack together spectra from all "fieldids" and both polarisations.
        for pol in pols:
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                if spectrumstack is None:
                    spectrumstack = tsysspectrum.data
                    flagstack = tsysspectrum.flag
                else:
                    spectrumstack = np.vstack((tsysspectrum.data,
                      spectrumstack))
                    flagstack = np.vstack((tsysspectrum.flag,
                      flagstack))

        # From the stack of Tsys spectra, create a median Tsys 
        # spectrum and corresponding flagging state, convert to a
        # SpectrumResult, and store this as a view in the 
        # final class result structure:
        if spectrumstack is not None:

            # Initialize median spectrum and corresponding flag list
            stackmedian = np.zeros(np.shape(spectrumstack)[1])
            stackmedianflag = np.ones(np.shape(spectrumstack)[1], np.bool)
            
            # Calculate median Tsys spectrum from spectrum stack
            for j in range(np.shape(spectrumstack)[1]):
                valid_data = spectrumstack[:,j][np.logical_not(flagstack[:,j])]
                if len(valid_data):
                    stackmedian[j] = np.median(valid_data)
                    stackmedianflag[j] = False

            # Create a view result from the median Tsys spectrum,
            # and store it in the final result.
            viewresult = commonresultobjects.SpectrumResult(
              data=stackmedian, flag=stackmedianflag,
              datatype='Median Normalised Tsys',
              filename=tsystable.name, spw=spwid,
              intent=intent)
            self.result.addview(viewresult.description, viewresult)


    def calculate_antenna_diff_channel_view(self, tsystable, spwid, intent, fieldids):
        """
        Data of the specified spwid, intent and range of fieldids are
        read from the given tsystable object. From all this a series
        of 'views' are created, one for each antenna. Each 'view'
        is the difference spectrum resulting from the subtraction
        of the 'spw median' Tsys spectrum from the 'antenna/spw median'
        spectrum.

        Results are added to self.result.

        Keyword arguments:
        tsystable -- CalibrationTableData object giving access to the tsys
                     caltable.
        spwid     -- view will be calculated using data for this spw id.
        intent    -- view will be calculated using data for this intent.
        fieldids  -- view will be calculated using data for all field_ids in
                     this list.
        """

        # Get antenna names, ids
        antenna_names, antenna_ids = commonhelpermethods.get_antenna_names(self.ms)

        # Get names of polarisations, and create polarisation index 
        corr_type = commonhelpermethods.get_corr_axis(self.ms, spwid)
        pols = range(len(corr_type))

        # Select Tsysspectra for specified spwid and fieldids
        tsysspectra, _ = self.get_tsystable_data(tsystable,
          spwid, fieldids, antenna_names, corr_type, normalise=True)

        # Initialize spectrum stacks for antennas and for spw
        spw_spectrumstack = None
        ant_spectrumstack = {}
        ant_flagstack = {}

        # Create a stack of all Tsys spectra for specified "spwid",
        # as well as separate stacks for each antenna. This will stack
        # together spectra from all "fieldids" and both polarisations.
        for pol in pols:
            for description in tsysspectra[pol].descriptions():
                tsysspectrum = tsysspectra[pol].last(description)
                
                # Update full stack of all Tsys spectra
                if spw_spectrumstack is None:
                    spw_spectrumstack = tsysspectrum.data
                    spw_flagstack = tsysspectrum.flag
                else:
                    spw_spectrumstack = np.vstack((tsysspectrum.data,
                      spw_spectrumstack))
                    spw_flagstack = np.vstack((tsysspectrum.flag,
                      spw_flagstack))
    
                # Update antenna-specific stacks
                for antenna_id in antenna_ids:
                    if tsysspectrum.ant[0] != antenna_id:
                        continue
    
                    if antenna_id not in ant_spectrumstack.keys():
                        ant_spectrumstack[antenna_id] = tsysspectrum.data
                        ant_flagstack[antenna_id] = tsysspectrum.flag
                    else:
                        ant_spectrumstack[antenna_id] = np.vstack((tsysspectrum.data,
                          ant_spectrumstack[antenna_id]))
                        ant_flagstack[antenna_id] = np.vstack((tsysspectrum.flag,
                          ant_flagstack[antenna_id]))

        # From the stack of all Tsys spectra for the specified spw, 
        # create a median Tsys spectrum and corresponding flagging state
        if spw_spectrumstack is not None:

            # Initialize median spectrum and corresponding flag list
            # and populate with valid data
            spw_median = np.zeros(np.shape(spw_spectrumstack)[1])
            spw_medianflag = np.ones(np.shape(spw_spectrumstack)[1], np.bool)
            for j in range(np.shape(spw_spectrumstack)[1]):
                valid_data = spw_spectrumstack[:,j][np.logical_not(spw_flagstack[:,j])]
                if len(valid_data):
                    spw_median[j] = np.median(valid_data)
                    spw_medianflag[j] = False

        # Create separate flagging views for each antenna
        for antenna_id in antenna_ids:

            if ant_spectrumstack[antenna_id] is not None:

                # Initialize the data and flagging state for the flagging view
                ant_median = np.zeros(np.shape(ant_spectrumstack[antenna_id])[1])
                ant_medianflag = np.ones(np.shape(ant_spectrumstack[antenna_id])[1], np.bool)
                
                # Populate the flagging view based on the flagging metric:
                # Calculate diff between median for spw/antenna and median for spw
                for j in range(np.shape(ant_spectrumstack[antenna_id])[1]):
                    valid_data = ant_spectrumstack[antenna_id][:,j][np.logical_not(ant_flagstack[antenna_id][:,j])]
                    if len(valid_data):
                        ant_median[j] = np.median(valid_data)
                        ant_medianflag[j] = False
                ant_median -= spw_median
                ant_medianflag = (ant_medianflag | spw_medianflag)
            
                # Convert flagging view into an SpectrumResult
                viewresult = commonresultobjects.SpectrumResult(
                  data=ant_median, flag=ant_medianflag,
                  datatype='Normalised Tsys Difference',
                  filename=tsystable.name, spw=spwid,
                  intent=intent,
                  ant=(antenna_id, antenna_names[antenna_id]))

                # Add the view result to the class result structure
                self.result.addview(viewresult.description, viewresult)
                