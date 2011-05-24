import sys
from asap import scantable, selector, rcParams, asapplotter
rcParams["verbose"] = False
rcParams["plotter.gui"] = False
from nose.tools import raises

# necessary for python < 2.6 otherwise it is in itertools
def permutations(iterable, r=None):
    # permutations('ABCD', 2) --> AB AC AD BA BC BD CA CB CD DA DB DC
    # permutations(range(3)) --> 012 021 102 120 201 210
    pool = tuple(iterable)
    n = len(pool)
    r = n if r is None else r
    if r > n:
        return
    indices = range(n)
    cycles = range(n, n-r, -1)
    yield tuple(pool[i] for i in indices[:r])
    while n:
        for i in reversed(range(r)):
            cycles[i] -= 1
            if cycles[i] == 0:
                indices[i:] = indices[i+1:] + indices[i:i+1]
                cycles[i] = n - i
            else:
                j = cycles[i]
                indices[i], indices[-j] = indices[-j], indices[i]
                yield tuple(pool[i] for i in indices[:r])
                break
        else:
            return


class TestPlotter(object):

    @classmethod
    def setupClass(self):
        self.plotter = asapplotter(False)
        st = scantable("data/MOPS.rpf", average=True)
        self.st = st.auto_quotient()


    def test_plot(self):
        self.plotter.plot(self.st)

    def test_histogram(self):
        self.plotter.plot(self.st)
        self.plotter.set_histogram(hist=True, linewidth=2)
        self.plotter.set_histogram(hist=False)

    def switch_mode(self, args):
        self.plotter.set_mode(*args)
        assert self.plotter._panelling, args[0]
        assert self.plotter._stacking, args[1]

    # generator for set_mode arguments
    def test_set_mode(self):
        combis = permutations('stbpi', 2)
        for mode in combis:
            yield self.switch_mode, mode

    @raises(TypeError)
    def test_fail_set_mode(self):
        self.plotter.set_mode('x', 'y')

