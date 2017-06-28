class Sensitivity(dict):

    '''Sensitivity template definition.'''

    def __init__(self, *args, **kwargs):
         self['array'] = None           # string
         self['field'] = None           # string
         self['spw'] = None             # string
         self['bandwidth'] = None       # string
         self['bwmode'] = None          # string
         self['beam'] = None            # string
         self['cell'] = None            # string
         self['robust'] = None          # string
         self['sensitivity'] = None     # string

         dict.__init__(self, *args, **kwargs)
