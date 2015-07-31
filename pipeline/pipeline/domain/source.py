from __future__ import absolute_import
import itertools

import pipeline.infrastructure.casatools as casatools


class Source(object):
    def __init__(self, source_id, name, direction, proper_motion):
        self.id = source_id
        self.name = name
        self.fields = []
        self._direction = direction
        self._proper_motion = proper_motion

    @property
    def dec(self):
        return casatools.quanta.formxxx(self.latitude, format='dms', prec=2)

    @property
    def frame(self):
        return self._direction['refer']

    @property
    def intents(self):
        return set(itertools.chain(*[f.intents for f in self.fields]))

    @property
    def latitude(self):
        return self._direction['m1']

    @property
    def longitude(self):
        return self._direction['m0']

    @property
    def pm_x(self):
        return self.__format_pm('longitude')

    @property
    def pm_y(self):
        return self.__format_pm('latitude')

    @property
    def proper_motion(self):
        qa = casatools.quanta
        return '%.3e %.3e %s' % (qa.getvalue(self.pm_x),
                                 qa.getvalue(self.pm_y),
                                 qa.getunit(self.pm_x))

    @property
    def ra(self):        
        return casatools.quanta.formxxx(self.longitude, format='hms', prec=3)

    def __format_pm(self, axis):
        qa = casatools.quanta
        val = qa.getvalue(self._proper_motion[axis])
        units = qa.getunit(self._proper_motion[axis])
        return '' if val == 0 else '%.3e %s' % (val, units)

    def __repr__(self):
        return ('Source({0}:{1}, pos={2} {3} ({4}), pm={5})'
                ''.format(self.id, self.name, self.ra, self.dec, self.frame, 
                          self.proper_motion))
