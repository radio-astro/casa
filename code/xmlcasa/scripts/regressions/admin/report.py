"""Create HTML report of regression test results

usage: python report.py testresult_dir report_dir revision generate_plot

testresult_dir: database top directory
report_dir: output directory
revision: 'latest' for latest trunk only, or 'all'
generate_plot: boolean, generates png plots iff true

expected format of logfiles:
[name]  =  [value] # [comment]

"""

from taskinit import casalog
import shutil
import os
import sys
import re
import sets
import commands
import pylab as pl
import datetime

# profile generation:
import time
from tw_utils import *
from matplotlib.font_manager import  FontProperties

colormania = False   # more or less colors in the generated HTML
colormania = True    #

SOURCE_DIR = os.environ["CASAPATH"].split()[0]
known_releases = ["CASA Version 2.3.0 (build #6654)",
                  "CASA Version 2.3.1 (build #6826)"]

#exclude_host = ['pc011896.hq.eso.org']
#exclude_test = ['fits-import-export']
exclude_host = []
exclude_test = []
same_version_per_host = False  # if False the latest run for each test is reported

def cmp_exec(a, b):
    if a[1] == 'exec':
        return -1
    elif b[1] == 'exec':
        return 1
    else:
        return 0

def shorten(s, maxlength=20):
    if len(s) > maxlength:
        # HTML tooltip
        return "<span title='"+s+"'>" + \
               s[0:maxlength-2] + ".." + \
               "</span>"
    else:
        return s

def architecture(s):
    try:
        arch = re.compile('([^\s]+ [^\s]+-bit)').search(s).groups()[0]
    except:
        arch = "???"
    return arch

def distribution(s):
    if s.find('Linux') >= 0:
        base = 'Linux'
        try:
            rev,name = re.compile(' ([^\s]+) \(([^\)]+)\)').search(s).groups()
        except:
            rev,name = "???", "???"
        if s.find('Fedora Core') >= 0:
            distro = 'FC'
        elif s.find('Fedora') >= 0:
            distro = 'FC'
        elif s.find('Red Hat Enterprise Linux') >= 0:
            distro = 'RHEL'
        elif s.find('openSUSE') >= 0:
            distro = 'SuSE'
        elif s.find('Ubuntu') >= 0:
            distro = 'Ubuntu'
        else:
            distro = '???'
        return base, distro, rev, name    
        # e.g.  Linux, RHEL, 5.2, Tikanga
    elif s.find('Darwin') >= 0:
        base = 'Darwin'   #fixme: detect this at runtime
        distro = 'OS X'
        rev = '10.5.5'
        name = 'Leopard'
        return base, distro, rev, name
    else:
        return "???", "???", "???", "???"


def selected_revisions(data):
    
    all = sets.Set()
    for log in data:
        all.add(log['CASA'])

    all_list = []
    for c in all:
        all_list.append(c)
    all_list.sort(reverse=True)

    # The following block selects which versions to use,
    # with lower density as we go back in time.
    # For example with b(base) = 5 and d(density) = 4
    # the following versions selected
    #
    # 0,1,2,3,4,       (density=1    in range < 5)
    # 8,12,16,20,24,   (density=1/4  in range < 25)
    # 32,48,64,80,96   (density=1/16 in range < 125)
    # etc.
    b = 5
    d = 4
    selected = sets.Set()
    for c in range(len(all_list)):
        interval = b
        density = 1
        use_it = False
        for j in range(20):
            # loops until b^20 (i.e. long enough)
            if c < interval and (c % density) == 0:
                use_it = True
                break
            interval *= b;
            density  *= d;

        # Use also
        #   - the oldest log of all times
        #   - the 2.3.0 release
        if all_list[c] == "CASA Version 2.0 Rev 5654" or \
           all_list[c] in known_releases:
            use_it = True

        if use_it:
            selected.add(all_list[c])
            print "Use ", all_list[c]
        else:
            print "Drop", all_list[c]

    # 'selected' now contains the revisions to be
    # used (still sorted).
    return selected
    

class report:
    def __init__(self, reg_dir, report_dir, \
                 revision, \
                 gen_plot=True,   # generate plots?
                 ):

        casalog.showconsole(onconsole=True)

        pl.ioff()  #turn of plots

        result_dir = reg_dir + '/Result/'
        
        data = self.read_log(result_dir, revision)

        if len(data) == 0:
            raise Exception, "No test log files found in %s, cannot determine CASA version" % result_dir

        if revision == 'all':
            # Select only some revisions (in order to reduce length of historical report)
            casas_set = selected_revisions(data)

            # Filter log files
            filtered_data = []
            for log in data:
                if log['CASA'] in casas_set:
                    filtered_data.append(log)
            data = filtered_data

        #
        # collect some general data
        #
        hosts_set    = sets.Set()
        tests_set    = sets.Set()
        self.subtests = {}
        casas_set    = sets.Set()
        for log in data:
            # Use only the hostname as key
            # (the 'platform' string format might change
            #  with time)
            # hosts.add((log['host'], log['platform']))

            tests_set.add   (log['testid'])
            hosts_set.add   (log['host'])
            casas_set.add   (log['CASA'])

            test = log['testid']
            if not self.subtests.has_key(test):
                self.subtests[test] = sets.Set()
                
            if log['type'] == 'exec':
                self.subtests[test].add(('', log['type']))
            else:
                self.subtests[test].add((log['image'],log['type']))
            
        #
        # Identify also hosts from entries in ./Log but no ./Result logfile exist
        if False:
            metalogs = os.listdir(reg_dir + '/Log/')
            for filename in metalogs:
                m = re.compile('-(casa-dev-15|[^-]*)\.log$').search(filename)
                if m != None:
                    h = m.groups()[0]
                    if not h in exclude_host:
                        hosts_set.add(h)
                else:
                    raise Exception, "Illegal filename: %s" % filename

        # sort tests alphabetically and
        # casa revisions chronologically
        # (for display purposes only)
        self.tests = []
        for t in tests_set:
            self.tests.append(t)
        self.tests.sort()

        print "tests =", self.tests
        
        self.casas = []
        for c in casas_set:
            self.casas.append(c)
        self.casas.sort(reverse=True)


        # Get test short description (from latest log)
        self.test_description = {}
        test_date = {}
        for log in data:
            test = log['testid']
            # The following if statement translates to
            # "if the logfile contains the test description
            # and (we didn't know it yet or it's newer than
            # the one we know)"
            if log.has_key('description') and \
               (not self.test_description.has_key(test) or \
                test_date[test] < log['date']):
                
                self.test_description[test] = log['description']
                test_date[test] = log['date']    

        # Get OS string + latest CASA revision per host
        self.platform = {}
        self.casa_revision = {}
        platform_date = {}
        # the platform string layout has changed,
        # use the newest one
        for log in data:
            host = log['host']
            if not self.platform.has_key(host) or \
                   platform_date[host] < log['date']:
                self.platform[host] = log['platform']
                platform_date[host] = log['date']

            v = log['CASA']
            if not self.casa_revision.has_key(host) or \
                   v > self.casa_revision[host]:
                self.casa_revision[host] = v

        #latest_on_stable = self.casa_revision['fc8tst64b']
        latest_on_stable = self.casas[0] # global latest

        if True:
            # this part is now obsolete but not harmful...
            
            # Don't use the very latest revision (it's usually under construction)
            # (or set to latest if only 1 revision exists)
            self.global_latest = max(self.casa_revision.values())
            print "Latest revision =", self.global_latest
            latest2 = {}
            for log in data:
                host = log['host']
                v = log['CASA']
                #if v < self.casa_revision[host] and \
                if v < self.global_latest and \
                       (not latest2.has_key(host) or \
                        v > latest2[host]):
                    latest2[host] = v

            for host in self.casa_revision.keys():
                if not latest2.has_key(host):
                    latest2[host] = self.casa_revision[host]
            

            #print "Latest", self.casa_revision
            #print "2Latest", latest2
            
            self.casa_revision = latest2 

        # we need to loop through the hosts in the
        # same order for each HTML table row, so
        # build a list of unique hosts
        self.hosts_rel=[]  
        self.hosts_devel=[]  
        for host in hosts_set:
            if host.find('tst') >= 0 or \
                   host.find('ma014655') >= 0 :
                self.hosts_rel.append(host)
            else:
                self.hosts_devel.append(host)

        self.hosts_rel.sort()
        self.hosts_devel.sort()
        print "hosts =", self.hosts_rel, self.hosts_devel

        self.hosts = self.hosts_devel[:]
        self.hosts.extend(self.hosts_rel)

        #
        # Done collecting. Generate reports
        #
        html_header = SOURCE_DIR+'/code/xmlcasa/scripts/regressions/admin/stdcasaheader.phtm'
        html_footer = SOURCE_DIR+'/code/xmlcasa/scripts/regressions/admin/stdcasatrail.phtm'
        pagename  = report_dir+'/test-report.html'

        if not os.path.isdir(report_dir):
            os.mkdir(report_dir)
        
        #shutil.copyfile(html_header, pagename)
        fd = open(pagename, "w")
        fd.write('<html></head>')

        print "Get archive size..."
        archive_size = commands.getoutput("du -hs " + result_dir + " | awk '{print $1}'")
        fd.write('<title>CASA regression tests</title>\n')
        fd.write('<body>\n')
        fd.write('Generated on <i>'+time.strftime('%a %Y %b %d %H:%M:%S %Z')+'</i>')

        fd.write('<br>Summary of <i>'+str(len(data))+'</i> tests, archive size is <i>'+ \
                 archive_size + '</i>')
        fd.write('<dl><dt>Jump to<dd><a href="#revision_platform">Revision vs. platform</a> (short)')
        fd.write('<dd><a href="#test_platform">Test vs. platform</a> (short)')
        fd.write('<dd><a href="#revision_platform_full">Revision vs. platform</a> (full)')
        fd.write('<dd><a href="#test_platform_full">Test vs. platform</a> (full)')
        fd.write('</dl>')
        if revision == 'all':
            fd.write('<a href="../CASA_latest/test-report.html">--> Latest test release</a><br>')
        else:
            fd.write('<a href="../CASA/test-report.html">--> All revisions</a><br>')
        fd.write('<center>')

        if revision != 'all':
            a = latest_on_stable.find('build #')
            if a >=0:
                a += len('build #')
                b = latest_on_stable.find(')', a)
                revision = latest_on_stable[a:b]
            else:
                raise Exception, "Could not parse revision number '%s'" \
                      % (latest_on_stable)

            fd.write('<br><h2>Revision '+revision+' only, trunk only</h2>')
            fd.write('<hr>')

            # Filter out all other versions

            self.casas = [latest_on_stable]
            
            data2 = []
            for d in data:
                if d['CASA'] == latest_on_stable:
                    data2.append(d)
            data = data2
                

        # Simple tables
        fd.write('<br><a name="revision_platform"></a>')

        extended = False
        self.generate_host_vs_revision('Revision vs. platform',
                                       reg_dir, report_dir,
                                       self.tests,
                                       False, extended, data, fd)

        self.dump_legend(fd)

        fd.write('<br><a name="test_platform"></a>')
        self.generate_host_vs_test(reg_dir, report_dir, revision,
                                   False, extended, data, fd)

        fd.write('<hr>')
        
        # Detailed tables
        fd.write('<br><a name="revision_platform_full"></a>')
        extended = True
        self.generate_host_vs_revision('Revision vs. platform',
                                       reg_dir, report_dir,
                                       self.tests,
                                       gen_plot, extended, data, fd)

        fd.write('<hr>')

        fd.write('<br><a name="test_platform_full"></a>')
        self.generate_host_vs_test(reg_dir, report_dir, revision,
                                   gen_plot, extended, data, fd)

        fd.write('</center></body>\n')
        fd.write('</html>\n')
        fd.close()
        #os.system('cat ' + html_footer + ' >> '+pagename)
        print "Wrote", pagename
        
        os.system('touch '+report_dir+'/success')
        # because casapy always returns success, even if
        # executing this program failed

    def generate_host_vs_revision(self, heading, reg_dir, report_dir,
                                  tests,             # the test(s) to consider
                                  gen_plot, extended, data, fd):

        result_dir = reg_dir + '/Result'

        status   = {}     # pass/fail/undetermined per (host, revision, test)
        run_date = {}     # latest run             per (host, revision, test)
        l        = {}     # logfilename            per (host, revision, test)
        summary  = {}     # no of pass/fail/undet. per (host, revision)
        for host in self.hosts:
            status[host] = {}
            run_date[host] = {}
            l[host] = {}
            summary[host] = {}
            for casa in self.casas:
                status[host][casa] = {}
                run_date[host][casa] = {}
                l[host][casa] = {}
                summary[host][casa] = {}
                summary[host][casa]['pass'] = 0
                summary[host][casa]['fail'] = 0
                summary[host][casa]['undetermined'] = 0
                for test in tests:
                    status  [host][casa][test] = {}
                    run_date[host][casa][test] = {}
                    l       [host][casa][test] = {}

        for log in data:
            if log['testid'] in tests:
                host = log['host']
                casa = log['CASA']
                test = log['testid']
                if log['type'] == 'exec':
                    subtest = ('', log['type'])
                else:
                    subtest = ((log['image'],log['type']))


                # If we have a 2nd entry for this (host,casa,test)
                # then one of them is obsolete, warn about that.
                if run_date[host][casa][test].has_key(subtest):
                    if run_date[host][casa][test][subtest] < log['date']:
                        print "%s deprecated by %s" %\
                              (l[host][casa][test][subtest], log['logfile'])
                    else:
                        print "%s deprecated by %s" %\
                              (log['logfile'], l[host][casa][test][subtest])

                if not run_date[host][casa][test].has_key(subtest) or \
                       run_date[host][casa][test][subtest] < log['date']:

                    run_date[host][casa][test][subtest] = log['date']
                    l[host][casa][test][subtest] = log['logfile']

                    status[host][casa][test][subtest] = log['status']

        for host in self.hosts:
            for casa in self.casas:
                # Count number of pass/fail/undet.
                for test in tests:
                    for subtest in self.subtests[test]:
                        if status[host][casa][test].has_key(subtest):
                            s = status[host][casa][test][subtest]
                            
                            summary[host][casa][s] += 1 # increments number
                                                        # of s=pass/fail/undet. by 1
                            
                            #print host, casa, test, subtest
                        else:
                            summary[host][casa]['undetermined'] += 1


        fd.write('<h2><I><a name="rev_plat_det">'+heading+'</a></I></h2><br>')
        if extended:
            fd.write('Number of passed / failed / not-run tests.<br>')
            fd.write('If a test was run more than once, the results of the <i>latest run</i> counts.')
            fd.write('<br>')
            fd.write('<TABLE border=1 cellpadding=0 cellspacing=0 summary="Quick view over tests.">\n')
        else:
            fd.write('<TABLE border=1 cellpadding=0 cellspacing=0 summary="Quick view over tests.">\n')

            
        fd.write('<TR><TD></TD>')
        if len(self.hosts_devel) > 0:
            fd.write('<td align=center colspan='+str(len(self.hosts_devel))+'>branches/active</td>')
        if len(self.hosts_rel) > 0:
            fd.write('<td align=center colspan='+str(len(self.hosts_rel))+'>trunk</td>')
        fd.write('</tr>')
        
        fd.write('<TR><TD></TD>')
        for host in self.hosts:
            if self.platform.has_key(host):
                fd.write('<td align=center title="'+self.platform[host]+'">')
            else:
                fd.write('<td align=center>')
            self.dump_host(fd, host, extended)
            fd.write('</td>')

        fd.write('<TD align=center><b>Revision<br>summary</b></TD>')
        fd.write('</TR>\n')    

        for casa in self.casas:
            print "Generate summary for", casa
            fd.write('<TR>')
            if casa in known_releases:
                fd.write('<TD><nobr><b>' + casa + '</b></nobr></TD>')
            else:
                fd.write('<TD><nobr>' + casa + '</nobr></TD>')
            tot_pass = 0
            tot_fail = 0
            tot_unde = 0
            tot_total = 0
            for host in self.hosts:
                passes       = summary[host][casa]['pass']
                failures     = summary[host][casa]['fail']
                undetermined = summary[host][casa]['undetermined']
                total = passes+failures+undetermined
                self.dump_td_start(fd, passes, failures, undetermined, " align=center")
                
                if extended:
                    fd.write('%s / %s / %s (total %s)' %
                             (passes, failures, undetermined, total))
                self.dump_td_end(fd)

                tot_pass += passes
                tot_fail += failures
                tot_unde += undetermined
                tot_total += total

            # summary for revision
            self.dump_td_start(fd, tot_pass, tot_fail, tot_unde, " align=center")
            if extended:
                fd.write('%s / %s / %s (total %s)' %
                         (tot_pass, tot_fail, tot_unde, tot_total))
            self.dump_td_end(fd)

            fd.write('</TR>\n')

        fd.write('</TABLE>\n')


    def generate_host_vs_test(self, reg_dir, report_dir, revision,
                              gen_plot, extended, data, fd):
        fd.write('<h2><I><a name="test_plat_det">Test vs. platform</a></I></h2><br>')
        if extended:
            if revision != 'all':
                if not same_version_per_host:
                    fd.write('For each combination of (test, host) the results of the <i>latest run</i> ')
                    fd.write('is reported. Note: The revision number might vary within the same column/row')
                else:
                    fd.write('For each host the results of the <i>latest available CASA revision</i>, ')
                    fd.write('excluding '+self.global_latest+' which is under construction, is reported. ')                
                    fd.write('<br>')
                    fd.write('If a test was run more than once with a given revision, the results of the <i>latest run</i> is reported. ')
            fd.write('<br>')
            fd.write('<dl><dt>')
            fd.write('<dd>Logfiles contain the Python session\'s output to stdout/stderr, messages sent to casalogger, and the contents of any *.log files created.')
            fd.write('<dd>On execution error the last error message from the log file is shown.')
            fd.write('<dd>Follow the history links to get an overview over how a test outcome correlates with platform and CASA version.')
            
            fd.write('<dd><small>meta-log: Messages from the testing framework, unrelated to the test itself. ')
            fd.write('Used to track problems with the casapy installation, X connection etc.</small>')
            fd.write('</dt></dl>')
            fd.write('<TABLE border=1 cellpadding=0 cellspacing=0 summary="Quick view over tests.">\n')
        else:
            fd.write('<TABLE border=1 cellpadding=0 cellspacing=0>\n')

        summary_subtest = {} # count the number of pass/fail per test and per host
        summary_host = {}
        fd.write('<TR><TD colspan=2 rowspan=2></TD>')

        if len(self.hosts_devel) > 0:
            fd.write('<td align=center colspan='+str(len(self.hosts_devel))+'>branches/active</td>')
        if len(self.hosts_rel) > 0:
            fd.write('<td align=center colspan='+str(len(self.hosts_rel))+'>trunk</td>')
        fd.write('</tr>')
        
        for host in self.hosts:
            summary_host[host] = {}
            summary_host[host]['pass'] = 0
            summary_host[host]['fail'] = 0
            summary_host[host]['undet'] = 0
            summary_host[host]['time'] = 0.0

            if self.platform.has_key(host):
                fd.write('<td align=center title="'+self.platform[host]+'">')
            else:
                fd.write('<td align=center>')
            self.dump_host(fd, host, extended)

            if extended:
                latest_metalog = commands.getoutput('/bin/ls -1tr ' + reg_dir + '/Log/ | grep -E "\-'+host+'.log$" | tail -1')
                if len(latest_metalog) > len('.log'):
                    shutil.copyfile(reg_dir + '/Log/' + latest_metalog, \
                                    report_dir + '/' + latest_metalog)
                    fd.write(' <small><a href="'+latest_metalog+'">latest run</a></small>')

                if same_version_per_host:
                    if self.casa_revision.has_key(host):
                        fd.write('<br><i>' + self.casa_revision[host]+'</i>')
                    else:
                        fd.write('<br><i>???</i>')

            fd.write('</td>')
            
        fd.write('<TD><b>Trunk summary</b></TD>')
        fd.write('</TR>\n')    

        # identify subtests for each test
        subtests = {}
        for test in self.tests:
            subtests[test] = sets.Set()
        subtests_list = {}
            
        for log in data:
            test = log['testid']

            if log['type'] == 'exec':
                subtests[test].add(('', log['type']))
            else:
                subtests[test].add((log['image'],log['type']))

        for test in self.tests:

            # loop through the subtests so that the 'exec' subtest
            # is always processed first
            subtests_list[test] = [s for s in subtests[test]]
            subtests_list[test].sort(cmp=cmp_exec)
            
            print "Subtests for", test, "=", subtests_list[test]

            summary_filename = report_dir+"/summary_"+test+".html"
            fd.write('<TR>')
            fd.write('<TD rowspan='+str(1+len(subtests[test]))+' align=center><big><b>')
            if extended:
                fd.write('<a href="summary_'+test+'.html">'+shorten(test)+'</a>')

                f = open(summary_filename, "w")
                
                f.write('<html><head></head><body><center>\n')

                print "Creating %s..." % (summary_filename)
                
                self.generate_host_vs_revision(test+' execution',
                                               reg_dir, report_dir,
                                               [test],
                                               False, False,
                                               data, f)
                self.dump_legend(f)
                f.write('</center></body></html>')
                f.close()               
            else:
                fd.write('<a href="summary_'+test+'.html">'+shorten(test, 10)+'</a>')
                
            fd.write('</big></b>')
            if extended and self.test_description.has_key(test):
                fd.write('<br><br>' + self.test_description[test].replace("<", "&lt;").replace(">", "&gt"))
            fd.write('</TD>')
            fd.write('</TR>\n')

            for subtest in subtests_list[test]:
                summary_subtest['pass'] = 0
                summary_subtest['fail'] = 0
                summary_subtest['undet'] = 0
                fd.write('<TR>')
                fd.write('<td align=left>')

                if extended:
                    fd.write('<dl><dt>')
                if subtest[1] == 'exec':
                    fd.write('Execution')
                else:
                    txt = {}
                    txt['simple'] = 'Statistics'
                    txt['cube']   = 'Cubefit'
                    txt['pol1']   = 'PolImage1'
                    txt['pol2']   = 'PolImage2'
                    txt['pol4']   = 'PolImage4'
                    if extended:
                        fd.write(shorten(subtest[0]))
                        fd.write('<dt>'+txt[subtest[1]])
                    else:
                        fd.write(shorten(subtest[0], 10))
                        # don't write test type
                if extended:
                    if subtest[1] == 'exec':
                        fd.write('<dd>History:')
                    label = {}
                    label['exec'] = 'Status'
                    if subtest[1] == 'exec':
                        label['time'] = 'Time'
                        label['mvirtual'] = 'Virt. memory'
                        label['mresident'] = 'Res. memory'
                        label['filedesc'] = 'File desc.'

                    for i in ['exec', 'time', 'mvirtual', 'mresident', 'filedesc']:
                        if label.has_key(i):
                            fd.write('<dd>')
                            if subtest[1] == 'exec':
                                basename = 'history-'+test+'-'+i
                                fd.write('<a href="'+basename+'.html">'+label[i]+'</a>')
                            else:
                                basename = ('history-'+test+'-'+subtest[0]+'-'+subtest[1]).replace('/', '-').replace('--', '-')
                                fd.write('<a href="'+basename+'.html">History</a>')

                            twin_plots = open(report_dir+'/'+basename+'.html', 'w')
                            twin_plots.write('<html><head><title>'+basename+'</title></head><body>')
                            twin_plots.write('<img src="'+basename+'-0.png"><br>')
                            twin_plots.write('<img src="'+basename+'-1.png">')
                            twin_plots.write('</body></html>')
                            twin_plots.close()
                    fd.write('</dl>')
                fd.write('</td>')
                
                for host in self.hosts:
                    self.dump_entry(test, subtest, host,
                                    summary_subtest,
                                    summary_host,
                                    data, fd, reg_dir, report_dir,
                                    extended)

                self.dump_td_start(fd, \
                                   summary_subtest['pass'], \
                                   summary_subtest['fail'], \
                                   summary_subtest['undet'], \
                                   ' align=center title="'+test+'"')
                if extended:
                    fd.write('%s / %s / %s' % \
                             (summary_subtest['pass'], \
                              summary_subtest['fail'], \
                              summary_subtest['undet']))
                self.dump_td_end(fd)
                fd.write('</TR>\n')
            # end for each subtest
            if gen_plot:
                self.history_plot(report_dir+'/history-'+test, test, data)
        # end for each test

        fd.write('<TR><TD COLSPAN=2><b>Platform summary</b></TD>')
        total={}
        total['pass'] = 0
        total['fail'] = 0
        total['undet'] = 0
        for host in self.hosts:
            self.dump_td_start(fd, \
                               summary_host[host]['pass'], \
                               summary_host[host]['fail'], \
                               summary_host[host]['undet'], \
                               ' align=center title="'+host+'"')
            if extended:
                fd.write('%s / %s / %s' % (summary_host[host]['pass'], summary_host[host]['fail'], summary_host[host]['undet']))
                fd.write('<br>Total time: %.1f h' % (summary_host[host]['time']/3600.0))
            self.dump_td_end(fd)
            total['pass']  += summary_host[host]['pass']
            total['fail']  += summary_host[host]['fail']
            total['undet'] += summary_host[host]['undet']

        self.dump_td_start(fd, \
                           total['pass'], \
                           total['fail'], \
                           total['undet'], \
                           " align=center")

        if extended:
            fd.write('%s / %s / %s' % (total['pass'], total['fail'], total['undet']))
        self.dump_td_end(fd)
        fd.write('</TR>\n')



        
        fd.write('</TABLE>\n')

    def dump_legend(self, fd):
        fd.write('Legend: <table border="1" cellpadding=0 cellspacing=0><tr>')
        self.dump_td_start(fd, 1, 0, 0, " align=center")
        fd.write('All tests passed')
        self.dump_td_end(fd)
        fd.write('</tr><tr>')
        
        self.dump_td_start(fd, 1, 0, 1, " align=center")
        fd.write('Some test(s) not run<br>No failures')
        self.dump_td_end(fd)
        fd.write('</tr><tr>')
        self.dump_td_start(fd, 1, 1, 0, " align=center")
        fd.write('Some test(s) passed<br>Some test(s) failed')
        self.dump_td_end(fd)
        fd.write('</tr><tr>')
        self.dump_td_start(fd, 0, 1, 0, " align=center")
        fd.write('All tests failed')
        self.dump_td_end(fd)        
        fd.write('</tr></table>')
        
    def dump_entry(self, test, subtest, host, \
                   summary_subtest, summary_host, \
                   data, fd, reg_dir, report_dir, \
                   extended):

        result_dir = reg_dir + '/Result'

        # Find the latest run
        log = None
        latest_date = '0000'
        for l in data:
            if l['host'] == host and \
               (same_version_per_host == False or l['CASA'] == self.casa_revision[host]) and \
               l['testid'] == test and \
               l['type'] == subtest[1] and \
               (subtest[0] == '' or l['image'] == subtest[0]):
                if l['date'] > latest_date:
                    latest_date = l['date']
                    log = l
        coords = ' title="'+test+' \\ '+host+'"'
        
        if log != None:
            #print log
            summary_host[host][log['status']] += 1
            if host.find('tst') >= 0 or \
               host.find('ma014655') >= 0:
                summary_subtest[log['status']] += 1

            if log['type'] != 'exec':
                coords = ' title="' + log['image']+'-'+log['type'] + ' \\ '+host+'"'
            self.dump_td_start(fd,
                               1,
                               log['status'] == 'fail',
                               0, coords)
                 
            if extended:
                #link to resultlog and runlog files
                if log['type'] == 'exec':
                    fd.write('<a href="'+log['logfile']+'">')
                else:
                    # delegate to subpage with more details
                    basename = (test+'-'+host+'-'+subtest[0]+'-'+subtest[1]).replace('/', '-').replace('--', '-')
                    fd.write('<a href="'+basename+'.html">')
                    f = open(report_dir+'/'+basename+'.html', 'w')
                    f.write('<html><head><title>'+basename+'</title></head><body>')
                    f.write('<a href="'+log['logfile']+'">log</a>')
                    
                # and copy the log file and runlog to the target dir
                from_dir = os.path.dirname('%s/%s' % (result_dir, log['logfile']))
                to_dir   = os.path.dirname('%s/%s' % (report_dir, log['logfile']))

                if not os.path.isdir(to_dir):
                    os.makedirs(to_dir)
                shutil.copyfile('%s/%s' % (result_dir, log['logfile']), \
                                '%s/%s' % (report_dir, log['logfile']))

                fd.write('%s' % log['status'])
                fd.write('</a>')
                if log.has_key('reason'):
                    fd.write(': '+log['reason'])
                if log.has_key('runlog'):
                    if os.path.isfile(from_dir+'/'+log['runlog']):
                        shutil.copyfile(from_dir+'/'+log['runlog'],
                                        to_dir+'/'+log['runlog'])
                    else:
                        print >> sys.stderr, \
                              "Error: %s: Missing file: %s" % \
                              (log['logfile'], log['runlog'])
                        commands.getstatusoutput('touch '+ to_dir+'/'+log['runlog'])
                        #fixme!!! touch

                    fd.write('<br>')
                    fd.write('<a href='+os.path.dirname(log['logfile'])+'/'+log['runlog']+'>log ')
                    fd.write('(')
                    # don't repeat CASA revision: fd.write('rev. ' % log['CASA'].split('#')[1].split(')')[0])
                    d = log['date'].split('_')
                    fd.write('%s-%s-%s %s:%s)</a>' % \
                             (d[0], d[1], d[2], d[3], d[4]))

                fd.write('<BR>')
                if log['type'] == 'exec':
                    
                    summary_host[host]['time'] += float(log['time'])
                    
                    if log['status'] == 'pass':
                        fd.write('Time of run: %.2f s'% float(log['time']))
                        if log.has_key('resource'):

                            profile_html = 'profile-'+test+'-'+host+'.html'
                            profile_png  = 'profile-'+test+'-'+host+'.png'
                            profile_cpu_png  = 'profile_cpu-'+test+'-'+host+'.png'

                            t, mvirtual, mresident, nfiledesc, \
                               cpu_us, cpu_sy, cpu_id, cpu_wa = \
                               self.parse_resources(log)

                            if (len(mvirtual) > 0):
                                if log['version'] == '1':
                                    max_virtual  = "%.0f MB" % (max(mvirtual)/1024.0/1024)
                                    max_resident = "%.0f MB" % (max(mresident)/1024.0/1024)
                                    max_filedesc = "%d" % max(nfiledesc)
                                else:
                                    max_virtual  = "%.0f MB" % (max(mvirtual))
                                    max_resident = "%.0f MB" % (max(mresident))
                                    max_filedesc = "%d" % max(nfiledesc)
                            else:
                                max_virtual  = "N/A"
                                max_resident = "N/A"
                                max_filedesc = "N/A"
                                
                                                                
                            fd.write('<br><a href='+profile_html+'>')
                            fd.write('Memory(max): ' + max_virtual + ' virt. / ' + max_resident + ' res.')
                            fd.write('</a>')

                            fd.write('<br><a href='+profile_html+'>')
                            fd.write('File desc.(max): ' + max_filedesc)
                            fd.write('</a>')

                            fd.write('<br><a href='+profile_html+'>')
                            
                            if (len(cpu_us) > 0):
                                avg_cpu_us = "%.0f" % (sum(cpu_us)*1.0/len(cpu_us))
                                avg_cpu_sy = "%.0f" % (sum(cpu_sy)*1.0/len(cpu_us))
                                avg_cpu_id = "%.0f" % (sum(cpu_id)*1.0/len(cpu_us))
                                avg_cpu_wa = "%.0f" % (sum(cpu_wa)*1.0/len(cpu_us))
                                fd.write("CPU(avg): %s%%us %s%%sy %s%%wa %s%%id" % \
                                         (avg_cpu_us, \
                                          avg_cpu_sy, \
                                          avg_cpu_wa, \
                                          avg_cpu_id))
                            else:
                                fd.write('CPU(avg): N/A')
                            fd.write('</a>')

                            self.create_profile_html(t, mvirtual, mresident, nfiledesc, \
                                                     cpu_us, cpu_sy, cpu_id, cpu_wa, \
                                                     report_dir,
                                                     profile_png, profile_html, \
                                                     profile_cpu_png, \
                                                     test, host)
                    elif log['status'] == 'fail':
                        # filter depends on error message format
                        sed_filter = 's/STDERR [^\:]*:[^\:]*: //g'
                        last_error_message = commands.getoutput( \
                            "egrep '^STDERR' "+to_dir+'/'+log['runlog']+" | tail -1 | sed '"+sed_filter+"'")

                        nexttolast_error_message = commands.getoutput( \
                            "egrep '^STDERR' "+to_dir+'/'+log['runlog']+" | tail -2 | head -1 | sed '"+sed_filter+"'")
                        fd.write('<small>')
                        fd.write(shorten(nexttolast_error_message, 40) + '<br>')
                        fd.write(shorten(last_error_message, 40))
                        fd.write('</small>')
                    # endif status == ...

                # endif type == 'exec'
                else:
                    if log['type'] == 'simple':
                        if log.has_key('image_min'):
                            # if image was not produced, these statistics were not calculated
                            
                            f.write('<pre>')
                            f.write('Image    min: %g\nmax: %g\nrms: %g \n' %
                                    (float(log['image_min']),
                                     float(log['image_max']),
                                     float(log['image_rms'])))
                            f.write('Template min: %g\nmax: %g\nrms: %g \n' %
                                    (float(log['ref_min']),
                                     float(log['ref_max']),
                                     float(log['ref_rms'])))
                        f.write('</pre>')
                    elif log['type'] == 'cube':
                        if log.has_key('image_x'):
                            f.write('<pre>')
                            f.write('On image    \n  optimized coord:\n   [%g,%g]\n  FWHM: %.6f\n\nfit #1\n  optimized coord:\n   [%g,%g]\n  FWHM: %g \n\n' %
                                    (float(log['image_x']),
                                     float(log['image_y']),
                                     float(log['image_fwhm']),
                                     float(log['image_fit1_x']),
                                     float(log['image_fit1_y']),
                                     float(log['image_fit1_fwhm'])))
                            f.write('On Template \n  optimized coord:\n   [%g,%g]\n  FWHM: %.6f\n\nfit #1\n  optimized coord:\n   [%g,%g]\n  FWHM: %g \n'%
                                    (float(log['ref_x']),
                                     float(log['ref_y']),
                                     float(log['ref_fwhm']),
                                     float(log['ref_fit1_x']),
                                     float(log['ref_fit1_y']),
                                     float(log['ref_fit1_fwhm'])))
                            f.write('</pre>')
                    elif log['type'] == 'pol1' or \
                         log['type'] == 'pol2' or \
                         log['type'] == 'pol4':
                        for pol in ['I', 'Q', 'U', 'V']:
                            if log.has_key('image_'+pol+'_ra'):
                                f.write('<pre>')
                                for i in [0, 1]:
                                    f.write('Pol %s: %s:\nra %s  dec %s \nbmax %s bmin %s \nbpa %s flux %s' %
                                             (pol, \
                                              ['Component found', 'Template'][i],
                                              log[['image_', 'ref_'][i]+pol+'_ra'],
                                              log[['image_', 'ref_'][i]+pol+'_dec'],
                                              log[['image_', 'ref_'][i]+pol+'_bmax'],
                                              log[['image_', 'ref_'][i]+pol+'_bmin'],
                                              log[['image_', 'ref_'][i]+pol+'_bpa'],
                                              log[['image_', 'ref_'][i]+pol+'_flux']))
                                    f.write('<br>')
                                f.write('</pre>')
                            if log.has_key('image_'+pol+'_min'):
                                f.write('<pre>')                        
                                f.write('Image    min: %g\nmax: %g\nrms: %g \n' %
                                         (float(log['image_'+pol+'_min']),
                                          float(log['image_'+pol+'_max']),
                                          float(log['image_'+pol+'_rms'])))
                                f.write('Template min: %g\nmax: %g\nrms: %g \n' %
                                         (float(log['ref_'+pol+'_min']),
                                          float(log['ref_'+pol+'_max']),
                                          float(log['ref_'+pol+'_rms'])))
                                f.write('</pre>')
                    else:
                        raise Exception, 'Unknown test type '+log['type']
                    
                    self.link_to_images(log, from_dir, to_dir, f)
                    f.write('</body></html>')
                    f.close()
                #endif type != 'exec'
            # endif extended
        # endif log != None
        else:
            fd.write('<td align=center '+coords+'>')
            if extended:
                if subtest[1] == 'exec':
                    fd.write('Test not run')
                else:
                    fd.write('Test not done')
            summary_host[host]['undet'] += 1
            if host.find('tst') >= 0 or \
                   host.find('ma014655') >= 0:
                summary_subtest['undet'] += 1

        if extended and log != None:
            if not same_version_per_host:
                fd.write('<br>'+log['CASA'])
            if log.has_key('data_version'):
                fd.write('<br>Data: '+log['data_version'])


        if extended and subtest[1] == 'exec':
            framework_log = 'run-'+test+'-'+host+'.log'
            if os.path.isfile(reg_dir + '/Log/' + framework_log):
                shutil.copyfile(reg_dir + '/Log/' + framework_log, \
                                report_dir + '/' + framework_log)
                fd.write('<br><small><a href="'+framework_log+'">meta-log</a></small>')
                
        self.dump_td_end(fd)            

    def link_to_images(self, log, from_dir, to_dir, fd):
        # Add link to images for image tests
        i = 1
        while log.has_key('imagefile_'+str(i)):
            fn = log['imagefile_'+str(i)]
            print "Copy image:", fn
            if os.path.isfile(from_dir+'/'+fn):
                shutil.copyfile(from_dir+'/'+fn,
                                to_dir  +'/'+fn)
            fd.write('<br>Image: '+fn+'<img src="'+os.path.dirname(log['logfile'])+'/'+fn+'">')
            i = i + 1

    def dump_td_start(self, fd, passed, failed, undetermined, extra=""):
        # The overall status is undetermined iff there
        # are only passed + still undetermined tests
        if failed > 0:
            if passed == 0:
                color = 'ff0000'
            else:
                color = 'ff6058'
        elif undetermined > 0:
            if passed > 0:
                color = 'ffff80'
            else:
                color = 'ffffff'
        else:
            color = '60ff60'

        if colormania:
            fd.write('<TD BGCOLOR='+color)
        else:
            fd.write('<TD><font COLOR='+color)
        fd.write('%s>' % extra)

    def dump_td_end(self, fd):
        if colormania:
            fd.write('</TD>')
        else:
            fd.write('</font></TD>')

    def parse_resources(self, log):
        # expected format:
        # version 1: t0,mv0,mr0,nf0;t1,mv1,mr1,nf1;...
        # version 2: t0,mv0,mr0,nf0,cpuus0,cpusy0,cpuid0,cpuwa0;...
        samples = log['resource'].split(';')
        t = []
        mvirtual = []
        mresident = []
        nfiledesc = []
        cpu_us = []
        cpu_sy = []
        cpu_id = []
        cpu_wa = []
        
        for s in samples:
            if (s != ''):
                ss = s.split(',')
                t.append(float(ss[0]))
                mvirtual.append(float(ss[1]))
                mresident.append(float(ss[2]))
                nfiledesc.append(int(ss[3]))
                if log['version'] != '1':
                    cpu_us.append(float(ss[4]))
                    cpu_sy.append(float(ss[5]))
                    cpu_id.append(float(ss[6]))
                    cpu_wa.append(float(ss[7]))
                    
        return t, mvirtual, mresident, nfiledesc, \
               cpu_us, cpu_sy, cpu_id, cpu_wa

    def read_log(self, result_dir, revision):
        #
        #  Convert logfiles in a list of dictionaries
        #
        #alllogs=os.listdir(result_dir)

        find_cmd = 'find '+result_dir+' -name result\*.txt'
        findout = commands.getoutput(find_cmd)
        alllogs = ['']
        if(findout != ''):
            alllogs=findout.split('\n')

        print "Parsing %s logfiles..." % str(len(alllogs))
        # It takes some time, seems to be dominated by i/o
        
        #print alllogs

        data = []
        i = 0
        for logfile in alllogs:
            i += 1
            if (i % 100) == 0:
                sys.stdout.write('.')
                sys.stdout.flush()
            #print "match:", logfile
            fd = open(logfile, "r")
            data_file = {}
            lineno = 0
            line = fd.readline().rstrip() ; lineno += 1
            data_file['logfile'] = logfile.split(result_dir)[1].lstrip('/')
            while line:
                #print line
                try:
                    k, v, c = re.compile(r"""
                    ^(\w+)              # key
                    \s*                 # whitespace
                    =                   # =
                    \s*                 # whitespace
                    ('.*'|[^#']+)       # value
                    \s*                 # whitespace
                    \#(.*)$             # comment
                    """, re.VERBOSE).search(line).groups()
                except:
                    raise Exception("%s:%d: Cannot parse '%s'" % \
                                    (logfile, lineno, line))
                k = k.strip()
                v = v.strip(" '")
                c = c.strip()
                #print k, "=", v, "=", c
                #data_file[k] = (v, c)  # include comments
                data_file[k] = v
                # next line
                line = fd.readline().rstrip()
                lineno += 1
                
            fd.close()

            # Test for mandatory entries' existence
            is_valid = True
            for key in ['host', 'testid', 'CASA', 'type', 'status']:
                if not data_file.has_key(key):
                    is_valid = False
                    print >> sys.stderr, \
                          "Warning: %s: Missing key: %s" % (logfile, key)

            # Mandatory entries for passed tests
            for req in [{'simple': ['image_min', 'image_max', 'image_rms', \
                                    'ref_min'  , 'ref_max'  , 'ref_rms'] },
                        {'cube': ['image_x', 'image_y', 'image_fwhm', \
                                  'image_fit1_x', 'image_fit1_y', 'image_fit1_fwhm', \
                                  'ref_x', 'ref_y', 'ref_fwhm', \
                                  'ref_fit1_x', 'ref_fit1_y', 'ref_fit1_fwhm',] }]:
                for type in req:
                    for key in req[type]:
                        if data_file['type'] == type and \
                           data_file['status'] == 'pass' and \
                           not data_file.has_key(key):
                            is_valid = False
                            print >> sys.stderr, \
                                  "Warning: %s: type=%s ; missing keys %s" % \
                                  (logfile, type, key)

            # Filter out test results where component
            # could not be found in reference image
            if is_valid and data_file['type'] in ['pol1','pol2','pol4']:
                found_ref = False
                for k in data_file.keys():
                    if len(k) > 4 and k[:4] == 'ref_':
                        found_ref = True
                        
                if not found_ref:
                    is_valid = False

            if is_valid and \
                   not data_file['host'] in exclude_host and \
                   not (data_file['testid'] in exclude_test \
                        and \
                        (data_file['host'].find('tst') >= 0 or \
                         data_file['host'].find('ma014655') >= 0 \
                         )):                # ignore pc011896 for now; it is out of date

                if revision == 'all' or \
                       data_file['host'].find('tst') >= 0 or \
                       data_file['host'].find('ma014655') >= 0:

                    data.append(data_file)
                    
        # end for each logfile
        
        return data

    def dump_host(self, fd, host, extended):
        if self.platform.has_key(host):
            base, dist, rev, name = distribution(self.platform[host])
        else:
            base, dist, rev, name = ("???", "???", "?", "???")
        fd.write('<b>')
        if extended:
            fd.write(base + '<br>' + dist + ' ' + rev + '</b>')
            fd.write('<br>' + name + '<br>')
            if self.platform.has_key(host):
                fd.write(architecture(self.platform[host]))
            else:
                fd.write("???");
            fd.write('</b><br>' + host)
        else:
            fd.write(dist + '<br>' + rev + '</b><br>')
            if self.platform.has_key(host):
                arch = architecture(self.platform[host])
                fd.write(re.sub('.* ', '', arch).replace('-',''))
            else:
                fd.write("???");

        
    def history_plot(self, png_filename_prefix, test, data):
        plotdata = {}      # the type is
                           # dictionary of (exec, time, mvirtual, mresident, filedesc,
                           #                [image]-[subtest])
                           #    dictionary of hosts:
                           #       list of
                           #          tuple (date, exectime)
        plotdata['exec'] = {}
        plotdata['time'] = {}
        plotdata['mresident'] = {}
        plotdata['mvirtual'] = {}
        plotdata['filedesc']  = {}

        label={}
        label['time'] = 'Execution time (s)'
        label['mresident'] = 'Peak resident memory (Bytes)'
        label['mvirtual'] = 'Peak virtual memory (Bytes)'
        label['filedesc'] = 'Max no. of open file descriptors'

        hosts = sets.Set()
        for log in data:
            if log['testid'] == test:

                if log['type'] != 'exec':
                    t = log['image'].replace('/', '-')+'-'+log['type']
                    if not plotdata.has_key(t):
                        plotdata[t] = {}
                        for h in hosts:
                            plotdata[t][h] = []
                else:
                    t = log['type']

                if not hosts.__contains__(log['host']):
                    hosts.add(log['host'])
                    for key in plotdata.keys():
                        if not plotdata[key].has_key(log['host']):
                            plotdata[key][log['host']] = []

                #print t

                # read the number after 'build #'
                # or after 'Rev ' (older versions)
                a = log['CASA'].find('build #')
                if a >=0:
                    a += len('build #')
                    b = log['CASA'].find(')', a)
                    revision = log['CASA'][a:b]
                else:
                    a = log['CASA'].find('Rev ')
                    if a >=0:
                        a += len('Rev ')
                        revision = log['CASA'][a:]
                    else:
                        raise Exception, "%s: Could not parse revision number '%s'" \
                              % (log['logfile'],log['CASA'])
                    
                # regression status (execution and image tests)
                plotdata[t][log['host']].append(
                    (log['date'],revision, [0,1][log['status']=='pass']) )

                # track the following only for successful runs
                if log['status'] == 'pass' and \
                       log['type'] == 'exec':

                    plotdata['time'][log['host']].append(
                        (log['date'],revision, log['time']) )

                    if log.has_key('resource'):
                        t, mvirtual, mresident, nfiledesc, \
                           dummy, dummy, dummy, dummy = \
                           self.parse_resources(log)
                        # discards CPU usage
                        if (len(mresident) > 0):
                            plotdata['mresident'][log['host']].append(
                                (log['date'],revision, max(mresident)) )
                        if (len(mvirtual) > 0):
                            plotdata['mvirtual'][log['host']].append(
                                (log['date'],revision, max(mvirtual)) )
                        if (len(nfiledesc) > 0):
                            plotdata['filedesc'][log['host']].append(
                                (log['date'],revision, max(nfiledesc)) )
        plot_symbols = ['o',
                        '^',
                        'v',
                        '<',
                        '>',
                        's',
                        '+',  #unsupported: '*',
                        'x',
                        'D',
                        'd',
                        '1',
                        '2',
                        '3',
                        '4',
                        'h',
                        'H',
                        'p',
                        '|']
        for key in plotdata.keys():

            # plot both * as fct of date and * as fct of revision
            for time_data in [0, 1]:
                pl.close()
                pl.clf()
                fig = pl.figure(figsize=(14,6))

                ax = fig.add_subplot(111)
                legend = []
                total_runs = 0  # for this test on any host
                hostno = 0
                for host in hosts:
                    #sort by dates
                    plotdata[key][host].sort(cmp=lambda a,b:cmp(a[0], b[0]))
                    x = []
                    y = []
                    for date,rev,t in plotdata[key][host]:
                        if time_data == 0:
                            x.append(rev)
                        else:
                            x.append(datetime.datetime.strptime(date, '%Y_%m_%d_%H_%M'))
                        y.append(t)
                    if time_data == 0:
                        ax.plot(x, y, plot_symbols[hostno % len(plot_symbols)]+'-.')
                    else:
                        x = pl.date2num(x)
                        ax.plot_date(x, y, plot_symbols[hostno % len(plot_symbols)]+'-.',
                                     xdate=True, ydate=False)
                        
                    total_runs += len(x)
                    legend.append(host)
                    hostno += 1

                if time_data == 0:
                    pass
                else:
                    fig.autofmt_xdate()
                    ax.xaxis.set_major_formatter(pl.DateFormatter("%d-%b-%Y"))

                if label.has_key(key):
                    # time or memory or filedesc plot
                    pl.ylabel(label[key])
                    if time_data == 0:
                        pl.title(test + '\n(' + str(total_runs) + [' successful', ''][key=='exec']+ ' runs)')
                else:
                    # pass/fail tests
                    if time_data == 0:
                        pl.title(test + ' ' + key + '\n(' + str(total_runs) + [' successful', ''][key=='exec']+ ' runs)')

                v = ax.axis()

                if not label.has_key(key):
                    ax.set_yticklabels([])
                    pl.text(v[0], 1, 'pass -', fontsize=20, \
                            horizontalalignment='right', \
                            verticalalignment='center')
                    pl.text(v[0], 0, 'fail -', fontsize=20, \
                            horizontalalignment='right', \
                            verticalalignment='center', name='sans')
                    ax.axis([v[0], v[1], -0.5, 1.5])
                else:
                    ax.axis([v[0], v[1], 0, v[3]])

                ax.legend(legend, loc='center left', shadow=True)
                if time_data == 0:
                    pl.xlabel('SVN revision')
                else:
                    pl.xlabel('Date')
                
                fn = png_filename_prefix+'-'+key+'-'+str(time_data)+'.png'
                print "Saving %s..." % fn,
                sys.stdout.flush()
                fig.savefig(fn)
                print "done"
                sys.stdout.flush()

    def create_profile_html(self, t, y11, y22, numfile, \
                            cpu_us, cpu_sy, cpu_id, cpu_wa, \
                            report_dir, \
                            png_filename, html_filename, \
                            png_cpu_filename, \
                            testname, host):
        # CPU profile plot
        pl.clf()
        if len(t) == len(cpu_us):  # memory and/or CPU data may not be available
            pl.plot(t,cpu_us,lw=1)
            pl.plot(t,cpu_id,lw=1)
            pl.plot(t,cpu_wa,lw=1)
            pl.plot(t,cpu_sy,lw=1)

        if len(t) > 0:
            pl.axis([1.1*min(t)-0.1*max(t), 1.1*max(t)-0.1*min(t), -5, 105])
        else:
            pl.axis([0, 1, -5, 105])
            
        pl.xlabel('time (sec)')
        pl.ylabel('CPU usage (percent)')
        font=FontProperties(size='small')
        pl.legend(('user', 'idle', 'wait', 'system'),
                  loc=[0.7,0.85], prop=font)

        pl.title('Machine CPU usage for '+testname+' on '+host)
        pl.savefig(report_dir + '/' + png_cpu_filename);



        # Memory profile plot
        pl.clf()
        pl.plot(t,y11,lw=2)
        pl.plot(t,y22,lw=2)
        if len(y11) > 0 and len(y22) > 0:
            if max(y11)>=max(y22):
                pl.axis([0.9*min(t),1.1*max(t),0,1.1*max(y11)])
            else:
                pl.axis([0.9*min(t),1.1*max(t),0,1.1*max(y22)])
        else:
            pl.axis([0, 1, 0, 1])
        pl.xlabel('time (sec)')
        pl.ylabel('memory footprint') #note virtual vs. resident
        font=FontProperties(size='small')
        pl.legend(('virtual','resident'),loc=[0.7,0.85], prop=font)
        ax2 = pl.twinx()
        pl.ylabel('No of open File Descriptors')
        ax2.yaxis.tick_right()
        pl.plot(t,numfile, 'r-.',lw=2)
        pl.legend(['No. of Open FDs'],loc=[0.7,0.8], prop=font)
        pl.title('memory usage of casapy for '+testname+' on '+host)

        pl.savefig(report_dir + '/' + png_filename);


        # Generate HTML
        ht=htmlPub(report_dir + '/' + html_filename, 'Profile of '+testname)

        body1=['<pre>CPU profile of run of %s at %s </pre>'%(testname,time.strftime('%Y/%m/%d/%H:%M:%S'))]
        body2=['']
        ht.doBlk(body1, body2, png_cpu_filename, 'CPU/Memory profiles')

        body1=['<pre>Memory profile of run of %s at %s </pre>'%(testname,time.strftime('%Y/%m/%d/%H:%M:%S'))]
        body2=['']
        ht.doBlk(body1, body2, png_filename, '')
        
        ht.doFooter()
