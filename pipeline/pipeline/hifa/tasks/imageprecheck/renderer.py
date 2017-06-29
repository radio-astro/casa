import collections

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.casatools as casatools

LOG = logging.get_logger(__name__)

TR = collections.namedtuple('TR', 'robust bmin bmin_maxAR cell bandwidth bwmode sensitivity')


class T2_4MDetailsCheckProductSizeRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,
                 uri='imageprecheck.mako',
                 description='Image pre-check',
                 always_rerender=True):
        super(T2_4MDetailsCheckProductSizeRenderer, self).__init__(uri=uri,
                                                           description=description,
                                                           always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        # as a multi-vis task, there's only one result for ImagePreCheck
        result = results[0]

        table_rows = self._get_table_rows(pipeline_context, result)

        mako_context.update({'table_rows': table_rows})

    def _get_table_rows(self, context, result):

        cqa = casatools.quanta

        rows = []

        maxAR_v = cqa.getvalue(cqa.convert(result.maxAcceptableAngResolution, 'arcsec'))

        for item in result.sensitivities:
            robust = item['robust']
            bmin_v = cqa.getvalue(cqa.convert(item['beam']['bmin'], 'arcsec'))
            bmin = '%#.2g arcsec' % (bmin_v)
            if maxAR_v != 0.0:
                bmin_maxAR = '%.1f%%' % (100. * bmin_v / maxAR_v)
            else:
                bmin_maxAR = 'N/A'
            cell = '%.2g arcsec' % (cqa.getvalue(cqa.convert(item['cell'][0], 'arcsec')))
            bandwidth = '%.4g MHz' % (cqa.getvalue(cqa.convert(item['bandwidth'], 'MHz')))
            bwmode = item['bwmode']
            sensitivity = '%.3g Jy/beam' % (cqa.getvalue(cqa.convert(item['sensitivity'], 'Jy/beam')))

            rows.append(TR(robust=robust, bmin=bmin, bmin_maxAR=bmin_maxAR, cell=cell, bandwidth=bandwidth, bwmode=bwmode, sensitivity=sensitivity))

        return utils.merge_td_columns(rows)
