#####################################
###  Re-sort an MS, to optimize for the CASA access pattern.
###
###  Desired order :
###        ARRAY
###        FIELD
###        SPW (or data_desc_id)
###        TIME
###        ANTENNA1
###        ANTENNA2
###
###  -- Antenna1,2 are the fastest-varying axes.
###  -- Data are by-default internally partitioned on field, spw, array (or observation).
###  -- For each field/spw/observation, data is accessed in "buffers" that 
###     contain all baselines for one timestep.
###
###  To check if an MS is in the correct order, open it in
###  'casabrowser', and check the ANTENNA1, ANTENNA2, 
###  DATA_DESC_ID,  FIELD_ID, ARRAY_ID, and TIME columns.
###
#####################################

#!/usr/bin/env python

import os;
import shutil;
from  casac import *;

def setToCasaOrder(inputMS='',outputMS=''):
  """
  This script will re-sort the input MS into the following sort order
     ARRAY,  FIELD, SPW (or data_desc_id),  TIME,  ANTENNA1, ANTENNA2

  For I/O operations, data are by-default partitioned on array, field and spw.
  CASA prefers data in which ANTENNA1,ANTENNA2 are the fastest-varying axes.

  To check if your MS is in the correct order (and if you need this or not),
  open the MS using 'casabrowser' or 'browsetable()', and look at the 
  ANTENNA1, ANTENNA2, DATA_DESC_ID,  FIELD_ID, ARRAY_ID, and TIME columns.
  The sort order in the MS should be apparant from the indices in these columns.

  Re-sorting makes the most difference in some cases when UVFITS files 
  written out of AIPS are read into CASA via importuvfits. Often, SPW (data_desc_id)
  is the fastest-varying index, and this is the worst-possible case for the CASA 
  data-access pattern. 
  Note : This is due to a difference in the choice of default sort order 
            in AIPS and CASA, which the importuvfits task currently ignores.

  This script will create a temporary reference-MS (a list of row indices containing the 
  mapping from the input to output MSs) called 'tmpreftable.tab' in the current
  working directory. Therefore, please do not run two instances of this script in
  the same directory. 

 """
  ## Make a local table tool
  tb = casac.table()

  ## Old MS Name
  if(not os.path.exists(inputMS)):
     print "Original MS ", inputMS, " not found ";
     return False;

  if(os.path.exists(outputMS)):
     print "An MS named ", outputMS, " already exists. Please delete it first, or choose another output MS name";
     return False;

  ## Make a reference table with the correct sort order.
  ## This creates a list of row-numbers that index
  ## into the data in the desired order.
  print "Making reference table.";
  tb.open(inputMS);
  tmptb = tb.query(name='tmpreftable.tab', query='DATA_DESC_ID>-1',sortlist='ARRAY_ID,FIELD_ID,DATA_DESC_ID,TIME,ANTENNA1,ANTENNA2');
  tmptb.close()
  tb.close();

  ## Save a new table with the sort-order
  ## specified by the list of indices in the
  ## reference table.
  print "Making deep-copy of ref-table";
  tb.open('tmpreftable.tab');
  tmptb = tb.copy(outputMS,deep=True);
  tmptb.close()
  tb.close();

  ## Remove the reference table
  print "Removing ref-table";
  if(os.path.exists('tmpreftable.tab')):
      shutil.rmtree('tmpreftable.tab');

#####################################
