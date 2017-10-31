from __future__ import absolute_import

import pipeline.h.tasks.tsysflag.tsysflag as tsysflag
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask

__all__ = [
    'Tsysflag',
    'TsysflagInputs'
]

LOG = infrastructure.get_logger(__name__)


class TsysflagInputs(tsysflag.TsysflagInputs):
    """
    TsysflagInputs defines the inputs for the Tsysflag pipeline task.
    """
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, caltable=None, 
                 flag_nmedian=None, fnm_limit=None, fnm_byfield=None,
                 flag_derivative=None, fd_max_limit=None,
                 flag_edgechans=None, fe_edge_limit=None,
                 flag_fieldshape=None, ff_refintent=None, ff_max_limit=None,
                 flag_birdies=None, fb_sharps_limit=None,
                 flag_toomany=None, tmf1_limit=None, tmef1_limit=None,
                 metric_order=None, normalize_tsys=None):
        super(TsysflagInputs, self).__init__(
            context=context, output_dir=output_dir, vis=vis, caltable=caltable,
            flag_nmedian=flag_nmedian, fnm_limit=fnm_limit, fnm_byfield=fnm_byfield,
            flag_derivative=flag_derivative, fd_max_limit=fd_max_limit,
            flag_edgechans=flag_edgechans, fe_edge_limit=fe_edge_limit,
            flag_fieldshape=flag_fieldshape, ff_refintent=ff_refintent, ff_max_limit=ff_max_limit,
            flag_birdies=flag_birdies, fb_sharps_limit=fb_sharps_limit,
            flag_toomany=flag_toomany, tmf1_limit=tmf1_limit, tmef1_limit=tmef1_limit,
            metric_order=metric_order, normalize_tsys=normalize_tsys)


class Tsysflag(tsysflag.Tsysflag):
    Inputs = TsysflagInputs
