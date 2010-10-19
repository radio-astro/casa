import os
import sys
import shutil
import subprocess
from __main__ import default
from tasks import *
from taskinit import *
import unittest

'''
test consistency of compiled in version & casa record version
'''
class version_test1(unittest.TestCase):

    # Input and output names

    def setUp(self):
        self.setup = True

    def tearDown(self):
        self.setup = False

    def test1(self):
        '''Version 1: does the compiled-in version equal the casa record version'''
        ccver = casalog.version().split('#')
        ccver = ccver[len(ccver)-1]
        ccver = ccver.split(')')[0]
        self.assertEqual(ccver,casa['source']['revision'])

def suite():
    return [version_test1]
