import os
from taskinit import *
from vishead_util import *

class Direction:
    def __init__(self, input):
        if type(input) == dict:
            # This disassembly and reassembly is unecesssary if input is valid,
            # but it allows input to be checked.  Any extra keys are dropped.
            refcode       = input['refer']
            longitude_str = str(input['m0']['value']) + input['m0']['unit']
            latitude_str  = str(input['m1']['value']) + input['m1']['unit']
        elif type(input) == str:
            refcode, longitude_str, latitude_str = input.split()

        self._dirmeas = me.direction(refcode, longitude_str, latitude_str)
        
        if self._dirmeas == {}:
            raise ValueError, "Could not make a Direction from " + str(input)

##     def __iadd__(self, offset):
##         if hasattr(offset, '_dirmeas'):
##             odm = offset._dirmeas
##         else:
##             # Should throw an exception on error.
##             odm = Direction(offset)._dirmeas
                
        
        
class Ptcs:
    def __init__(self, input):
        """
        Gets the phase tracking centers of input as a pair of dicts.
        """
        self._ptcs     = -1  # Initialize to invalidity
        self._measinfo = -1
        
        if type(input) == str and os.path.isdir(input):
            # input is a MS
            self._ptcs, self._measinfo =  getput_keyw('get', input,
                                                      ['FIELD', 'PHASE_DIR',
                                                       valref2direction_strs],
                                                      '')
            
        elif input.__class__ == self.__class__:
            self._ptcs = input._ptcs
            self._measinfo = input._measinfo
        elif type(input) in (tuple, list):
            # (ptcs, measinfo) list or tuple
            if type(input[0]) == dict and input[0].has_key('r1'):
                self._ptcs = input[0]
            if type(input[1]) == dict and input[1].has_key('MEASINFO') and input[1].has_key('QuantumUnits'):
                self._measinfo = input[1]

        if self._ptcs == -1 or self._measinfo == -1:
            raise ValueError, "Could not make a Ptcs from " + str(input)

    def __getitem__(self, index):
        k = 'r' + str(index + 1)
        lonlatarr = self._ptcs[k].flatten()
        return me.direction(self._measinfo['MEASINFO']['Ref'],
                            {'unit':  self._measinfo['QuantumUnits'][0],
                             'value': lonlatarr[0]},
                            {'unit':  self._measinfo['QuantumUnits'][1],
                             'value': lonlatarr[1]})

    def __len__(self):
        return len(self._ptcs)
		
    def __repr__(self):
        return "Ptcs((" + repr(self._ptcs) + ", " + repr(self._measinfo) + "))"

    def __str__(self):
        return str(self.as_strs())

    def as_strs(self):
        """
        Returns the phase tracking centers of vis as a list of strings ordered
        by field number.
        """
        return valref2direction_strs((self._ptcs, self._measinfo))

##     def __iadd__(self, offset):
##         """
##         Apply an offset.
##         """
##         if isinstance(offset, Ptcs):
##         elif # (fldnum, dirstr or direction measure) list or tuple:
##              # Apply offset to that field (using me.offset?  wrapped in a private
##              # member function so it can be used above?)
