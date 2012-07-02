C     Variables used as logical units for files
C     NAME         TYPE        Description
C     LUCGDB       INTEGER     Logical unit used for reading the
C                              corrected geomagnetic data base
C     LUCNOP       INTEGER     Logical unit used for reading the NO+
C                              coefficients
C     LUCO2P       INTEGER     Logical unit used for reading the O2+
C                              coefficients
C     LUCOP        INTEGER     Logical unit used for reading the O+
C                              coefficients
C     LUFNOP       INTEGER     Logical unit used for reading the NO+
C                              Empirical orthogonal functions
C     LUFO2P       INTEGER     Logical unit used for reading the O2+
C                              Empirical orthogonal functions
C     LUFOP        INTEGER     Logical unit used for reading the O+
C                              Empirical orthogonal functions
C     LUIDL        INTEGER     Logical unit of the IDL graphics out-
C                              put file.
C     LUNATM       INTEGER     Logical unit of the neutral atmosphere
C                              parameters.
C     LUNUMB       INTEGER     Logical unit of the text output file
C                              file
C     LURATE       INTEGER     Logical unit of the
C     LURT         INTEGER     Logical unit of the file with the
C                              "real-time" data
C     LURT1        INTEGER     Logical unit of the file with the
C                              output station list
C     LUSGP        INTEGER     Logical unit of the station geophysical
C                              parameters
C
         INTEGER LUNATM
         INTEGER LURT,LURT1,LURT2,LUNUMB,LUIDL,LURATE,LUTEXT
         INTEGER LUSGP,LUFOP,LUFNOP,LUFO2P,LUCOP
         INTEGER LUCNOP,LUCO2P,LUCGDB,LUSTAT
C
         PARAMETER (LUNATM=13,
     2   LURT=20,LURT1=21,LURT2=22,LUNUMB=22,LUIDL=23,LURATE=24,
     3   LUTEXT=25,LUSGP=26,LUSTAT=29,
     4   LUFOP=31,LUFNOP=32,LUFO2P=33,
     5   LUCOP=41,LUCNOP=42,LUCO2P=43,
     6   LUCGDB=99)
C
