
To install the CASA application drag the icon to your desired installation
location (probably your applications folder). Then double click on the CASA
icon. You will be prompted to create several casapy symbolic links, it is
advisable to do so as this will allow you to run casapy from a terminal
window by typing casapy. Additionally, the viewer (casaviewer), table browser
(casabrowser), plotms (casaplotms), and buildmytasks will also be available
via the command line. Creating the symbolic links will require that you have
administrator priviledges.

If you had previously installed CASA rev 8909 or later you will need to
unload the dbus before the copy will work

launchctl remove org.freedesktop.dbus-session
launchctl remove org.freedesktop.dbus-system

Wes Young
wyoung at aoc dot nrao dot edu
17-December-2009

