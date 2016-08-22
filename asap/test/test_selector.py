from nose.tools import *
from asap.selector import selector


# call constructor with args
def constructor(d):
    p = selector(**d)
    for k, v in d.items():
        meth = getattr(p, 'get_%s' % k)
    val = v
    if not hasattr(val, "__len__"):
        val = [val]
    assert_equals(meth(), val)

# generator for constructor arguments
def test_constructor_keys():
    args = [ { 'rows': 0 },
             { 'rows': [1,2]}, 
             { 'ifs': 0 }, 
             { 'ifs': [1,2]}, 
             { 'beams': 0 },
             { 'beams': [1,2] },
             { 'pols': 0 },
             { 'pols': range(4) },
             { 'cycles': 0 },
             { 'cycles': range(5) },
             { 'scans': 0 },
             { 'scans': range(5) },
             { 'scans': range(5), 'beams': 0 },
             ]
    for arg in args:
        yield constructor, arg
        
def test_copy_constructor():
    v = [0]
    s = selector(ifs=v)
    scpy = selector(s)
    assert_equals(s.get_ifs(), scpy.get_ifs())
    scpy.set_beams(v)
    assert_not_equals(s.get_beams(), scpy.get_beams())


def test_add():
    v = [0]
    s1 = selector(ifs=v)
    s2 = selector(beams=v)
    s3 = s1 + s2
    assert_equals(s3.get_ifs(), v)
    assert_equals(s3.get_beams(), v)
    assert_not_equals(id(s3), id(s1))
    assert_not_equals(id(s3), id(s2))

def test_fields():
    base = ['pols', 'ifs', 'beams', 'scans', 'cycles', 'name', 
            'query', 'types', 'rows']
    assert_equals(selector.fields, base)

def test_reset():
    v = [0]
    s = selector(ifs=v)
    assert_equals(s.get_ifs(), v)
    s.reset()
    assert_true(s.is_empty())

