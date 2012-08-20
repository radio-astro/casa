import os

def mask_and(a, b):
    assert(len(a)==len(b))
    return [ a[i] & b[i] for i in xrange(len(a)) ]

def mask_or(a, b):
    assert(len(a)==len(b))
    return [ a[i] | b[i] for i in xrange(len(a)) ]

def mask_not(a):
    return [ not i for i in a ]

def _n_bools(n, val):
    return [ val for i in xrange(n) ]

def _is_sequence_or_number(param, ptype=int):
    if isinstance(param,tuple) or isinstance(param,list):
        if len(param) == 0: return True # empty list
        out = True
        for p in param:
            out &= isinstance(p,ptype)
        return out
    elif isinstance(param, ptype):
        return True
    return False

def _to_list(param, ptype=int):
    if isinstance(param, ptype):
        if ptype is str: return param.split()
        else: return [param]
    if _is_sequence_or_number(param, ptype):
        return param
    return None

def unique(x):
    """
    Return the unique values in a list
    Parameters:
        x:      the list to reduce
    Examples:
        x = [1,2,3,3,4]
        print unique(x)
        [1,2,3,4]
    """
    return dict([ (val, 1) for val in x]).keys()

def list_files(path=".",suffix="rpf"):
    """
    Return a list files readable by asap, such as rpf, sdfits, mbf, asap
    Parameters:
        path:     The directory to list (default '.')
        suffix:   The file extension (default rpf)
    Example:
        files = list_files("data/","sdfits")
        print files
        ['data/2001-09-01_0332_P363.sdfits',
        'data/2003-04-04_131152_t0002.sdfits',
        'data/Sgr_86p262_best_SPC.sdfits']
    """
    if not os.path.isdir(path):
        return None
    valid = "ms rpf rpf.1 rpf.2 sdf sdfits mbf asap".split()
    if not suffix in valid:
        return None
    files = [os.path.expanduser(os.path.expandvars(path+"/"+f)) for f in os.listdir(path)]
    return filter(lambda x: x.endswith(suffix),files)

def page(message):
    """Run the input message through a pager. This is only done if
    ``rcParams["verbose"]`` is set.
    """
    verbose = False
    try:
        from asap.parameters import rcParams
        verbose = rcParams['verbose']
    except:
        pass
    if verbose:
        try:
            from IPython.genutils import page as pager
        except ImportError:
            from pydoc import pager
        pager(message)
        return None
    else:
        return message

def toggle_verbose():
    from asap import rcParams
    rcParams['verbose'] = not bool(rcParams['verbose'])
