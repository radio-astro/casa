from __future__ import absolute_import
import abc
import sys
import traceback

from . import api
from . import displays
import pipeline.infrastructure.casatools as casatools


class DisplayTemplate(object):
    def __init__(self, context):
        self._context = context
        self._logger = context.logger

    @abc.abstractmethod
    def add_plots(self, results):
        raise NotImplementedError

    def display(self, task, results, stage):
        logger = self._logger
        name = stage['name']
        description = stage['description']

        # write a description to the screen of the stage being executed
        # switched this to a higher level. clean this up in the future
        logger.logHTML('<h1>{name}</h1>'.format(name=name))
            
        self.add_plots(results)
                
        logger.logHTML('<h2>Description</h2>')

        # write recipe description of stage if available
        if description:
            logger.logHTML('<p>{0}</p>'.format(description))

        # brief description
        task.writeGeneralHTMLDescription()

        logger.logHTML('<h2>Results</h2>')
        # flagging logs would go here, but this release does not flag data

        # add links to the CASA log
        logger.logHTML('<h3>Logs</h3>')
        logger.logHTML('The relevant section of the casapy log is ')
        anchor = casatools.log.logfile() + '#stage{0}'.format(name)
        logger.logHTML('<a href="{0}">here</a>.'.format(anchor))

        logger.logHTML('<h2>Algorithms</h2>')
        task.writeDetailedHTMLDescription(topLevel=True)


class CalplotDisplay(DisplayTemplate):
    def add_plots(self, results):
        logger = self._logger
        context = self._context
        
        logger.logHTML('<h3>Displays</h3>')

        # create the inputs for the plot generator, one plot for each caltable
        # in the results
        display_inputs = displays.BandpassDisplay.Inputs(context, 
                                                         results.best,
                                                         results.basis)

        if not display_inputs:
            logger.logHTML('Nothing to display')
        else:        
            # plot() returns the list of Plots it has generated, so we just 
            # need to add each one to the HTML logger with logger.addPlot()
            plots = displays.BandpassDisplay(display_inputs).plot()
            map(logger.addPlot, plots)
            
        logger.flushNode()


class ImageDisplay(DisplayTemplate):
    def add_plots(self, results):
        logger = self._logger
        context = self._context
        
        logger.logHTML('<h3>Displays</h3>')

        display_inputs = displays.ImageDisplay.Inputs(context, results)

        if not display_inputs:
            logger.logHTML('Nothing to display')
        else:        
            # plot() returns the list of Plots it has generated, so we just 
            # need to add each one to the HTML logger with logger.addPlot()
            plots = displays.ImageDisplay(display_inputs).plot()
            map(logger.addPlot, plots)
            
        logger.flushNode()


class NullDisplay(DisplayTemplate):
    """
    Null display class.
    """
    def add_plots(self, results):
        pass


class SliceDisplay(DisplayTemplate):
    def add_plots(self, results):
        logger = self._logger
        context = self._context
        
        logger.logHTML('<h3>Displays</h3>')

        display_inputs = displays.SliceDisplay.Inputs(context, results)

        if not display_inputs:
            logger.logHTML('Nothing to display')
        else:        
            # plot() returns the list of Plots it has generated, so we just 
            # need to add each one to the HTML logger with logger.addPlot()
            plots = displays.SliceDisplay(display_inputs).plot()
            map(logger.addPlot, plots)
            
        logger.flushNode()


class SummaryDisplay(DisplayTemplate):
    def add_plots(self, results):
        logger = self._logger
        context = self._context
        
        logger.logHTML('<h3>Displays</h3>')

        # create the inputs for the summary inputs, which can be shared
        # between all three summary plot types
        display_inputs = displays.PlotAnts.Inputs(context)

        # plot() returns the list of Plots it has generated, so we just 
        # need to add each one to the HTML logger with logger.addPlot()
        plots = displays.PlotAnts(display_inputs).plot()
        plots.extend(displays.PlotXY(display_inputs).plot())
        plots.extend(displays.ScanIntentsGanntChart(display_inputs).plot())
        map(logger.addPlot, plots)
            
        logger.flushNode()



class TaskReportInputs(object):
    def __init__(self, context, task=None, description='', 
     stage_name='', plot=True, results=None):

        # report can either be given the results or a task to be used
        # to derive them
        self.results = results
#        self.task = task
        
#        self.context = context
##        self.bookkeeper = context.bookkeeper
#        self.casa_log = context.casa_log
#        self.logger = context.logger
#        self.output_dir = context.output_dir

#        self.plot = plot

#        self.stage = {}
#        self.stage['number'] = context.stage
#        self.stage['name'] = '%s %s' % (self.stage['number'], stage_name)
#        self.stage['description'] = description

##    @property
##    def display(self):
##        return NullDisplay(self.context)

        
class TaskReport(api.Task):
    Inputs = TaskReportInputs

    def __init__(self, inputs):
        self.inputs = inputs
        
#    @abc.abstractmethod
#    def createGeneralHTMLDescription(self, name):
#        raise NotImplementedError

#    @abc.abstractmethod
#    def createDetailedHTMLDescription(self, name):
#        raise NotImplementedError
        
#    def writeGeneralHTMLDescription(self):
#        logger = self.inputs.logger
#        name = self.inputs.stage['name']

#        description = self.createGeneralHTMLDescription(name)
#        logger.logHTML(description)

#    def writeDetailedHTMLDescription(self, **kw):
#        logger = self.inputs.logger
#        name = self.inputs.stage['name']

#        description = self.createDetailedHTMLDescription(name, **kw)
#        logger.logHTML(description)
    
#    def execute(self, dry_run=True, **parameters):
#        stage = self.inputs.stage
#        name = stage['name']
#        casatools.log.origin(name)
        
#        logger = self.inputs.logger
#        logger.logHTML('<li>')
#        logger.openNode(name, name, True)

        # write tag name for this stage to the logfile
##        stage_num = self.inputs.context.stage
#        casatools.log.postHTML('<a name="{0}">{1}</a>'.format(
#            'stage{0}'.format(name), name))

#        task = self.inputs.task
#        display = self.inputs.display
        
#        try:
#            if task is None:
#                results = self.inputs.results
#            else:
#                results = task.execute(dry_run=dry_run, **parameters)
 
#            display.display(self, results, stage)
#            logger.closeNode()
#            return results
            
#        except KeyboardInterrupt:
#            print 'keyboard interrupt in doStage' 
#            raise 
#        except:
#            logger.logHTML('<p>Failed with exception<pre>')
#            traceback.print_exc()
#            traceback.print_exc(file=logger._htmlFiles[-1][0])
#            logger.logHTML('</pre>')
#            logger.closeNode()
#            logger.logHTML('<font color="red">Failed</font>')
#        finally:
#            sys.stdout.flush()
