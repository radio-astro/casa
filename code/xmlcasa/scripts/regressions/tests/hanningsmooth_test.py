#############################################################################
## $Id:$
# Test Name:                                                                #
#    Regression Test Script for hanningsmooth                               #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    It ensures that the task is working properly. It compares the          #
#    values of the data calculated using the task hanningsmooth with        #
#    theoretical values calculated in this test using the following         #
#    equation:                                                              #
#    S[i] = 0.25*(D[i-1] + D[i+1]) + 0.5*D[i]                               #
#    where:                                                                 #
#    S[i] is the smoothed data                                              #
#    D[i] is the original data                                              #
#                                                                           # 
# Features tested:                                                          #
#    1) Is the task working properly?                                       #
#    2) Is the task giving the correct results?                             #
#                                                                           #
# Input data:                                                               #
#    ngc5921.fits                                                           #
#                                                                           #
#############################################################################

import os
import string
import sys
import time

from tasks import *
from taskinit import *

startTime = time.time()

input_file="ngc5921.fits"
msfile="ngc5921.hanning.ms"
stars = "*************"

def description():
    return "Test of hanningsmooth using ngc5921.fits"

def data():
    return [input_file]

# Calculate the hanning smooth of each element
def calculate(dataB,data,dataA):
    const0 = 0.25
    const1 = 0.5
    const2 = 0.25
    S = const0*dataB + const1*data + const2*dataA
    return S

# Calculate the difference between task and method
def check(Cd,Sm,max):
    flag = True
    if abs(Cd-Sm) > max :
        flag = False
        
    return flag
 
def run():
  
  fail = 0
  total = 0
  chan = 0
  maximum = 0.00001

  # Create MS
  importuvfits(fitsfile=input_file,vis=msfile)

  # Run hanningsmooth on new table
  print " Run hanningsmooth on data "

  # It will first create the scratch columns
  hanningsmooth(vis=msfile)
  
  # Get the column with the original data
  tb.open(msfile)
  corr_col = tb.getvarcol('CORRECTED_DATA')
  data_col = tb.getvarcol('DATA')
  tb.close()
  
  # get the length of rows
  nrows = len(corr_col)
  print "-- Comparing the values of %s rows in 63 channels --" %(nrows/2)
    
  # Loop over every 2nd row,pol and get the data for each channel
  for i in range(1,nrows,2) :
      row = 'r%s'%i
      
      # polarization is 0-1
      for pol in range(0,2) :
          
          # array's channels is 0-62
          for chan in range(1,61) :
              total += 1
              # channels must start from second and end before the last
              data = data_col[row][pol][chan]
              dataB = data_col[row][pol][chan-1]
              dataA = data_col[row][pol][chan+1]

              Smoothed = calculate(dataB,data,dataA)
              CorData = corr_col[row][pol][chan]
              
              # Check the difference
              status = True
              status = check(CorData, Smoothed, maximum)
              if not status :
                  fail += 1
              
 # raise an exception if values are not within a maximum   
  if fail > 0 :
    perc = fail*100/total
    raise Exception, "Regression test failed: %s %% of values are different "\
        "by more than the allowed maximum %s" %(perc,maximum)
  else:
      print "-- TEST succeeded! Compared %s values --" %total
            
  endTime = time.time()
  print "-- Time taken by the test %s seconds --" %(endTime-startTime)           
  

  
  return []
  