#!/bin/sh
`which casapy` --nologger --log2term  -c `echo $CASAPATH | awk '{print $1}'`/gcwrap/python/scripts/regressions/admin/runUnitTest.py test_imhead

