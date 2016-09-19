#!/bin/bash
branch=`git rev-parse --abbrev-ref HEAD`
if [ $branch == "master" ];then
    #echo "Resolving master"
    masterTag=`git tag --points-at HEAD | grep \\\-mas- | xargs`
    if [ $masterTag == "" ]; then
        # Get the nearest tag and add Desc
        CASA_VERSION_DESC="No tag found for the current commit. Using nearest tag."
        masterTag=`git tag --points-at HEAD | grep \\\-mas- | xargs`
    fi
    # Return only the revision number
    echo "${masterTag##*-};$CASA_VERSION_DESC"
else
    #echo "Resolving branch"
    b1=${branch%/*}
    b2=${branch##*/}
    tagMatcher=$b1-$b2
    branchTag=`git tag --points-at HEAD | grep \\\-$tagMatcher- | xargs`
    if [ $branchTag == "" ]; then
        # Get the nearest tag and add Desc
        branchTag=`git tag --points-at HEAD | grep \\\-$tagMatcher- | xargs`
        CASA_VERSION_DESC="No tag found for the current commit. Using nearest tag. $branchTag"
    else
        CASA_VERSION_DESC="$branchTag"
    fi
    # Remove the feature information
    mTag=${branchTag%-feature-*}
    #echo $mTag
    # Return the master tag version
    echo "${mTag##*-};$CASA_VERSION_DESC"
fi
