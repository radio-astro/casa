import pipeline.infrastructure.api as api


class Standard(api.Heuristic):
    ephemeris_fields = set(['Mars', 'Jupiter', 'Uranus', 'Neptune', 'Pluto',
                            'Io', 'Europa', 'Ganymede', 'Callisto', 'Titan',
                            'Triton', 'Ceres', 'Pallas', 'Vesta', 'Juno',
                            'Victoria', 'Davida'])

    def calculate(self, field):
        field = set(field.split(','))
        if field.issubset(self.ephemeris_fields):
            return 'Butler-JPL-Horizons 2012'
            #return 'Butler-JPL-Horizons 2010'
        elif field.isdisjoint(self.ephemeris_fields):
            return 'Perley-Butler 2010'
        else:
            raise Exception, 'not all fields in same standard'
