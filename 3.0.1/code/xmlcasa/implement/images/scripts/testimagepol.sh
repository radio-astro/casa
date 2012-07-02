#!/bin/bash
killall casalogger
rm casapy.log
cat ./triplequote ../imagepol.xml ./triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ./imagepol.py
casapy -c imagepol.py
