# flagging.py:  A test suite for testing the MS plot tool (mp)
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
# @author Shannon Jaeger
# @version 
#############################################################################

# Unit test for flagging capibilities.
#
# Data requirements.
# Only dataset needed is ngc5921.ms
#
# This test is expected to be run in this directory.
# (xmlcasa/implement/ms/scripts)
#
import sys;
import filecmp;
import os;

####
## NOTES:
## Need to add flagging tests for each type of plot possible with ms plot.
##
## Need to add tests that use ALL of the arguments to the flagging functions
##
## overplotting flagged/unflagged data doesn't work if the flags aren't
## written to file
##
## array labels are wrong.


###############################################################################
### Define the location of the various measurement sets used for testing.
ngc5921PATH = 'data/ngc5921.ms';

###############################################################################
### Define the global variables that store the test results
numTests           = 0;
numFailedTests     = 0;
numSuccessfulTests = 0;

failedTests = [];


###############################################################################
### Function test_plotting()
### This function is used to provide a consistent way of executing each
### of the commands, to ensure proper error reporting, ...
def test_plotting( fnName='' ) :
    global numTests, numFailedTests, numSuccessfulTests, failedTests
    #print 'Executing command ... '
    #print '     ', fnName
    numTests += 1;
    try:
        # Execute the command
        if ( not eval( fnName ) ) :
            
            failedTests += [ str( numTests - 1 ) + ': ' + fnName ];
            numFailedTests += 1;
            print 'Execution FAILED for command ...' 
            print '     ', fnName
        else: 
            #print 'SUCCESSFUL execution of command ...' 
            #print '     ' + fnName
            numSuccessfulTests += 1;
    except:
        print 'EXCEPTION occured with command ...' 
        print '     ', fnName
        failedTests += [ str( numTests - 1 ) + ': ' + fnName ];
        numFailedTests +=1

###############################################################################
### Function prompt_yesno( String prompt )
### Prompt the user for a yes/no answer to a question.
### 'Y' is returned if yes is answered, and 'N' is returned of no is given.
### The user is reprompted if an invalid answer is given.
def prompt_yesno( prompt ) :
    firstTime = False;
    ans = '';
    while( len(ans) < 1 or not ( ans[0] == 'Y' or ans[0] == 'N' ) ) :
        if ( not firstTime ) :
            print( "Invalid answer: " + ans + ". Please answer yes or no" );
        ans = raw_input( "pompt" + ", (y,n) > " );
        ans = ans.upper();
        firstTime = True;
        
    return ans[0];

        

###############################################################################
### Function print_results()
### This function displays a summary of the tests.  Including the list
### of commands that failed.
def print_results() :
    print("#############################################################################");
    print("                                 TEST REULTS\n" );
    print("Number of Tests:               %3d" % numTests );
    print("Number of SUCCESSFUL tests:    %3d" % numSuccessfulTests );
    print("Number of FAILED tests:        %3d" % numFailedTests );
    print("\n List of FAILED commands" );
    for i in range (0,numFailedTests) :
        print( failedTests[i] );
############################################################################ 
        
############################################################################ 
## Make sure the testing area is clean
##
## This is the rather complicated way of doing a rm *.png
for root, dirs, files in os.walk( '.', topdown=false ) :
    for name in files :
        fullName = os.path.join( root, name );
        if ( os.path.dirname( fullName ) == '.' and len( name.split( 'png' ) ) > 1 ) :
            os.remove( fullName )


############################################################################
## test clearflags()
## 
test_plotting( "mp.open(ngc5921PATH)" );
#test_plotting( "mp.clearflags()" );
#test_plotting( "mp.close()" );

##################################################################################
print("#############################################################################");
print( "TEST FLAGGING SPECIFYING REGIONS ON CMD LINE" );
print( "There will be no user input for this test." );
print("#############################################################################");

#test_plotting( "mp.open(ngc5921PATH)" );
#test_plotting( "mp.clearflags()" );
#Since clearflags clears more flags then we want we are unflagging the whole area!!!!
test_plotting( "mp.plot( type='vischannel')" );
test_plotting( "mp.markregion(region=[0,70,0,80])"  );
test_plotting( "mp.unflagdata()" );

## flag some data with specific region
test_plotting( "mp.markregion(region=[55,63,0.0,2.0])" );
test_plotting( "mp.flagdata()" );
pl.savefig( "flagTest1.png" );

numTests += 1
if ( not filecmp.cmp( 'flagTest1.png', 'data/flagTest1.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest1"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

## View just the flags, need to call vischannel again to do this.
test_plotting( "mp.plotoptions( showflags=1 )" );
test_plotting( "mp.plot( type='vischannel')" );
pl.savefig( "flagTest2.png" );
numTests += 1
if ( not filecmp.cmp( 'flagTest2.png', 'data/flagTest2.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest2"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

# Now close the ms plot, and reopen to see if the flagging really
# was stored on disk!
test_plotting( "mp.done()" );

test_plotting( "mp.open(ngc5921PATH)" );
test_plotting( "mp.plot( type='vischannel')" );
test_plotting( "mp.plotoptions( showflags=1, overplot=1 )" );
test_plotting( "mp.plot( type='vischannel')" );
pl.savefig( 'flagTest3.png');
numTests += 1
if ( not filecmp.cmp( 'flagTest3.png', 'data/flagTest3.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image 3 DOESN'T match flagTest3"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

#test_plotting( "mp.clearflags()" );
##Since clearflags doesn't work yet, we'll unflag everything.
test_plotting( "mp.markregion(region=[0,70,0,80])"  );
test_plotting( "mp.unflagdata()" );
test_plotting( "mp.reset()" );
print("\n" );


###############################################################################
## Unflag Test
##
print("#############################################################################");
print( "TEST UNFLAGGING DATA" )
print( "There will be no user input for this test." );
print("#############################################################################");

test_plotting( "mp.open(ngc5921PATH)" );
test_plotting( "mp.plot( type='vischannel')" );
#test_plotting( "mp.clearflags()" );
##Since clearflags doesn't work yet, we'll unflag everything.
test_plotting( "mp.markregion(region=[0,80,0,70])"  );
test_plotting( "mp.unflagdata()" );


## flag some data with specific region
test_plotting( "mp.markregion(region=[55,63,0.0,2.0])" );
# view data with region flagged
test_plotting( "mp.flagdata()" );
pl.savefig( "flagTest4.png" );

numTests += 1
if ( not filecmp.cmp( 'flagTest4.png', 'data/flagTest1.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image 4 DOESN'T match flagTest1"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

    
test_plotting( "mp.markregion(region=[55,58,0.0,2.0])" );    
test_plotting( "mp.unflagdata()" );
pl.savefig( "flagTest5.png" );

numTests += 1
if ( not filecmp.cmp( 'flagTest5.png', 'data/flagTest5.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image 5 DOESN'T match flagTest5"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

#Test if the flagging was written to disk
test_plotting( "mp.reset()" );
test_plotting( "mp.clearplot()" );
test_plotting( "mp.closeMS()" );

test_plotting( "mp.open(ngc5921PATH)" );
test_plotting( "mp.plot( type='vischannel')" );
pl.savefig( "flagTest6.png" );

numTests += 1
if ( not filecmp.cmp( 'flagTest6.png', 'data/flagTest5.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image 5 DOESN'T match flagTest6"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

#test_plotting( "mp.clearflags()" );
##Since clearflags doesn't work yet, we'll unflag everything.
test_plotting( "mp.markregion(region=[0,80,0,70])"  );
test_plotting( "mp.unflagdata()" );    
#test_plotting( "mp.done()" );
print("\n" );

#######################################################################
## test flagging on antennas and transmitting to MAIN table
##
print("#############################################################################");
print("FLAG TEST on ARRAY");
print("This test will test the basic marking, flagging, and unflagging of");
print("data, please be sure to follow the directions!" );
print("#############################################################################");
#test_plotting( "mp.open(ngc5921PATH)" );
test_plotting( "mp.plot( type='array')" );
## Since clearflags isn't implemented we unflag the entire region.
#mp.clearflags();
test_plotting( "mp.markregion( region=[-600, 600, -600, 600 ] )" );
test_plotting( "mp.unflagdata(  )" );

### COMMENT the next 3 lines to make this test interactive
test_plotting( "mp.markregion(region=[-40, 40, 3, 167 ] )" );
test_plotting( "mp.markregion(region=[-32, 50, -110, -55 ] )" );
test_plotting( "mp.markregion(region=[-410, -200, -230, -80] )" );

### UNCOMMENT  The section below to make this an interactive test.
#test_plotting( "mp.markregion()" );
#print( "Please flag, by making boxes around them on the the display, the");
#print("following antennas: " );
#print("   28 & 21" );
#print("   11, 13, & 15" );
#print("   4" );
#raw_input( "When you are finished PRESS return to continue > " )

test_plotting( "mp.flagdata()" );
pl.savefig( "flagTest7.png" );
numTests += 1
if ( not filecmp.cmp( 'flagTest7.png', 'data/flagTest7.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest6"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

test_plotting( "mp.plotoptions( overplot=1, showflags=1 )" );
test_plotting( "mp.plot( type='array')" );
pl.savefig( "flagTest8.png" );
if ( not filecmp.cmp( 'flagTest8.png', 'data/flagTest8.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest8"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

### Test unflagging of a couple of antennas
test_plotting( "mp.plotoptions( overplot=0, showflags=0 )" );
test_plotting( "mp.plot( type='array')" );

### COMMENT the next 3 lines to make this test interactive
test_plotting( "mp.markregion(region=[-20, 45, 100, 150 ] )" );
test_plotting( "mp.markregion(region=[-40, 25, 110, 20 ] )" );

### UNCOMMENT  The section below to make this an interactive test.
#test_plotting( "mp.markregion()" );
#print( "Please unflag, by making a box on the display, where the  following" );
#print( "purple antennas are shown: " );
#print( "    4 -- just below antenna 2");
#print( "   11 -- just below antenna 9");
#raw_input( " PRESS return to continue > " ); 
test_plotting( "mp.unflagdata()" );
test_plotting( "mp.plotoptions( overplot=1, showflags=1 )" );
#test_plotting( "mp.plot( type='array')" );
pl.savefig( "flagTest9.png" );

numTests +=1;
if ( not filecmp.cmp( 'flagTest9.png', 'data/flagTest9.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest9"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1

##test_plotting( "mp.clearflags()" );
##Since clearflags doesn't work yet, we'll unflag everything.
test_plotting( "mp.plotoptions( overplot=0, showflags=0 )" );
test_plotting( "mp.markregion( region=[-600, 600, -600, 600 ] )" );
test_plotting( "mp.unflagdata()" );
#test_plotting( "mp.done()" );


#######################################################################
## test flagging after selecting spectral data
#  Temporarily ommited since MSSeletion is rejecting field selections
print("#############################################################################");
print( "FLAGGING WITH SPECTRAL CHANNELS" )
print( "This test displays data that has spectral channels.  You will be" );
print( "asked to flag, and unflag areas - no particular areas. You will then" );print( "be asked if the image displayed changed appropriately." );
print("#############################################################################");
#test_plotting( "mp.open(ngc5921PATH)" );
test_plotting( "mp.plot( type='vischannel')" );
#test_plotting( "mp.clearflags()" );
#Since clearflags doesn't work we are unflagging the whole area!!!!
test_plotting( "mp.markregion(region=[0,80,0,70])"  );
# This command is failing
test_plotting( "mp.unflagdata()" );

test_plotting( "mp.setdata( field='1',spw='>0:5~10^5')" );
test_plotting( "mp.plot( type='vischannel')" );

# Comment the next fes lines to make the test interative
test_plotting( "mp.markregion( region=[0,50,50,65] )" );
# This command is failing
test_plotting( "mp.flagdata(  )" );
pl.savefig( "flagTest10.png" );

numTests += 1
if ( not filecmp.cmp( 'flagTest10.png', 'data/flagTest10.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest10"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

# Uncomment the next few lines to make this test interactive.
#print( "Please mark, 'flag' areas by selecting them on the display. When" )
#junk=raw_input( "you are finished PRESS return to continue > " )
#test_plotting( "mp.flagdata()" );
#ans = prompt_yesno( "Are the flagged areas still shown" );
#numTests +=1;
#if ( ans[0]=='N' ):
#    numSuccessfulTests += 1;    
#else :
#    failedTests += [ str( numTests - 1 ) + ': Flagged areas ARE shown'];
#    numFailedTests += 1;


### WARNING: For some reason the flagged areas aren't plotted !
test_plotting( "mp.plotoptions( overplot=1, showflags=1 )" );
#test_plotting( "mp.setdata( field='1', spw='>0:5~10^5')" );
test_plotting( "mp.plot( type='vischannel')" );

# Comment the next fes lines to make the test interative
pl.savefig( "flagTest11.png" );

numTests += 1
if ( not filecmp.cmp( 'flagTest11.png', 'data/flagTest11.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest11"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;
# Uncomment the next few lines to make this test interactive.
#print( "The flagged and unflagged areas should both be shown now, the");
#print( "flagged areas are magenta in colour" );
#ans = prompt_yesno( "Are both the flagged and unflagged areas shown" );
#numTests +=1;
#if ( ans[0]=='Y' ):
#    numSuccessfulTests += 1;    
#else :
#    failedTests += [ str( numTests - 1 ) + ': flagged and unflagged areas NOT shown'];
#    numFailedTests += 1;

#Since clearflags doesn't work we are unflagging the whole area!!!!
#test_plotting( "mp.clearflags()" );
test_plotting( "mp.plot( type='vischannel')" );
test_plotting( "mp.markregion(region=[0,80,0,70])"  );
test_plotting( "mp.unflagdata()" );
test_plotting( "mp.reset()" );

## test multi-spwids???
## We don't seem to be able to specify multiple spectral channels!
test_plotting( "mp.open(ngc5921PATH)" );
test_plotting( "mp.setdata( field='1',spw='>0:6~9' )" );
test_plotting( "mp.plotoptions( overplot=0, showflags=0 )" );
test_plotting( "mp.plot( type='vischannel')" );

# Comment the next few lines to make the test interative
test_plotting( "mp.markregion( region=[2.5,3.5,0,80] )" );
test_plotting( "mp.flagdata()" );
pl.savefig( "flagTest12.png" );
numTests += 1
if ( not filecmp.cmp( 'flagTest12.png', 'data/flagTest12.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest12"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;

# Uncomment the next few lines to make this test interactive.
#test_plotting( "mp.markregion()" );
#print( "Please mark, 'flag' areas by selection them on the display. When" )
#raw_input( "you are finished PRESS return to continue > " )
#test_plotting( "mp.flagdata()" );
##ans = prompt_yesno( "Are the flagged areas shown" );
#numTests +=1;
#if ( ans[0]=='n' or ans[0]=='N' ):
#    numSuccessfulTests += 1;    
#else :
#    failedTests += [ str( numTests - 1 ) + ': flagged ARE shown'];
#    numFailedTests += 1;

test_plotting( "mp.reset()" );
test_plotting( "mp.plotoptions( showflags=1 )" );
#test_plotting( "mp.setdata( field='1',spw='>0:6~9')" );
test_plotting( "mp.plot( type='vischannel')" );

# Comment the next few lines to make the test interative
pl.savefig( "flagTest13.png" );
numTests += 1
if ( not filecmp.cmp( 'flagTest13.png', 'data/flagTest13.png' ) ) :
    failedTests += [ str( numTests - 1 ) + ": image DOESN'T match flagTest13"];
    numFailedTests += 1;
else :
    numSuccessfulTests += 1;
    
#print( "Only the magenta unflagged areas should both be shown now." );
##ans = prompt_yesno( "Are the only the unflagged (magenta) areas shown" );
#numTests +=1;
#if ( ans[0]=='y' or ans[0]=='Y' ):
#    numSuccessfulTests += 1;    
#else :
#    failedTests += [ str( numTests - 1 ) + ': More then the uflagged areas shown'];
#    numFailedTests += 1;

#Since clearflags doesn't work we are unflagging the whole area!!!!
#test_plotting( "mp.clearflags()" );
test_plotting( "mp.markregion(region=[0,60,1,6])"  );
test_plotting( "mp.unflagdata()" );

test_plotting( "mp.done()" );
print("\n")


############################################################################
## Display the results of the tests
##
print_results();


############################################################################ 
## Make sure the testing area is clean
##

## This is the rather complicated way of doing a rm *.png
#for root, dirs, files in os.walk( '.', topdown=false ) :
#    for name in files :
#        fullName = os.path.join( root, name );
#        if ( os.path.dirname( fullName ) == '.' and len( name.split( 'png' ) ) > 1 ) :

#            os.remove( fullName )

