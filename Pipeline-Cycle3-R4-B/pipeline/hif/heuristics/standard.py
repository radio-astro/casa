import pipeline.infrastructure.api as api
import pipeline.infrastructure.utils as utils

class Standard(api.Heuristic):
    ephemeris_fields = set(['Venus', 'Mars', 'Jupiter', 'Uranus', 'Neptune',
                            'Pluto', 'Io', 'Europa', 'Ganymede', 'Callisto',
			    'Titan', 'Triton', 'Ceres', 'Pallas', 'Vesta',
			    'Juno', 'Victoria', 'Davida'])

    def calculate(self, field):
        field = set(utils.safe_split(field))
        if field.issubset(self.ephemeris_fields):
            return 'Butler-JPL-Horizons 2012'
        elif field.isdisjoint(self.ephemeris_fields):
            return 'manual'
        else:
            raise Exception, 'not all fields in same standard'
