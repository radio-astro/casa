#
# This file implements a plugin for nose, which monitors the net memory leaked
# and opened file descriptors from a test run. The result are written to
# an XML file. This plugin is just an adaptation of nose's built-in
# XUnit plugin.
#

import nose.plugins.xunit
import traceback
import inspect
import time
import commands
import os

def nice_classname(obj):
    """Returns a nice name for class object or class instance.
    
        >>> nice_classname(Exception()) # doctest: +ELLIPSIS
        '...Exception'
        >>> nice_classname(Exception)
        'exceptions.Exception'
    
    """
    if inspect.isclass(obj):
        cls_name = obj.__name__
    else:
        cls_name = obj.__class__.__name__
    mod = inspect.getmodule(obj)
    if mod:
        name = mod.__name__
        # jython
        if name.startswith('org.python.core.'):
            name = name[len('org.python.core.'):]
        return "%s.%s" % (name, cls_name)
    else:
        return cls_name

def write_message(fileleak, memoryleak):

    if fileleak != 0:
        print "Net file descriptors opened:", fileleak
    if memoryleak != 0:
        print "Net memory allocated:", memoryleak, "kB"
    return "<system-out>\
&lt;measurement&gt;&lt;name&gt;Files leaked&lt;/name&gt;&lt;value&gt;" + str(fileleak) + "&lt;/value&gt;&lt;/measurement&gt;\
&lt;measurement&gt;&lt;name&gt;Memory leaked (kB)&lt;/name&gt;&lt;value&gt;" + str(memoryleak) + "&lt;/value&gt;&lt;/measurement&gt;\
</system-out>"


class MemTest(nose.plugins.xunit.Xunit):

    name = "memtest"

    def options(self, parser, env):
        # Identical to the base class method, except that the command line
        # option needs to be called something different
        """Sets additional command line options."""
        nose.plugins.base.Plugin.options(self, parser, env)
        parser.add_option(
            '--memtest-file', action='store',
            dest='xunit_file', metavar="FILE",
            default=env.get('NOSE_XUNIT_FILE', 'nosetests.xml'),
            help=("Path to xml file to store the xunit report in. "
                  "Default is nosetests.xml in the working directory "
                  "[NOSE_XUNIT_FILE]"))

        # Find the lsof executable
        self.lsof = "/usr/sbin/lsof"
        if not os.path.exists(self.lsof):
            self.lsof = "/usr/bin/lsof"
        if not os.path.exists(self.lsof):
            print "Warning: Could not find lsof at /usr/sbin/lsof or /usr/bin/lsof"

    def report(self, stream):
        """Writes an Xunit-formatted XML file

        The file includes a report of test errors and failures.

        """
        self.stats['encoding'] = self.encoding
        self.stats['total'] = (self.stats['errors'] + self.stats['failures']
                               + self.stats['passes'] + self.stats['skipped'])
        self.error_report_file.write(
            '<?xml version="1.0" encoding="%s"?>'
            '<testsuites>' % self.encoding)
        
        self.error_report_file.write(''.join(self.errorlist))
        self.error_report_file.write('</testsuites>')
        self.error_report_file.close()
        if self.config.verbosity > 1:
            stream.writeln("-" * 70)
            stream.writeln("XML: %s" % self.error_report_file.name)

    def startTest(self, test):
        """Initializes a timer before starting a test."""
        self._timer = time.time()
        self._pid = os.getpid()
        msg = '***** List of open files after running %s\n'%test
        infile = open('ListOpenFiles','a')
        infile.write(msg)
        infile.close()

        (errorcode, n) = commands.getstatusoutput(self.lsof + ' -p ' + str(self._pid) + ' | wc -l')
        if errorcode == 0:
            self._openfiles = n
        else:
            self._openfiles = 0

        (errorcode, n) = commands.getstatusoutput('ps -p ' + str(self._pid) + ' -o rss | tail -1')
        if errorcode == 0:
            self._resident_memory = n
        else:
            self._resident_memory = 0           

    def _update_after_test(self):
        # The predefined hooks stopTest() and afterTest() cannot be used
        # because they get called after addError/addFailure/addSuccess
        os.system(self.lsof + ' -p ' + str(self._pid) + ' | grep -i nosedir >> ListOpenFiles')

        (errorcode, n) = commands.getstatusoutput(self.lsof + ' -p ' + str(self._pid) + ' | wc -l')

        if errorcode == 0:
            self._fileleak = int(n) - int(self._openfiles)
        else:
            self._fileleak = -1

        (errorcode, n) = commands.getstatusoutput('ps -p ' + str(self._pid) + ' -o rss | tail -1')
        if errorcode == 0:
            self._memoryleak = int(n) - int(self._resident_memory)
        else:
            self._memoryleak = 0

    def addError(self, test, err, capt=None):
        """Add error output to Xunit report.
        """
        self._update_after_test()
        taken = time.time() - self._timer
        if issubclass(err[0], nose.exc.SkipTest):
            self.stats['skipped'] +=1
            return
        tb = ''.join(traceback.format_exception(*err))
        self.stats['errors'] += 1
        id = test.id()
        name = self._xmlsafe(id)[-self._xmlsafe(id)[::-1].find("."):]
        self.errorlist.append(
            '<testsuite name="nosetests" tests="1" errors="1" failures="0" skip="0">'
            '<testcase classname="%(cls)s" name="%(name)s" time="%(taken)d">'
            '<error type="%(errtype)s">%(tb)s</error></testcase>' %
            {'cls': self._xmlsafe('.'.join(id.split('.')[:-1])),
             'name': name,
             'errtype': self._xmlsafe(nice_classname(err[0])),
             'tb': self._xmlsafe(tb),
             'taken': taken,
             })
        self.errorlist.append(write_message(self._fileleak, self._memoryleak))
        self.errorlist.append('</testsuite>')

    def addFailure(self, test, err, capt=None, tb_info=None):
        """Add failure output to Xunit report.
        """
        self._update_after_test()
        taken = time.time() - self._timer
        tb = ''.join(traceback.format_exception(*err))
        self.stats['failures'] += 1
        id = test.id()
        name = self._xmlsafe(id)[-self._xmlsafe(id)[::-1].find("."):]
        self.errorlist.append(
            '<testsuite name="nosetests" tests="1" errors="0" failures="1" skip="0">'
            '<testcase classname="%(cls)s" name="%(name)s" time="%(taken)d">'
            '<failure type="%(errtype)s">%(tb)s</failure></testcase>' %
            {'cls': self._xmlsafe('.'.join(id.split('.')[:-1])),
             'name': name,
             'errtype': self._xmlsafe(nice_classname(err[0])),
             'tb': self._xmlsafe(tb),
             'taken': taken,
             })
        self.errorlist.append(write_message(self._fileleak, self._memoryleak))
        self.errorlist.append('</testsuite>')
        
    def addSuccess(self, test, capt=None):
        """Add success output to Xunit report.
        """
        self._update_after_test()
        taken = time.time() - self._timer
        self.stats['passes'] += 1
        id = test.id()
        name = self._xmlsafe(id)[-self._xmlsafe(id)[::-1].find("."):]
        self.errorlist.append(
            '<testsuite name="nosetests" tests="1" errors="0" failures="0" skip="0">'
            '<testcase classname="%(cls)s" name="%(name)s" '
            'time="%(taken)d" />' %
            {'cls': self._xmlsafe('.'.join(id.split('.')[:-1])),
             'name': name,
             'taken': taken,
             })
        self.errorlist.append(write_message(self._fileleak, self._memoryleak))
        self.errorlist.append('</testsuite>')
