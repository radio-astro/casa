#
# listing.py
#
# Module for testing the output of listing tasks (eg. liscal and listvis).
#

import os, sys, re, math, decimal

#=============================================================================
# METHOD: compare
#
# Compare the listvis output (test) with the listvis STANDARD.  
#
# Parameters:
#   test     = list of strings
#   STANDARD = list of strings.  
#
def compare(test, standard):
    testFile = open(test,'r')
    testList = testFile.readlines()
    standardFile = open(standard,'r')
    standardList = standardFile.readlines()
    if (testList == standardList):
        return True
    else:
        return False
#=============================================================================

#=============================================================================
# METHOD: diffFiles
#
# Run diff on two files.  Write output to a file.
#
# Parameters:
#   test     = string; name of runtime output file
#   STANDARD = string; name of standard file
#   prefix   = string; prefix for diff output
#
def diffFiles(testOut, standardOut, prefix=""):
    diffOut = prefix + '.diff' # diff output
    print "  - Running command line utility 'diff' on output files."
    print "  - Writing to " + diffOut
    cmdstr = 'diff ' + standardOut + ' ' + testOut + ' > ' + diffOut
    print cmdstr
    os.system(cmdstr)
#=============================================================================

#=============================================================================
# METHOD: runTests
#
# Automate the testing of metadata and data.
#
# Parameters:
#   test     = string; name of runtime output file
#   standard = string; name of standard file
#   precision= string; lower limit of visibility precision allowed
#   prefix   = string; prefix for diff output
#
def runTests(test, standard, precision, prefix):

    equal = True
    
    # Test metadata
    print "  1. Checking that metadata agree."
    if (diffMetadata(test,standard,prefix=prefix)):
        print "  Metadata agree"
    else:
        print "  Metadata do not agree!"
        equal = False
    
    # Test data (floats)
    print "  2. Checking that data agree to within allowed imprecision..."
    print "     Allowed visibility imprecision is " + precision
    if (diffAmpPhsFloat(test,standard,prefix,precision)):
        print "  Data agree"
    else:
        print "  Data do not agree!"
        equal = False

    return equal
#=============================================================================


#=============================================================================
# METHOD: diffMetadata
#
# Compare all metadata.  Actually, this compares all non-float, non-space 
# data in the two files passed by filename.
#
# Parameters:
#   test     = string; name of runtime output file
#   STANDARD = string; name of standard file
#   prefix   = string; prefix for output file
#
def diffMetadata(testOut, standardOut, prefix=""):
    diffOut = prefix + '.diffMetadata' # output

    print "  - Comparing all non-floats in listing (ignore spaces)"

    testList = open(testOut,'r').readlines()
    stndList = open(standardOut,'r').readlines()

    # Pattern to capture floats
    floatPat = re.compile(r"[ |]([+-]?[0-9]*\.[0-9]+)")

    # Filter all floats from a list of strings
    def filterFloats(list):
        newList = []
        for linenum in range(len(list)):
            newList.append( floatPat.sub('x',list[linenum]) )
        return newList
            
    newTestList = filterFloats(testList)
    newStndList = filterFloats(stndList)

    # Remove all spaces from list of strings
    for linenum in range(len(newTestList)):
        newTestList[linenum] = newTestList[linenum].replace(' ','')
        newStndList[linenum] = newStndList[linenum].replace(' ','')

    # If everything is equal, return True
    if (newTestList == newStndList): return True
    
    # else... do the rest

    print "  - Writing differences to ", diffOut
    
    sys.stdout = open(diffOut,'w') # redirect stdout
    printDiff('','',[standardOut,testOut]) # Print some header info

    countDiff = 0 # Count number of different lines
    for linenum in range(len(newTestList)):
        if ( newTestList[linenum] != newStndList[linenum] ):
            countDiff += 1
            print "- (line ", linenum, ") Non-float data differs:" 
            printDiff(stndList[linenum],testList[linenum])

    print ""
    print "SUMMARY (diffMetadata): "
    print " %10i = Total number of lines with non-float differences" % countDiff 

    # Restore stdout
    sys.stdout = sys.__stdout__

    return False
#=============================================================================

#=============================================================================
# METHOD: diffAmpPhsFloat
#
# Compare floating point values of Amplitude and Phase.
#

def diffAmpPhsFloat(test, standard, prefix="", precision="1e-6"):

    # Preliminary messages
    floatOut = prefix + '.diffAmpPhsFloat'  
    print "  - Comparing float content of output."
    #print "  - Assuming all floats are Amplitude-Phase pairs!"
    print "  - Writing to " + floatOut

    sys.stdout = open(floatOut,'w')

    # Read in files
    testFile = open(test,'r')
    standardFile = open(standard,'r')
    testList = testFile.readlines()
    standardList = standardFile.readlines()

    # Reg exp for matching floats
    floatPat = re.compile(r"[ |]([+-]?[0-9]*\.[0-9]+)")

    # Verify same number of lines
    if ( len(standardList) != len(testList) ):
        print "- Standard and test files do not have the same number of lines."
        return False

    # Initialize some variables
    equal = True     # To start, assume floats are equal
    countUnequal = 0 # Count of unequal Amp-Phs pairs
    countBigDif  = 0 # Count of Amp-Phs pairs that differ above precision
    maxAmpDiff   = 0 # Maximum difference amplitude
    precision = decimal.Decimal(precision) # Allowed precision

    # For each line, compare Amp-Phs in both files
    
    for linenum in range(min(10, len(testList))):
        # Generate a list of all floats
        tFloatList = floatPat.findall(testList[linenum])
        sFloatList = floatPat.findall(standardList[linenum])
        #print 'tFloatList=', tFloatList
        # Test floats
        # Same number of floats in both lines?
        if ( len(tFloatList) != len(sFloatList) ):
            equal = False
            print "- (line ", linenum, ") Number of floats not equal:"
            printDiff(standardList[linenum],testList[linenum])
            continue
        # Number of floats per line should be even
        #elif ( len(tFloatList) % 2 ): 
            #print "- (line ", linenum, ") Odd number of floats! All must be Amp-Phs pairs!"
        #    continue
            #print "stopping listing.diffAmpPhsFloat now!"
            #return 
        
        # Compare all Amp-Phs pairs on this line
        for i in range(len(tFloatList)/2):

            # If the Amp or Phs not exactly equal
            if ( ( tFloatList[i*2] != sFloatList[i*2] ) or 
                 ( tFloatList[i*2+1] != sFloatList[i*2+1] ) ):

                countUnequal += 1

                # Calculate complex number for test file
                tamp = decimal.Decimal(tFloatList[i*2])
                tphs_deg = decimal.Decimal(tFloatList[i*2+1])
                pi = decimal.Decimal(str(math.pi))
                tphs = tphs_deg * pi / 180
                def dcos(x): return decimal.Decimal(str(math.cos(x)))
                def dsin(x): return decimal.Decimal(str(math.sin(x)))
                tre = dcos(tphs) * tamp
                tim = dsin(tphs) * tamp

                # Calculate complex number for standard file
                samp = decimal.Decimal(sFloatList[i*2])
                sphs_deg = decimal.Decimal(sFloatList[i*2+1])
                sphs = sphs_deg * pi / 180
                sre = dcos(sphs) * samp
                sim = dsin(sphs) * samp

                # Compute number of significant figures
                #   Find the minimum sig figs for both Amp-Phs pairs.
                #   Here significant figures means the number of
                #   digits displayed, not including an exponent.
                #   Examples: '0.00' has 3 sig figs
                #             '0.001' has 4 sig fig
                #             '-1e-15' has 1 sig fig
                def remove_exp(x): return x.split("e")[0]
                digitPattern = re.compile(r"([0-9])")
                minSigFigs = 8
                for i in [tamp, tphs_deg, tphs, samp, sphs_deg]:
                    num1 = str(i)
                    num1 = remove_exp(num1)
                    sigFigs = digitPattern.findall(num1)
                    if (len(sigFigs) < minSigFigs): minSigFigs = len(sigFigs)
                                    
                # Compute difference of real and imaginary parts
                re_diff = (sre - tre)
                im_diff = (sim - tim)

                # Compute the amplitude of the difference
                amp_diff = str(math.sqrt( re_diff**2 + im_diff**2 ))
                amp_diff = decimal.Decimal(amp_diff)
                
                # If necessary, reduce amp_diff to minimum sig figs
                #   I do this by taking advantage of the decimal module, 
                #   which can output and input a tuple.
                #   * I truncate the number. Rounding would be better,
                #     I just have not implemented it yet.
                adTup = amp_diff.as_tuple()
                digits_adTup = len(adTup[1])
                if (digits_adTup > minSigFigs):
                    amp_diff = decimal.Decimal( (adTup[0], 
                                                 adTup[1][0:minSigFigs], 
                                                 adTup[2]+(digits_adTup-minSigFigs) ) )

                # Keep track of the maximum Amp difference between both files
                if (amp_diff > maxAmpDiff): maxAmpDiff = amp_diff

                # Are Amp-Phs pairs equal within precision?
                if (amp_diff > precision):
                    if (countBigDif == 0): printDiff("","",[standard,test]) # Print header info
                    equal = False # test evaluates false
                    countBigDif += 1
                    print "- (line ", linenum, ") Amp,Phs differ by more than precision:" 
                    printDiff(standardList[linenum],testList[linenum])
                    print "  (Amp,Phs): (", samp,",",sphs_deg,") , (",tamp,",",tphs_deg,")"
                    print "  ( Re, Im): (", sre ,",",sim ,") , (",tre ,",",tim ,")"
                    print "  Min sig figs = ", minSigFigs
                    print "  Difference amplitude :", amp_diff, " > ", precision
                # else:
                #     print "- (line ", linenum, ") Amp,Phs within required precision:"
                #     printDiff(standardList[linenum],testList[linenum])
                #     print "  Difference amplitude :", amp_diff, " <= ", precision

    print ""
    print "SUMMARY (diffAmpPhsFloat): "
    print "  %10i = Total number of Amp-Phs pair differences" % countUnequal
    print "  %10i = Total number of differences above precision" % countBigDif
    print "  %10f = Input precision requirement" % precision
    print "  %10f = Largest difference amplitude" % maxAmpDiff

    # Restore stdout
    sys.stdout = sys.__stdout__
    
    return equal

#=============================================================================

#=============================================================================
# METHOD: printDiff
#
# Print two different strings
def printDiff(s1, s2, filenames=[]):
    if (filenames):
        print "Priting lines with differences for comparison:"
        print "  < ", filenames[0]
        print "  > ", filenames[1]
    else:
        print "< " + s1,
        print "> " + s2,

#=============================================================================

#=============================================================================
# METHOD: removeOut
#
# Remove old output file if it exists
#
# Parameters:
#   outputFile = string; file to be removed
def removeOut(outputFile):
    if (os.path.exists(outputFile)): 
        print "Removing old test file " + outputFile
        os.remove(outputFile)
#=============================================================================

#=============================================================================
# METHOD: resetData
#
# Reset local data?  Returns True or False.
#
# Test data is typically acquired from the local data repository, and is often
# copied to the local directory (in the case of a Measurement Set) or converted
# to a measurement set in the local directory (in the case of a FITS file).
# This method determines whether or not the test should rebuild the data in the
# working directory or use what already exists.
#
# Parameters:
#   msname = list of strings; Names of Tables that will be created
#
def resetData(msname, automate=True):
    
    reset_for_test = " "

    # If running in automated mode, always refresh data
    if (automate): 
        return True
    
    # Does the data already exist?
    for dataFile in msname:
        if (not os.path.exists(dataFile)): return True

    # If data exists, prompt for user direction
    while ( ( reset_for_test[0] != "y" and 
              reset_for_test[0] != "n" and 
              reset_for_test[0] != "\n" ) ):
        for dataFile in msname:
            sys.stdout.write("  " + dataFile+"\n")
        sys.stdout.write("Reset above data from scratch? y/[n]: ")
        reset_for_test = sys.stdin.readline()
    if (reset_for_test[0] == "y"): return True
    else: return False
#=============================================================================

#=============================================================================
# METHOD: listcalFix
#
# Remove first line of listcal output.
# (First line contains hard-coded path to input files)

def listcalFix(listcalOut):
    os.system('mv ' + listcalOut + ' ' + listcalOut + '.tmp')
    os.system('tail -n +2 ' + listcalOut + '.tmp > ' + listcalOut)
    os.system('rm -f ' + listcalOut + '.tmp')
    return
#=============================================================================

#=============================================================================
# METHOD: reduce
#
# Reduce the size of the output listing by printing every Nth line of file.
#
# This is necessary because the standard listings are stored in the data
# repository and we want to keep these files small.
#
# Parameters:
#   N = integer; beginning with line one, print every Nth line of the 
#                 original file
def reduce(filename, N):

# This command prints every 11th line of the file:
# sed -n '1,${p;n;n;n;n;n;n;n;n;n;n;}' infile > outfile

    infile = open(filename,'r')
    listing = infile.readlines()

    rangeL = range(len(listing))

    # picks holds the indices of listing that I want to keep
    def f(x): 
        return x % (N-1) == 0
    picks = filter(f,rangeL)
    
    reducedListing = [ listing[i] for i in picks ]

    #for i in range(20): print reducedListing[i]

    return reducedListing
#=============================================================================

## #=============================================================================
## # METHOD: studyDiff
## #
## # Return: 1) the number of leading identical digits
## #         2) the order of magnitude of the difference.
## def studyDiff(sfloat, tfloat, diffDict=diffDict_template):
##     import math
##     order = 1
##     # Order of magnitude of the difference
##     diffOrd = int(round(math.log10(abs(sfloat - tfloat))))
##     # Order of magnitude of sfloat
##     ord = int(round(math.log10(abs(sfloat))+0.5))
##     # Number of digits in common, starting from left
##     digits = ord - diffOrd - 1
##     # Sign difference?
##     signDiff = ( abs(sfloat - tfloat) != abs( abs(sfloat) - abs(tfloat) ) )
##     if (signDiff): digits = 0 # correct digits
##     # Absolute difference
##     absDiff = abs(sfloat - tfloat)
##     # Relative difference
##     relDiff = abs( (sfloat - tfloat) / sfloat)
##     print "Difference order = ", diffOrd
##     print "Digits same = ", digits
##     print "Sign difference = ", signDiff
##     print "Absolute difference = ", absDiff
##     print "Relative difference = ", relDiff
## 
##     diffDict['diffOrd'].append(diffOrd)
##     diffDict['digits'].append(digits)
##     diffDict['signDiff'].append(signDiff)
##     diffDict['absDiff'].append(absDiff)
##     diffDict['relDiff'].append(relDiff)
## 
##     return 
## 
## #=============================================================================

