#!/bin/bash
killall casalogger
rm casapy.log
cat ./triplequote ../quanta.xml ./triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ./quanta.py
casapy -c quanta.py
