from __future__ import absolute_import

import collections
import copy 
import os
import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)


class CalibrationTableData(object):
    """CalibrationTableData holds the name of a CASA calibration table 
    and its contents.
    """
    def __init__(self, name):
        self.name = name
        self.vis = None
        self.row = []

    def addrow(self, columns, *coldata):
        self.row.append(CalibrationTableRow(columns, *coldata))

    def setvis(self, vis):
        self.vis = vis

    @property
    def rows(self):
        return copy.deepcopy(self.row)

    def __repr__(self):
        return '<CalibrationTable({name})>'.format(name=self.name)


class CalibrationTableRow(object):
    """CalibrationTableRow represents a row in a CASA calibration table.
    """
    def __init__(self, columns, *coldata):
        self.rowdata = {}
        for i,col in enumerate(columns):
            self.rowdata[col.upper()] = coldata[i]

    def get(self, field):
        upfield = field.upper()
        return self.rowdata[upfield]


class CalibrationTableDataFiller(object):
    """CalibrationTableDataFiller contains methods to fill a 
    CalibrationTableData object.
    """
    @staticmethod
    def getcal(caltable, columns=[]):
        cal = CalibrationTableData(name=caltable)
        if columns==[]:
            columns = CalibrationTableDataFiller._colnames(caltable)
        # Aug 2012, CASA 3.4, there was a problem reading the WEIGHT column.
        # Not being filled. Leave out for now
        if 'WEIGHT' in columns:
            columns.remove('WEIGHT')
        vis = CalibrationTableDataFiller._readvis(caltable)
        cal.setvis(vis)
        for row in CalibrationTableDataFiller._read_table(caltable, columns):
            cal.addrow(columns, *row)
        return cal

    @staticmethod
    def _colnames(caltable):
        with casatools.TableReader(caltable) as table:
            colnames = table.colnames()        
            return colnames

    @staticmethod
    def _readvis(caltable):
        with casatools.TableReader(caltable) as table:
            vis = table.getkeyword('MSName')
            # return the absolute rather than the base filename as the vis may
            # not be visible from the current path, such as when the context 
            # working directory is set. This makes the assumption that the
            # caltable is in the same directory as the measurement set.
            return os.path.join(os.path.dirname(caltable), vis)

    @staticmethod
    def _read_table(caltable, columns):
        """Read the given calibration table.
        """
        LOG.debug('Analysing calibration table %s' % caltable)
        # works for new style calibration tables
        # to do. read in sub-tables ANTENNA, FIELD, HISTORY and SPECRAL_WINDOW
        with casatools.TableReader(caltable) as table:
            coldata = []
            for colname in columns:
                if colname in ['FPARAM', 'CPARAM', 'PARAMERR', 'FLAG', 'SNR']:
                    if table.isvarcol(colname):
                        coldict = table.getvarcol(colname)
                        temp = []
                        for k in range(len(coldict)):
                            temp.append(coldict['r%s' % (k+1)])
                    else:
                        temp = table.getcol(colname)
                        nrows = np.shape(temp)[-1]
                        temp = np.array_split(temp, nrows, -1)
                    coldata.append(temp)
                else:
                    coldata.append(table.getcol(colname))

            return zip(*coldata)

