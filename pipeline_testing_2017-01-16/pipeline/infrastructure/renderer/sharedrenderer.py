'''
Created on 9 Sep 2014

@author: sjw
'''
import os

import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.basetemplates as basetemplates


class PlotGroupRenderer(basetemplates.CommonRenderer):
    def __init__(self, context, result, plot_group, prefix='',
                 uri='plotgroup.mako'):
        super(PlotGroupRenderer, self).__init__(context=context,
                                                result=result,
                                                uri=uri)
        self.plot_group = plot_group

        if prefix != '':
            prefix = '%s-' % prefix
        basename = '%s%s-%s-thumbnails.html' % (plot_group.x_axis,
                                                plot_group.y_axis,
                                                prefix)
        self.basename = filenamer.sanitize(basename)
        self.path = os.path.join(self.dirname, self.basename)

    def update_mako_context(self, mako_context):
        mako_context.update({'plot_group':self.plot_group})
