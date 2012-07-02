/^[^*]/{p;d;}

s|("given")|(<EM>given</EM>)|g
s|("returned")|(<EM>returned</EM>)|g
s|(Given)|(<EM>Given</EM>)|g
s|(Given & returned)|(<EM>Given \& returned</EM>)|g
s|(Returned)|(<EM>Returned</EM>)|g
/^\*/s|"|\&quot;|g

# All standard.
s|BITPIX|<TT><B>&</B></TT>|g
s|CDELTia|<TT><B>CDELT</B>ia</TT>|g
s|CNAMEia|<TT><B>CNAME</B>ia</TT>|g
s|\([^_]\)COMMENT|\1<TT><B>COMMENT</B></TT>|g
s|CRDERia|<TT><B>CRDER</B>ia</TT>|g
s|CRPIXja|<TT><B>CRPIX</B>ja</TT>|g
s|CRVALia|<TT><B>CRVAL</B>ia</TT>|g
s|CSYERia|<TT><B>CSYER</B>ia</TT>|g
s|CTYPEia|<TT><B>CTYPE</B>ia</TT>|g
s|CUNITia|<TT><B>CUNIT</B>ia</TT>|g
s|DATE-AVG|<TT><B>&</B></TT>|g
s|DATE-OBS|<TT><B>&</B></TT>|g
s|END|<TT><B>&</B></TT>|g
s|EQUINOXa|<TT><B>EQUINOX</B>a</TT>|g
s|EXTLEV|<TT><B>&</B></TT>|g
s|EXTNAME|<TT><B>&</B></TT>|g
s|EXTVER|<TT><B>&</B></TT>|g
s|HISTORY|<TT><B>&</B></TT>|g
s|LATPOLEa|<TT><B>LATPOLE</B>a</TT>|g
s|LONPOLEa|<TT><B>LONPOLE</B>a</TT>|g
s|MJD-AVG|<TT><B>&</B></TT>|g
s|MJD-OBS|<TT><B>&</B></TT>|g
s|NAXIS|<TT><B>&</B></TT>|g
s|OBSGEO-[XYZ]|<TT><B>&</B></TT>|g
s|RADESYSa|<TT><B>RADESYS</B>a</TT>|g
s|RESTFRQa|<TT><B>RESTFRQ</B>a</TT>|g
s|RESTWAVa|<TT><B>RESTWAV</B>a</TT>|g
s|SIMPLE|<TT><B>&</B></TT>|g
s|SPECSYSa|<TT><B>SPECSYS</B>a</TT>|g
s|SSYSOBSa|<TT><B>SSYSOBS</B>a</TT>|g
s|SSYSSRCa|<TT><B>SSYSSRC</B>a</TT>|g
s|TTYPEn|<TT><B>TTYPE</B>n</TT>|g
s|VELOSYSa|<TT><B>VELOSYS</B>a</TT>|g
s|WCSAXESa|<TT><B>WCSAXES</B>a</TT>|g
s|WCSNAMEa|<TT><B>WCSNAME</B>a</TT>|g
s|VELANGLa|<TT><B>VELANGL</B>a</TT>|g
s|ZSOURCEa|<TT><B>ZSOURCE</B>a</TT>|g
s|PCi_ja|<TT><B>PC</B>i<B>_</B>ja</TT>|g
s|CDi_ja|<TT><B>CD</B>i<B>_</B>ja</TT>|g
s|PSi_ma|<TT><B>PS</B>i<B>_</B>ma</TT>|g
s|PVi_ma|<TT><B>PV</B>i<B>_</B>ma</TT>|g

s|PLEASE NOTE:|<B>&</B>|g

# To prevent " -> &quot; above.
s|<OLa>|<OL TYPE="a">|g
s|<TD>-</TD>|<TD></TD>|g
