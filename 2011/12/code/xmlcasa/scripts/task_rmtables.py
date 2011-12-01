import os
import glob
from taskinit import *
from tasksinfo import *

def rmtables(tablenames=None):
       """ Removes tables cleanly 
           Arguments may contain *, ?. Ranges [] also supported but not ~ expansion.
       """

       casalog.origin('rmtables')
       tablelist = []
       for tables in tablenames :
	  for table in glob.glob(tables) :
	     tablelist.append(table)
       for table in tablelist :
          casalog.post('Removing '+table)
       cu.removetable(tablelist)

