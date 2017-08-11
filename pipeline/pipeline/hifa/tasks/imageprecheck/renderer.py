import collections
from math import sqrt

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.casatools as casatools

LOG = logging.get_logger(__name__)

TR = collections.namedtuple('TR', 'robust uvtaper beam beam_vs_minAR_maxAR cell bandwidth bwmode sensitivity')


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

        minAR_v = cqa.getvalue(cqa.convert(result.minAcceptableAngResolution, 'arcsec'))
        maxAR_v = cqa.getvalue(cqa.convert(result.maxAcceptableAngResolution, 'arcsec'))

        for item in result.sensitivities:
            robust = item['robust']
            uvtaper = item['uvtaper']
            try:
                bmin_v = cqa.getvalue(cqa.convert(item['beam']['minor'], 'arcsec'))
            except:
                bmin_v = 'N/A'
            try:
                bmaj_v = cqa.getvalue(cqa.convert(item['beam']['major'], 'arcsec'))
            except:
                bmaj_v = 'N/A'
            try:
                bpa_v = cqa.getvalue(cqa.convert(item['beam']['positionangle'], 'deg'))
            except:
                bpa_v = 'N/A'
            try:
                beam = '%#.3g x %#.3g arcsec @ %#.3g deg' % (bmaj_v, bmin_v, bpa_v)
            except:
                beam = 'N/A'
            try:
                beamsize = sqrt(bmaj_v * bmin_v)
                if (minAR_v != 0.0) and (maxAR_v != 0.0):
                    beam_vs_minAR_maxAR = '%.1f%% / %.1f%%' % (100. * (beamsize / minAR_v - 1.0), 100. * (beamsize / maxAR_v - 1.0))
                else:
                    beam_vs_minAR_maxAR = 'N/A'
            except:
                beam_vs_minAR_maxAR = 'N/A'
            if cqa.getvalue(item['cell'][0]) != 0.0 and cqa.getvalue(item['cell'][1]) != 0.0:
                cell = '%.2g x %.2g arcsec' % (cqa.getvalue(cqa.convert(item['cell'][0], 'arcsec')), cqa.getvalue(cqa.convert(item['cell'][1], 'arcsec')))
            else:
                cell = 'N/A'
            if cqa.getvalue(item['bandwidth']) != 0.0:
                bandwidth = '%.4g MHz' % (cqa.getvalue(cqa.convert(item['bandwidth'], 'MHz')))
            else:
                bandwidth = 'N/A'
            bwmode = item['bwmode']
            if cqa.getvalue(item['sensitivity']) != 0.0:
                sensitivity = '%.3g Jy/beam' % (cqa.getvalue(cqa.convert(item['sensitivity'], 'Jy/beam')))
            else:
                sensitivity = 'N/A'

            rows.append(TR(robust=robust, uvtaper=uvtaper, beam=beam, beam_vs_minAR_maxAR=beam_vs_minAR_maxAR, cell=cell, bandwidth=bandwidth, bwmode=bwmode, sensitivity=sensitivity))

        return rows
