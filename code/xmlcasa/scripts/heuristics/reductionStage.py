"""Module to build and execute a single stage in the reduction."""

# History:
# 16-Jul-2007 jfl First version.
# 30-Aug-2007 jfl Flux calibrating version.
#  5-Sep-2007 jfl Removed () after class declaration.
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
# 31-Jan-2008 fb  In the "try-except" code also print the exception
#                 to the python console (easier for debugging).
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import sys
import traceback

# alma modules

import noDisplay
import noFlagger


class ReductionStage:
    """Class to execute a single stage in the recipe.
    """

    def __init__(self, htmlLogger, stageDescription, view, operator=None,
     display=None):
        """Constructor.

        Keyword arguments:
        htmlLogger         -- Object through which to write to the html 
                              structure.
        stageDescription   -- Dictionary carrying a description of the
                              reduction stage.                             
        view               -- Object giving the data 'view'.
        operator           -- Object that modifies the data, e.g. flagging.
        display            -- Object to display the data 'view'.
        """

#        print 'ReductionStage constructor called'
        self._stageDescription = stageDescription
        self._view = view
        if operator == None:
            self._dataOperator = noFlagger.NoFlagger(htmlLogger)
        else:
            self._dataOperator = operator
        if display == None:
            self._dataDisplay = noDisplay.NoDisplay(htmlLogger)
        else:
            self._dataDisplay = display
        self._htmlLogger = htmlLogger


    def reduce(self, doReduction, log):
        """Method that tells the object to do the reduction.

        Keyword arguments:
        doReduction -- True if work of reduction is to be done.
                       False if work has been done by a previous run and
                       this method is to merely link those previous results
                       to the html structure for this run.
        log         -- Provides access to the casapy log.
        """
 
        self._htmlLogger.timing_start('ReductionStage.reduce')

# write a description to the screen of the stage being executed

        stageName = self._stageDescription['name']
        print '\nStage: %s' % (stageName)
        temp = self._view.description()
        print '   data view: %s' % temp[0]
        blanks = '                '
        for item in temp[1:]:
            print '%s%s' % (blanks, item)
            blanks = blanks + '  '
        operatorDescription = self._dataOperator.description()
        if operatorDescription != None:
            print '   flagging : %s' % operatorDescription[0]
            for item in operatorDescription[-1]:
                print '       rule: %s' % item

        displayDescription = self._dataDisplay.description()
        if displayDescription != None:
            print '   display  : %s' % displayDescription

        if doReduction:
            stageName = self._stageDescription['name']
            self._htmlLogger.logHTML('<h1>%s</h1>' % stageName)

# do the work. Use a try/except block so that exceptions can be caught and
# a description written to html before reduction proceeds further. Trap
# KeyboardInterrupt explicitly so that ctrl-C can be used to halt the reduction.

            try:

# save flag state on entry
        
                self._view._msFlagger.saveFlagState('StageEntry')
                self._view._msFlagger.saveFlagStateToFile(
                 self._stageDescription['name'])
 
# do the operation

                self._dataOperator.operate(self._stageDescription, self._view)

# display the results

                self._dataDisplay.display(self._stageDescription, self._view,
                 self._dataOperator, log['logName'])

            except KeyboardInterrupt:
                raise
            except:
                self._htmlLogger.logHTML('Failed with exception<pre>')
                traceback.print_exc()
                traceback.print_exc(file=self._htmlLogger._htmlFiles[-1][0])
                self._htmlLogger.logHTML('</pre>')
                self._htmlLogger._failed = True

#            finally:

## close the html node for this Reducer - tried leaving it to the destructor
## but this may not be called before the next constructor runs

#                self._htmlLogger.closeNode()
#                sys.stdout.flush()

        self._htmlLogger.timing_stop('ReductionStage.reduce')
        return
