def pybot_install( ):
    import tempfile
    import subprocess
    import shutil
    archdir = casa['dirs']['arch']
    pybot_url = "https://svn.cv.nrao.edu/svn/casa/development_tools/testing/pybot"
    tmp = tempfile.mkdtemp( )

    checkout = subprocess.Popen( "svn co %s ." % pybot_url, \
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, cwd=tmp )
    (output, err) = checkout.communicate()
    if len(err) > 0:
        print "OUTPUT: ", output
        print "ERROR:  ", err

    install = subprocess.Popen( "python setup.py install --install-lib=%s/python/2.7 --install-scripts=%s/bin" % (archdir,archdir), \
                                    stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True, cwd=tmp )
    (output, err) = install.communicate()
    if len(err) > 0:
        print "OUTPUT: ", output
        print "ERROR:  ", err

    shutil.rmtree(tmp)

def pybot_setup( ):
    import subprocess
    regression_url = "https://svn.cv.nrao.edu/svn/casa/development_tools/testing/pybot-regression"
    checkout = subprocess.Popen( "svn co %s casa" % regression_url, \
                                     stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True )
    (output, err) = checkout.communicate()
    if len(err) > 0:
        print "OUTPUT: ", output
        print "ERROR:  ", err

import publish_summary
import runUnitTest
###
### runRegressionTest is not currently set up to be
### imported and invoked but rather run like:
###
###   casa --nologger --log2term -c scripts/runRegressionTest.py cleanhelper
###
### importing it, breaks it...
###
#import runRegressionTest
