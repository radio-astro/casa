class CleanTarget(dict):

    '''Clean target template definition.'''

    def __init__(self, *args, **kwargs):
         self['field'] = ''
         self['intent'] = ''
         self['spw'] = ''
         self['spwsel_lsrk'] = ''
         self['spwsel_topo'] = []
         self['cell'] = ''
         self['imsize'] = ''
         self['phasecenter'] = ''
         self['specmode'] = ''
         self['gridder'] = ''
         self['imagename'] = ''
         self['start'] = ''
         self['width'] = ''
         self['nbin'] = -1
         self['nchan'] = -1
         self['uvrange'] = ''
         self['stokes'] = ''
         self['nterms'] = None
         self['heuristics'] = None

         dict.__init__(self, *args, **kwargs)
