import sys
import os
import string
import numpy
import os
import shutil
from task_vishead import vishead
import unittest

'''
Unit tests for task vishead. It tests the following modes:
    list, summary, get, put
    
 This example shows how to run it manually from within casapy.
 runUnitTest.main(['test_imhead'])

 or outside casapy like this:
 casapy -c runUnitTest.py test_imhead

'''

input_file = 'n4826_16apr98.ms'  # 128 channels
stars = "*************"

stop_on_first_error = False

# Keeps track of number of passes, failures
# Private class
class tester:
    def __init__(self):
        self.total = 0
        self.fail = 0
        self.current_test = ""

    def start(self, msg):
        self.total += 1
        print
        print stars + " Test %s (" % self.total + msg + ") start " + stars
        self.current_test = msg

    def end(self, condition, error_msg):
        status = "OK"
        if not is_true(condition):
            print >> sys.stderr, error_msg
            self.fail += 1
            status = "FAIL"
            if stop_on_first_error:
                raise Exception, "Halt!"
        print stars + " Test " + self.current_test + " " + status + " " + stars

    def done(self):
        print "%s/%s tests passed" % (self.total-self.fail, self.total)
        if self.fail > 0:
            raise Exception, "%s/%s failures" % (self.fail, self.total)
        else:
            print "All tests passed, congratulations!"


def is_true(x):
    # numpy array comparison yields a list of booleans
    # (not a single boolean). If necessary, convert from
    # list of booleans to single boolean (all elements must be True)
    if type(x) != bool and type(x) != numpy.bool_:
        return False not in x
    else:
        return x

# Unittest class
class vishead_test(unittest.TestCase):
    
    def setUp(self):
        if(os.path.exists(input_file)):
            os.system('rm -rf ' +input_file)

        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/ATST2/NGC4826/'
        os.system('cp -r ' +datapath + input_file +' ' + input_file)

    def tearDown(self):
        os.system('rm -rf ' +input_file)
        
    def test_list(self):
        '''Vishead: List mode'''
        t = tester()
    
        #os.system('pwd')
        #os.system('find ./pointingtest.ms -type f | xargs cksum | grep OBSERVATION | grep -v svn')
        
        t.start("vishead( '" + input_file + "', 'list', [])")
        orig_hdr = vishead(input_file, 'list', [])  # default listitems seems to work when
                                                    # run manually, but not from here.
        print "Original header ="
        print orig_hdr
        t.end(type(orig_hdr) == type({'key1':'val1', 'key2':'val2'})
              and orig_hdr['source_name'][0][2] == 'NGC4826',
              "... is a bad header")
    
    def test_summary(self):
        '''Vishead: Summary mode'''
        t = tester()
        t.start("summary")
        vishead( input_file, 'summary' )
        t.end( True, "summary failed" )

    def test_accessors(self):
        '''Vishead: Test put/get modes'''
        t = tester()
        orig_hdr = vishead(input_file, 'list', [])
        
        # Test the set/get value routines.  All of them
        for keyword in orig_hdr:
            print "List value of %s:" % (keyword), orig_hdr[keyword]
            
            # Test getting.
            valref = vishead(input_file, mode='get', hdkey=keyword, hdindex='')
            print "Read value:     ", valref
            #sys.stdout.flush()
            for j in range(2):
                t.start("get " + keyword + "[%d]" % j)
                val = valref[j]
                
                # numpy array comparison returns a list of booleans
                # therefore we have to manually traverse (sigh...) larger data
                # structures containing numpy arrays, in order to check
                # for equality
                if type(orig_hdr[keyword][j]) is dict:
                    are_equal = (orig_hdr[keyword][j].keys() == val.keys())
                    for k in val.keys():
                        are_equal = (are_equal and is_true(orig_hdr[keyword][j][k] == val[k]))
                        if not is_true(orig_hdr[keyword][j][k] == val[k]):
                            print orig_hdr[keyword][j][k] == val[k]
                else:
                    are_equal = (val == orig_hdr[keyword][j])
                if hasattr(are_equal, 'all'):
                    are_equal = are_equal.all()
                t.end(are_equal, \
                      "Got "+str(val)+", expected "+str(orig_hdr[keyword][j]))
                
            # Test putting.
            # Put does not yet use the ref part of valref.
            val = valref[0]
            if type(val) is dict:
                print str(keyword) + ' is probably a column ' + \
                'with variable length arrays, don\'t try to write that'
                # because the task doesn't support it
                continue
    
            if len(val) == 1:
                if type(val[0]) == numpy.string_:
                    myval = 'the_coolest_' + val[0]
                else:
                    myval = 42.0 + val[0]
                    
                t.start("put/get " + keyword)
                print "New value:      ", myval
                vishead(input_file, mode='put', hdkey=keyword, hdindex='', hdvalue=myval)
                
                newval = vishead(input_file, mode='get', hdkey=keyword, hdindex='')[0]
                print "Read new value: ", newval
                
                t.end(newval == myval, "Got "+str(newval)+", expected "+str(myval))
            else:
                # read/write full column
                all_values = vishead(input_file, mode='get', hdkey=keyword)[0]
                vishead(input_file, mode='put', hdkey=keyword, hdindex='', hdvalue=all_values)
                
                i = 0
                for e in val:
                    if type(e) == numpy.string_:
                        myval = 'the_coolest_' + e
                    else:
                        myval = 42.0 + e
        
                    t.start("put/get " + keyword + '[' + str(i) + ']')
                    
                    print "New value:      ", myval
                    vishead(input_file, mode='put', hdkey=keyword, hdindex=str(i),
                            hdvalue=myval)
                    
                    newval = vishead(input_file, mode='get', hdkey=keyword, hdindex=str(i))[0]
                    print "Read new value: ", newval
                    
                    t.end(newval == myval, "Got "+str(newval)+", expected "+str(myval))
                    
                    i += 1
            
            #imhead( input_file, 'put', 'object', val['value'] )
        
        t.done()
    

def suite():
    return [vishead_test]

