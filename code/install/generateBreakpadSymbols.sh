#!/bin/bash

# # generateBreakpadSymbols.sh
# #
# # Generates the breakpad symbol file for a library or executable after 
# # they are built.  

# binary=$1 # name of the binary (e.g., libmsvis.so.1.2.3)
# dumper=$2 # path to o/s dependent symbol extracter program
# symbolRoot=$3 # directory root where symbols will reside
# moduleDirectory=$4 # directory holding the binary file

# # Move to the module directory and create a symbol file

# cd ${moduleDirectory}
# $dumper $binary > $binary.sym

# # Extract the binary's symbol hashcode from the first line of the symbol file.
# # Use hashcode to create the expected path for the symbol file

# hashCode=`head -n1 ${binary}.sym | awk '{print $4}'`
# hashCodePath=${symbolRoot}/${binary}/${hashCode}
# mkdir -p ${hashCodePath}

# # Move the symbol file into the appropriate place.  This should be
# # theBuildBinaryRoot/symbols/binaryName/hashCode/binaryName.sym

# mv ${binary}.sym ${hashCodePath}

# #echo "-- Created breakpad symbol file: ${hashCodePath}/${binary}.sym"