#! /bin/bash

DIR=.
if [ $# -ge 1 ]; then
    DIR=$1
fi

cd ${DIR}
${DIR}/getsvnrev.sh
