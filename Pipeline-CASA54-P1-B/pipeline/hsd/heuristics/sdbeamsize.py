import re
import string
from decimal import *
import pipeline.infrastructure.api as api

ANTENNA_LIST = { 'DV[0-6][0-9]': 12.0,
                 'DA[0-6][0-9]': 12.0,
                 'PM0[1-4]': 12.0,
                 'CM[0-1][0-9]': 7.0,
                 'APEX': 12.0,
                 'AP-': 12.0,
                 'NRO': 45.0,
                 'ASTE': 10.0,
                 'MRT': 30.0,
                 'IRAM30m': 30.0,
                 'Effelsberg': 100.0,
                 'GBT': 104.9,
                 'SMT': 10.0,
                 'HHT': 10.0,
                 # from asap/src/STAttr.cpp
                 'MOPRA': 22.0,
                 'PKS': 64.0,
                 'TIDBINBILLA': 70.0,
                 'CEDUNA': 30.0,
                 'HOBART': 26.0 }

class AntennaDiameter(api.Heuristic):
    """
    get antenna diameter in metre from its name.
    """
    def calculate(self, name):
        """
        get antenna diameter in metre from its name.

        name: antenna name
        """
        d = None
        for (key,item) in ANTENNA_LIST.iteritems():
            if re.match(key, name) is not None:
                #print 'matched %s'%(key)
                d = item
                break
        if d is None:
            raise Exception('No data in lookup table: %s'%(name))
        return d
        

class SingleDishBeamSize(api.Heuristic):
    """
    calculate beam size in arcsec.
    """
    def calculate(self, diameter, frequency):
        """
        calculate beam size in arcsec.
        returned value is rounded, like 33.0 for 32.199992 or
        9.8 for 9.783333331.
        NOTE: CURRENTLY the rounding is DISABLED to match manual reduction
        
        diameter: antenna diameter in metre 
        frequency: observing frequency in GHz
        """
        accuratesize = self.__accuratebeamsize(diameter, frequency)
        #print 'accurate size=',accuratesize
        #return self.__rounding(accuratesize)
        return accuratesize

    def __accuratebeamsize(self, diameter, frequency):
        """
        calculate accurate beam size in arcsec.

        diameter: antenna diameter in metre
        frequency: observing frequency in GHz

        Formula:
          beam size = 1.13 * lambda / D
                    = 1.13 * (c / pi * 180.0 * 3600.0) / (nu * D) [arcsec]
                    = 1.13 * 6.188e4 / {(nu [GHz]) * (D [m])} [arcsec]
        """
        factor = 1.13
        accurateSize = factor * 6.188e4 / diameter / frequency
        return accurateSize

    def __rounding(self, v):
        """
        round value

        v: any numerical value
        """
        s = '%e'%(v)
        p = int(s.split('e')[1])
        if p > 0:
            dstr = '1.'
        else:
            dstr = '.'+string.join(['0' for i in xrange(abs(p))],'')+'1'
        #ret = Decimal(s).quantize(Decimal(dstr),rounding=ROUND_HALF_UP)
        ret = Decimal(s).quantize(Decimal(dstr),rounding=ROUND_UP)
        return float(ret)
        
class SingleDishBeamSizeFromName(SingleDishBeamSize):
    """
    calculate beam size in arcsec.
    """
    def calculate(self, name, frequency):
        """
        calculate beam size in arcsec.
        antenna diameter is taken from its name.

        name: antenna name
        frequency: observing frequency in GHz
        """
        h = AntennaDiameter()
        diameter = h(name)
        return super(SingleDishBeamSizeFromName,self).calculate(diameter,frequency)
    
