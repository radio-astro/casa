import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.basetemplates as basetemplates

class SingleDishGenericPlotsRenderer(basetemplates.JsonPlotRenderer):
    def __init__(self, context, result, name, plots, plot_title):
        outfile = filenamer.sanitize('%s-%s.html' % (plot_title.lower(), name))
        new_title = '%s for %s' % (plot_title, name)

        super(SingleDishGenericPlotsRenderer, self).__init__(
                'hsd_generic_plots.mako', context, result, plots, new_title, 
                outfile)
