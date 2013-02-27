import pipeline.infrastructure.api as api

class NullHeuristic(api.Heuristic):
    @staticmethod
    def calculate(*args, **kw):
        return None


class EchoHeuristic(api.Heuristic):
    def __init__(self, val):
        self._val = val
    
    def calculate(self, *args, **kw):
        return self._val

    def __repr__(self):
        return str(self._val)
    
