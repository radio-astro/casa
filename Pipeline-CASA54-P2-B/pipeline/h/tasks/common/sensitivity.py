class Sensitivity(dict):

    '''Sensitivity template definition.'''

    def __init__(self, *args, **kwargs):
         self['array'] = None           # string
         self['field'] = None           # string
         self['spw'] = None             # string
         self['bandwidth'] = None       # quanta
         self['bwmode'] = None          # string
         self['beam'] = None            # a beam dictionary
         self['cell'] = None            # quanta array
         self['robust'] = None          # string
         self['uvtaper'] = None         # list
         self['sensitivity'] = None     # quanta
         self['effective_bw'] = None    # quanta

         dict.__init__(self, *args, **kwargs)
