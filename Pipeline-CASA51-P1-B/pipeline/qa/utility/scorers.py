#!/usr/bin/env python

'''Generic scorer classes.'''

import numpy as np
from scipy.special import erf


class erfScorer(object):

    def __init__(self, level1, level2):
        self.level1 = level1
        self.level2 = level2
        self.slope = 6.0 / np.sqrt(2.0) / (level1 - level2)
        self.offset = 3.0 / np.sqrt(2.0) * (1.0 - 2.0 * level1 / (level1 - level2))

    def __call__(self, x):
        return (erf(x * self.slope + self.offset) + 1.0) / 2.0


class gaussScorer(object):

    def __init__(self, x0, sigma):
        self.x0 = x0
        self.sigma = sigma

    def __call__(self, x):
        return np.exp(-4.0 * np.log(2.0) * np.power((x - self.x0) / self.sigma, 2))
