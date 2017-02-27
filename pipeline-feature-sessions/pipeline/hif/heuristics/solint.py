import pipeline.infrastructure.api as api

class PhaseUpSolInt(api.Heuristic):
    """PhaseUpSolInt is a heuristic to calculate which phase-up solution
    intervals should be used to evaluate the phase-up bandpass.

    """ 

    def calculate(self, int_time):
        """Get the range of phase-up solution intervals.
        
        :param int_time: the integration time for the bandpass observation
            under consideration
        :type int_time: an integer or float
        :rtype: a list of solution intervals that be given to gaincal
        """
        sol_ints = [int_time * m for m in (1, 2, 5, 10, 20)]
        sol_ints.append('int')
        return sol_ints
