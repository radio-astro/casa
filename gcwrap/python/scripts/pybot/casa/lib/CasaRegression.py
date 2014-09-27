
import os
import re
import sys
import gdbm
import time
import shutil
import subprocess
import urllib
import tempfile
import smtplib
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import warnings

class CasaRegression:

    # regression suite
    # individual regression
    #def __init__( self, topdir, casadir, testdir, cachedir, outputdir ):
    def __init__( self, arglist ):
        ###
        ### avoid warnings about tmpnam( )...
        ###
        warnings.filterwarnings('ignore')
        ###
        ( bindir, topdir, casadir, testdir, cachedir, outputdir ) = str(arglist).split(";;")
        self._path = { 'top': str(topdir), 'bin': str(bindir), \
                       'casa': str(casadir), 'testbase': str(testdir), \
                       'cache': str(cachedir), 'output': str(outputdir) }
        self._state = { 'script': '', 'statedb': '', 'maildb': '', 'result': 'fail', 'stamp': str(0), 'time': str(0), 'master': 'Darrell Schiebel', 'master-email': 'drs@nrao.edu' }
        if not os.path.isdir(self._path['top']):
            raise RuntimeError('top directory (' + self._path['top'] + ') must exist...')
        temppath = self._path['top'] + "/tmp"
        if os.path.exists(temppath) and not os.path.isdir(temppath):
            raise RuntimeError("temp directory(" + temppath + ") exists but is not a directory...")
        if not os.path.exists(temppath):
            os.mkdir(temppath,0755)
        tempfile.tempdir = temppath
        self._path['tmp'] = temppath
        self._status = ''
        self._rerun = False			### set to true to avoid the download/install and reuse the last download


    # individual regression
    def set_time( self, stamp ):
        self._state['time'] = str(stamp)

    def set_smtp( self, host ):
        self._state['smtp-host'] = host
        
    # regression suite
    def clean_casaroot( self ):

        if self._rerun:
            return

        if os.environ.has_key('__PYBOT_CASA_PRECONFIG__'):
            return

        for k, v in self._path.iteritems( ):
            print str(k) + " -> " + str(v)
        if ( not self._path['casa'].startswith(self._path['top']) or self._path['casa'] == self._path['top'] ) :
            raise RuntimeError('casa root (' + self._path['casa'] + ') must be a subdirectory of top dir (' + self._path['top'] +')...')
        if os.path.exists(self._path['casa']):
            if not os.path.isdir(self._path['casa']):
                raise RuntimeError('casa root (' + self._path['casa'] + ') exists but is not a directory...')
            else:
                shutil.rmtree(self._path['casa'])

        if os.path.exists(self._path['testbase']):
            if not os.path.isdir(self._path['testbase']):
                raise RuntimeError('run-test root (' + self._path['testbase'] + ') exists but is not a directory...')
            else:
                shutil.rmtree(self._path['testbase'])

        os.makedirs(self._path['testbase'],0755)


    # regression suite
    def initialize_dbs( self, resultdb, maildb ):
        for path in resultdb, maildb:
            dir = os.path.dirname(path)
            if not os.path.exists(dir):
                os.makedirs(dir)
            elif not os.path.isdir(dir):
                raise RuntimeError("DB path (" + str(dir) + ") is not a directory")
        if os.path.exists(maildb):
            os.remove(maildb)

    # regression suite
    def fetch_build( self, url, checksum_url ):

        if self._rerun:
            return

        if os.environ.has_key('__PYBOT_CASA_PRECONFIG__'):
            return

        cleanup = [ ]
        ###
        ### url & checksum_url have one of two forms:
        ###
        ###       <URL>;;<LOCAL_FILE_PATH>
        ### or    <URL>
        ###
        urlvec = str(url).split(';;')
        sumvec = str(checksum_url).split(';;')
        if len(urlvec) == 2 and len(sumvec) == 2 and \
           os.path.isfile(urlvec[1]) and \
           os.path.isfile(sumvec[1]):
            ### copy the files even though they're available locally...
            ### as they are prone to being overwritten...
            cs = os.tmpnam( ) + '.' + str.join('.',sumvec[1].split('.')[1:])
            f = os.tmpnam( ) + '.' + str.join('.',urlvec[1].split('.')[1:])
            print "fetching " +  sumvec[1] + " as " + cs + " ..."
            shutil.copyfile(sumvec[1],cs)
            cleanup.append(cs)
            print "fetching " + urlvec[1] + " as " + f + " ..."
            shutil.copyfile(urlvec[1],f)
            cleanup.append(f)
        else:
            print "fetching " + sumvec[0] + "..."
            (cs,ch) = urllib.urlretrieve(sumvec[0])
            print "fetching " + urlvec[0] + "..."
            (f,fh) = urllib.urlretrieve(urlvec[0])

        try:
            calculated = ''
            p = subprocess.Popen( [ "/usr/bin/md5sum", f ], stdout=subprocess.PIPE )
            for line in p.stdout:
                if line.endswith(f+'\n'):
                    calculated = line.split( )[0]
                    break
            p.wait( )
            if calculated == '':
                raise RuntimeError('checksum calculation failed...')
            found = False
            for line in open(cs,'r'):
                if line.startswith(calculated):
                    found = True
                    break
            if not found:
                raise RuntimeError('could not match checksum with download...')
    
            if not os.path.exists(self._path['casa']):
                os.mkdir(self._path['casa'],0755)
    
            p = subprocess.Popen( [ "/bin/tar", '-C', self._path['casa'], '-x', '--strip-components', '1', '-f', f ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
            for line in p.stdout:
                print "tar> " + line.rstrip( )
            p.wait( )
        finally:
            while len(cleanup) > 0:
                try:
                    os.unlink(cleanup.pop( ))
                except:
                    continue


    # regression suite
    def setup_build_state(self,datadir):

        if self._rerun:
            return

        casaroot = self._path['casa'] + '/build'
        if os.environ.has_key('__PYBOT_CASA_PRECONFIG__'):
            print "creating " + casaroot + "/init.sh"
            f = open(casaroot + "/init.sh",'w')
            f.write( "export __PYBOT_CASA_PRECONFIG__=\"" + os.environ['__PYBOT_CASA_PRECONFIG__'] + "\"\n" )
            f.close( )
            print "creating " + casaroot + "/init.pl"
            f = open(casaroot + "/init.pl",'w')
            f.write( "$ENV{'__PYBOT_CASA_PRECONFIG__'} = '" + os.environ['__PYBOT_CASA_PRECONFIG__'] + "';\n" )
            f.close( )
            return

        datalink = casaroot + '/data'
        if os.path.exists(datalink):
            if os.path.islink(datalink) or os.path.isfile(datalink):
                os.unlink(datalink)
            elif os.path.isdir(datalink):
                shutil.rmtree(datalink)
            else:
                raise RuntimeError('data directory (' + datalink +') exists and is of an unknown type...')
        os.symlink(datadir,datalink)

        print "creating " + casaroot + "/init.sh"
        f = open(casaroot + "/init.sh",'w')
        f.write( "export CASAPATH=\"%s linux-x86_64\"\n" % casaroot )
        f.write( "export LD_LIBRARY_PATH=%s/linux-x86_64/lib\n" % casaroot )
        f.write( "export PATH=%s/linux-x86_64/bin:$PATH\n" % casaroot )
        f.write( "export __CASAPY_PYTHONDIR=%s/linux-x86_64/python/2.6\n" % casaroot )
        f.write( "export __CASAPY_TASKDIR=%s/linux-x86_64/python/2.6\n" % casaroot )
        f.close( )

        print "creating " + casaroot + "/init.pl"
        f = open(casaroot + "/init.pl",'w')
        f.write( "$ENV{'CASAPATH'} = '%s linux-x86_64';\n" % casaroot )
        f.write( "$ENV{'LD_LIBRARY_PATH'} = '%s/linux-x86_64/lib';\n" % casaroot )
        f.write( "$ENV{'PATH'} = \"%s/linux-x86_64/bin:$ENV{'PATH'}\";\n" % casaroot )
        f.write( "$ENV{'__CASAPY_PYTHONDIR'} = '%s/linux-x86_64/python/2.6';\n" % casaroot )
        f.write( "$ENV{'__CASAPY_TASKDIR'} = '%s/linux-x86_64/python/2.6';\n" % casaroot )
        f.write( "$ENV{'CASA_INSTALLATION_TYPE'} = 'robotframework_test';\n" )
        f.close( )

    def initialize_version( self ):
        versions = [ ]
        if os.environ.has_key('__PYBOT_CASA_PRECONFIG__'):
            versions = os.environ['__PYBOT_CASA_PRECONFIG__'].split( )
        else:
            casainit = self._path['casa'] + '/build/init.pl'
            if not os.path.isfile(casainit):
                raise RuntimeError("no initialization file")
            p = subprocess.Popen( [ self._path['bin'] + "/casapy-version", casainit ], stdout=subprocess.PIPE )
            for line in p.stdout:
                if line.startswith('VERSION>'):
                    versions = line.split( )[1:]
                    break
            p.wait( )

        if len(versions) != 2:
            raise RuntimeError("could not find casapy version number")
        print "version: " + str(versions[0]) + " revision: " + str(versions[1])
        out = open(self._path['output'] + "/version.txt", "w")
        out.write( 'VERSION> ' + str(versions[0]) + " " + str(versions[1]) + "\n" )
        out.close( )
        self._state['version'] = versions[0]
        self._state['revision'] = versions[1]

    # individual regression
    def run( self, script, profile ):
        profile_list = str(profile).split(';;');
        py_prof = 'python' in profile_list
        cpp_prof = 'cpp' in profile_list
        self._state['script'] = script
        self._path['test'] = self._path['testbase'] + "/" + script
        if ( not self._path['test'].startswith(self._path['top']) or self._path['test'] == self._path['top'] ) :
            raise RuntimeError('casa root (' + self._path['test'] + ') must be a subdirectory of top dir (' + self._path['top'] +')...')

        if os.path.exists(self._path['test']):
            if os.path.islink(self._path['test']) or os.path.isfile(self._path['test']):
                os.unlink(self._path['test'])
            elif os.path.isdir(self._path['test']):
                shutil.rmtree(self._path['test'])
            else:
                raise RuntimeError('test directory (' + self._path['test'] +') exists and is of an unknown type...')

        os.mkdir(self._path['test'],0755);

        if False:
            pyroot = self._path['casa'] + '/build/linux-x86_64/python/2.6'
            script_path=''
            for root, dirs, files in os.walk(pyroot):
                if script in files:
                    script_path = root + '/' + script
                    print "copying " + script_path + " to " + self._path['test']
                    shutil.copy(script_path,self._path['test'])
                    break
            if not script_path:
                raise RuntimeError('could not find regression script: ' + script)

        self._path['log'] = self._path['test'] + "/execution-log.txt";
        print "execution log: " + self._path['log']
        casapy = self._path['bin'] + '/xvfb-casapy'

        if py_prof:
            ### PY_PROFILE defaults to true...
            PYPROFILE = ''
        else:
            PYPROFILE = ", PY_PROFILE=False"

        print "starting: " + casapy
        print "(CasaRegression.py) current directory: " + os.getcwd( )

        #CALL TEST SCRIPT (Determine if a regression test or a unit test to determine the proper test method)
        #UNIT TEST
        if script.startswith("test_"):
            print "invoking " + " ".join( [ casapy,'-cd', self._path['test'],
                                          '--eval=' + self._path['casa'] + '/build/init.pl',
                                          '--tmpdir=' + self._path['tmp'], '-c', "runUnitTest.main(['" + script + "'])" ] )
            p = subprocess.Popen( [ casapy,'-cd', self._path['test'],
                                  '--eval=' + self._path['casa'] + '/build/init.pl',
                                  '--tmpdir=' + self._path['tmp'], '-c', "runUnitTest.main(['" + script + "'])" ],
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT );
        #REGRESSION TEST
        else:
            print "invoking " + " ".join( [ casapy, '-cd', self._path['test'],
                                            '--eval=' + self._path['casa'] + '/build/init.pl',
                                            '--tmpdir=' + self._path['tmp'], '-c',
                                            "publish_summary.runTest( '" + script + "', WORKING_DIR='"+self._path['test']+'/pubsum'+"', RESULT_DIR='"+self._path['output']+"', RESULT_SUBDIR='"+script+"', REDIRECT=False" + PYPROFILE + " )" ] )
	    p = subprocess.Popen( [ casapy, '-cd', self._path['test'],
                                '--eval=' + self._path['casa'] + '/build/init.pl',
                                '--tmpdir=' + self._path['tmp'], '-c',
                                "publish_summary.runTest( '" + script + "', WORKING_DIR='"+self._path['test']+'/pubsum'+"', RESULT_DIR='"+self._path['output']+"', RESULT_SUBDIR='"+script+"', REDIRECT=False" + PYPROFILE + " )" ],
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT );
	

        log = open( self._path['log'], 'w' );
        for line in p.stdout:
            print line.rstrip( )
            log.write(line)

    # individual regression
    def scan_log( self, statedb_path, maildb_path, maintainer, email, logfile ):
        self._state['maintainer'] = str(maintainer)
        self._state['email'] = email
        self._state['statedb'] = str(statedb_path)
        self._state['maildb'] = str(maildb_path)
        if not os.path.isfile(self._path['log']):
            raise RuntimeError("log file (" + self._path['log'] + ") does not exist")
        log = open( self._path['log'], "r" )
        num = re.compile(r".*?([0-9.]+).*?")
        for line in log:
            line = line.rstrip( )
            if line.startswith("Regression PASSED"):
                self._state['result'] = 'pass'
	    elif line == 'OK':
		# from UNIT TEST
		self._state['result'] = 'pass'
	    elif line.startswith("OK (SKIP="):
		#from UNIT TEST (special cases with skipped tests)
		self._state['result'] = 'pass'
            elif line == 'status = pass # execution status':
                # from publish_summary
                self._state['result'] = 'pass'
            elif line == 'status = fail # execution status':
                # from publish_summary
                self._state['result'] = 'fail'
            elif line.startswith('time = '):
                # from publish_summary
                self._state['time:wall'] = re.match(num,line).group(1)
            elif line.startswith("Total wall clock time was"):
                self._state['time:wall'] = re.match(num,line).group(1)
            elif line.startswith("Total CPU        time was"):
                self._state['time:cpu'] = re.match(num,line).group(1)
            elif line.startswith("[result:exit status]"):
                self._state['status'] = re.match(num,line).group(1)
            elif line.startswith("[result:exit condition]"):
                self._state['condition'] = re.match(r"\[result:exit condition\]\s+(.*)",line).group(1)
        log.close( )

        for k, v in self._state.iteritems( ):
            print str(k) + " -> " + str(v)

        divider = ";;"
        self._state['stamp'] = str(self._state['script']).split('.')[0] + ":" + self._state['time']
        statedb = gdbm.open(self._state['statedb'],"c")
        statedb[self._state['stamp']] = (str(self._state['result']) if self._state.has_key('result') else "malfunction") + divider + \
                                        (str(self._state['time:wall']) if self._state.has_key('time:wall') else "0") + divider + \
                                        (str(self._state['time:cpu']) if self._state.has_key('time:cpu') else "0")
        statedb.close( )
        if not self._state.has_key('result'):
            maildb = gdbm.open(self._state['maildb'],"c")
            maildb[self._state['stamp']] = "malfunction" + divider + str(self._state['script']) + divider + self._state['master'] + divider + self._state['master-email'] + divider + logfile
            maildb.close( )
        elif self._state['result'] != 'pass':
            maildb = gdbm.open(self._state['maildb'],"c")
            maildb[self._state['stamp']] = "fail" + divider + str(self._state['script']) + divider + str(maintainer) + divider + str(email) + divider + logfile
            maildb.close( )

        ###
        ### send a separate email to the maintainer if one of the benchmarks is undefined...
        ### ...decided not to force benchmarking to be defined for every test for some tests, it
        ### probably doesn't make sense, e.g. async-segv... however, this may change in the future...
        ### if all unit tests are stripped out of the regression tests...
        ###
        #if not self._state.has_key('time:wall') or \
        #   not self._state.has_key('time:cpu'):
        #    maildb = gdbm.open(self._state['maildb'],"c")
        #    maildb['*' + self._state['stamp']] = "malfunction" + divider + str(self._state['script']) + divider + self._state['master'] + divider + self._state['master-email'] + divider + logfile
        #    maildb.close( )
            

    # individual regression
    def result( self ):
        if not self._state.has_key('result'):
            raise RuntimeError('The test script ' + str(self._state['script']) + ' malfunctioned...')
        elif self._state['result'] != 'pass':
            raise RuntimeError('The execution of ' + str(self._state['script']) + ' failed...')

    # regression suite
    def process_results( self, msg_file, maildb_path, statedb_path, report_path, log_path, summary_email_list ):
        ###
        ### list of people to recieve a failure digest
        ###
        #summary_email_list = self._state['master-email']      #### override to prevent email from going to summary subscribers
        #summary_email_list="drs@nrao.edu"
        summary_email_recipients = summary_email_list.split(';;')
        summary_email_recipients.append(self._state['master-email'])
        
        ###
        ### web-published reports should have the jenkins '/userContent/' in the path...
        ### otherwise, we'll assume it's a local file...
        ###
        ### time_val = time.strftime("%Y%m%d_%H%M",time.localtime(int(self._state['time'])))
        ###
        webreport = re.match(r".*/userContent/(.*)",report_path)
        if webreport:
            url = "http://casa-jenkins.nrao.edu/userContent/" + webreport.group(1)
        else:
            url = "file://" + report_path

        fp = open( msg_file, "rb" )
        msg = fp.read( )
        fp.close( )

        s = smtplib.SMTP(self._state['smtp-host'])
        maildb = gdbm.open(maildb_path,"c")

        ###
        ### send email to regression owners...
        ###
        key = maildb.firstkey( )
        count = 0
        while key != None:
            (regression,tepoch) = key.split(':')
            (failure,script,full_name,email,logfile) = maildb[key].split(';;')
            key = maildb.nextkey(key)
            name = full_name.split( )
            count += 1

            if os.path.isfile(logfile):
                mime = MIMEMultipart( )
                ### preamble is not needed...
                #mime.preamble = "this is the preamble"
                logfp = open(logfile,'rb')
                txt = MIMEText( msg % (name[0],script,url), 'plain')
                html = MIMEText( logfp.read( ), 'html')
                html.add_header('Content-Disposition', 'attachment', filename=os.path.basename(logfile))
                html.add_header('Content-Description', script + ' failure log')
                mime.attach(txt)
                mime.attach(html)
                logfp.close( )
            else:
                mime = MIMEText( msg % (name[0],script,url) )

            if failure == "malfunction":
                mime['Subject'] = "[regression malfunction] " + regression
            else:
                mime['Subject'] = "[regression failure] " + regression
            mime['From'] = "CASA Jenkins <" + self._state['master-email'] + ">"
            #email = self._state['master-email']              #### override to prevent email from going to regression owners
            #email = 'drs@nrao.edu'
            mime['To'] = full_name + " <" + email + ">"
            s.sendmail( "CASA Jenkins <" + self._state['master-email'] + ">", [full_name + " <" + email + ">", self._state['master'] + " <" + self._state['master-email'] + ">"], mime.as_string( ) )

        ###
        ### send composite email...
        ###
        if count > 0:
            mime = MIMEMultipart( )
            mime['From'] = "CASA Jenkins <" + self._state['master-email'] + ">"
            mime['Subject'] = "casa " + str(self._state['version']) +  " regression digest (r" + self._state['revision'] + ")"
            txt = MIMEText( "\n\n" + url + "\n\n", 'plain' )
            mime.attach(txt)
            key = maildb.firstkey( )
            while key != None:
                (regression,tepoch) = key.split(':')
                (failure,script,full_name,email,logfile) = maildb[key].split(';;')
                key = maildb.nextkey(key)

                txt = MIMEText( "\n\nRegression " + script + " failed" + (":" if os.path.isfile(logfile) else "...") +"\n", 'plain' )
                mime.attach(txt)
                if os.path.isfile(logfile):
                    logfp = open(logfile,'rb')
                    html = MIMEText( logfp.read( ), 'html')
                    html.add_header('Content-Disposition', 'attachment', filename=os.path.basename(logfile))
                    html.add_header('Content-Description', script + ' failure log')
                    mime.attach(html)
                    logfp.close( )

            ###
            ### when sent to a mailing list, the last log attachment and the mailing list
            ### footer are formatted on the same line... (at least with OSX's mail app)...
            ###
            txt2 = MIMEText( "\n\n\n", 'plain' )
            txt2.add_header('Content-Disposition', 'inline')
            mime.attach(txt2)

            mime['To'] = summary_email_list.replace(';;',', ')
            s.sendmail( "CASA Jenkins <" + self._state['master-email'] + ">", summary_email_recipients, mime.as_string( ) )

        maildb.close( )
        s.quit( )

        if count <= 0:
            ###
            ### generate tarball distros upon successful builds...
            ###
            version_file = self._path['casa'] + "/build/share/version.txt"
            major = ''
            minor = ''
            bug = ''
            svn = ''
            if os.path.isfile(version_file):
                version_re = re.compile(r"([0-9]+)\.([0-9]+)\.([0-9]+)\s+([0-9]+).*?")
                for line in open(version_file,'r'):
                    if line.startswith('#'):
                        continue			# comment
                    else:
                        m = re.match(version_re,line);
                        if m is not None:
                            major = m.group(1)
                            minor = m.group(2)
                            bug = m.group(3)
                            svn = m.group(4)

            if major and minor and bug and svn:
                p = subprocess.Popen( [ self._path['bin'] + "/cbtdistro", '--version', major + "." + minor + "." + bug + "." + svn ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
                for line in p.stdout:
                    print "cbt> " + line.rstrip( )
                p.wait( )

        ###
        ### do report post processing (to set the title)...
        ### report post processing was moved to the end when distro packaging was added
        ### because the extra distro processing time pushes finalization of the pybot run
        ### past the two minute delay... pybot finalization and exit is required prior to
        ### processing the report because the html report is *only* generated after the
        ### final pybot actions take places (since these may add to the log)...
        ###
        print self._path['bin'] + "/postprocess-report " + "[DELAY:120]::" + report_path
        p = subprocess.Popen( [ self._path['bin'] + "/postprocess-report", "[DELAY:120]::" + report_path ], stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
        for line in p.stdout:
            print "postprocess> " + line.rstrip( )
        p.wait( )
