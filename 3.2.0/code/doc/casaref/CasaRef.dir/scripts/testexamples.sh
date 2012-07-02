#!/bin/bash
killall casalogger
rm casapy.log
cat ./triplequote ../quantaModule.tex ./triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ./quantaModule.py
cat ./triplequote ../measuresModule.tex ./triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ./measuresModule.py
cat ./triplequote ../imagesModule.tex ./triplequote | sed 's/\&gt;/>/g' | sed 's/\&lt;/</g' | sed 's/\&amp;/\&/g' > ./imagesModule.py
casapy -c quantaModule.py
casapy -c measuresModule.py
casapy -c imagesModule.py
