from __future__ import absolute_import

import os.path
import types

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.tablereader as tablereader
#import pipeline.hif.heuristics as heuristics
from pipeline.hif.tasks.clean.clean import Clean
from pipeline.hif.tasks.cleanlist.resultobjects import CleanListResult
from pipeline.hif.tasks.clean.resultobjects import CleanResult

from pipeline.infrastructure.displays.sky import SkyDisplay

LOG = logging.get_logger(__name__)


class CleanListInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, 
     vis=None, target_list=None):
        self._init_properties(vars())

    @property
    def target_list(self):
        if self._target_list is None:
            return {}
        return self._target_list

    @target_list.setter
    def target_list(self, value):
        self._target_list = value


class CleanList(basetask.StandardTaskTemplate):
    Inputs = CleanListInputs

    def is_multi_vis_task(self):
        return True

    def prepare(self):
        inputs = self.inputs

        # make sure inputs.vis is a list, even it is one that contains a
        # single measurement set
        if type(inputs.vis) is not types.ListType:
            inputs.vis = [inputs.vis]

        # get the target list; if none specified get from context
        target_list = inputs.target_list
        if target_list == {}:
            target_list = inputs.context.clean_list_pending

        result = CleanListResult()

        for image_target in target_list:
            # format a report of the clean to be run next and output it
            target_str = ''
            for k,v in image_target.iteritems():
                target_str += '%s=%s ' % (k,v)
            LOG.debug('Cleaning %s' % target_str)

            # add inputs values to image_target string to complete the
            # parameter list to the clean task
            full_image_target = dict(image_target)
            full_image_target['output_dir'] = inputs.output_dir
            full_image_target['vis'] = inputs.vis

            # build the task to do the cleaning and execute it
            datainputs = Clean.Inputs(context=inputs.context,
              **full_image_target)
            datatask = Clean(datainputs)

            try:
                clean_result = self._executor.execute(datatask)
                result.add_result(clean_result, image_target,
                  outcome='success')
            except:
                LOG.error('clean failed')
                result.add_result(CleanResult(), image_target,
                  outcome='failure')

        return result

    def analyse(self, result):
        return result

