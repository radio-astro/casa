from vishead_util import *

def get_ptcs(vis):
    """
    Returns the phase tracking centers of vis as a dict.
    """
    return getput_keyw('get', vis, ['FIELD', 'PHASE_DIR',
                                    valref2direction_strs], '')

def get_ptcs_strs(vis):
    """
    Returns the phase tracking centers of vis as strings.
    """
    return valref2direction_strs(get_ptcs(vis))

