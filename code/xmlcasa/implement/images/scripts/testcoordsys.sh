#!/bin/bash
killall casalogger
rm casapy.log
cat ./triplequote ../coordsys.xml ./triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ./coordsys.py
casapy -c coordsys.py
