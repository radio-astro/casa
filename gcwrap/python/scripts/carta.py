import subprocess
import os
import sys

class carta:
    def __init__(self):
       print __file__
       print sys.platform
       scriptdir = os.path.dirname(__file__)
       relativepath = "/../../etc/carta/bin/carta.sh"
       if sys.platform == "darwin" :
           relativepath = "/../Carta.app/Contents/MacOS/applet"
       cartapath = scriptdir + relativepath
       print cartapath
       subprocess.Popen([cartapath])
