"""
This module adds multiMS support to CASA tasks.

Example usage:

    1) Create the multiMS on disk. This will eventually be done by a
    parallel filler.

      pCASA.create("my.ms")
      pCASA.add("my.ms", "spw0.ms", "some_hostname")
      pCASA.add("my.ms", "spw1.ms", "some_other_hostname")
      pCASA.add("my.ms", "spw2.ms")

   where spw2.ms is a MS on a locally mounted disk. Show the contents
   with

      pCASA.list("my.ms")

   2) Then the following

      flagdata("my.ms", <parameters>)

   will have the effect of running

      flagdata("spw0.ms", <parameters>)
      flagdata("spw1.ms", <parameters>)
      flagdata("spw2.ms", <parameters>)

   on the given hosts using parallel_go. parallel_go requires
   password-less ssh in order to function.
"""
import parallel_go
import pickle
import sets
import socket
import os

debug = False

class pCASA:
    """Allows user to manipulate parallel configuration,
    e.g. number of engines
    """
    def __init__(self):

        # default to setting up N engines on each host,
        # where N is the number of cores on the local host

        # Python 2.6:  self.num_of_engines = multiprocessing.cpu_count()

        # POSIX
        try:
            self.num_of_engines = int(os.sysconf('SC_NPROCESSORS_ONLN'))
        except (AttributeError,ValueError):
            # As fallback
            self.num_of_engines = 1
        
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

            print "new host: ", host, _ip(host)
            print "existing hosts:", already_running_nodes_ip
            if not _ip(host) in already_running_nodes_ip:
                if debug:
                    print "Start engines on host", host
                self.cluster.start_engine(host,
                                          self.num_of_engines,
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

def execute(taskname, parameters):
    """Runs the given task on the given multiMS"""

    global pc

    mms_name = parameters['vis']
    mms = _load(mms_name)

    m = len(mms.sub_mss)
    n = pc.num_of_engines

    print "Process %s using %s engines" % (mms_name, n)

    for s in mms.sub_mss:
        pc.register_host(s.host)
        
    pc.start()

    non_processed_submss = mms.sub_mss[:]

    while len(non_processed_submss) > 0:
        if debug:
            print "Still to process =", non_processed_submss

        # Dictionary of MSs assigned to each engine
        vis = {}

        for engine in pc.cluster.get_engines():
            engine_id = engine[0]
            if debug:
                print "engine", engine_id, "is", engine[1], _ip(engine[1])

            for subms in non_processed_submss:
                if debug:
                    print "subMS", subms.ms, "is", subms.host, _ip(subms.host)
                if _ip(engine[1]) == _ip(subms.host):
                
                    if not vis.has_key(engine_id):
                        
                        vis[engine_id] = subms.ms
                        non_processed_submss.remove(subms)
                        break

        for engine in pc.cluster.get_engines():
            engine_id = engine[0]
            if not vis.has_key(engine_id):
                vis[engine_id] = "non-existing-dummy"

        for engine_id, ms in vis.items():
            for engine in pc.cluster.get_engines():
                if engine[0] == engine_id:
                    print "Run on host %s: %s(\"%s\", ...)" % \
                          (engine[1], taskname, ms)
        if debug:
            print "Still to process =", non_processed_submss
            print "casalogs =", pc.cluster.get_casalogs()
            print "ids =", pc.cluster.get_ids()
            print "properties =", pc.cluster.get_properties()
            print "engines =", pc.cluster.get_engines()
            print "nodes = ", pc.cluster.get_nodes()
    
        args = []
        pc.cluster.pgk(vis=vis)
        for (p, val) in parameters.items():
            if p == "vis":
                args.append(p + " = vis")
            else:
                if isinstance(val, str):
                    args.append(p + " = '" + val + "'")
                else:
                    args.append(p + " = " + str(val))
            
        cmd = taskname + "(" + ", ".join(args) + ")"
        if debug:
            print cmd
        pc.cluster.pgc('inp("' + taskname + '")')
        pc.cluster.pgc(cmd)


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
