import sys
import os
import string
import numpy
import os
import shutil
from task_vishead import vishead


input_file = 'n4826_16apr98.ms'  # 128 channels
stars = "*************"

stop_on_first_error = False

# Keeps track of number of passes, failures
class tester:
    def __init__(self):
        self.total = 0
        self.fail = 0
        self.current_test = ""

    def start(self, msg):
        self.total += 1
        print
        print stars + " Test " + msg + " start " + stars
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

def description():
    return "Test of vishead task"

def data():
    return [input_file]

def run():
    t = tester()

    #os.system('pwd')
    #os.system('find ./pointingtest.ms -type f | xargs cksum | grep OBSERVATION | grep -v svn')

    t.start("vishead( '" + input_file + "', 'list' )")
    orig_hdr = vishead( input_file, 'list' )
    print "Original header ="
    print orig_hdr
    t.end( type(orig_hdr) == \
              type({'key1':'val1', 'key2':'val2'}), \
              "... is a bad header")
    
    t.start("summary")
    vishead( input_file, 'summary' )
    t.end( True, "summary failed" )

    # Test the set/get value routines.  All of them
    for keyword in ['schedule',
                    'observer',
                    'project',
                    'release_date',
                    'schedule_type',
                    'telescope',
                    'field',
                    'freq_group_name',
                    'spw_name']:
        
        t.start("get " + keyword)
        print "List value:     ", orig_hdr[keyword]
        
        val = vishead( input_file, 'get', keyword , '')
        print "Read value:     ", val

        # numpy array comparison returns a list of booleans
        # therefore we have to manually traverse (sigh...) larger data
        # structures containing numpy arrays, in order to check
        # for equality
        if type(orig_hdr[keyword]) is dict:
            are_equal = (orig_hdr[keyword].keys() == val.keys())
            for k in val.keys():
                are_equal = (are_equal and is_true(orig_hdr[keyword][k] == val[k]))
                if not is_true(orig_hdr[keyword][k] == val[k]):
                    print orig_hdr[keyword][k] == val[k]
        else:
            are_equal = (val == orig_hdr[keyword])
        t.end(are_equal, \
              "Got "+str(val)+", expected "+str(orig_hdr[keyword]))

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
            vishead( input_file, 'put', keyword, myval )
            
            newval = vishead( input_file, 'get', keyword )
            print "Read new value: ", newval
            
            t.end(newval == myval, "Got "+str(newval)+", expected "+str(myval))
        else:
            # read/write full column
            all_values = vishead( input_file, 'get', keyword, '' )
            vishead( input_file, 'put', keyword, '', all_values )
            
            i = 0
            for e in val:
                if type(e) == numpy.string_:
                    myval = 'the_coolest_' + e
                else:
                    myval = 42.0 + e
    
                t.start("put/get " + keyword + '[' + str(i) + ']')
                
                print "New value:      ", myval
                vishead( input_file, 'put', keyword, str(i), myval )
                
                newval = vishead( input_file, 'get', keyword, str(i))
                print "Read new value: ", newval
                
                t.end(newval == myval, "Got "+str(newval)+", expected "+str(myval))
                
                i += 1
        
        #imhead( input_file, 'put', 'object', val['value'] )

    t.done()

    return []
