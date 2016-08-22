import sys
import os
from nose.tools import *
from nose.plugins.skip import Skip, SkipTest
from asap.logging import asaplog
from asap.env import is_casapy
from asap import rcParams

# no logging if not verbose
rcParams['verbose'] = True

if is_casapy():
    raise SkipTest("Can't test against casalog")


class WritableObject:
    def __init__(self):
        self.content = []

    def write(self, string):
        self.content.append(string)

    def clear(self):
        self.content = []

stdout_redirect = WritableObject()

def redirect_setup():
    sys.stdout = stdout_redirect

def redirect_teardown():
    stdout_redirect.clear()
    sys.stdout = sys.__stdout__

@with_setup(redirect_setup, redirect_teardown)
def test_enabled():
    asaplog.enable()
    msg = "TEST"
    asaplog.push(msg)
    asaplog.post()
    out = "".join(stdout_redirect.content).strip()
    assert_equals(out, msg)

@with_setup(redirect_setup, redirect_teardown)
def test_disabled():
    asaplog.disable()
    msg = "TEST"
    asaplog.push(msg)
    asaplog.post()
    out = "".join(stdout_redirect.content).strip()
    assert_equals(out, '')

@with_setup(redirect_setup, redirect_teardown)
def test_push():
    asaplog.enable()
    msg = "TEST"
    asaplog.push(msg)
    asaplog.push(msg)
    asaplog.post()
    input = "\n".join([msg]*2)
    out = "".join(stdout_redirect.content).strip()
    assert_equals(out, input)


@with_setup(redirect_setup, redirect_teardown)
def test_level():
    asaplog.enable()
    msg = "TEST"
    asaplog.push(msg)
    asaplog.post('ERROR')
    out = "".join(stdout_redirect.content).strip()
    assert_equals(out, "SEVERE: "+msg)
