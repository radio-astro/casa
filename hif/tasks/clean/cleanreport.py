from __future__ import absolute_import

import numpy as np 
import os.path
import sys
import traceback

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.logging as logging
from pipeline.infrastructure.displays.sky import SkyDisplay
import pipeline.infrastructure.taskreport as taskreport

LOG = logging.get_logger('pipeline.tasks.cleanreport')

class CleanReportInputs(object):
    def __init__(self, context, result, task_name):
        self.context = context
        self.result = result
        self.link_name = '%s %s' % (context.stage, task_name)


class CleanReport(taskreport.TaskReport):
    Inputs = CleanReportInputs
    
    def execute(self, dry_run=False):
        inputs = self.inputs

        logger = self.inputs.context.logger
        logger.logHTML('<li>')
        logger.openNode(inputs.link_name, inputs.link_name, True)

        context = inputs.context
        result = inputs.result
       
        try:
            logger.logHTML('<h2>Description</h2>')

            logger.logHTML('<h2>Results</h2>')

            if result.empty():
                description = '''No image results were generated by this 
                 task.'''
            else:
                description = '''The following image results were 
                 generated by this task:<ul>'''
                description += '<li>psf: %s' % os.path.basename(result.psf)

                iterations = result.iterations.keys()
                iterations.sort()
                # output for last iteration only
                iterations = iterations[-1:]
                for iter in iterations:
                    description += '<li>iteration: %s' % iter
                    description += '<ul>'
                    # using try-blocks in case result is not filled correctly
                    # or files don't exist
                    try:
                        description += '<li>image: %s' % os.path.basename(
                         result.iterations[iter]['image'])
                    except:
                        pass
                    try:
                        description += '<li>model: %s' % os.path.basename(
                         result.iterations[iter]['model'])
                    except:
                        pass
                    try:
                        description += '<li>residual: %s' % os.path.basename(
                         result.iterations[iter]['residual'])
                    except:
                        pass
                    try:
                        description += '<li>cleanmask: %s' % os.path.basename(
                         result.iterations[iter]['cleanmask'])
                    except:
                        pass
                    description += '</ul>'
                description += '</ul>'
            logger.logHTML(description)
      
            heading = True 

            # plot() returns the list of Plots it has generated, so we just
            # need to add each one to the HTML logger with logger.addPlot()
            plots = SkyDisplay(context, result.psf).plot()
            if heading:
                logger.logHTML('<h3>Displays</h3>')
                heading = False
            map(logger.addPlot, plots)

            iterations = result.iterations.keys()
            iterations.sort()
            for iter in iterations:
                # image for this iteration
                plots = SkyDisplay(context,
                 result.iterations[iter]['image']).plot()
                map(logger.addPlot, plots)

##                # model for this iteration
##                plots = SkyDisplay(context,
##                 result.iterations[iter]['model']).plot()
##                map(logger.addPlot, plots)

                # residual for this iteration
                plots = SkyDisplay(context,
                 result.iterations[iter]['residual']).plot()
                map(logger.addPlot, plots)

#                # cleanmask for this iteration
#                if result.iterations[iter].has_key('cleanmask'):
#                    plots = SkyDisplay(context,
#                     result.iterations[iter]['cleanmask']).plot()
#                map(logger.addPlot, plots)

        except KeyboardInterrupt:
            LOG.warning('keyboard interrupt in doStage') 
            raise 
        except:
            logger.logHTML('<p>Failed with exception<pre>')
            traceback.print_exc()
            raise
        finally:
            sys.stdout.flush()
            logger.closeNode()
            return result
            
