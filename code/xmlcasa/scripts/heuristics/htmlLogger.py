"""Module to build and access the html logging structure."""

# History:
# 16-Jul-2007 jfl First version.
# 16-Aug-2007 jfl Add write_timing method, sort methods in decreasing time 
#                 order.
# 30-Aug-2007 jfl Flux calibrating release.
#  6-Nov-2007 jfl Best bandpass release.
# 17-Dec-2007 jfl Recipe release.
#  7-Jan-2008 jfl Added self._nopen to ensure closed files that were opened.
# 23-Jan-2008 jfl Added process timing.
# 14-Jul-2008 jfl last 4769 release.
# 26-Sep-2008 jfl mosaic release.
# 10-Oct-2008 jfl complex display release.
# 14-Nov-2008 jfl documentation upgrade release.
# 12-Dec-2008 jfl 12-dec release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import os
import time
from numpy import *

# alma modules


def time_key(x):
    """Used by 'sort' to sort a dictionary. 
    """
    return x[1][1]


class HTMLLogger:
    """Class to build and access the html logging structure."""

    def __init__(self, msName):
        """Constructor.

        Keyword arguments:
        msName -- the name of the MeasurementSet containing the data.
        """
#        print 'HTMLLogger constructor called'

        self._msName = msName

# create the directory to hold html files

        self._dirName = os.getcwd()
        if not(os.path.exists('html')):
            os.mkdir('html')

        fullName = os.path.abspath(msName)
        baseName = os.path.basename(msName)

# open root file and start building frame structure

        rootName = os.path.join(self._dirName, 'html/AAAROOT.html')
        self._htmlRoot = open(rootName, 'w')
        print >> self._htmlRoot, ('''
         <html>
          <head>
           <title>Reduction of %s</title>
          </head>
          <frameset cols="250,800">
          <frame src="contents.html">
          <frame src="msSummary.html" name="showframe">
          </frameset>''' % msName)
        self._htmlRoot.flush()

# create the Contents file as the first in the html file list. Write a <ul>
# to it to prepare it for list entries.

        self._htmlFiles = []
        self._nodeList = []

        contentsName = os.path.join(self._dirName, 'html/contents.html')
        self._htmlFiles.append([open(contentsName, 'w'), msName])
        self.logHTML('<ul>')
        self.flushHTML()

# create the Glossary file and fill it.

        self._createGlossary()

# initialise timer list

        data = [1, 0.0, time.time(), 0.0, time.clock()]
        self._timing = [data]
        self._failed = False


    def __del__(self):
        """Destructor.
        """

#        print 'HTMLLogger destructor called'

# close all open nodes except the first

        while len(self._htmlFiles) > 1:
            self._htmlFiles.pop()[0].close()

# tidy up the html and close the files

        self.logHTML('''
        </ul>
        </html>''')
        self._htmlFiles.pop()[0].close()
        self._htmlRoot.close()


    def _createGlossary(self):
        """
        """
        glossaryName = os.path.join(self._dirName, 'html/glossary.html')
        glossary = open(glossaryName, 'w')

        self._glossary = {}

        self._glossary['ANTENNA1'] = True
        print >> glossary, '''
         <a name="ANTENNA1">
         <h3>ANTENNA1</h3>
         This is the name of the column in the main table of a MeasurementSet
         that contains the ID number of the first antenna associated with
         the data in that row.</a>'''

        self._glossary['ANTENNA2'] = True
        print >> glossary, '''
         <a name="ANTENNA2">
         <h3>ANTENNA2</h3>
         This is the name of the column in the main table of a MeasurementSet
         that contains the ID number of the second antenna associated with
         the data in that row. Together with ANTENNA1 it specifies the
         baseline.</a>'''

        self._glossary['chunk'] = True
        print >> glossary, '''
         <a name="chunk">
         <h3>chunk</h3>
         A 'chunk' is the name given to a a series of contiguous timestamps 
         of data taken looking at a particular source. Typically an 
         observation contains a series of chunks looking at the target source
         interspersed with chunks looking at the gain calibrator, with a few 
         single chunks for the bandpass and flux calibrators.
         <p>The algorithm for determining the start and finish of the chunks 
         for a particular source is to select all data for that source, 
         calculate the median difference between adjacent timestamps, then set
         chunk breaks at any interval greater than 1.5 times the median.
         <p>On the image plots with a TIME axis, the boundaries of chunks 
         are indicated by white lines crossing the image, rooted at a time
         value for the start of the chunk. It is important to note in this case
         that the TIME axis is not linear as the time gaps between chunks are 
         removed.
         <p>Lastly, the 'chunks' apparent to the eye on the plotted summary of
         the measurement set may sometimes seem to differ from the chunks 
         apparent in the flagging plots. This happens when chunks are 
         separated by small time gaps that are not visible at the resolution 
         of the measurement set summary.</a>'''

        self._glossary['complex median'] = True
        print >> glossary, '''
         <a name="complex median">
         <h3>complex median</h3>
         The 'complex median' is the median of a sample of complex numbers.
         The real part is the median of the real parts of the numbers in 
         the sample; the imaginary part that of the imaginary parts.</a>'''

        self._glossary['FLAG'] = True
        print >> glossary, '''
         <a name="FLAG">
         <h3>FLAG</h3>
         This column in a MeasurementSet contains a boolean array for each row,
         with the shape of the array matching that of the row data. 
         If the value for a datum in the array is True then that datum is 
         flagged bad.</a>'''

        self._glossary['FLAG_ROW'] = True
        print >> glossary, '''
         <a name="FLAG_ROW">
         <h3>FLAG_ROW</h3>
         This column in a MeasurementSet contains a boolean value for each row
         of data. If the value is True then the data in that row are flagged
         bad.</a>'''

        self._glossary['MAD'] = True
        print >> glossary, '''
         <a name="MAD">
         <h3>MAD</h3>
         The median absolute deviation (MAD) is a robust measure
         of the variability of a sample. The MAD is defined as:
         <pre>
         MAD = median_i (|X_i - median_j(X_j)|)
         </pre>
         That is, starting with the deviations from the data's median, the
         MAD is the median of their absolute values.</a>'''

        self._glossary['MeasurementSet'] = True
        print >> glossary, '''
         <a name="MeasurementSet">
         <h3>MeasurementSet</h3>
         MeasurementSet is the name given to the directory structure holding
         data in the internal format of AIPS++ or CASA. A full description
         is given <a href="http://aips2.nrao.edu/docs/notes/229/229.html">
         here</a>.'''

        self._glossary['TaQL'] = True
        print >> glossary, '''
         <a name="TaQL">
         <h3>TaQL - Table Query Language</h3>
         The Table Query Language is an SQL-like high level language to do 
         operations like selection, sort and update on a table (a 
         MeasurementSet is a collection of 'tables'). A full description
         is given <a href="http://aips2.nrao.edu/docs/notes/199/199.html">
         here</a>.'''

        glossary.close()

# create a file to go to if no glossary entry is available

        noEntryName = os.path.join(self._dirName, 'html/noEntry.html')
        noEntry = open(noEntryName, 'w')

        self._glossary['no entry'] = True
        print >> noEntry, '''
         <a name="no entry">
         <h3>not found</h3>
         There is no entry for this item in the glossary.</a>'''
   
        noEntry.close()


    def appendNode(self, linkName, linkFile):
        """Each 'node' in the html structure is a file. This method appends
        a new link to another file ('node') to a list of them that will be
        written to the current html file.

        Keyword arguments:
        linkName -- the name to be given to the link.
        linkFile -- the file to be linked to.
        """
        self._nodeList.append([linkName, linkFile])
 

    def closeNode(self):
        """Flush output to the current html file ('node') then close it. Pop the
        'file' so that now output will be routed to the file linked to
        the one just closed.
        """
#        print 'closeNode called'

        self.flushNode()
        self._htmlFiles.pop()[0].close()
        if self._failed:
            self.logHTML('<font COLOR="RED">Failed</font>')
            self._failed = False


    def flushHTML(self):
        """Flush pending output to the current html file.
        """
        self._htmlFiles[-1][0].flush()


    def flushNode(self):
        """Flush the 'node' list to the current html file. Write html to
        set up 'up', 'next' and 'previous' buttons in the current html file.
        """
#        print 'flushNode called'
        self.logHTML('<ul>')

# sort the list by link string

        self._nodeList.sort()
        for i,item in enumerate(self._nodeList):
            if item[1] != None:
                fullName = item[1]
                relName = os.path.basename(fullName)
                self.logHTML('<li><a href="%s.html">%s</a></li>' % (relName,
                 item[0]))
                link = open('%s.html' % fullName, 'w')

# make up button

                print>>link, '<table><tr>'
                print>>link, '<td><FORM ACTION="%s" METHOD=GET>' % (
                 os.path.basename(self._htmlFiles[-1][0].name))
                print>>link, '<INPUT TYPE=submit VALUE="up">'
                print>>link, '</FORM></td>'

# previous button - look for first non-empty node, disable button if none
# found 

                for j in range(i-1,-2,-1):
                    if j < 0:
                        print>>link, '<td><FORM ACTION="None" METHOD=GET>'
                        print>>link, '<INPUT TYPE=submit VALUE="previous" DISABLED="disabled">'
                        print>>link, '</FORM></td>'
                        break
                    elif self._nodeList[j][1] == None:
                        continue
                    else:
                        print>>link, '<td><FORM ACTION="%s.html" METHOD=GET>' % (
                         os.path.basename(self._nodeList[j][1]))
                        print>>link, '<INPUT TYPE=submit VALUE="previous">'
                        print>>link, '</FORM></td>'
                        break

# next button

                for j in range(i+1, len(self._nodeList)+1):
                    if j > len(self._nodeList)-1:
                        print>>link, '<td><FORM ACTION="None" METHOD=GET>'
                        print>>link, '<INPUT TYPE=submit VALUE="next" DISABLED="disabled">'
                        print>>link, '</FORM></td>'
                        break
                    elif self._nodeList[j][1] == None:
                        continue
                    else:
                        print>>link, '<td><FORM ACTION="%s.html" METHOD=GET>' % (
                         os.path.basename(self._nodeList[j][1]))
                        print>>link, '<INPUT TYPE=submit VALUE="next">'
                        print>>link, '</FORM></td>'
                        break
                print>>link, '</tr></table>'
                print>>link, '<img src="%s"><br>' % os.path.basename(item[1])
                link.close()
            else:
                self.logHTML('<li>%s</li>' % item[0])
        self.logHTML('</ul>')
        self._nodeList = []


    def logHTML(self, message):
        """Write a string to the current html file.

        Keyword arguments:
        message -- The string to write.
        """
        print >> self._htmlFiles[-1][0], message
        self.flushHTML()


    def openNode(self, link, fileName, doReduction, comment='',
     stringOutput=False):
        """The html structure consists of a set of linked files ('nodes').
        This method constructs a link between the current html file and
        a new one.

        Keyword arguments:
        doReduction -- If True then the link is written to the current 
                       file then the new file is opened and html output
                       switched to it.
                       If False, as would happen if the script was being
                       rerun and accepting earlier results for this stage, 
                       then the link between the current file and the
                       previous results structure is written.  
        """
        relNodeName = '%s.html' % fileName
        relNodeName = relNodeName.replace('/', '')
        relNodeName = relNodeName.replace(' ', '')
        relNodeName = relNodeName.replace('-', '')
        relNodeName = relNodeName.replace(':', '')
        relNodeName = relNodeName.replace('{', '')
        relNodeName = relNodeName.replace('}', '')
        relNodeName = relNodeName.replace(',', '')
        relNodeName = relNodeName.replace("'", "")
        relNodeName = relNodeName.replace('\n', '')
        fullNodeName = os.path.join(self._dirName, 'html', relNodeName) 

        output = ''
        if doReduction:

# put a link to the new node in the current node - switch output to the
# new node
 
# behaviour differs slightly if node is in contents file.

            contents = False
            if len(self._htmlFiles) == 1:
                contents = True

            if contents:
                linkString = '<a href="%s" target="showframe">%s</a> %s' % (
                 relNodeName, link, comment)
            else:
                linkString = '<a href="%s">%s</a> %s' % (relNodeName, link,
                 comment)

            if stringOutput:
                output += linkString
            else:
                self.logHTML(linkString)
            self._htmlFiles.append([open(fullNodeName, 'w'), link])

# make an 'up' button in the new node

            if not contents:
                self.logHTML('<FORM ACTION="%s" METHOD=GET>' % 
                 os.path.basename(self._htmlFiles[-2][0].name))
                self.logHTML('<INPUT TYPE=submit VALUE="up">')

        else:

# possibly a re-run. If the linked-to file exists, link to it.

            contents = False
            if len(self._htmlFiles) == 1:
                contents = True

            if os.path.exists(fullNodeName):
                if contents:
                    self.logHTML('<a href="%s" target="showframe">%s</a> %s'
                     % (relNodeName, link, comment))
                else:
                    self.logHTML('<a href="%s">%s</a> %s' % (relNodeName, link,
                     comment))
            else:
                self.logHTML('%s not done' % (link))

        return output


    def timing_start(self, name):
        """Timing is achieved by bracketing code by calls to timing_start
        and timing_stop. The bracket is given a 'name'. This method starts
        the timing bracket.

        Keyword arguments:
        name -- A name given to the code bracket being timed. Usually this
                will be a method name but need not be. 
        """

#        print 'timing_start', name
        if self._stageTiming.has_key(name) :
             data = self._stageTiming[name]
             data[0] += 1
             data[2] = time.time()
             data[4] = time.clock()
        else :
             data = []
             data.append(1)
             data.append(0.0)
             data.append(time.time())
             data.append(0.0)
             data.append(time.clock())
        self._stageTiming[name] = data


    def timingStageStart(self, stageName):
        """Start timings for a new recipe stage.

        Keyword arguments:
        stageName -- The name of the recipe stage.
        """

#        print 'timingStageStart', stageName
        self._stageTiming = {'name':stageName}
        self.timing_start('total')


    def timing_stop(self, name):
        """Method to close the timing bracket.

        Keyword arguments:
        name -- A name given to the code bracket being timed.
        """
#        print 'timing_stop', name

        if self._stageTiming.has_key(name):
            data = self._stageTiming[name]
            data[1] += (time.time() - data[2])
            data[2] = None
            data[3] += (time.clock() - data[4])
            data[4] = None
            self._stageTiming[name] = data  
        else :
            raise (Exception('bad name: ' + name))


    def timingStageStop(self):
        """Stop timing of recipe stage.
        """

#        print 'timingStageStop'
        self.timing_stop('total')
        self._timing.append(self._stageTiming.copy())


    def glossaryLink(self, item):
        """
        Keyword parameters:
        item  -- The glossary item to be linked.
        """

        if self._glossary.has_key(item):
            link = '<a href="glossary.html#%s">%s</a>' % (item, item)
        else:
            link = '<a href="noEntry.html#%s">%s</a>' % ('no entry', item)
        return link


    def writeTiming(self):
        """Write out the timing results to a 'Timing Results' 'node'
        in the html structure.
        """
        self.logHTML('<li>')
        self.openNode('Timing Results', 'timingResults', True)

# heading

        self.logHTML("""<h4>Timing Results</h4>
         All times are in seconds.""")

# total time

        total = self._timing[0]
        total[1] += (time.time() - total[2])
        total[3] += (time.clock() - total[4])
        self.logHTML("""<h5>Total: %s (wall) %s (process)</h5>""" % (total[1],
         total[3]))

# times for each stage

        for stage in self._timing[1:]:
            stageName = stage.pop('name')
            items = stage.items()
            self.logHTML("""<h5>Stage: %s</h5>""" % stageName)
            self.logHTML("""<table><tr><td>Method</td><td>ncalls</td>
             <td>time used (wall clock)</td><td>time used (process)</td></tr>
             """)

# sort timings in order of descending time taken

            items.sort(key=time_key)
            items.reverse()
            for item in items :
                key = item[0]
                ncalls = item[1][0]
                self.logHTML(
                 '<tr><td>%s</td><td>%s</td><td>%5.2f</td><td>%5.2f</td></tr>' % (
                 key, ncalls, item[1][1], item[1][3]))
            self.logHTML("</table>")
        self.closeNode()
