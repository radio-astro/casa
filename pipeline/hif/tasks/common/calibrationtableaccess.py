from __future__ import absolute_import

import collections
import copy 
import numpy as np

import pipeline.infrastructure.api as api
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.logging as logging

LOG = logging.get_logger(__name__)


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
        return '<CalibrationTable({name}}>'.format(name=self.name)


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
    """CalibrationTableDataFiller contains methods to fill a CalibrationTableData
    object.
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
            return vis

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
                    temp = table.getcol(colname)
                    nrows = np.shape(temp)[-1]
                    temp = np.array_split(temp, nrows, -1)
                    coldata.append(temp)
                else:
                    coldata.append(table.getcol(colname))

            return zip(*coldata)


class CalibrationTableFlagSetterInputs(basetask.StandardInputs):
    """This class handles the setting of flags in calibration files.
    """
    def __init__(self, context, output_dir=None, vis=None):
        self._init_properties(vars())

class CalibrationTableFlagSetterResult(api.Results):
    def merge_with_context(self, context):
        pass

class CalibrationTableFlagSetter(basetask.StandardTaskTemplate):
    Inputs = CalibrationTableFlagSetterInputs

    def is_multi_vis_task(self):
        return True

    def flags_to_set(self, flags):
        self._flags_to_set = flags

    def prepare(self):
        result = CalibrationTableFlagSetterResult()
        tabledata = {}
        setflags = collections.defaultdict(list)
        for f in self._flags_to_set:

            # get caltable results
            if f.filename not in tabledata.keys():
                tabledata[f.filename] = CalibrationTableDataFiller.getcal(f.filename)

            for rownumber,row in enumerate(tabledata[f.filename].rows):
                if row.get('SPECTRAL_WINDOW_ID') != f.spw:
                    continue

                for flagcoord in f.flagcoords:
                    setflag = True
                    for i,axisname in enumerate(f.axisnames):
                        if row.get(axisname) != flagcoord[i]:
                            setflag = False
                            break

                    if setflag:
                        setflags[f.filename].append(
                          (f.cell_index, rownumber))
                        break

        # modify tables
        for caltable in tabledata.keys():
            if caltable in setflags.keys():
                with casatools.TableReader(caltable, nomodify=False) as table:
                    flag = table.getcol('FLAG')
                    for sf in setflags[caltable]:
                        flag[sf[0],:,sf[1]] = True
                    flag = table.putcol('FLAG', flag)

        self._flags_to_set = []

        return result

    def analyse(self, result):
        return result

