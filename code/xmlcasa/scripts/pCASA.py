"""
This module adds multiMS support to CASA tasks.

Example usage:

    1) Create the multiMS on disk. This will eventually be done by a
    parallel filler.

      pCASA.create("my.ms")
      pCASA.add("my.ms", "spw0.ms", "some_hostname")
      pCASA.add("my.ms", "spw1.ms", "some_hostname")
      pCASA.add("my.ms", "spw2.ms", "some_other_hostname")
      pCASA.add("my.ms", "spw3.ms")

   When no hostname is given (spw3.ms), the MS is assumed to be available
   from a locally mounted disk. When finised, show the contents of the multiMS
   with

      pCASA.list("my.ms")

   2) Now the following

      flagdata("my.ms", <parameters>)

   will have the effect of running

      flagdata("spw0.ms", <parameters>)
      flagdata("spw1.ms", <parameters>)
      flagdata("spw2.ms", <parameters>)
      flagdata("spw3.ms", <parameters>)

   on the given hosts, through parallel_go. parallel_go requires
   password-less ssh in order to function.

   The user does not have to explicitly define the available hosts
   or the number of engines per host. The number of engines per host is
   determined (automatically) from the number of CPU cores on the
   local host. The available hosts are determined from the contents of
   the multiMS.

   It is possible (and a way to avoid bottlenecks) to wrap more subMSs
   into the multiMS than the number of parallel engines.
   The parallel engines are assigned and reassigned to individual subMSs
   on the fly, as they become idle.
"""
import parallel_go
import pickle
import sets
import socket
import time
import os

debug = False

class pCASA:
    """Allows user to manipulate parallel configuration,
    e.g. number of engines
    """
    def __init__(self):

        # default to setting up N engines on each host,
        # where N is the number of cores on the local host

        # Python 2.6:  self.engines_per_host = multiprocessing.cpu_count()

        # POSIX
        try:
            self.engines_per_host = int(os.sysconf('SC_NPROCESSORS_ONLN'))
        except (AttributeError,ValueError):
            # As fallback
            self.engines_per_host = 1
        
        self.hosts = sets.Set()
        self.cluster = parallel_go.cluster()
            

    def register_host(self, hostname):
        self.hosts.add(hostname)

    def start(self):

        already_running_nodes_ip = sets.Set()
        # Convert to IP addresses, for reasons of uniqueness
        for node in self.cluster.get_nodes():
            already_running_nodes_ip.add(_ip(node))
            
        for host in self.hosts:

            if debug:
                print "new host: ", host, _ip(host)
                print "existing hosts:", already_running_nodes_ip
            if not _ip(host) in already_running_nodes_ip:
                if debug:
                    print "Start engines on host", host
                self.cluster.start_engine(host,
                                          self.engines_per_host,
                                          os.getcwd())

                if debug:
                    print "ipengines started on host", host

def _load(mms_name):
    """Returns the multiMS object, or throws
    an exception if the file could not be loaded as
    a multiMS"""

    if not os.path.lexists(mms_name):
        raise Exception("%s does not exist" % mms_name)
    
    try:
        f = open(mms_name, "r")
        mms = pickle.load(f)
        f.close()
    except:
        raise Exception("Could not load %s" % mms_name)

    if not isinstance(mms, multiMS):
        raise Exception("%s is not a multiMS. Its python type is %s" % \
                        (mms_name, type(mms)))
        
    if mms.version != multiMS.code_version:
        raise Exception(mms_name + " file version " + str(mms.version) +
                        " is incompatible with this code version: " +
                        str(multiMS.code_version))
    return mms

def is_mms(mms_name):
    """Returns true if and only if
    the file with the given name contains a multiMS object
    """
    try:
        _load(mms_name)
        return True
    except:
        return False

def list(mms_name):
    """Prints the contents of the given multiMS
    """
    mms = _load(mms_name)

    print mms_name, "(multiMS v" + str(mms.version) + "):"
    
    for s in mms.sub_mss:
        print "  %s %s" % (s.host, s.ms)
    if len(mms.sub_mss) == 0:
        print "  <empty>"

def create(mms_name):
    """Create an empty multiMS"""

    if os.path.lexists(mms_name):
        raise Exception("%s already exists" % mms_name)

    mms = multiMS()

    f = open(mms_name, "w")
    pickle.dump(mms, f)
    f.close()

def add(mms_name, subms_name, hostname = "localhost"):
    """Add a subMS entry to a multiMS"""

    mms = _load(mms_name)

    s = subMS(subms_name, hostname)
    mms.add(s)
    
    # Overwrite existing file
    f = open(mms_name, "w")
    pickle.dump(mms, f)
    f.close()

def remove(mms_name, subms_name):
    """Remove a subMS entry from the multiMS"""

    mms = _load(mms_name)
    if not mms.remove(subms_name):
        print "%s does not contain a subMS with name %s" % (mms_name, subms_name)
    else:
        f = open(mms_name, "w")
        pickle.dump(mms, f)
        f.close()
        
        if debug:
            print "Removed %s from %s" %(subms_name, mms_name)
       
def _ip(host):
    """Returns the IP address of the given hostname,
    but not 127.0.0.1 for localhost but localhost's global IP
    """
    
    ip = socket.gethostbyname(host)
    
    if ip == "127.0.0.1":
        ip = socket.gethostbyname(socket.gethostname())

    return ip


def _launch(engine, taskname, ms, parameters):
    """Launches a job"""

    print "Run %s on host %s: %s(\"%s\", ...)" % \
          (engine['id'], engine['host'], taskname, ms)

    args = []
    for (p, val) in parameters.items():
        if p == "vis":
            args.append(p + " = '" + ms + "'")
        else:
            if isinstance(val, str):
                args.append(p + " = '" + val + "'")
            else:
                args.append(p + " = " + str(val))
                
    cmd = taskname + "(" + ", ".join(args) + ")"
    if debug:
        print cmd
    engine['job'] = pc.cluster.odo(cmd, engine['id'])
    engine['ms'] = ms
    engine['idle'] = False

def _poor_mans_wait(engines, taskname):
    """Returns engine id and any exception thrown by the job.
    The return value of the job is not made available.
    
    Each pending job is polled once per second; this is inefficient
    and should be replaced with a 'wait' call that blocks
    until any job terminates and returns the ID of the job that
    terminated. But that 'wait' function does not seem to exist."""

    while True:
        for engine in engines.values():
            if not engine['idle']:
                ex = None
                try:
                    # get_result() will
                    #   return None: if the job is not done
                    #   return ResultList: if the job is done
                    #   rethrow the job's exception if the job threw
                    r = engine['job'].get_result(block = False)
                except Exception, e:
                    ex = e

                if ex != None or r != None:
                    if ex == None:
                        state = "success"
                    else:
                        state = "fail"
                    print "Run %s on host %s: %s(\"%s\", ...) %s" % \
                      (engine['id'], engine['host'], taskname, engine['ms'],
                       state)
                    engine['idle'] = True
                    return (engine['id'], ex)

        time.sleep(1)

def execute(taskname, parameters):
    """Runs the given task on the given multiMS.
    If any of the jobs throw an exception, execution stops, and the
    first exception that happened is rethrown by this function"""

    global pc

    mms_name = parameters['vis']
    mms = _load(mms_name)

    for s in mms.sub_mss:
        pc.register_host(s.host)
        
    pc.start()

    # Convert engines to a more handy format (from lists to dictionaries),
    # and add idleness as an engine property
    engines = {}
    for e in pc.cluster.get_engines():
        engines[e[0]] = {}
        engines[e[0]]['id'] = e[0]
        engines[e[0]]['host'] = e[1]
        engines[e[0]]['pid'] = e[2]
        engines[e[0]]['idle'] = True
        engines[e[0]]['job'] = None
        
    m = len(mms.sub_mss)

    n = len(engines)

    print "Process %s using %s engines" % (mms_name, n)

    non_processed_submss = mms.sub_mss[:]

    status = None   # None: success

    #  The availble engines will be assigned to the subMSs
    #  on the fly, as engines become idle
    #  
    #  Pseudo code:
    #
    #  while still_data_to_process:
    #      search for an idle engine with access to the data
    #      if found:
    #          launch job
    #      else:
    #          if there are running engines:
    #              wait next idle engines
    #          else:
    #              give up
    #  while there are running engines:
    #      wait
    #
    while len(non_processed_submss) > 0 and status == None:
        if debug:
            print "Still to process =", non_processed_submss
            print "Engines =", engines

        found = False
        
        for engine in engines.values():
            if engine['idle']:
                if debug:
                    print "idle engine", engine['id'], "is", engine['host'], _ip(engine['host'])
            
                for subms in non_processed_submss:
                    if debug:
                        print "subMS", subms.ms, "is", subms.host, _ip(subms.host)

                    if _ip(engine['host']) == _ip(subms.host):
                    
                        _launch(engine, taskname, subms.ms, parameters)

                        non_processed_submss.remove(subms)
                        
                        found = True
                        break

                if found:
                    break

        if not found:
            pending_job = False
            for engine in engines.values():
                if not engine['idle']:
                    pending_job = True
                    break
            if pending_job:
                (engine_id, s) = _poor_mans_wait(engines, taskname)

                # If an exception was thrown, bail out ASAP
                if s != None and status == None:
                    status = (engine_id, s)
                    
            else:
                raise Exception("All engines are idle but none have access to the data")       


        if debug:
            print "Still to process =", non_processed_submss
            print "casalogs =", pc.cluster.get_casalogs()
            print "ids =", pc.cluster.get_ids()
            print "properties =", pc.cluster.get_properties()
            print "engines =", pc.cluster.get_engines()
            print "nodes = ", pc.cluster.get_nodes()

    pending_job = True
    while pending_job:
        pending_job = False
        
        for engine in engines.values():
            if not engine['idle']:
                pending_job = True
                break
            
        if pending_job:
            (engine_id, s) = _poor_mans_wait(engines, taskname)
            
            if s != None and status == None:
                status = (engine_id, s)

    if status != None:
        print >> sys.stderr, "Engine %s error while processing %s" % \
              (status[0], engines[status[0]]['ms'])
        
        raise status[1]


# A subset of a multi MS
class subMS:
    def __init__(self, ms_name, host):
        self.ms = ms_name
        self.host = host

# A collection of subMSs
class multiMS:

    code_version = "0.2"
    # Should be incremented every time this class changes (incompatibly
    # wrt to its pickle representation), in order to fail cleanly if
    # we try to load a multiMS pickel file which was written with a previous
    # (incompatible) version of this class

    def __init__(self):
        self.sub_mss = []
        self.version = multiMS.code_version

    def add(self, sub_ms):
        self.sub_mss.append(sub_ms)

    def remove(self, subms_name):
        """Removes all subMSs with the given MS name.
        Returns true if and only if a matching subMS existed"""

        found = False
        for s in self.sub_mss:
            if s.ms == subms_name:
                found = True
                self.sub_mss.remove(s)
                
        return found


pc = pCASA()
