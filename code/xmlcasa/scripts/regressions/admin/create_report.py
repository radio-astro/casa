import sys
import os
from taskinit import casalog

casalog.showconsole(onconsole=True)

print sys.argv

i = sys.argv.index("-c")
admin_dir  = sys.argv[i+2]
reg_dir    = sys.argv[i+3]
report_dir = sys.argv[i+4]
if len(sys.argv) > i+5:
    rev = sys.argv[i+5]
    gp = False
else:
    rev = 'all'
    gp = True

sys.path.append(admin_dir)
print sys.path
import report

try:
    report.report(reg_dir, report_dir, gen_plot=gp, revision=rev, skull=admin_dir+'/skullnbones.jpg')
except:
    print "report execution failed"
    raise
    exit()  # sys.exit(1) does not work with IPython
exit()
