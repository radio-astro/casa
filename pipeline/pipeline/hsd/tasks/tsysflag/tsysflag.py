from __future__ import absolute_import

import os

import pipeline.h.tasks.tsysflag.tsysflag as tsysflag
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.sessionutils as sessionutils
from pipeline.infrastructure import task_registry

__all__ = [
    'Tsysflag',
    'TsysflagInputs'
]

LOG = infrastructure.get_logger(__name__)


class TsysflagInputs(tsysflag.TsysflagInputs):
    """
    TsysflagInputs defines the inputs for the Tsysflag pipeline task.
    """
    fnm_byfield = vdp.VisDependentProperty(default=True)

    def __init__(self, context, output_dir=None, vis=None, caltable=None,
                 flag_nmedian=None, fnm_limit=None, fnm_byfield=None,
                 flag_derivative=None, fd_max_limit=None,
                 flag_edgechans=None, fe_edge_limit=None,
                 flag_fieldshape=None, ff_refintent=None, ff_max_limit=None,
                 flag_birdies=None, fb_sharps_limit=None,
                 flag_toomany=None, tmf1_limit=None, tmef1_limit=None,
                 metric_order=None, normalize_tsys=None, filetemplate=None):
        super(TsysflagInputs, self).__init__(
            context=context, output_dir=output_dir, vis=vis, caltable=caltable,
            flag_nmedian=flag_nmedian, fnm_limit=fnm_limit, fnm_byfield=fnm_byfield,
            flag_derivative=flag_derivative, fd_max_limit=fd_max_limit,
            flag_edgechans=flag_edgechans, fe_edge_limit=fe_edge_limit,
            flag_fieldshape=flag_fieldshape, ff_refintent=ff_refintent, ff_max_limit=ff_max_limit,
            flag_birdies=flag_birdies, fb_sharps_limit=fb_sharps_limit,
            flag_toomany=flag_toomany, tmf1_limit=tmf1_limit, tmef1_limit=tmef1_limit,
            metric_order=metric_order, normalize_tsys=normalize_tsys, filetemplate=filetemplate)


#@task_registry.set_equivalent_casa_task('hsd_tsysflag')
#@task_registry.set_casa_commands_comment('The Tsys calibration and spectral window map is computed.')
class Tsysflag(tsysflag.Tsysflag):
    Inputs = TsysflagInputs


### Tier-0 parallelization
class HpcTsysflagInputs(TsysflagInputs):
    # use common implementation for parallel inputs argument
    parallel = sessionutils.parallel_inputs_impl()
    
    def __init__(self, context, output_dir=None, vis=None, caltable=None,
                 flag_nmedian=None, fnm_limit=None, fnm_byfield=None,
                 flag_derivative=None, fd_max_limit=None,
                 flag_edgechans=None, fe_edge_limit=None,
                 flag_fieldshape=None, ff_refintent=None, ff_max_limit=None,
                 flag_birdies=None, fb_sharps_limit=None,
                 flag_toomany=None, tmf1_limit=None, tmef1_limit=None,
                 metric_order=None, normalize_tsys=None, filetemplate=None,
                 parallel=None):
        super(HpcTsysflagInputs, self).__init__(
            context=context, output_dir=output_dir, vis=vis, caltable=caltable,
            flag_nmedian=flag_nmedian, fnm_limit=fnm_limit, fnm_byfield=fnm_byfield,
            flag_derivative=flag_derivative, fd_max_limit=fd_max_limit,
            flag_edgechans=flag_edgechans, fe_edge_limit=fe_edge_limit,
            flag_fieldshape=flag_fieldshape, ff_refintent=ff_refintent, ff_max_limit=ff_max_limit,
            flag_birdies=flag_birdies, fb_sharps_limit=fb_sharps_limit,
            flag_toomany=flag_toomany, tmf1_limit=tmf1_limit, tmef1_limit=tmef1_limit,
            metric_order=metric_order, normalize_tsys=normalize_tsys, filetemplate=filetemplate)
        self.parallel = parallel


@task_registry.set_equivalent_casa_task('hsd_tsysflag')
@task_registry.set_casa_commands_comment('The Tsys calibration and spectral window map is computed.')
class HpcTsysflag(sessionutils.ParallelTemplate):
    Inputs = HpcTsysflagInputs
    Task = Tsysflag

    @basetask.result_finaliser
    def get_result_for_exception(self, vis, exception):
        LOG.error('Error operating target flag for {!s}'.format(os.path.basename(vis)))
        LOG.error('{0}({1})'.format(exception.__class__.__name__, exception.message))
        import traceback
        tb = traceback.format_exc()
        if tb.startswith('None'):
            tb = '{0}({1})'.format(exception.__class__.__name__, exception.message)
        return basetask.FailedTaskResults(self, exception, tb)
