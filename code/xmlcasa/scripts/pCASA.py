#
# This module is a wrapper around parallel_go
#
# Example usage:
#
#   1) Create the multiMS on disk. This will eventually be done by a
#   parallel filler.
#
#     subms0 = pCASA.subMS("spw0.ms", "some_hostname")
#     subms1 = pCASA.subMS("spw1.ms", "some_hostname")
#     subms2 = pCASA.subMS("spw2.ms", "other_hostname")
#
#     pCASA.create("multi.ms", [subms0, subms1, subms2])
#
#   2) Then the following
#
#      flagdata("multi.ms", <parameters>)
#
#   will have the effect of running
#
#      flagdata("spw0.ms", <parameters>)
#      flagdata("spw1.ms", <parameters>)
#      flagdata("spw2.ms", <parameters>)
#
#   on the given hosts through parallel_go.
#


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

        # default to setting up N engines on localhost,
        # where N is the number of cores

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
        for host in self.hosts:
            if not host in self.cluster.get_nodes():
                if debug:
                    print "Start engines on host", host
                self.cluster.start_engine(host,
                                          self.num_of_engines,
                                          os.getcwd())

                if debug:
                    print "ipengines started on host", host

def load(mms_name):
    """Returns either the multiMS object, or None
    if the file is not a multiMS
    """
    
    try:
        f = open(mms_name, "r")
        mms = pickle.load(f)
        f.close()
    except:
        mms = None
        pass

    if isinstance(mms, multiMS):
        if mms.version != multiMS.code_version:
            raise Exception(mms_name + " file version " + str(mms.version) +
                            " is incompatible with this code version: " +
                            str(multiMS.code_version))
        return mms
    else:
        if debug:
            print mms_name, "is not a multiMS"
        return None

def is_mms(mms_name):
    """Returns true if and only if
    the file with the given name contains a multiMS object
    """
    return load(mms_name) != None

def show(mms_name):
    """Prints the contents of the given multiMS
    """
    mms = load(mms_name)

    if mms != None:
        print mms_name, "(multiMS v" + str(mms.version) + "):"

        for s in mms.sub_mss:
            print "  %s %s" % (s.host, s.ms)

# A subset of a multi MS
class subMS:
    def __init__(self, ms_name, host):
        self.ms = ms_name
        self.host = host

def create(mms_name, sub_mss):
    """Create a multiMS"""

    mms = multiMS()

    for s in sub_mss:

        if not isinstance(s, subMS):
            raise Exception("Not a subMS: " + str(s))

        mms.add(s)

    f = open(mms_name, "w")
    pickle.dump(mms, f)
    f.close()

# A collection of subMSs
class multiMS:

    code_version = "0.2"
    # Should be incremented every time this class changes,
    # in order to fail cleanly if trying to load a multiMS
    # which was written with a previous, incompatible, version
    # of this class

    def __init__(self):
        self.sub_mss = []
        self.version = multiMS.code_version

    def add(self, sub_ms):
        self.sub_mss.append(sub_ms)

        
def _ip(host):
    """Returns the IP address of the given hostname,
    but not 127.0.0.1 for localhost but localhost's global IP
    """
    
    ip = socket.gethostbyname(host)
    
    if ip == "127.0.0.1":
        ip = socket.gethostbyname(socket.gethostname())

    return ip

def execute(taskname, mms_name, parameters):
    """Runs the given task on the given multiMS"""

    global pc
    
    mms = load(mms_name)

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

pc = pCASA()
