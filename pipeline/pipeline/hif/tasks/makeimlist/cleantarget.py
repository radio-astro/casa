class CleanTarget(dict):

    '''Clean target template definition.'''

    def __init__(self, *args, **kwargs):
         self['field'] = None           # string
         self['intent'] = None          # string
         self['spw'] = None             # string
         self['spwsel_lsrk'] = None     # dictionary
         self['spwsel_topo'] = None     # list
         self['cell'] = None            # string
         self['imsize'] = None          # string / list
         self['phasecenter'] = None     # string
         self['specmode'] = None        # string
         self['gridder'] = None         # string
         self['deconvolver'] = None     # string
         self['imagename'] = None       # string
         self['start'] = None           # string
         self['width'] = None           # string
         self['nbin'] = None            # int
         self['nchan'] = None           # int
         self['stokes'] = None          # string
         self['nterms'] = None          # int
         self['robust'] = None          # float
         self['uvrange'] = None         # string / list
         self['uvtaper'] = None         # list
         self['scales'] = None          # list
         self['niter'] = None           # int
         self['cycleniter'] = None      # int
         self['cyclefactor'] = None     # int
         self['sensitivity'] = None     # string
         self['threshold'] = None       # string
         self['reffreq'] = None         # string
         self['heuristics'] = None      # object
         self['nsigma'] = None          # float
         # vis list only for special setups like CHECK source
         # imaging per EB for ALMA
         self['vis'] = None             # list of strings
         self['is_per_eb'] = None       # boolean

         dict.__init__(self, *args, **kwargs)
