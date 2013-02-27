import pipeline.infrastructure.api as api

class Gaincurve(api.Heuristic):
    def calculate(self, array_name):
        return array_name == 'VLA'
