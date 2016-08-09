from __future__ import absolute_import

import asap as sd
from taskinit import gentools

from ..common import temporary_filename

def createExportTable(table_name):
    table = gentools(['tb'])[0]

    desc = dict()

    desc['Row'] = {
        'comment': 'Row number',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer'
        }

    desc['Ant'] = {
        'comment': 'Antenna IDr',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer'
        }

    desc['FitFunc'] = {
        'comment': 'Baseline Fitting Function',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'string'
        }

    desc['SummaryFlag'] = {
        'comment': 'Summary Flag applied',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean'
        }

    desc['Sections'] = {
        'comment': 'Spectral baseline section coefficients',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer',
        'ndim': 2
        }

    desc['LineMask'] = {
        'comment': 'Line detected region',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'integer',
        'ndim': 2
        }

    desc['SectionCoefficients'] = {
        'comment': 'Spectral baseline section coefficients',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'double',
        'ndim': 2
        }

    desc['Statistics'] = {
        'comment': 'Spectral baseline RMS',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'double',
        'ndim': 1
        }

    desc['StatisticsFlags'] = {
        'comment': 'Statistics Flags by category',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean',
        'ndim': 1
        }

    desc['PermanentFlags'] = {
        'comment': 'Permanent Flags by category',
        'dataManagerGroup': 'StandardStMan',
        'dataManagerType': 'StandardStMan',
        'maxlen': 0,
        'option': 0,
        'valueType': 'boolean',
        'ndim': 1
        }

    table.create(table_name, tabledesc=desc)
    table.close()

# @contextlib.contextmanager
# def temporary_filename(name='_heuristics.temporary.table'):
#     try:
#         yield name
#     finally:
#         os.system('rm -rf %s'%(name))

def create_dummy_scan(name, datatable, index_list):
    param_org = sd.rcParams['scantable.storage']
    with temporary_filename() as temporary_name:
        for index in index_list:
            try:
                sd.rcParams['scantable.storage'] = 'disk'
                s = sd.scantable(name, average=False)
                sel = sd.selector()
                sel.set_rows([datatable.getcell('ROW', index)])
                s.set_selection(sel)
                s.save(temporary_name, overwrite=True)
                s.set_selection()
                sd.rcParams['scantable.storage'] = 'memory'
                dummy_scan = sd.scantable(temporary_name, average=False)
                return dummy_scan
            except:
                pass
            finally:
                sd.rcParams['scantable.storage'] = param_org
    raise RuntimeError('Failed to create dummy scantable')
