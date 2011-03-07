# msplot_test.py:  A test suite for testing the MS plot tool (mp)
#
# Copyright (C) 2007
# Associated Universities, Inc. Washington DC, USA.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be addressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#   
# @author  Gary Li & Shannon Jaeger
# @version 
#############################################################################

import sys
import os
#import casac
#msplottool = casac.homefinder.find_home_by_name('msplotHome') # works
#mp = msplottool.create()                                     # works

###############################################################################
### Define the location of the various measurement sets used for testing.
ngc5921PATH = './data/ngc5921.ms'
g1310_0506PATH = './data/G1310_0506.ms'
#_3C273XC1PATH = './data/3C273XC1_tmp.ms'
_3C273XC1PATH = './data/3C273XC1.ms'
ngc7538PATH = './data/ngc7538.ms'
g0336PATH = './data/G0336_0317.ms'

#casalog.filter('DEBUGGING')
#casalog.filter('INFO4')
logfile=open('msplot_test.log', 'w' )

###############################################################################
### This function is used to provide a consistent way of executing each
### of the commands, to ensure proper error reporting, ...
def test_plotting( fnName='' ) :
    print >>logfile,'Executing command ... '
    print >>logfile,'     ', fnName
    print fnName
    try:
        # Execute the command
        if ( not eval( fnName ) ) :
            print >>logfile, 'Execution FAILED for command ...' 
            print >>logfile, '     ', fnName
        else: 
            print >>logfile, 'SUCCESSFUL execution of command ...' 
            print >>logfile, '     ' + fnName
    except:
        print >>logfile, 'EXCEPTION occured with command ...' 
        print >>logfile, '     ', fnName

############################################################################
print >>logfile, "TESTING OPENING OF FILES"
test_plotting( "mp.open( ngc5921PATH )" )
test_plotting( "mp.done()" )
test_plotting( "mp.open( ngc7538PATH )" )
test_plotting( "mp.done()" )
test_plotting( "mp.open( g1310_0506PATH )" )
test_plotting( "mp.done()" )
test_plotting( "mp.open( ngc5921PATH )" )
test_plotting( "mp.done()" )
test_plotting( "mp.open(_3C273XC1PATH)" )
test_plotting( "mp.done()" )
#test_plotting( "mp.done()" )
print >>logfile, "\n"

########################################################################
## test subplot after refactorin
## array() is not working with other methods yet!.
print >>logfile, "TESTING subplot - 4 subplots at once." 
test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions(subplot=221)" )
test_plotting( "mp.plot( type='uvcoverage')" )
test_plotting( "mp.plotoptions(subplot=222)" )
test_plotting( "mp.plot( type='baseline')" )
test_plotting( "mp.plotoptions(subplot=223)" )
test_plotting( "mp.plot( type='vischannel')" )
test_plotting( "mp.plotoptions(subplot=224)" )
test_plotting( "mp.plot( type='elevation')" )
test_plotting( "mp.clearplot()" )
test_plotting( "mp.reset()" )
test_plotting( "mp.done()" )
print >>logfile, "\n"

########################################################################
## test iteration after refactorin
print >>logfile, "TESTING ITERATION with vischannel and vistime" 
test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions(subplot=221)" )
#test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='time')" )    
test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='antenna1' )" )
#test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='antenna2' )" )
#test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='baseline' )" )
#test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='field_id' )" )
test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='field' )" )
#test_plotting( "mp.plot( type='vischannel',iteration='antenna1')" )
test_plotting( "mp.plot( type='vischannel',iteration='baseline')" )
test_plotting( "mp.iterplotnext()" )
#test_plotting( "mp.plot( type='vistime', column='data', value='amp', iteration='antenna1')" )
#test_plotting( "mp.plot( type='vistime', column='data', value='amp', iteration='antenna2')" )
test_plotting( "mp.plot( type='vistime', column='data', value='amp', iteration='baseline')" )
#test_plotting( "mp.plot( type='vistime', column='data', value='amp', iteration='field_id')" )
test_plotting( "mp.plot( type='vistime', column='data', value='amp', iteration='field')" )
test_plotting( "mp.iterplotnext()" )
test_plotting( "mp.iterplotstop()" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >>logfile,"\n"

#######################################################################
## test plotxy()
##
## Those commented out are not available yet.
print >>logfile, "TESTING plotxy function"

#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotxy(x='antenna1',y='data' )" )
test_plotting( "mp.plotxy(x='antenna2',y='data' )" )
test_plotting( "mp.plotxy(x='feed1', y='data')" )
test_plotting( "mp.plotxy(x='feed2', y='data')" )
test_plotting( "mp.plotxy(x='field_id', y='data')" )
##test_plotting( "mp.plotxy(x='array_id', y='data')" )
test_plotting( "mp.plotxy(x='scan_number',y='data') " ) 
test_plotting( "mp.plotxy(x='time', y='data')" )
test_plotting( "mp.plotxy(x='channel', y='data')" )
test_plotting( "mp.plotxy(x='uvdist', y='data')" )
##test_plotting( "mp.plotxy(x='frequency', y='data'" )
test_plotting( "mp.plotxy(x='U',y='V')" )
test_plotting( "mp.plotxy(x='V',y='W')" )
test_plotting( "mp.plotxy(x='W',y='U')" )
test_plotting( "mp.plotxy(x='hourangle',y='data')" )
test_plotting( "mp.plotxy(x='azimuth', y='data')" ) 
test_plotting( "mp.plotxy(x='elevation', y='data')" ) 
test_plotting( "mp.plotxy(x='parallactic_angle', y='data')" ) 

##One below doesn't work -- both X & Y quantities are data quantites.
## Plotter doesn't know what to do with this.
#test_plotting( "mp.plotxy(x='data', y='model'" )

test_plotting( "mp.plotoptions(plotsymbol='o')" )
test_plotting( "mp.plotxy(x='antenna1',y='field_id')" )
test_plotting( "mp.plotxy(x='scan_number',y='time')" )
test_plotting( "mp.plotxy(x='scan_number',y='weighteddata')" ) 
test_plotting( "mp.plotxy(y='data',x='antenna1')" )

### Some of these have issues with data/time
print >>logfile, "TESTING uvdist with different data types"
test_plotting( "mp.plot( 'uvdist', 'weighteddata' )" )
test_plotting( "mp.plot( 'uvdist', 'residual' )" )
test_plotting( "mp.plot( 'uvdist', 'corrected' )" )
test_plotting( "mp.plot( 'uvdist', 'weighted_model' )" )
test_plotting( "mp.plot( 'uvdist', 'model' )" )
test_plotting( "mp.plot( 'uvdist', 'residual_corrected' )" )
test_plotting( "mp.plot( 'uvdist', 'weighted_corrected' )" )

### Some of these have issues with data/time
print >>logfile, "TESTING plotxy with ITERATIONS"
test_plotting( "mp.plotxy(x='antenna1',y='data', iteration='field_id' )" ) 
test_plotting( "mp.plotxy(x='time',y='data', iteration='field_id')" )
## Doesn't work yet.
#test_plotting( "mp.plotxy(x='time',y='data', iteration='channel')" )     
#test_plotting( "mp.plotxy(x='channel',y='data', iteration='time')" )
test_plotting( "mp.plotxy(x='channel',y='data', iteration='antenna1')" )
test_plotting( "mp.plotxy(x='channel',y='data', iteration='baseline')" )
test_plotting( "mp.plotxy(x='channel',y='data', iteration='field_id')" )
test_plotting( "mp.plotxy(x='baseline',y='data', iteration='field_id')" )
## not implemented yet.
#test_plotting( "mp.plotxy(x='baseline',y='data', iteration='channel')" )
test_plotting( "mp.plotxy(x='baseline',y='data', iteration='antenna1')" )
#test_plotting( "mp.plotxy(x='baseline',y='data', iteration='time')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >>logfile,"\n"

#######################################################################
## test clearflags()
## MAY WISH TO COMMENT THIS OUT AS IT CLEARS ALL FLAGS!
print >>logfile, "Testing FLAGGING and UNFLAGGING" 
#test_plotting( "mp.open(ngc5921PATH)" )
#test_plotting( "mp.clearflags()" )
test_plotting( "mp.emperorsNewClose()" )

#######################################################################
## test flagging on antennas and transmitting to MAIN table
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plot( type='array')" )
test_plotting( "mp.markregion( region=[-400, -300, -400, 250] )" )
test_plotting( "mp.markregion( region=[ 0, 50, -100, 50] )" )
test_plotting( "mp.flagdata()" )
test_plotting( "mp.markregion(region=[ 0, 50, -100, 50] )" )
test_plotting( "mp.unflagdata()" )
test_plotting( "mp.markregion( region=[-400, -300, -400, 250] )" )
test_plotting( "mp.unflagdata()" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
#test_plotting( "mp.close()" )

#######################################################################
## test flagging after setdata()
#  Temporarily ommited since MSSeletion is rejecting field selections
print >>logfile, "Tesing FLAGGING with spectral channels" 
#
# Take out the close and open and things crash; bug for HongLin
#
test_plotting( "mp.close()" )
test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.setdata( field='1', spw='0:0~10^5' )" )
test_plotting( "mp.plot( type='vischannel')" )
test_plotting( "mp.markregion( region=[4.1,10.9,4.0,8.0] )" )
test_plotting( "mp.flagdata()" )
test_plotting( "mp.markregion( region=[4.1,10.9,4.0,8.0] )" )
test_plotting( "mp.unflagdata()" )
#test_plotting( "mp.done()" )
test_plotting( "mp.close()" )

## test multi-spwids
test_plotting( "mp.open(ngc7538PATH)" )
test_plotting( "mp.clearflags()" )
test_plotting( "mp.setdata( field='1', spw='0:6~10^2')" )
test_plotting( "mp.plot( type='vischannel')" )
test_plotting( "mp.markregion( region=[30, 40, 60, 70] )" )
test_plotting( "mp.flagdata()" )
test_plotting( "mp.markregion( region=[30, 40, 60, 70] )" )
test_plotting( "mp.unflagdata()" )
#test_plotting( "mp.done()" )
print >>logfile,"\n"


#######################################################################
## iteration test
print >>logfile, "ITERATION test" 
test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions( subplot=221 )" )
test_plotting( "mp.plot( type='vischannel',iteration='antenna1')" )
test_plotting( "mp.iterplotnext()" )
test_plotting( "mp.iterplotnext()" )
test_plotting( "mp.iterplotstop()" )
test_plotting( "mp.close()" )
#test_plotting( "mp.done()" )
print >>logfile,"\n"

#######################################################################
## test BIMA data
print >>logfile, "Testing BIMA data" 
test_plotting( "mp.open(g1310_0506PATH)" )
test_plotting( "mp.plot( type='uvcoverage')" )
test_plotting( "mp.plot( type='vischannel')" )
test_plotting( "mp.close()" )
#test_plotting( "mp.done()" )
print >>logfile,"\n"

#######################################################################
## test vistime()
print >>logfile, "Testing VISTIME" 
test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.setdata( spw='0:0~50^5' )" )
test_plotting( "mp.plot( type='vistime')" )
test_plotting( "mp.plotoptions(overplot=True,plotsymbol='b+')" )
test_plotting( "mp.setdata()" )
test_plotting( "mp.setdata( spw='0:2~25^2', chanavemode='scalarstep', corravemode='scalarstep' )" )
test_plotting( "mp.plot( type='vistime',value='corrected')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >>logfile,"\n"

##################################################
## test setepectral()
# Temporarly commented out since MSSelection is failing on field
# selections.
print >>logfile, "Testing SET SPECTRAL" 
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.setdata( field='1', spw='0:0~50^5', chanavemode='scalarstep')" )

test_plotting( "mp.plot( type='uvdist')" )
test_plotting( "mp.plotoptions(overplot=true, plotcolor='g')" )

## should reset spectral averaging *and* data column
test_plotting( "mp.setdata()" )

## NOTE: I'm using the same spectral setup as the first time!!
## this seems to be the key and the only time it fails
test_plotting( "mp.setdata( spw='0:0~50^5', chanavemode='scalarstep' )")

## note: this gives the 'data' column labeled as 'corrected' column
test_plotting( "mp.plot( type='uvdist',column='corrected')" )   
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >>logfile,"\n"

##################################################
## testing iteration
print >>logfile, "Testing ARRAY plotting"
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plot( type='array')" )
test_plotting( "mp.close()" )
print >>logfile,"\n"

#################################################
## overplot test
print >>logfile, "Testing OVERPLOT"
test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plot( type='vistime', value='phase')" )
test_plotting( "mp.plotoptions( overplot=1, plotcolor='b' )" )
test_plotting( "mp.plot( type='vistime')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )

##test_plotting( "mp.done()" )

##
##test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions( overplot=0,plotcolor='r' )" )
test_plotting( "mp.plot( type='vischannel', value='phase')" )
test_plotting( "mp.plotoptions( overplot=1,plotcolor='g' )" )
test_plotting( "mp.plot( type='vischannel')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
#test_plotting( "mp.close()" )

##
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions( overplot=0,plotcolor='r' )" )
test_plotting( "mp.plot( type='baseline', value='phase')" )
test_plotting( "mp.plotoptions( overplot=1 )" )
test_plotting( "mp.plot( type='baseline')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
test_plotting( "mp.emperorsNewclose()" )

##
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions( overplot=0,plotcolor='r' )" )
test_plotting( "mp.plot( type='uvdist', value='phase')" )
test_plotting( "mp.plotoptions( overplot=1, plotcolor='g' )" )
test_plotting( "mp.plot( type='uvdist')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
test_plotting( "mp.emperorsNewclose()" )
##
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions( overplot=0, plotcolor='r' )" )
test_plotting( "mp.plot( type='elevation', value='phase')" )
test_plotting( "mp.plotoptions( overplot=1, plotcolor='k' )" )
test_plotting( "mp.plot( type='elevation')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
test_plotting( "mp.emperorsNewClose() " )
print >>logfile, "\n" 

##############################################################################
## multi-plot test
print >>logfile, "MULTI PLOT test" 
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.clearplot()" )
test_plotting( "mp.plotoptions( subplot=221 )" )
test_plotting( "mp.plot( type='array')" )
test_plotting( "mp.plotoptions( subplot=222)" )
test_plotting( "mp.plot( type='uvdist')" )
test_plotting( "mp.plotoptions(subplot=223)" )
test_plotting( "mp.plot( type='uvcoverage')" )
test_plotting( "mp.plotoptions(subplot=224)" )
test_plotting( "mp.plot( type='baseline')" )
test_plotting( "mp.plotoptions(subplot=222,overplot=1, plotcolor='g')" )
test_plotting( "mp.plot( type='uvdist',column='corrected_data')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )


#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plotoptions( subplot=221 )" )
test_plotting( "mp.plot( type='array')" )
test_plotting( "mp.plotoptions(subplot=222)" )
test_plotting( "mp.plot( type='uvcoverage')" )
test_plotting( "mp.plotoptions( subplot=212 )" )
test_plotting( "mp.plot( type='uvdist')" )
test_plotting( "mp.plotoptions(overplot=1, plotcolor='b')" )
test_plotting( "mp.plot( type='uvdist',column='corrected_data')" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >>logfile, "\n" 

##################################################################################
## test of flagdata()
print >>logfile, "Another FLAGGING test" 
#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plot( type='vischannel')" )
## flag some data with specific region
test_plotting( "mp.markregion(region=[59,63,0.0,0.2])" )

# view data with region flagged
test_plotting( "mp.flagdata()" )
test_plotting( "mp.markregion(region=[59,63,0.0,0.2])" )
test_plotting( "mp.unflagdata()" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )

#test_plotting( "mp.open(ngc5921PATH)" )
test_plotting( "mp.plot( type='vischannel')" )                  # really flaged?
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >> logfile, "\n" 

##################################################################################
## test refreshment of axes
print >>logfile, "Testing refreshment of axes - SET DATA" 

#test_plotting( "mp.open(ngc5921PATH)" )

# look at antenna locations -
test_plotting( "mp.plot( type='array')" )                 

# look at phase versus channel
test_plotting( "mp.plot( type='vischannel',value='phase')" )

 # flag some data with specific region
test_plotting( "mp.markregion(region=[59,63,0.0,0.2])" )

# view data with region flagged
test_plotting( "mp.flagdata()" )

# Set up 3 channels with a width of 20 channels and starting at channel 1
test_plotting( "mp.setdata( spw='0:0~20^3', chanavemode='scalarchunk' )" )
test_plotting( "mp.plot( type='vistime')" )

# Clear the flags.
test_plotting( "mp.setdata()" )
test_plotting( "mp.plot( type='vischannel',value='phase')" )
test_plotting( "mp.markregion(region=[59,63,0.0,0.2])" )
test_plotting( "mp.unflagdata()" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
test_plotting( "mp.close()" )


##
# call setdata(..) before call any other methods
#
# Comments about setdata() method:
# Based on the present structure of MSSelection, TablePlot and MsPlot, the best
# policy is for the users to perform all the data selection within one setdata() 
# call in order to avoid conflict between criteria(MSSelection) and only the last
# call taking effect( TablePlot ). In the future, in order to make this more robust
# we need:
#    1. MSSelection add methods to check if all the select rules set by user are
#       consistent to each other
#    2. TablePlot add a method, say, resetTable(), so that the previously set Table 
#       will be cleaned off.
#
# temporarily out until fixed.

#
# Rearranged so all those from one measurement set are together!
print >>logfile, "\nBegin testing of setdata with ngc7538"
test_plotting( "mp.open( ngc7538PATH )" )

## Those commented out DON'T work
test_plotting( "mp.setdata( baseline='5&6' )" )                      
test_plotting( "mp.setdata( baseline='5,6,7' )" )
test_plotting( "mp.setdata( baseline='1,2,3,4' )" )
# The next three throw exceptions.
test_plotting( "mp.setdata( baseline='5 & *' )" )
test_plotting( "mp.setdata( baseline='(5~8)&(9,10)' )" )
test_plotting( "mp.setdata( baseline='(5~8)&*' )" )

#The next two throw exceptions
test_plotting( "mp.setdata( field='NGC*,1328*' )" )
test_plotting( "mp.setdata( field='A' )"  )          
test_plotting( "mp.setdata( field='0,1,2' )" )
# The next three throw exceptions.
test_plotting( "mp.setdata( uvrange='10~25kl' )" )      
test_plotting( "mp.setdata( uvrange='25kl:5%' )" )
test_plotting( "mp.setdata( spw='0~1:0~3', correlation='rr rl')" )
test_plotting( "mp.setdata( spw='0:0~3,1:0~3', correlation='rr' )" )
test_plotting( "mp.setdata( spw='0~1:3~6',correlation='rl'  )" )
test_plotting( "mp.setdata( spw='0~1:1~3^2', correlation='rr' )" )

test_plotting( "mp.setdata( spw='0~1:0~3', correlation='rr,rl' )" )
test_plotting( "mp.setdata( spw='0:1~3^2,1:1~3^2', correlation='rr' )" )

test_plotting( "mp.setdata( spw='0:16~40^1' )" )
test_plotting( "mp.setdata( spw='1' )" )                               
test_plotting( "mp.setdata( spw='1,3' )" )
### NOTE This fails, but shouldn't fail since there are uvdists >280km
test_plotting( "mp.setdata( baseline='5&6', spw='1', field='N5*', uvrange='>280km', time='', correlation='rr' )" )
test_plotting( "mp.close()" )


## Again those commented out don't work!
print >>logfile, "\nBegin testing of setdata with 3C273XC1"
test_plotting( "mp.open(_3C273XC1PATH)" )
test_plotting( "mp.setdata( baseline='VLA:N*' )" )
test_plotting( "mp.setdata( baseline='5:R & *' )" )
test_plotting( "mp.setdata( baseline='5:R & 7:L' )" ) 
test_plotting( "mp.setdata( baseline='5:R & (3,4,7,8):L' )" )

##
test_plotting( "mp.setdata( field='3C*' )" )
test_plotting( "mp.setdata( field='N592*' )" ) 
test_plotting( "mp.setdata( field='0~3' )" )   

test_plotting( "mp.setdata( field='0')" ) 
test_plotting( "mp.setdata( field='1')" ) 

## TIP: do not set range conflic with other calls!
test_plotting( "mp.setdata( uvrange='>0l' )" )     
test_plotting( "mp.setdata( uvrange='>25kl' )" ) 
test_plotting( "mp.setdata( uvrange='<125kl' )" )
test_plotting( "mp.setdata( uvrange='10~25kl' )" ) 
test_plotting( "mp.setdata( uvrange='25kl:5%' )" ) 
test_plotting( "mp.setdata( uvrange='0.02Ml:5%' )" ) 

## did not work. MSSelection error:type mismatch!
test_plotting( "mp.setdata( time='1989/06/27/03:31:40', correlation='rr' )")
## MsPlot takes care of this.

##
## set all the criteria in one call. works ( for 3C273XC1.ms)
## Temporarily commented out MS selection failing on field selections
test_plotting( "mp.setdata( baseline='5 & 6', spw='0', field='3C273', uvrange='>25kl', time='', correlation='rr' )" )

##
## test spwNames and spwIndex
test_plotting( "mp.setdata( spw='0:1~1^1', correlation='rr rl' )" )

##
test_plotting( "mp.setdata( spwNames='0', correlation='rr' )" )
test_plotting( "mp.close()" )
#test_plotting( "mp.done()" )
print >> logfile, "\n"

##
###############################################################################
# the following condition must be met:
# nchan*step+ start <= (numberOfChannelOriginally-1))

print >>logfile, "Testing SET SPECTRAL with NGC7538" 
test_plotting( "mp.open( ngc7538PATH )" )
test_plotting( "mp.setdata( spw='0~200:0~6^3', chanavemode='vectorchunk' )" )

##
##
## work for ngc7538.ms
test_plotting( "mp.setdata( spw='0~1:1~8^2,0~1:4~10^4', field='1' )" )
test_plotting( "mp.setdata( spw='0~1:1~2,0~1:4~4', field='1' )" )
test_plotting( "mp.setdata( spw='0~1:2~4,0~1:>10', field='1' )" )
test_plotting( "mp.setdata( spw='0~1:1~3^2,0~1:4~10^3', field='1' )" )

##
test_plotting( "mp.setdata( field='1', spw='0~50:0~8^3,0~50:4~10^2' )" )
test_plotting( "mp.close()" )
#test_plotting( "mp.done()" )
print >>logfile, "\n" 

#############################################################################
## test methods
print >>logfile, "Testing SIMPLE functions" 

test_plotting( "mp.open( ngc5921PATH )" )
test_plotting( "mp.plot( type='uvcoverage')" ) 
test_plotting( "mp.plot( type='array')" )      
test_plotting( "mp.plot( 'uvdist','data','amp')" )
test_plotting( "mp.plot( 'uvdist')" )            
test_plotting( "mp.plot( type='uvdist', column='data', value='amp')" )
#
test_plotting( "mp.plot( type='vistime')" ) 
#test_plotting( "mp.plot( type='vistime', column='data', value='amp', iteration='antenna1')" ) 
#
test_plotting( "mp.plot( type='vischannel')" )
test_plotting( "mp.plot( type='vischannel', column='data', value='amp')" )
# not work yet even if spwNames are set.
test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='baseline')" ) 
test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='antenna1')" )
#test_plotting( "mp.plot( type='vischannel', column='data', value='amp', iteration='time')" )
test_plotting( "mp.plotxy()" )

## draw nothinng.
test_plotting( "mp.plotxy( x='antenna1', y='antenna1' )" ) 
test_plotting( "mp.plotxy( x='antenna1', y='antenna2' )" )

## not working yet.
# test_plotting( "mp.plotxy( x='channel', y='antenna2' )" ) 
test_plotting( "mp.plotxy( x='antenna1', y='data', yvalue='amp' )" ) 
test_plotting( "mp.plotxy( x='uvdist', y='data', iteration='antenna1', yvalue='amp' )" ) 
test_plotting( "mp.plotxy( x='antenna1', y='feed1' )" )
test_plotting( "mp.plot( type='baseline', column='data', value='amp' )" )

# create the msplot object and setdata() first
test_plotting( "mp.plot( type='hourangle', column='data', value='amp' )" )

# create the msplot object and setdata() first
test_plotting( "mp.plot( type='azimuth', column='data', value='amp' )" )

# create the msplot object and setdata() first
test_plotting( "mp.plot( type='elevation', column='data', value='amp' )" )

# create the msplot object and setdata() first
test_plotting( "mp.plot( type='parallacticangle',  column='data', value='amp' )" )
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >>logfile, "\n" 

##############################################################################
## For iterplotstart, cannot pass in the record properly!
print >>logfile, "Testing ITERPLOT again but with OVERPLOT" 
#test_plotting( "mp.open( ngc5921PATH )" )
test_plotting( "mp.clearplot()" )
test_plotting( "mp.plotoptions( title='Amplitude vs UVdist (iterating over Baseline)')" )
test_plotting( "mp.plotoptions( xlabel='uvdist', ylabel='amplitude')" )
test_plotting( "mp.plotoptions( overplot=0, plotsymbol='b,' )" )
test_plotting( "mp.setdata( field='0', spw='0:4~50')" )
test_plotting( "mp.plot( type='uvdist' )" )
test_plotting( "mp.plotoptions( overplot=1, plotsymbol='c,' )" )
for ant in range( 1,28 ):
    mp.setdata( field='0', baseline=str( ant ) )
    test_plotting( "mp.plot( type='uvdist' )" )
    test_plotting( "mp.plotoptions( overplot=1, plotsymbol='c,', replacetopplot=1 )" )
    
test_plotting( "mp.reset()" )
test_plotting( "mp.clearplot()" )
print >>logfile, "\n" 

###############################################################################
#
print >>logfile, "Miscellaneous tests" 
#test_plotting( "mp.open( ngc5921PATH )" )
test_plotting( "mp.clearplot()" )

# works, but with some odd behaviour.
#test_plotting( "mp.flagdata()" )         

# works, but with some odd behaviour. -- NO ZOOM PLOT
#test_plotting( "mp.zoomplot(panel=1,direction=1)" )            

#test_plotting( "mp.done()" )
print >>logfile, "\n" 


print >>logfile, '\nDONE all tests'
