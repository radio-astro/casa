#!/bin/bash

# Get the hint from an environment variable. This is used for detached head builds
# Default grep is master
headGrep="\\\-mas-"
#echo $CASABRANCHHINT
if [ ! -z $CASABRANCHHINT ]; then
    if [[ $CASABRANCHHINT =~ ^feature.*CAS.* ]] ; then
        b1=${CASABRANCHHINT%/*} # part before the slash
        b2=${CASABRANCHHINT##*/} # part after the slash
        headGrep=$b1-$b2
    elif [[ $CASABRANCHHINT =~ .*release.* ]] ; then
        headGrep=$CASABRANCHHINT
    fi
fi
#echo $headGrep

# Check where the current "HEAD" points to.
branch=`git rev-parse --abbrev-ref HEAD`

# Detached HEAD, should have a tag
if [ $branch == "HEAD" ];then
    headTag=`git tag --points-at HEAD`
    if [[ -z "${headTag// }" ]]; then
        # Get the nearest tag and add Desc
        headCommit=`git rev-parse HEAD`
        headTag=`git tag --points-at HEAD | grep $headGrep | xargs`
    fi
    CASA_VERSION_DESC=$headTag
    # $CASAFORKPOINTHINT is the fork point commit
    # You can obtain this by executing  "git merge-base --fork-point master"
    # while in the branch, but before detaching the HEAD
    if [ -z $CASAFORKPOINTHINT ]; then
        CASAFORKPOINTHINT=`git merge-base origin/master $branch`
    fi
    headTag=`git describe --abbrev=0 --match='[0-9]*.[0-9]*.[0-9]*-mas-[0-9]*' $(git rev-parse $CASAFORKPOINTHINT)`
    #echo "${headTag##*-};$CASA_VERSION_DESC"
    echo "${headTag##*-}; "
# Master
elif [ $branch == "master" ];then
    #echo "Resolving master"
    masterTag=`git tag --points-at HEAD | grep \\\-mas- | xargs`
    if [[ -z "${masterTag// }" ]]; then
        # Get the nearest tag and add Desc
        headCommit=`git rev-parse HEAD`
        masterTag=`git describe --abbrev=0 | grep \\\-mas- | xargs`
        CASA_VERSION_DESC="No tag. Nearest tag: $masterTag HEAD commit: $headCommit "
    fi
    # Return only the revision number
    echo "${masterTag##*-};$CASA_VERSION_DESC"
# Any other branch
else
    #echo "Resolving branch"
    # Using parameter expansion to split the strings
    b1=${branch%/*} # part before the slash
    b2=${branch##*/} # rpart after the slash
    tagMatcher=$b1-$b2
    branchTag=`git tag --points-at HEAD | grep \\\-$tagMatcher- | xargs`
    if [[ -z "${branchTag// }" ]]; then
        # Get the nearest tag and add Desca
        headCommit=`git rev-parse HEAD`
        branchTag=`git describe --abbrev=0 | grep \\\-$tagMatcher- | xargs`
        CASA_VERSION_DESC="No tag. Nearest tag: $branchTag HEAD commit: $headCommit "
    else
        CASA_VERSION_DESC="Branch tag: $branchTag"
    fi
    # Do our best to resolve the master tag for revision even when we have
    # Branch tag
    if [ -z $CASAFORKPOINTHINT ]; then
        CASAFORKPOINTHINT=`git merge-base origin/master origin/$branch`
    fi
    masterTag=`git describe --abbrev=0 --match='[0-9]*.[0-9]*.[0-9]*-mas-[0-9]*' $(git rev-parse $CASAFORKPOINTHINT)`
    #masterTag=`git describe --tags $(git rev-parse $CASAFORKPOINTHINT)`
    echo "${masterTag##*-};$CASA_VERSION_DESC"
    # Remove the feature/release information
    # and return a simple version number
    #mTag=${branchTag%-feature-*}
    #mTag=${branchTag%-release-*}
    #echo $mTag
    # Return the master tag version
    #echo "${mTag##*-};$CASA_VERSION_DESC"
fi
