from __future__ import absolute_import

import os
import contextlib

import asap as sd

@contextlib.contextmanager
def _temporary_file_name():
    name = '_heuristics.temporary.table'
    try:
        yield name
    finally:
        os.system('rm -rf %s'%(name))

def _create_dummy_scan(name, datatable, index_list):
    with _temporary_file_name() as temporary_name:
        for index in index_list:
            try:
                s = sd.scantable(name, average=False).get_row(datatable.getcell('ROW',index),insitu=False)
                s.save(temporary_name, overwrite=True)
                param_org = sd.rcParams['scantable.storage'] 
                sd.rcParams['scantable.storage'] = 'memory'
                dummy_scan = sd.scantable(temporary_name, average=False)
                sd.rcParams['scantable.storage'] = param_org
                return dummy_scan
            except:
                pass
