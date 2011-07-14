#asapconfig.py

observatory = {'rpfpath': [],
               'name': 'observatory',
               'lines' : {} }

asapmonhome = "/var/www/htdocs"
asapmonrel = "/asapmon/"
import os,sys
# where matplotlib puts it temporary font files
# this location can also have a custom .matplotlibrc
os.environ["HOME"]="/var/www/htdocs/asapmon/tmp"
