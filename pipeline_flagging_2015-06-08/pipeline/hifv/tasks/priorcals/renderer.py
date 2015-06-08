'''
Created on 24 Oct 2014

@author: brk
'''
import collections
import os

import pipeline.infrastructure.displays.vla.opacitiesdisplay as opacitiesdisplay
import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.basetemplates as basetemplates
import pipeline.infrastructure.utils as utils

LOG = logging.get_logger(__name__)





class T2_4MDetailspriorcalsRenderer(basetemplates.T2_4MDetailsDefaultRenderer):
    def __init__(self, uri='priorcals.mako', description='Priorcals (gaincurves, opacities, and rq gains)', 
                 always_rerender=False):
        super(T2_4MDetailspriorcalsRenderer, self).__init__(uri=uri,
                description=description, always_rerender=always_rerender)
    
    def get_display_context(self, context, results):
        super_cls = super(T2_4MDetailspriorcalsRenderer, self)
        ctx = super_cls.get_display_context(context, results)
        
        weblog_dir = os.path.join(context.report_dir,
                                  'stage%s' % results.stage_number)
        
        opacity_plots = {}
        spw = {}
        center_frequencies = {}
        opacities = {}
        
        for result in results:
            
            ms = os.path.basename(result.inputs['vis'])
            spw[ms] = result.oc_result[0].spw.split(',')
            center_frequencies[ms] = result.oc_result[0].center_frequencies
            opacities[ms] = result.oc_result[0].opacities
            
            plotter = opacitiesdisplay.opacitiesSummaryChart(context, result)
            plots = plotter.plot()
            opacity_plots[ms] = plots
            
            # generate testdelay plots and JSON file
            #plotter = testgainsdisplay.testgainsPerAntennaChart(context, result, 'amp')
            #plots = plotter.plot() 
            #json_path = plotter.json_filename
            
             # write the html for each MS to disk
            #renderer = VLASubPlotRenderer(context, result, plots, json_path, 'testgains_plots.html', 'amp')
            #with renderer.get_file() as fileobj:
            #    fileobj.write(renderer.render())
            #    testgainsamp_subpages[ms] = renderer.filename
           
        
        ctx.update({'opacity_plots'        : opacity_plots,
                    'spw'                  : spw,
                    'center_frequencies'   : center_frequencies,
                    'opacities'            : opacities,
                    'dirname'              : weblog_dir})
                
        return ctx