import subprocess
import os
import sys
import __casac__


class carta:
    def __init__(self):
        # Default paths for packaged applications
        #print __file__
        #print sys.platform
        script_dir = os.path.dirname(__file__)
        relative_path = "/../../etc/carta/bin/carta.sh"
        if sys.platform == "darwin" :
            relative_path = "/../Carta.app/Contents/MacOS/carta.sh"
        carta_path = script_dir + relative_path

        # Get Carta home from .casarc
        casa_util = __casac__.utils.utils()
        cartahome = casa_util.getrc("CartaHome")
        if cartahome != "" and cartahome !="Unknown value":
            carta_path = cartahome + "/bin/carta.sh"
            if sys.platform == "darwin" :
                carta_path = cartahome + "/Contents/MacOS/carta.sh"
            print "Starting carta from:" + carta_path
        subprocess.check_output([carta_path])
