
import shutil
import random
import string

import regression_utility as tstutl

this="maclog_regression"

#print out usage

nrows=100000000
nrows=1000

tstutl.note('*****************************************',"INFO",this)
tstutl.note('******maclog regression test start ******',"INFO",this)
for i in range(nrows):
    usage="i=%s random=%s" % (i, random.random())
    tstutl.note(usage, "INFO", this)
tstutl.note('******maclog regression test finish******','INFO',this)
tstutl.note('*****************************************','INFO',this)
tstutl.note('casalogger, in a test done on 2009-10-29  ','INFO',this) 
tstutl.note('with nrows=100000000 on a mac machine     ','INFO',this)
tstutl.note('followed the logging to near 7,000,000 rows','INFO',this)
tstutl.note('and quit when used 1.22G real memory and','INFO',this)
tstutl.note('4G of virtual memory. It quit when it ','INFO',this)
tstutl.note('could not allocate more memory. ','INFO',this)
tstutl.note('While is possible to follow big logging,','INFO',this)
tstutl.note('the maximum log casalogger can load statically','INFO',this)
tstutl.note('is set to 500000 rows.','INFO',this)
tstutl.note('*****************************************','INFO',this)
