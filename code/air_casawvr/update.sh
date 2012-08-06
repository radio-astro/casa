#!/bin/bash
echo "============================================================================="
echo "Script to update the air_casawvr module code from a new"
echo "version of libair (typically got from http://www.mrao.cam.ac.uk/~bn204/soft )"
echo "Initial version: D. Petry, ESO, March 2012"
echo "============================================================================="
export LIBAIRROOT=$1
if [ $LIBAIRROOT"" == "" ]; then
    echo " Usage:" 
    echo "   cd air_casawvr"
    echo "   ./update.sh <path to the new libair version."
    echo " Example:"
    echo "   cd code/air_casawvr"
    echo "   ./update.sh /diska/home/dpetry/temp/wvrgcal/libair-1.1.0"
    exit 0
fi
if ! [ -e $LIBAIRROOT ]; then
    echo "ERROR: The path you provided is incorrect: "$LIBAIRROOT
    exit 1
fi
# for safety, check we are in the right directory
cd ../air_casawvr/
if [ $? == 0 ]; then
    echo Using $LIBAIRROOT
    
    export BACKUPEXTENSION=""
    uname -a | grep "Darwin"
    if [ $? == 0 ]; then
	# we are on a Mac, need to use sed with a backup extension
	export BACKUPEXTENSION=".bak"
    fi

    for j in $( ls -p | grep "/" ); do
	cd $j
	echo Now in directory $j
	for i in $( ls *.cpp *.hpp ); do
	    echo item: $i
	    export CORRESPONDING=`find $LIBAIRROOT -name $i`
	    if [ $CORRESPONDING == "" ]; then
		echo ==============================================
		echo WARNING: File $i not found in $LIBAIRROOT
		echo ==============================================
	    else
		diff $i $CORRESPONDING
		if [ $? == 0 ]; then
		    echo No difference between $i and $CORRESPONDING
		else
		    echo Testing with fixed include paths ...
		    cp $CORRESPONDING ${i}.new
		    
		    sed -i $BACKUPEXTENSION "s|calibration/CalTables/|synthesis/CalTables/|g" $i.new
		    sed -i $BACKUPEXTENSION "s|casawvr/|../casawvr/|g" $i.new
		    sed -i $BACKUPEXTENSION "s|../src/apps/|almawvr/|g" $i.new
		    sed -i $BACKUPEXTENSION "s|src/apps/|almawvr/|g" $i.new
		    sed -i $BACKUPEXTENSION "s|../src/|almawvr/|g" $i.new
		    sed -i $BACKUPEXTENSION "s|src/|almawvr/|g" $i.new

		    diff $i $i.new
		    if [ $? == 0 ]; then
			echo No difference between $i and $CORRESPONDING with fixed paths.
			rm $i.new
		    else
			mv $i.new ${i}
			echo ==============================================
			echo Replaced $i by a copy of $CORRESPONDING and fixed include paths.
			echo ==============================================
		    fi
		    rm -f *.new$BACKUPEXTENSION 
		fi
	    fi
	done
	cd ..
    done
    echo "Don't forget to check if the file data/alma/wvrgcal/libair-ddefault.csv needs updating as well!"
else
    echo You should be in directory air_casawvr to run this script.
fi

            

        
