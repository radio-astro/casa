#!/bin/bash
killall casalogger
rm casa.log
cat ./triplequote ../measures.xml ./triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ./measures.py
casapy -c measures.py
