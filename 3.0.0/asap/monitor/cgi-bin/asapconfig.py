#asapconfig.py

observatory = {'rpfpath': [],
               'name': 'observatory',
               'lines' : {} }

import os,sys
# This is where asap lives
sys.path.insert(2,'/opt/lib/python2.3/site-packages')
os.environ["AIPSPATH"]="/opt/share/asap linux_gnu somewhere localhost"

#overwrite /usr/local/... as default
sys.path.insert(2,'/usr/lib/python2.3/site-packages')

# This is needed for plotting with matplotlib
# where matplotlib data is located
os.environ["MATPLOTLIBDATA"]="/opt/share/matplotlib"
# where matplotlib puts it temporary font files
# this location can also have a custom .matplotlibrc
os.environ["HOME"]="/var/www/asapmon/tmp"
