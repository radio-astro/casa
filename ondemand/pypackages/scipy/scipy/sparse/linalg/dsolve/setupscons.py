#!/usr/bin/env python
from os.path import join
import sys

def configuration(parent_package='',top_path=None):
    from numpy.distutils.misc_util import Configuration
    from numpy.distutils.system_info import get_info

    config = Configuration('dsolve',parent_package,top_path,
                           setup_name = 'setupscons.py')

    config.add_sconscript('SConstruct')
    config.add_data_dir('tests')
    config.add_subpackage('umfpack')

    return config

if __name__ == '__main__':
    from numpy.distutils.core import setup
    setup(**configuration(top_path='').todict())
