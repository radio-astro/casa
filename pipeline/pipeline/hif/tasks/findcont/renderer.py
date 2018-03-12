"""
Created on 24 Aug 2015

@author: sjw
"""
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


TR = collections.namedtuple('TR', 'field spw min max frame status spectrum')


class T2_4MDetailsFindContRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self,
                 uri='findcont.mako',
                 description='Detect continuum frequency ranges',
                 always_rerender=False):
        super(T2_4MDetailsFindContRenderer, self).__init__(uri=uri,
                                                           description=description,
                                                           always_rerender=always_rerender)

    def update_mako_context(self, mako_context, pipeline_context, results):
        # as a multi-vis task, there's only one result for FindCont
        result = results[0]

        table_rows = self._get_table_rows(pipeline_context, result)

        mako_context.update({'table_rows': table_rows})

    def _get_table_rows(self, context, result):
        ranges_dict = result.result_cont_ranges

        # structure used to recognise non-detections
        non_detection = (['NONE'], [], [{'range': 'NONE', 'refer': 'LSRK'}], [{'range': 'NONE', 'refer': 'TOPO'}])

        rows = []
        for field in sorted(set(ranges_dict.keys())):
            for spw in map(str, sorted(map(int, set(ranges_dict[field].keys())))):
                plotfile = self._get_plotfile(context, result, field, spw)

                status = ranges_dict[field][spw]['status']

                ranges_for_spw = ranges_dict[field][spw].get('cont_ranges', ['NONE'])

                if ranges_for_spw in non_detection:
                    rows.append(TR(field=field, spw=spw, min='None', max='',
                                   frame='None', status=status, spectrum=plotfile))
                else:
                    raw_ranges_for_spw = [item['range'] for item in ranges_for_spw]
                    refers = numpy.array([item['refer'] for item in ranges_for_spw])
                    if (refers == 'TOPO').all():
                        refer = 'TOPO'
                    elif (refers == 'LSRK').all():
                        refer = 'LSRK'
                    else:
                        refer = 'UNDEFINED'
                    sorted_ranges = sorted(raw_ranges_for_spw, key=operator.itemgetter(0))
                    for (range_min, range_max) in sorted_ranges:
                        # default units for Frequency is GHz, which matches the
                        # units of cont_ranges values
                        min_freq = measures.Frequency(range_min).str_to_precision(5)
                        max_freq = measures.Frequency(range_max).str_to_precision(5)
                        rows.append(TR(field=field, spw=spw, min=min_freq, max=max_freq, frame=refer, status=status,
                                       spectrum=plotfile))

        return utils.merge_td_columns(rows)

    def _get_plotfile(self, context, result, field, spw):
        ranges_dict = result.result_cont_ranges
        raw_plotfile = ranges_dict[field][spw].get('plotfile', None)

        if raw_plotfile in (None, 'none', ''):
            return 'No plot available'

        # move plotfile from working directory to weblog directory if required
        src = os.path.join(context.output_dir, raw_plotfile)
        dst = os.path.join(context.report_dir, 'stage%s' % result.stage_number, raw_plotfile)
        if os.path.exists(src):
            shutil.move(src, dst)

        # create a plot object so we can access (thus generate) the thumbnail
        plot_obj = logger.Plot(dst)

        fullsize_relpath = os.path.relpath(dst, context.report_dir)
        thumbnail_relpath = os.path.relpath(plot_obj.thumbnail, context.report_dir)
        title = 'Detected continuum ranges for %s spw %s' % (field, spw)

        html_args = {
            'fullsize': fullsize_relpath,
            'thumbnail': thumbnail_relpath,
            'title': title,
            'alt': title,
            'rel': 'findcont_plots_%s' % field
        }

        html = ('<a href="{fullsize}"'
                '   title="{title}"'
                '   data-fancybox="{rel}"'
                '   data-caption="{title}">'
                '    <img data-src="{thumbnail}"'
                '         title="{title}"'
                '         alt="{alt}"'
                '         class="lazyload img-responsive">'
                '</a>'.format(**html_args))

        return html
