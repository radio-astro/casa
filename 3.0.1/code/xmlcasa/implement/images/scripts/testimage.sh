#!/bin/bash
killall casalogger
rm casapy.log
mkdir image_test
cd image_test
rm casapy.log
pwd
cat ../triplequote ../../image.xml ../triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ../image.py
casapy -c ../image.py
