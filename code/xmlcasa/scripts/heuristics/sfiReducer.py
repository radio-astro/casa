"""Pipeline Heuristics Reduction Script for Single Field Interferometry"""

# History:
#  6-Nov-2007 jfl Best bandpass release.
# 28-Nov-2007 jfl Recipe release.
#  4-Dec-2007 jfl Use __import__ instead of execfile to red recipes.
# 17-Dec-2007 jfl Modify __import__ call to look for recipes in the heuristics
#                 directory.
# 31-jan-2008 fb  Import the class continuumSubClean
#                 to also clean the line and continuum cubes
# 20-Nar-2008 jfl BookKeeper release.
# 10-Apr-2008 jfl F2F release.
#  1-May-2008 jfl recipe parameter moved to constructor.
# 13-May-2008 jfl 13 release.
# 25-Jun-2008 jfl regression release.
# 10-Sep-2008 jfl msCalibrater release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
#  3-Nov-2008 jfl amalgamated stage release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.
#  2-Jun-2009 jfl line and continuum release.

import os
import re
import shutil
import sys
import traceback
import types

import casac

# Import heuristics modules

import bandpassCalibration
import bandpassEdgeFlagger
import bandpassMerit
import baseData 
import baseFlagger
import baselineData
import baseTools
import bestMethod
import bestMethodSelector
import bookKeeper
import buildImage
import casapyTasks
import checkMS
import chunkMedian
import cleanImageV2
import closureError
import coaddedAntennaAmplitude
import continuumSubtractedCleanImage
import complexDisplay
import complexSliceDisplay
import deviations
import dirtyImageV2
import fluxCalibration
import gainCalibration
import htmlLogger
import imageDisplay
import imageFlagger
import medianAndMAD
import medianJump
import mosaicCleanImage
import msCalibrater
import msFlagger
import noDisplay
import noFlagger
import psf
import rangeTrial
import reductionStage
import sequenceFlagger
import skyDisplay
import sliceDisplay
import sourceSpectra
import taqlFlagger
import util

class SFIReducer:
    """Class to reduce Single Field Interferometry data.
    """
    def __init__(self, msName, recipe='vla_recipe', tasks={}):
        """Constructor.

        Keyword arguments:
        msName -- the name of the MeasurementSet to be reduced.
        recipe -- The name of the file containing the recipe. If this is not
                  present in the working directory then the full path must 
                  be given. 
        """

        self._msName = msName
        self._recipeName = recipe
        self._stage = {}

# create the log tool

        self._log = {}
        self._logTool = casac.homefinder.find_home_by_name('logsinkHome')
        self._log['logTool'] = self._logTool.create()
        self._log['logTool'].origin('sfiReducer')
        if len(tasks) == 0:
            # as workaround for CAS-1508 (crash in CASA's casalog component)
            # this is disabled
            #
            self._log['logTool'].setglobal(True)
            #
            pass

# open the html log

        self._html = htmlLogger.HTMLLogger(self._msName)

# get the tools

        self._tools = baseTools.BaseTools()

# and tasks

        self._tasks = tasks

# and the MSCalibrater

        self._msCalibrater = msCalibrater.MSCalibrater(self._tools, self._html,
         self._msName, verbose=False)

# and the MSFlagger

        self._msFlagger = msFlagger.MSFlagger(self._tools, self._html,
         self._msName, self._msCalibrater, verbose=False)

# and the BookKeeper

        self._bookKeeper = bookKeeper.BookKeeper(self._tools, self._html,
         self._msName, self._msFlagger, verbose=False)


    def _doStage(self, stage, doIt):
        """Utility method to execute a recipe stage.

        Keyword arguments:
        stage -- A Python dictionary containing information on the 
                 recipe stage. Format is:

                    {'comment': <comment to be written to HTML output>}

                 or:
 
                    {'name': <stage name>,
                     'colour': <colour associated with data flagged during 
                     this stage>}

        doIt  -- True if the calculations are to be done for this stage.
                 False if the calculations have been done in a previous
                 run of the reducer method and are not to be redone.
                 In this case the links from the top level HTML
                 file point to the results derived previously.
        """

        if stage.has_key('comment'):
            self._html.logHTML('''
             </ul>
             %s
             <ul>''' % stage['comment'])
            return

        stageName = stage['name']
        if not doIt:

# presume have already done this stage in an earlier run - just
# re-establish html links with the results, without instantiating
# any objects 

            print '\n  %s - Establishing HTML links with results of previous run' % (
             stageName)
            sys.stdout.flush()

        self._html.logHTML('<li>')
        self._html.openNode(stageName, stageName, doIt)

# write tag name for this stage to the logfile

        self._log['logTool'].postLocally(
         '<a name="%s">STAGE %s</a>' % (stageName, stageName))

        if doIt:

# store flag state on entry to this stage, create an entry in FLAG_CMD to
# bookmark the start of the stage. Do it here in case a mistake in the recipe
# causes an exception at stage creation.

            self._msFlagger.saveFlagStateToFile(stage['name'])
            self._msFlagger.setFlags(stage, [], {})

# build the ReductionStage object and tell it to do the work

# build a string with the code to construct the ReductionStage object
# specified by the recipe for this stage. Some insertion of 
# parameters into the recipe code is required.

            codeString = stage['stage']

# .first get rid of comment lines and line breaks

            p = re.compile('#.*(\n|$)')
            comments = p.finditer(codeString)
            for c in comments:
                codeString = codeString.replace(c.group(), '')
            codeString = codeString.replace('\n', '')

# .next add a few general parameters to all objects.

            codeString = codeString.replace('(',
             '(htmlLogger=self._html, msName=self._msName,')
            codeString = codeString.replace(',)', ')')

# .add the tools, bookKeeper, msCalibrater and msFlagger objects to all data
#  view objects.

            p = re.compile('view\s*=[^(]*\([^)]*\)')
            viewString = p.search(codeString).group()
            newViewString = viewString.replace('(',
             '(tools=self._tools, bookKeeper=self._bookKeeper, msCalibrater=self._msCalibrater, msFlagger=self._msFlagger, stageName="%s",' %
             stage['name'])
            codeString = codeString.replace(viewString, newViewString)

# .add the tasks to all display objects, if present.

            p = re.compile('display\s*=[^(]*\([^)]*\)')
            search = p.search(codeString)
            if type(search) != types.NoneType:
                viewString = search.group()
                newViewString = viewString.replace("(",
                 "(tasks=self._tasks, ")
                codeString = codeString.replace(viewString, newViewString)

            codeString = """redStage = reductionStage.ReductionStage(self._html, 
             stage, %s)""" % codeString

# use exec to construct redStage then tell it to do the work

            try:
                exec codeString
                redStage.reduce(doIt, self._log)
            except KeyboardInterrupt:
                raise
            except:
                self._html.logHTML('''<p>Failed to construct stage with 
                 exception<pre>''')
                traceback.print_exc()
                traceback.print_exc(file=self._html._htmlFiles[-1][0])
                self._html.logHTML('</pre>')
                self._html.logHTML('''<p>Failed stage code:<pre>''')
                self._html.logHTML(codeString)
                self._html.logHTML('</pre>')
                self._html.closeNode()
                self._html.logHTML('<font color="red">Failed</font>')


    def reduce(self, start=None, flux='', gain='', bandpass=''):
        """Method to reduce the MeasurementSet.

        Keyword arguments:
        start      -- The name of the recipe 'stage' where the reduction is to
                      begin.
        """

# load the recipe.

        #print "Initialize calibrators to gain='%s', flux='%s', bandpass='%s'" % \
        # (gain, flux, bandpass)
        util.gaincal = gain
        util.fluxcal = flux
        util.bandpasscal = bandpass

        recipeModule = __import__('heuristics.' + self._recipeName, fromlist=
         ['heuristics'])
        recipe = recipeModule.recipe

# Find out the version of the logfile to be written (to handle multiple runs).
# The suffix .html is added so that Mac Safari knows to treat the file as HTML.

        log_version = 0
        while os.path.exists('html/casapy.log.%s.text' % log_version):
            log_version += 1
        self._log['logName'] = 'casapy.log.%s.html' % log_version

# write the html tag <pre> to the log file - this stops a browser 
# reformatting it.

        self._log['logTool'].postLocally('<pre>')

# if start is undefined set it to be the first stage in the recipe

        for item in recipe:
            if item.has_key('name'):
                firstStage = item['name']
                break

        if start == None:
            start = firstStage
        else:

# check that start is defined in the recipe

            startStage = None
            for item in recipe:
                if item.has_key('name'):
                    name = item['name']
                    if start == name:
                        startStage = start
                        break
            if startStage == None:
                raise NameError, 'unknown start: %s' % start

# set flagging state of MS to value at start stage - in case a reduction run has
# already been done on the data

        self._msFlagger.resetFlagsToStageEntry(start, firstStage)

# OK, ready to go
# write measurement set summary info

        self._html.timingStageStart('checkMS')
        checkMS.CheckMS(self._tools, self._tasks, self._html, self._msName).\
         writeHtmlSummary()
        self._html.timingStageStop()

# execute the reduction stages in turn.

        started = False
        for stage in recipe:
            if stage.has_key('name'):
                if stage['name'] == start:
                    started = True
                self._html.timingStageStart(stage['name'])
                self._doStage(stage, started)
                self._html.timingStageStop()
            else:
                self._doStage(stage, started)

        self._log['logTool'].postLocally('</pre>')

# move the log file to where the browser will expect find it

        shutil.move('casapy.log', 'html/%s' % self._log['logName'])  

# Tell the html logger to write out the timing info. Leaving this to the 
# __del__ method caused problems.

        self._html.writeTiming()
