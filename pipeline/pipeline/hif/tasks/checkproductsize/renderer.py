import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)

TR = collections.namedtuple('TR', 'nbins hm_imsize hm_cell field spw')


class T2_4MDetailsCheckProductSizeRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,
                 uri='checkproductsize.mako',
                 description='Check product size',
                 always_rerender=False):
        super(T2_4MDetailsCheckProductSizeRenderer, self).__init__(
            uri=uri, description=description, always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        # as a multi-vis task, there's only one result for CheckProductSize
        result = results[0]

        table_rows = self._get_table_rows(pipeline_context, result)

        mako_context.update({'table_rows': table_rows})

    @staticmethod
    def _get_table_rows(context, result):
        
        if 'nbins' in result.size_mitigation_parameters:
            nbins = result.size_mitigation_parameters['nbins']
        else:
            nbins = 'default'

        if 'hm_imsize' in result.size_mitigation_parameters:
            hm_imsize = str(result.size_mitigation_parameters['hm_imsize'])
        else:
            hm_imsize = 'default'

        if 'hm_cell' in result.size_mitigation_parameters:
            hm_cell = str(result.size_mitigation_parameters['hm_cell'])
        else:
            hm_cell = 'default'

        if 'field' in result.size_mitigation_parameters:
            fieldnames = str(result.size_mitigation_parameters['field']).split(',')
            if len(fieldnames) > 5:
                for i in range(5, len(fieldnames), 5):
                    fieldnames[i] = '\n%s' % (fieldnames[i])
            field = ','.join(fieldnames)
        else:
            field = 'default'

        if 'spw' in result.size_mitigation_parameters:
            spw = str(result.size_mitigation_parameters['spw'])
        else:
            spw = 'default'

        rows = [TR(nbins=nbins, hm_imsize=hm_imsize, hm_cell=hm_cell, field=field, spw=spw)]

        return utils.merge_td_columns(rows)
