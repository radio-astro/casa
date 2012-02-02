from taskinit import casalog
import sys
import os

casalog.showconsole(onconsole=True)

print sys.argv
i = sys.argv.index("-c")
data_dir  = [sys.argv[i+2]]
work_dir  = sys.argv[i+3]
test_name = sys.argv[i+4]
if sys.argv[i+5] != "0":
    p = True
else:
    p = False

os.chdir(work_dir)
sys.path.append(work_dir + '/admin/')
import publish_summary

try:
    publish_summary.runTest(test_name,
                            data_dir, work_dir + '/work', work_dir + '/result',
                            CPP_PROFILE = p)
except:
    print "runTest execution failed"
    raise
    exit()  # sys.exit(1) does not work with IPython
exit()
