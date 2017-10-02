from __future__ import absolute_import
import os
import shutil
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.tablereader as tablereader
from pipeline.h.tasks.mstransform import mssplit

LOG = infrastructure.get_logger(__name__)


class TransformimagedataResults(basetask.Results):
    def __init__(self, vis, outputvis):
        super(TransformimagedataResults, self).__init__()
        self.vis = vis
        self.outputvis = outputvis
        self.ms = None

    def merge_with_context(self, context):
        # Check for an output vis
        if not self.ms:
            LOG.error('No hif_transformimagedata results to merge')
            return

        target = context.observing_run
        parentms = None
        #if self.vis == self.outputvis:
        # The parent MS has been removed.
        if not os.path.exists(self.vis):
            for index, ms in enumerate(target.get_measurement_sets()):
                #if ms.name == self.outputvis:
                if ms.name == self.vis:
                    parentms = index
                    break

        if self.ms:
            if parentms is not None:
                LOG.info('Replace {} in context'.format(self.ms.name))
                del target.measurement_sets[parentms]
                target.add_measurement_set(self.ms)

            else:
                LOG.info('Adding {} to context'.format(self.ms.name))
                target.add_measurement_set(self.ms)

        # Remove original measurement set from context
        context.observing_run.measurement_sets.pop(0)

        for i in range(0,len(context.clean_list_pending)):
            context.clean_list_pending[i]['heuristics'].observing_run.measurement_sets[0].name = self.outputvis
            newvislist = [self.outputvis]
            context.clean_list_pending[i]['heuristics'].vislist = newvislist

    def __str__(self):
        # Format the MsSplit results.
        s = 'Transformimagedata:\n'
        s += '\tOriginal MS {vis} transformed to {outputvis}\n'.format(
            vis=os.path.basename(self.vis),
            outputvis=os.path.basename(self.outputvis))

        return s

    def __repr__(self):
        return 'Transformimagedata({}, {})'.format(os.path.basename(self.vis), os.path.basename(self.outputvis))


class TransformimagedataInputs(mssplit.MsSplitInputs):
    
    clear_pointing = basetask.property_with_default('clear_pointing', True)
    modify_weights = basetask.property_with_default('modify_weights', False)
    wtmode = basetask.property_with_default('wtmode', '')
    
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 outputvis=None, field=None, intent=None, spw=None,
                 datacolumn=None, chanbin=None, timebin=None, replace=None,
                 clear_pointing=None, modify_weights=None, wtmode=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        
        if clear_pointing is not False:
            clear_pointing = True
        self.clear_pointing = clear_pointing

        if modify_weights is not True:
            modify_weights = False
        self.modify_weights = modify_weights

        self.wtmode = wtmode
        self.context = context

    replace = basetask.property_with_default('replace', False)
    datacolumn = basetask.property_with_default('datacolumn', 'corrected')

    @property
    def outputvis(self):

        output_dir = self.context.output_dir

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('outputvis')

        if not self._outputvis:
            # Need this to be in the working directory
            # vis_root = os.path.splitext(self.vis)[0]
            vis_root = os.path.splitext(os.path.basename(self.vis))[0]
            return output_dir + '/' + vis_root + '_split.ms'

        if type(self._outputvis) is types.ListType:
            idx = self._my_vislist.index(self.vis)
            return output_dir + '/' + os.path.basename(self._outputvis[idx])

        return output_dir + '/' + os.path.basename(self._outputvis)

    @outputvis.setter
    def outputvis(self, value):
        if value in (None, ''):
            value = []
        elif type(value) is types.StringType:
            value = list(value.replace('[', '').replace(']', '').replace("'", "").split(','))
        self._outputvis = value


class Transformimagedata(mssplit.MsSplit):
    Inputs = TransformimagedataInputs

    def prepare(self):

        inputs = self.inputs

        # Test whether or not a split has been requested
        """
        if inputs.field == '' and inputs.spw == '' and inputs.intent == '' and \
            inputs.chanbin == 1 and inputs.timebin == '0s':
            result = TransformimagedataResults(vis=inputs.vis, outputvis=inputs.outputvis)
            LOG.warning('Output MS equals input MS %s' % (os.path.basename(inputs.vis)))
            return
        """

        # Split is required so create the results structure
        result = TransformimagedataResults(vis=inputs.vis, outputvis=inputs.outputvis)

        # Run CASA task
        #    Does this need a try / except block

        visfields = []
        visspws = []
        for imageparam in inputs.context.clean_list_pending:
            visfields.extend(imageparam['field'].split(','))
            visspws.extend(imageparam['spw'].split(','))

        visfields = set(visfields)
        visfields = list(visfields)
        visfields = ','.join(visfields)

        visspws = set(visspws)
        visspws = list(visspws)
        visspws.sort()
        visspws = ','.join(visspws)

        mstransform_args = inputs.to_casa_args()
        mstransform_args['field'] = visfields
        mstransform_args['reindex'] = False
        mstransform_args['spw'] = visspws

        for dictkey in ('clear_pointing', 'modify_weights', 'wtmode'):
            try:
                del mstransform_args[dictkey]
            except KeyError:
                pass

        mstransform_job = casa_tasks.mstransform(**mstransform_args)

        self._executor.execute(mstransform_job)

        return result

    def analyse(self, result):
        # Check for existence of the output vis.
        if not os.path.exists(result.outputvis):
            return result

        inputs = self.inputs

        # There seems to be a rerendering issue with replace. For now just
        # remove the old file.
        if inputs.replace:
            shutil.rmtree(result.vis)
            #shutil.move (result.outputvis, result.vis)
            #result.outputvis = result.vis

        # Import the new MS
        to_import = os.path.abspath(result.outputvis)
        observing_run = tablereader.ObservingRunReader.get_observing_run(to_import)

        # Adopt same session as source measurement set
        for ms in observing_run.measurement_sets:
            LOG.debug('Setting session to %s for %s', self.inputs.ms.session, ms.basename)
            ms.session = self.inputs.ms.session
            ms.is_imaging_ms = True

        # Note there will be only 1 MS in the temporary observing run structure
        result.ms = observing_run.measurement_sets[0]
        
        if inputs.clear_pointing:
            LOG.info('Removing POINTING table from ' + ms.name)
            with casatools.TableReader(ms.name + '/POINTING', nomodify=False) as table:
                rows = table.rownumbers()
                table.removerows(rows)

        if inputs.modify_weights:
            LOG.info('Re-initializing the weights in ' + ms.name)
            if inputs.wtmode:
                task = casa_tasks.initweights(ms.name, wtmode=inputs.wtmode)
            else:
                task = casa_tasks.initweights(ms.name)
            self._executor.execute(task)

        return result



