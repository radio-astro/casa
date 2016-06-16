import os
import sys
import shutil
import unittest
from time import sleep
import datetime as dt

##########################################################################
# These tests were adapted from the ADMIT code tree
# admit/admit/test/unittest_*.py
##########################################################################

print "*** No tests will run if admit module is not installed in casa"
admit_import = True
try:
    import admit
    from admit.util.AdmitLogging import AdmitLogging as logging
    from admit.Admit import Admit as Project
    from admit.AT import AT
    from admit.bdp.BDP import BDP
    from admit.bdp.File_BDP import File_BDP
    import admit.util.bdp_types as bt
except ImportError:
    admit_import = False

# Paths for data
#datapath = os.environ.get('CASAPATH').split()[0] + "/data/regression/unittest/admit/"
#datapath = ""
#print 'admit tests will use data from '+ datapath

class admit_test_base(unittest.TestCase):
    """ For tests which require an admit.Project instance """

    # test initialization
    def setUp(self):
        self.assertTrue(admit_import)

        # sometimes CWD is set to self.outputDir that is deleted by
        # tearDown() function, then we need to change back to the parent dir
        try:
            os.getcwd()
        except OSError:
            os.chdir('..')

        self.outputDir = "AdmitCodeTest.admit"
        self.project = Project(self.outputDir)

    # clean up
    def tearDown(self):
        if os.path.exists(self.outputDir):
            # create blank project then delete it!
            # otherwise "Device or resource busy" on files in dir
            self.project = Project(self.outputDir)
            shutil.rmtree(self.outputDir)

# -----------------------------------------------------------------------

class admit_test_base2(unittest.TestCase):
    """ For tests which require admit.Project """

    # test initialization
    def setUp(self):
        self.assertTrue(admit_import)

        # sometimes CWD is set to self.outputDir that is deleted by
        # tearDown() function, then we need to change back to the parent dir
        try:
            os.getcwd()
        except OSError:
            os.chdir('..')

        self.outputDir = "/tmp/AdmitCodeTest_%d.admit" % os.getpid()
        admit.Project = Project(self.outputDir)

    # clean up
    def tearDown(self):
        basedir = admit.Project.baseDir
        del admit.Project
        if os.path.exists(basedir):
            shutil.rmtree(basedir)

# ===========================================================================

class admit_test_Project(admit_test_base):
    """ Test basic Admit Project class functionality """
    # admit/admit/test/unittest_Admit.py

    def test_Project_strlen(self):
        """ test Project.__str__, __len__ """
        self.project.__str__()
        # to get the number of tasks
        ret = self.project.__len__()  # should be 0
        self.assertEqual(ret, 0)

    def test_Project_logging(self):
        """ test Project.setlogginglevel(), getlogginglevel(), and __del__ """
        self.project.setlogginglevel(logging.ERROR)
        ret = self.project.getlogginglevel()  # logging.ERROR value is 40
        # logging shutdown
        self.project.__del__()
        self.assertEqual(ret, logging.ERROR)

    def test_Project_mkdir(self):
        """ test Project.mkdir() """
        # temp directory name
        dir = '/tmp/admit_unit_test_%s' % os.getpid()
        self.assertFalse(os.path.exists(dir))
        self.project.mkdir(dir)
        self.assertTrue(os.path.exists(dir))
        # change back to preset current dir
        self.project.tesdir()
        # cleanup
        if os.path.exists(dir):
            os.rmdir(dir)

    def test_Project_addtask(self):
        """ test Project.addtask(), __getitem__() """
        # add first task
        task = admit.File_AT(touch=True)
        task.setkey("file", "File.dat")
        tid1 = self.project.addtask(task)
        # add another task
        task = admit.Flow11_AT(alias="at" + self.project.baseDir[-2]) # at1 or at2
        task.setkey("file", "Flow11.dat")
        tid2 = self.project.addtask(task, [(tid1,0)])

        # to get the AT with task_id = 0
        at = self.project.__getitem__(0)
        self.assertIsInstance(at, admit.File_AT)
        # to get the AT with task_id = 1
        at = self.project.__getitem__(1)
        self.assertIsInstance(at, admit.Flow11_AT)

    def test_Project_getFM(self):
        """ test Project.getFlow(), getManager() """
        fm = self.project.getFlow()
        self.assertIsInstance(fm, admit.Flow)
        mg = self.project.getManager()
        self.assertIsInstance(mg, admit.Manager)

    def test_Project_findtask(self):
        """ test Project.findtask() """
        # add first task
        task = admit.File_AT(touch=True)
        task.setkey("file", "File.dat")
        tid1 = self.project.addtask(task)
        # add another task
        task = admit.Flow11_AT()
        task.setkey("file", "Flow11.dat")
        tid2 = self.project.addtask(task, [(tid1,0)])
        tasks = self.project.findtask(lambda at: at.id() < 100)
        # check class types of tasks
        self.assertIsInstance(tasks[0], admit.File_AT)
        self.assertIsInstance(tasks[1], admit.Flow11_AT)

    def test_Project_dir(self):
        """ test Project.dir() """
        # dir() has trailing '/'
        baseDir = os.path.join(os.getcwd(), self.outputDir) + '/'
        self.assertEqual(baseDir, self.project.dir())

    def test_Project_userdata(self):
        """ test Project.userdata() and get() """
        val = ['test1', 1, 'admit']

        task = admit.Flow11_AT()
        task._userdata = {}
        task._userdata['admit_unit_test'] = val
        self.project.addtask(task)
        self.project.userdata()
        self.assertEqual(val, self.project.get('admit_unit_test'))

    def test_Project_script(self):
        """ test Project.script() """
        # add one task
        task = admit.File_AT(touch=True)
        task.setkey("file", "File.dat")
        tid1 = self.project.addtask(task)
        # add another task
        task = admit.Flow11_AT()
        task.setkey("file", "Flow11.dat")
        tid2 = self.project.addtask(task, [(tid1,0)])

        name = '/tmp/test_script.%s' % os.getpid()
        self.project.script(name)
        # cleanup
        if os.path.exists(name):
            os.remove(name)

    def test_Project_showsetkey(self):
        """ test Project.showsetkey() """
        # add one task
        task = admit.File_AT(touch=True)
        task.setkey("file", "File.dat")
        tid1 = self.project.addtask(task)
        # add another task
        task = admit.Flow11_AT()
        task.setkey("file", "Flow11.dat")
        tid2 = self.project.addtask(task, [(tid1,0)])
 
        keyfile = '/tmp/test_showsetkeys.%s' % os.getpid()
        self.project.showsetkey(keyfile)
        self.assertTrue(os.path.isfile(keyfile))
        # cleanup
        if os.path.exists(keyfile):
            os.remove(keyfile)

    def test_Project_set(self):
        """ test set(), get(), and has() on userData """
        # add one task
        task = admit.File_AT(touch=True)
        name = "File.dat"
        task.setkey("file", name)
        self.project.addtask(task)
        # try to get the value of 'file' - should be None because it is not in user data
        self.assertIsNone(self.project.get('file'))

        # set the key
        key = 'admit_unit_test'
        val = ['test2', 2, 'admit']
        userdata = {}
        userdata[key] = val
        # set()
        self.project.set(**userdata)
        self.assertEqual(self.project.get('admit_unit_test'), val)

        # has()
        self.assertTrue(self.project.has('admit_unit_test'))

    def test_Project_find_bdp(self):
        """ test find_bdp() """
        project = Project()

        # add one task
        task = admit.File_AT(touch=True)
        name = "File.dat"
        task.setkey("file", name)
        project.addtask(task)  # add task
        # now add an output bdp
        obdp = admit.File_BDP('Test')
        task.addoutput(obdp)
        self.project.addtask(task)

        # find_bdp() will search Admit output data directory for *.bdp files
        # should return an empty list since no *.bdp file created by this test
        ret = self.project.find_bdp()
        self.assertTrue(len(ret) == 0)

    def test_Project_find_files(self):
        """ test find_files() """

        # find_files() will search Admit data directory for files with a given pattern
        ret = self.project.find_files(pattern="*.log")
        outdir = self.project.dir()
        self.assertTrue(len(ret) >= 0)

    def test_Project_setdir(self):
        """ test setdir() """
        # temp directory name
        dir = '/tmp/test_setdir_%s' % os.getpid()
        self.assertFalse(os.path.exists(dir))  # should be false
        self.project.setdir(dir)
        self.assertTrue(os.path.exists(dir))

        # change back to preset current dir
        self.project.tesdir()
        # cleanup
        if os.path.exists(dir):
            os.rmdir(dir)

    def test_Project_tesdir(self):
        """ test tesdir() """
        self.project.tesdir()
        cwd1=''
        try:
            cwd1 = os.getcwd()
        except OSError:
            print "\nCannot get current work directory."
        cwd2 = self.project.currDir
        self.assertEqual(cwd1, cwd2)

    def test_Project_markstalefrom(self):
        """ test _markstalefrom() """
        # add one task
        task1 = admit.File_AT(touch=True)
        task1.setkey("file", "File.dat")
        task1._stale = False
        tid1 = self.project.addtask(task1)
        # add another task
        task2 = admit.Flow11_AT()
        task2.setkey("file", "Flow11.dat")
        task2._stale = False
        tid2 = self.project.addtask(task2, [(tid1,0)])
        # mark - only the second task state will be changed
        self.project._markstalefrom(tid1)
        self.assertEqual(task1._stale, False)
        self.assertEqual(task2._stale, True)

# -----------------------------------------------------------------------
 
class admit_test_AT(admit_test_base):
    """ Test basic Admit Task (AT) class functionality """
    # admit/admit/test/unittest_AT.py

    def test_AT_input(self):
        """ test AT input bdp """
        at = AT({'alias': 'a'})
        bdpin = len(at._bdp_in)
        self.assertEqual(bdpin, 0)  ## should have no input bdp

    def test_AT_output(self):
        """ test AT __len__() """
        at = AT({'alias': 'b'})
        bdpout = len(at)
        self.assertEqual(bdpout, 0)  ## should have no output bdp
 
    def test_AT_len2(self):
        """ test AT len2() """
        at = AT({'alias': 'c'})
        tuple = at.len2() # bdp_in and bdp_out tuple
        self.assertEqual(tuple, (0,0))  ## should be (0,0)
 
    def test_AT_version(self):
        """ test AT _version """
        at = AT({'alias': 'd'})
        self.assertEqual(at._version, '0.0.0')
 
    def test_AT_logginglevel(self):
        """ test AT setlogginglevel and getlogginglevel methods """
        # CRITICAL    50
        at = AT({'alias': 'e'})
        at.setlogginglevel(50)
        level = at.getlogginglevel()
        self.assertEqual(level, 50)
        self.assertEqual(level, logging.CRITICAL)
 
    def test_AT_effectivelevel(self):
        """ test AT seteffectivelevel and geteffectivelevel methods """
        at = AT({'alias': 'f'})
        at.seteffectivelevel(40)
        level = at.geteffectivelevel()
        self.assertEqual(level, 40)

    def test_AT_loggername(self):
        """ test AT set/getloggername() """
        at = AT({'alias': 'log'})
        name = "admit_logger"
        at.setloggername(name)
        ret = at.getloggername()
        self.assertEqual(ret, name)

    def test_AT_baseDir(self):
        """ test AT baseDir(), dir() """
        at = AT({'alias': 'g'})
        basedir = at.baseDir("/tmp/")
        self.assertEqual(basedir, "/tmp/")
        fullpath = at.dir("test.test")
        self.assertEqual(fullpath, "/tmp/test.test")
 
    def test_AT_mkext(self):
        """ test AT mkext() """
        at = AT({'alias': 'k'})
        t1 = at.mkext("x","z")         # return 'x-k.z'
        t2 = at.mkext("x.y","z")       # return 'x-k.z'
        t3 = at.mkext("x.y","z", "a")  # return 'x-a.z'

        self.assertEqual(t1, "x-k.z")
        self.assertEqual(t2, "x-k.z")
        self.assertEqual(t3, "x-a.z")
 
    def test_AT_enabled(self):
        """ test AT enabled() """
        at = AT({'alias': 'm'})
        at.enabled(False)
        self.assertFalse(at._enabled)

    def test_AT_markUpToDate(self):
        """ test AT isstale(), markUpToDate() """
        at = AT({'alias': 'n'})
        self.assertTrue(at.isstale())

        at.markUpToDate()
        self.assertFalse(at.isstale())
 
    def test_AT_markChanged(self):
        """ test AT markChanged() """
        at = AT({'alias': 'p'})
        at.markChanged()
        self.assertTrue(at._stale)
 
    def test_AT_projectID(self):
        """ test AT setProject(), getProject(), id() """
        at = AT({'alias': 'q'})
        self.project.addtask(at)
        # get the taskid before adding project id
        tid = at._taskid
        # now set the project id
        at.setProject(2)
        self.assertEqual(at.getProject(), 2)
        # now strip out the project id from _taskid
        self.assertEqual(at.id(True), tid)
 
    def test_AT_mkdir(self):
        """ test AT mkdir() """
        at = AT({'alias': 's'})
        # test with full path
        tmpdir = "/tmp/test1_%d" % os.getpid()
        at.mkdir(tmpdir)
        self.assertTrue(os.path.exists(tmpdir))
        os.rmdir(tmpdir)
 
        # test with relative path
        basedir = at.baseDir("/tmp/")
        tail = "test2_%d" % os.getpid()
        tmpdir = "/tmp/" + tail
        at.mkdir(tail)
        self.assertTrue(os.path.exists(tmpdir))
        os.rmdir(tmpdir)
 
    def test_AT_key(self):
        """ test AT setkey(), getkey(), and haskey() """
        at = AT({'alias': 't', 'test_key': 'at_test'})
        key = "test_key"

        # test invalid key
        self.assertFalse(at.haskey("testtesttest"))

        # test haskey()
        # AT should have the key set at init
        self.assertTrue(at.haskey(key))

        # test setkey()
        at.setkey(name=key, value="TEST", isinit=True)

        # test getkey()
        self.assertEqual(at.getkey(key), "TEST")
 
        # test {key:val} way of setting a key
        at.setkey(name={key:"TEST2"})
        self.assertEqual(at.getkey(key), "TEST2")
 
    def test_AT_isAutoAlias(self):
        """ test AT isAutoAlias() """
        at = AT()
        self.assertTrue(at.isAutoAlias())

        at = AT({'alias': 'alias_test', 'test_key': 'at_test'})
        ret = at.isAutoAlias(withEmpty=False)
        self.assertFalse(ret)

    def test_AT_getsetAlias(self):
        """ test AT get(), set(), setAlias """
        at = AT({'alias': 'test'})
        self.assertEqual(at.get('_alias'), 'test')

        at.set('_alias', 'alias_test')
        self.assertEqual(at.get('_alias'), 'alias_test')

        alias = ({'w':0},0)
        at.setAlias(alias, 'alias_test_2')
        self.assertEqual(at.get('_alias'), 'alias_test_2')

    def test_AT_checktype(self):
        """ test AT checktype """
        at = AT()
        bdp = BDP()
        # should be None without raising an exception
        self.assertIsNone(at.checktype(bdp))
    
    # test set_bdp_in, addinput, clearinput, set_bdp_out
    # addoutput, clearoutput, __contains__, __iter__, __getitem__
    def test_AT_bdp(self):
        """ test AT bdp methods """
        at = AT({'alias': 'w'})
        self.project.addtask(at)

        # input bdp
        at.set_bdp_in([(BDP,1, bt.REQUIRED)])
        self.assertEqual(len(at._bdp_in), 1)
 
        at.clearinput()
        self.assertIsNone(at._bdp_in[0])
 
        at.addinput(BDP())
        self.assertIsNotNone(at._bdp_in[0])
         
        # output bdp
        at.set_bdp_out([(BDP,1)])
        self.assertEqual(len(at._bdp_out), 1)
 
        # test clearoutput
        at.clearoutput()
        self.assertIsNone(at._bdp_out[0])

        output1 = BDP()
        at.addoutput(output1)
        isIn = output1 in at   # call __contains__
        self.assertTrue(isIn)

        output2 = File_BDP({'file': 'test.file'})
        at[0] = output2   # call __setitem__
        isIn = output2 in at   # call __contains__ again
        self.assertTrue(isIn)

        # test __iter__
        counter = 0
        for b in at._bdp_out:
            counter += 1
        self.assertEqual(counter, 1)

        item1 = at[0]  # call __getitem__ at index = 0
        self.assertIsInstance(item1, admit.File_BDP)

# -----------------------------------------------------------------------
 
class admit_test_FM(admit_test_base2):
    """ Test basic Admit Flow Manager (FM) functionality """
    # admit/admit/test/unittest_FM.py

    def test_FM_add_remove(self):
        """ test FM add() and remove() """
        fm = admit.Flow()

        # connection map diagram: a0->a1->a2->a3
        # structure of an element of the triple-nested dictionary of connmap:
        #     src_taskid: {des_taskid: {des_bdpport: (si,sp, di,dp)}}

        self.correct_connmap = {0: {1: {0: (0, 0, 1, 0)}},
                                1: {2: {0: (1, 0, 2, 0)}},
                                2: {3: {0: (2, 0, 3, 0)}}}
        tasklist= list()
        for i in range(0,4):
            a = AT()
            a._baseDir = admit.Project.baseDir
            # Each AT needs an output BDP
            b = File_BDP();
            b.type="TEST_BDP%d" % i
            a._bdp_out.append( b )
            tasklist.append( a )
            if i == 0:
                taskid = fm.add( tasklist[i] )
            else:
                taskid = fm.add( tasklist[i], [(tasklist[i-1]._taskid,0)])
        self.assertEqual(fm._connmap, self.correct_connmap)

        # Remove a2 and its downstream from a0->a1->a2->a3
        # The result diagram is a0->a1
        self.correct_connmap = {0: {1: {0: (0, 0, 1, 0)}}}
        fm.remove(2)
        self.assertEqual(fm._connmap, self.correct_connmap)

    def test_FM_find_replace(self):
        """ test FM find() and replace() """
        fm = admit.Flow()

        # add first task File_AT
        task = admit.File_AT(touch=True)
        tid1 = fm.add(task)
        # add another task Flow11_AT
        task = admit.Flow11_AT()
        tid2 = fm.add(task, [(tid1,0)])
        task.setkey("file", "Flow11.dat")

        # now try to find the tasks
        tasks = fm.find(lambda at: at.id() < 100)
        # check class types of the tasks
        self.assertIsInstance(tasks[0], admit.File_AT)
        self.assertIsInstance(tasks[1], admit.Flow11_AT)

        # test replace()
        task = admit.FlowMN_AT()
        fm.replace(tid2, task)

        # to find new tasks
        tasks = fm.find(lambda at: at.id() < 100)
        # check class types of the tasks
        self.assertIsInstance(tasks[0], admit.File_AT)
        self.assertIsInstance(tasks[1], admit.FlowMN_AT)
 
        # check the number of tasks
        self.assertEqual(len(fm), 2)

        # Test __contains__()
        self.assertFalse(100 in fm)
        self.assertTrue(tid1 in fm)

        # test __iter__
        # we should have two tasks
        counter = 0
        for t in fm:
            counter += 1
        self.assertEqual(counter, 2)

    def test_FM_getsetdelitem(self):
        """ test FM __getitem__, __setitem__, __delitem__ """
        # test add(), connectInputs(), verify(), and show()
        #__getitem__(), __delitem__(), __setitem__()
        fm = admit.Flow()
        # add first task
        task1 = admit.File_AT(touch=True)
        tid1 = fm.add(task1)
        bdp = admit.File_BDP()
        task1.addoutput(bdp)
        # add another task
        task2 = admit.Flow11_AT()
        task2.setkey("file", "Flow11.dat")
        tid2 = fm.add(task2, [(tid1,0)])

        # test connectInputs
        fm.connectInputs()
        # check the number of tasks
        self.assertEqual(len(fm), 2)

        # first task (File_AT)
        # fm[tid1]    # call __getitem__
        self.assertIsInstance(fm[tid1], admit.File_AT)
        # second task (Flow11_AT)
        # fm[tid2]    # call __getitem__
        self.assertIsInstance(fm[tid2], admit.Flow11_AT)

        # test __setitem__
        newtask = admit.FlowMN_AT()
        newtask.setkey("file", "FlowMN.txt")
        fm[tid2] = newtask  # call __setitem__
        # check to see if task2 got overwritten
        self.assertIsInstance(fm[tid2], admit.FlowMN_AT)

        # now restore Flow11_AT
        fm[tid2] = task2  # call __setitem__
        self.assertIsInstance(fm[tid2], admit.Flow11_AT)

        # test __delitem__ (delete Flow11)
        at = fm[tid2]
        del fm[tid2]    # call __delitem__
        self.assertEqual(len(fm), 1)

        # Add task back.
        fm[tid2] = at  # call __setitem__
        # test verify()
        self.assertTrue(fm.verify())

    def test_FM_inFlow_downstream(self):
        """ test FM inFlow(), downstream(), stale(), clone() """
        fm = admit.Flow()
        # Construct a flow: File_AT -> Flow11_AT -> Flow1N_AT

        # add first task
        task1 = admit.File_AT(touch=True)
        tid1 = fm.add(task1)
        bdp = admit.File_BDP()
        task1.addoutput(bdp)

        # add second task
        task2 = admit.Flow11_AT()
        tid2 = fm.add(task2, [(tid1,0)])
        task2.setkey("file", "Flow11.dat")

        # add third task
        task3 = admit.Flow1N_AT()
        tid3 = fm.add(task3,[(tid2,0)])
        task3.setkey("file", "Flow1N.dat")

        # test inFlow()
        self.assertTrue(fm.inFlow(task1))
        # test downstream of tid2 (including tid2)
        dstream = fm.downstream(tid2)
        self.assertEquals(dstream, set([tid2, tid3]))

        # test stale()
        for ds in dstream:
            isStale = fm._tasks[ds].isstale()  # check before mark
            if isStale:
                fm._tasks[ds].markUpToDate()
            isStale = fm._tasks[ds].isstale()  # check after mark
            self.assertFalse(isStale)  # should not be stale

        fm.stale(tid2)
        # all ATs in downstream should be stale now
        for ds in dstream:
            self.assertTrue(fm._tasks[ds].isstale())

        # clone from tid2 (Flow11_AT) in the flow
        cloned_tid = fm.clone(tid2)
        self.assertEqual(cloned_tid, 3)

    def test_FM_showsetkey(self):
        """ test FM showsetkey() and script() """
        fm = admit.Flow()
        # add one task
        task1 = admit.File_AT(touch=True)
        task1.setkey("file", "File.dat")
        tid1 = fm.add(task1)
        # add another task
        task2 = admit.Flow11_AT()
        task2.setkey("file", "Flow11.dat")
        tid2 = fm.add(task2, [(tid1,0)])

        # test showsetkey()
        keyfile = '/tmp/test_FM_showsetkey.%s' % os.getpid()
        fm.showsetkey(keyfile)
        self.assertTrue(os.path.isfile(keyfile))
        # test script
        scriptfile = '/tmp/test_FM_script.%s' % os.getpid()
        file = open(scriptfile, mode='w')
        fm.script(file)
        file.close()
        self.assertTrue(os.path.isfile(scriptfile))

        # cleanup
        if os.path.exists(keyfile):
            os.remove(keyfile)
        if os.path.exists(scriptfile):
            os.remove(scriptfile)

# -----------------------------------------------------------------------
 
class admit_test_multiflow(admit_test_base2):
    """ Test Admit multiflow project manager """
    # admit/admit/test/unittest_multiflow1.py

    def test_multiflow1(self):
        """ Test multiflow with two Project inputs """
        # Parent projects.
        p1 = Project(self.outputDir+"/p1")
        p2 = Project(self.outputDir+"/p2")
        for p in [p1, p2]:
          # add task 1
          task = admit.File_AT(touch=True)
          task.setkey("file", "File.dat")
          tid1 = p.addtask(task)
          # add task 2
          task = admit.Flow11_AT(alias="at" + p.baseDir[-2]) # at1 or at2
          task.setkey("file", "Flow11.dat")
          tid2 = p.addtask(task, [(tid1,0)])
          # run flow
          p.run()

        # Multiflow project.
        mflow = Project(self.outputDir+"/mflow")

        # Add parent projects to the multiflow.
        # Note they must be completely up-to-date for this to succeed.
        pid1 = mflow.pm.addProject(self.outputDir+"/p1")
        pid2 = mflow.pm.addProject(self.outputDir+"/p2")

        # Find some ATs to link into the multiflow.
        # Here searching is done by alias name.
        stuples = []
        for pid in [pid1, pid2]:
            alias = "at" + mflow.pm[pid].baseDir[-2]
            ats = mflow.pm.findTaskAlias(pid, alias)
            self.assertEqual(len(ats), 1, "Found wrong number of matches")
            self.assertEqual(ats[0]._alias, alias, "Alias mismatch")
            self.assertNotEqual(ats[0].getProject(), 0, "Null project ID")

            # Add task to the multiflow (must be a root task---no stuples).
            tid = mflow.addtask(ats[0])
            self.assertNotEqual(tid, -1, "mflow.addtask(" + alias + ") failed")
            stuples.append((tid, 0))

        # Combine output from the two newly linked tasks.
        tid = mflow.addtask(admit.FlowN1_AT(file="FlowN1.dat", touch=True), stuples)
        self.assertNotEqual(tid, -1, "mflow.addtask(FlowN1) failed")

        mflow.show()
        # Run the multiflow.
        mflow.run()

        # Make at2 out of date, then re-run the multiflow to update everything.
        at2 = mflow.findtask(lambda at: at._alias == "at2")
        self.assertEqual(len(at2), 1, "Found wrong number of matches for at2")
        self.assertEqual(at2[0]._alias, "at2", "Alias mismatch for at2")
        at2[0].setkey("file", "Flow11-at2.dat")
        mflow.show()
        mflow.run()

# -----------------------------------------------------------------------
 
def suite():
    if admit_import:
        return [admit_test_Project,
                admit_test_AT,
                admit_test_FM,
                admit_test_multiflow
               ]
    else:
        return [] 
 
