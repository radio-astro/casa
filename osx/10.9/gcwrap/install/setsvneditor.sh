#!/bin/sh
#
# This used to be a little script to set the SVN editor and load in some
# required infomation for committing changes, with a vi bias.
#
# It now just sets SVN_EDITOR to be a script itself, which is more convenient
# and supports most editors.

#Routine for setting the SVN_EDITOR to get commit template
AIPSROOT=`echo $CASAPATH | awk '{print $1}'`

export SVN_EDITOR=$AIPSROOT/gcwrap/install/svn_editor
echo "SVN_EDITOR is set to $SVN_EDITOR"

if [ "$1" != "" ]; then
   echo 'It loads the template in $VISUAL, $EDITOR, or vi, respectively,'
   echo "ignoring your request here for $1."
fi
