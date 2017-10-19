import collections
import operator
import os
import shutil

import numpy

import pipeline.domain.measures as measures
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger

LOG = logging.get_logger(__name__)

TR = collections.namedtuple('TR', 'nbins hm_imsize hm_cell field spw')


class T2_4MDetailsCheckProductSizeRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,
                 uri='checkproductsize.mako',
                 description='Check product size',
                 always_rerender=False):
        super(T2_4MDetailsCheckProductSizeRenderer, self).__init__(uri=uri,
                                                           description=description,
                                                           always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        # as a multi-vis task, there's only one result for CheckProductSize
        result = results[0]

        table_rows = self._get_table_rows(pipeline_context, result)

        mako_context.update({'table_rows': table_rows})

    def _get_table_rows(self, context, result):
        
        if result.size_mitigation_parameters.has_key('nbins'):
            nbins = result.size_mitigation_parameters['nbins']
        else:
            nbins = 'default'

        if result.size_mitigation_parameters.has_key('hm_imsize'):
            hm_imsize = str(result.size_mitigation_parameters['hm_imsize'])
        else:
            hm_imsize = 'default'

        if result.size_mitigation_parameters.has_key('hm_cell'):
            hm_cell = str(result.size_mitigation_parameters['hm_cell'])
        else:
            hm_cell = 'default'

        if result.size_mitigation_parameters.has_key('field'):
            field = str(result.size_mitigation_parameters['field'])
        else:
            field = 'default'

        if result.size_mitigation_parameters.has_key('spw'):
            spw = str(result.size_mitigation_parameters['spw'])
        else:
            spw = 'default'

        rows = [TR(nbins=nbins, hm_imsize=hm_imsize, hm_cell=hm_cell, field=field, spw=spw)]

        return utils.merge_td_columns(rows)
