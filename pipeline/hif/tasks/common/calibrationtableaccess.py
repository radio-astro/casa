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
            # return the absolute rather than the base filename as the vis may
            # not be visible from the current path, such as when the context 
            # working directory is set. This makes the assumption that the
            # caltable is in the same directory as the measurement set.
            return os.path.join(os.path.dirname(caltable),
                                vis)

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


class CalibrationTableFlagSetterResult(basetask.Results):
    def __init__(self):
        super(CalibrationTableFlagSetterResult, self).__init__()

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

                # do the column values for this row match the flagop?
                # All rows satisfy this condition if f.axisnames is empty.
                rowcoords = []
                for axis in f.axisnames:
                    rowcoords.append(row.get(axis))
                rowcoords=tuple(rowcoords)

                if f.axisnames==[] or rowcoords in f.flagcoords:
                    if f.flagchannels is None:
                        # set all the flags for this rownumber
                        setflags[f.filename].append(
                          (f.cell_index, None, rownumber))
                    else:
                        setflags[f.filename].append(
                          (f.cell_index, f.flagchannels, rownumber))
                

        # modify tables
        for caltable in tabledata.keys():
            if caltable in setflags.keys():
                with casatools.TableReader(caltable, nomodify=False) as table:
                    flag = table.getcol('FLAG')
                    for sf in setflags[caltable]:
                        if sf[0] is None:
                            if sf[1] is None:
                                flag[:,:,sf[2]] = True
                            else:
                                flag[:,sf[1],sf[2]] = True
                        else:
                            if sf[1] is None:
                                flag[sf[0],:,sf[2]] = True
                            else:
                                flag[sf[0],sf[1],sf[2]] = True

                    flag = table.putcol('FLAG', flag)

        self._flags_to_set = []

        return result

    def analyse(self, result):
        return result

