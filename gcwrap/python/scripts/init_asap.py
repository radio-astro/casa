from asap_init import *

### Try loading ASAP
try:
    asap_init()
except ImportError, e:
    casalog.post("%s\nCould not load ASAP. sd* tasks will not be available." % e,'WARN')
except Exception, instance:
    casalog.post("Could not load ASAP. sd* tasks will not be available.",'WARN')
    casalog.post(str(instance),'WARN',origin="asap_init")
