###############################
# Regression 
# 
###############################
DO_G192			= True
DO_H121			= True
DO_L02D			= True
DO_NGC5921		= True
DO_NGC7538		= True
#----------
DO_NGC1333		= True
DO_NGC4826		= True
#---------
DO_NGC4826C		= True
DO_ORION		= True
#---------
DO_B0319		= True
#---------
DO_ORI_SIO_TASK		= True
DO_IRC_HC3N_TASK	= True
DO_IRC_CS_TASK		= True
DO_FLS3A_HI		= True
#---------
DO_COORDSYSTEST         = True
DO_IMAGEPOLTEST         = True
DO_IMAGETEST            = True
#---------
DO_IC2233               = False
#---------
DO_POINTING             = False
#---------
###############################

import time
import os

pathname=os.environ.get('CASAPATH').split()[0]
libIndex=pathname.find('lib')

if (libIndex !=-1):
    # Copy test scripts from an installed package directory. 
    scriptpath=pathname+'/lib/python2.6/regressions'
else:
    # Copy test scripts from the source directory.
    scriptpath=pathname+'/gcwrap/python/scripts/regressions'

command='cp '+scriptpath+'/*_regression.py .'
#command='ls '+scriptpath+'/*_regression.py'

os.system(command)

import string
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='REGRESSION.'+datestring+'.log'
regfile=open(outfile,'w')

print >>regfile,''
print >>regfile,'******** Report '
print >>regfile,'         ------'
#print >>regfile,'SOURCE \t PASSED  TIME \t\t RATE \t \t LOGFILE'
print >>regfile,'%9s %6s %9s %9s %39s'%('SOURCE','PASSED','TIME','RATE','LOGFILE')

scriptlist=['G192','H121','L02D','NGC5921','NGC7538','NGC1333','NGC4826','NGC4826C','ORION','B0319','ORI_SIO_TASK','IRC_HC3N_TASK','IRC_CS_TASK','FLS3A_HI','COORDSYSTEST', 'IMAGEPOLTEST', 'IMAGETEST', 'IC2233', 'POINTING']
ratedict={'G192': 634.9,'H121': 500.,'L02D': 500.,'NGC5921': 35.1,'NGC7538': 240.3,'NGC1333': 500.,'NGC4826': 662.,'NGC4826C': 760., 'ORION': 500., 'B0319': 5.0,'ORI_SIO_TASK': 74,'IRC_HC3N_TASK': 694., 'IRC_CS_TASK': 694, 'FLS3A_HI':4100., 'COORDSYSTEST':44., 'IMAGEPOLTEST':41., 'IMAGETEST':32., 'IC2233':8051, 'POINTING':1080}

for mysource in scriptlist:
    execute_script='DO_'+mysource
    if (eval(execute_script)):
        try:
            print 'source ',mysource
            execfile(str.lower(mysource)+'_regression.py')
            scriptpass=regstate
            scriptlog=outfile
            scripttime=(endTime-startTime)
            scriptrate=ratedict[mysource]/(endTime-startTime)
            print >>regfile,'%9s %6s %9s %9s %39s'%(mysource,scriptpass,scripttime,scriptrate,scriptlog)
        except:
            print 'Test failed:', sys.exc_info()[0]

regfile.close()

