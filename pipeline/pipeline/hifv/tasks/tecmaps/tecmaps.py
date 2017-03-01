from __future__ import absolute_import
from recipes import tec_maps
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.infrastructure import casa_tasks


LOG = infrastructure.get_logger(__name__)


class TecMapsInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, caltype=None, parameter=[]):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        setattr(self, 'caltype', 'tecim')

    @property
    def caltable(self):
        # The value of caltable is ms-dependent, so test for multiple
        # measurement sets and listify the results if necessary
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('caltable')

        # Get the name.
        if callable(self._caltable):
            casa_args = self._get_partial_task_args()
            return self._caltable(output_dir=self.output_dir,
                                  stage=self.context.stage, **casa_args)
        return self._caltable

    @caltable.setter
    def caltable(self, value):
        if value is None:
            value = caltable_heuristic.TecMapstable()
        self._caltable = value

    @property
    def parameter(self):
        return self._parameter

    @parameter.setter
    def parameter(self, value):
        if value is None:
            value = []
        self._parameter = value

    # Avoids circular dependency on caltable.
    # NOT SURE WHY THIS IS NECESSARY.
    def _get_partial_task_args(self):
        return {'vis': self.vis, 'caltype': self.caltype}

    # Convert to CASA gencal task arguments.
    def to_casa_args(self):

        return {'vis': self.vis,
                'caltable': self.caltable,
                'caltype': self.caltype,
                'parameter': self.parameter}


class TecMapsResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, tec_image=None, tec_rms_image=None):

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []

        super(TecMapsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.tec_image = tec_image
        self.tec_rms_image = tec_rms_image

    def merge_with_context(self, context):
        if not self.final:
            LOG.error('No results to merge')
            return

        for calapp in self.final:
            LOG.debug('Adding calibration to callibrary:\n'
                      '%s\n%s' % (calapp.calto, calapp.calfrom))
            context.callibrary.add(calapp.calto, calapp.calfrom)

    def __repr__(self):
        # Format the GainCurve results.
        s = 'GainCurvesResults:\n'
        for calapplication in self.final:
            s += '\tGaincurves caltable written to {name}\n'.format(
                name=calapplication.gaintable)
        return s


class TecMaps(basetask.StandardTaskTemplate):
    Inputs = TecMapsInputs

    def prepare(self):
        inputs = self.inputs

        tec_image = None
        tec_rms_image = None
        tec_image, tec_rms_image = tec_maps.create(vis=inputs.vis, doplot=True, imname='iono')

        gencal_args = inputs.to_casa_args()
        gencal_args['infile'] = tec_image
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)

        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='tecim', interp='', calwt=False)
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return TecMapsResults(pool=callist, final=callist, tec_image=tec_image, tec_rms_image=tec_rms_image)

    def analyse(self, result):
        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        return result

    def _do_tecmaps(self):

        #tec_image, tec_rms_image = tec_maps.create('vlass3C48.ms')
        tec_maps.create(vis=self.vis, doplot=True, imname='iono')
        # gencal_job = casa_tasks.gencal(**gencal_args)
        gencal_job = casa_tasks.gencal(vis=self.vis, caltable='file.tec', caltype='tecim', infile='iono.IGS_TEC.im')
        self._executor.execute(gencal_job)
