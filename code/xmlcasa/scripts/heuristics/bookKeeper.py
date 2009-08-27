"""Module with book-keeping class to keep track of parameter and data 
changes."""

# History:
# 16-Jul-2007 jfl First version.
# 10-Apr-2008 jfl F2F release.
# 13-May-2008 jfl 13 release.
#  2-Jun-2008 jfl 2-jun release.
# 25-Jun-2008 jfl regression release.
# 14-Nov-2008 jfl documentation upgrade release.
# 21-Jan-2009 jfl ut4b release.
#  7-Apr-2009 jfl mosaic release.

# package modules

import fnmatch
from numpy import *
import os.path
import pickle
from types import *
import re
import sys

# alma modules

import util

class BookKeeper:
    """
    """

    def __init__(self, tools, htmlLogger, msName, msFlagger, verbose=False):
        """
        """
#        print 'BookKeeper constructor called'

        htmlLogger.timingStageStart('BookKeeper')
        tools.copy(self)
        self._html = htmlLogger
        self._msName = msName
        self._msFlagger = msFlagger
        self._verbose = verbose
        self._entries = {}
        self._entryID = 0


    def _getFieldsOfType (self, source_type):
        """Return the list of field IDs in msfile that have the specified
        source type.
            
        Keyword arguments:
        source_type -- the type of source; '*BANDPASS*', '*GAIN*', '*FLUX*' or
                       '*SOURCE*'

        """
        result_ids = []

        if type(source_type) == IntType:
            result_ids = [source_type]
        elif type(source_type) == ListType:
            for st in source_type:
                st_ids = self._getFieldsOfType (st)
                result_ids += st_ids
        else:

            if source_type != None:

# verify that the source type is allowed

                if source_type not in ['*BANDPASS*', '*GAIN*', '*FLUX*', '*SOURCE*']:
                    raise NameError, 'bad source_type %s' % source_type

# Open the FIELD sub-Table and from it get the field type.

                self._table.open(self._msName + '/FIELD')
                source_type_col = util.util.get_source_types(self._table)
    
                for field_id in range(len(source_type_col)):
                    column_type = source_type_col[field_id]
                    column_type = column_type.strip()
                    column_type = column_type.upper()
                    pattern = fnmatch.translate(source_type)
                    if re.match(pattern, column_type):
                        result_ids.append (field_id)
                self._table.close()

        return result_ids


    def available(self, objectType, sourceType, furtherInput, outputFiles,
     dependencies=[]):
        """
        """
        if self._verbose:
            print '''
             
BookKeeper.AVAILABLE called type:%s
                            sourceType:%s
                            furtherInput:%s
                            outputFiles:%s
                            dependencies:%s

''' % (
             objectType, sourceType, furtherInput, outputFiles, dependencies)

# find if dependencies have been stored

        dependencyIDs = []
        for dependency in dependencies:
            if self._verbose:
                print 'BookKeeper - %s checking dependency %s' % \
                 (objectType, dependency['objectType'])

            entryID, ignore = self.available(
             objectType=dependency['objectType'],
             sourceType=dependency['sourceType'],
             furtherInput=dependency['furtherInput'],
             outputFiles=dependency['outputFiles'],
             dependencies=dependency['dependencies'])

            dependencyIDs.append(entryID)
            if entryID == None:
                if self._verbose:
                    print 'BookKeeper - %s dependency %s is out of date' %\
                     (objectType, dependency['objectType'])
                break

        dependencyIDs = tuple(dependencyIDs)

# get a form of hash for the inputs, work in field IDs instead of source type
# to cope with situation where 1 field does 2 jobs (e.g. FLUX and BANDPASS).

        field_id_range = self._getFieldsOfType(sourceType)
        inputs = pickle.dumps({'BKfieldIDrange':field_id_range,
         'BKfurtherInput':furtherInput, 'BKoutputFiles':outputFiles})

# what are the flag marks for the sourceType

        flag_marks = []
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        for field_id in field_id_range:
            flag_marks.append(flag_mark_col[field_id])
        flag_marks = tuple(flag_marks)

# has desired data been stored?

        if self._verbose:
            print 'looking for', objectType, inputs, flag_marks, dependencyIDs

        try:
            entryID = self._entries[objectType][inputs][flag_marks]\
             [dependencyIDs]['entryID']
            outputParameters = self._entries[objectType][inputs][flag_marks]\
             [dependencyIDs]['outputParameters']
            outputFiles = self._entries[objectType][inputs][flag_marks]\
             [dependencyIDs]['outputFiles']

# see if output files still exist

            for file in outputFiles:
                if not os.path.exists(file):
                    raise NameError

            if self._verbose:
                print 'BookKeeper - %s data are available' % objectType
        except:
            entryID = None
            outputParameters = None
            if self._verbose:
                print 'BookKeeper - %s data are not available' % objectType
                print sys.exc_info()

        return entryID, outputParameters


    def enter(self, objectType, sourceType=None, furtherInput=None,
     outputFiles=None, outputParameters=None, dependencies=[]):
        """Enter a calculated result into the BookKeeper.

        Keyword arguments:
        objectType       -- The class of the object whose results are being
                            stored. 
        sourceType       -- The type of source the object is associated with;
                            'BANDPASS', 'GAIN', 'FLUX', 'SOURCE' or None.
        furtherInput     -- A dictionary of parameter:value pairs describing
                            further details of the object.
        outputFiles      -- A list of output files associated with the object.
        outputParameters -- A dictionary containing the important 'results'
                            for the object.
        dependencies     -- A list with descriptions of other objects that this
                            object depends upon.
        """
        if self._verbose:
            print '''

BookKeeper.ENTER called type:%s
                        sourceType:%s
                        furtherInput:%s
                        outputFiles:%s
                        dependencies:%s

             ''' % (
             objectType, sourceType, furtherInput, outputFiles, dependencies)

# find where dependencies have been stored

        dependencyIDs = []
        for dependency in dependencies:
            entryID, ignore = self.available(
             objectType=dependency['objectType'],
             sourceType=dependency['sourceType'],
             furtherInput=dependency['furtherInput'],
             outputFiles=dependency['outputFiles'],
             dependencies=dependency['dependencies'])

            dependencyIDs.append(entryID)
            if entryID == None:
                raise NameError, 'dependency not found: %s' % dependency

        dependencyIDs = tuple(dependencyIDs)

# get a form of hash for the inputs, work in field IDs instead of source type
# to cope with situation where 1 field does 2 jobs (e.g. FLUX and BANDPASS).

        field_id_range = self._getFieldsOfType(sourceType)
        inputs = pickle.dumps({'BKfieldIDrange':field_id_range,
         'BKfurtherInput':furtherInput, 'BKoutputFiles':outputFiles})

# what are the flag marks for the sourceType

        flag_marks = []
        ignore,flag_mark_col = self._msFlagger.getFlagMarkInfo()
        for field_id in field_id_range:
            flag_marks.append(flag_mark_col[field_id])
        flag_marks = tuple(flag_marks)

# make the entry

        if not self._entries.has_key(objectType):
            self._entries[objectType] = {}
        if not self._entries[objectType].has_key(inputs):
            self._entries[objectType][inputs] = {}
        if not self._entries[objectType][inputs].has_key(flag_marks):
            self._entries[objectType][inputs][flag_marks] = {}
        if not self._entries[objectType][inputs][flag_marks].has_key(
         dependencyIDs):
            self._entries[objectType][inputs][flag_marks][dependencyIDs] = {}

        self._entryID += 1
        if self._verbose:
            print 'entry ID', self._entryID
        self._entries[objectType][inputs][flag_marks][dependencyIDs]\
         ['entryID'] = self._entryID
        self._entries[objectType][inputs][flag_marks][dependencyIDs]\
         ['outputParameters'] = outputParameters
        self._entries[objectType][inputs][flag_marks][dependencyIDs]\
         ['outputFiles'] = outputFiles

