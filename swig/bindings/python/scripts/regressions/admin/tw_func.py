import pylab as pl
#from Scientific.Functions.LeastSquares import *
#note that 'import math' && 'from math import *' don't work

def gauss(param,x):
 return param[0]+param[1]*pl.exp(-param[2]*(x-param[3])**2)

def gauss3d(param,data):
 return param[0]+param[1]*pl.exp(-param[2]*(data[0]-param[3])**2-param[4]*(data[1]-param[5])**2)

#param[0]=z0
#param[1]=amplitude
#param[2]=cx, a constant multiplier
#param[3]=x0, position of x peak
#param[4]=cy, a constant multiplier
#param[5]=y0, position of y peak
#data[0]=x variable
#data[1]=y variable
