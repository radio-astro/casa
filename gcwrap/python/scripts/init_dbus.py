## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##     first try to find executables using casapyinfo...
##            (since system area versions may be incompatible)...
##     next try likely system areas...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##
##   note:  hosts which have dbus-daemon-1 but not dbus-daemon seem to have a broken dbus-daemon-1...
##
for info in [ (['dbus-daemon'],'dbus'),
              (['CrashReportPoster'],'crashPoster'),
              (['ipcontroller','ipcontroller-2.6'], 'ipcontroller'),
              (['ipengine','ipengine-2.6'], 'ipengine') ]:
    exelist = info[0]
    entry = info[1]
    for exe in exelist:
        if casa['helpers']['info']:
            casa['helpers'][entry] = (lambda fd: fd.readline().strip('\n'))(os.popen(casa['helpers']['info'] + " --exec 'which " + exe + "'"))
        if casa['helpers'][entry] and os.path.exists(casa['helpers'][entry]):
            break
        else:
            casa['helpers'][entry] = None

        ### first look in known locations relative to top (of binary distros) or known casa developer areas
        for srchdir in [ __casapath__ + '/MacOS', __casapath__ + '/lib/casa/bin', '/usr/lib64/casa/01/bin', '/opt/casa/01/bin' ] :
            dd = srchdir + os.sep + exe
            if os.path.exists(dd) and os.access(dd,os.X_OK) :
                casa['helpers'][entry] = dd
                break
        if casa['helpers'][entry] is not None:
            break

    ## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    ##     next search through $PATH for executables
    ## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
    if casa['helpers'][entry] is None:
        for exe in exelist:
            for srchdir in os.getenv('PATH').split(':') :
                dd = srchdir + os.sep + exe
                if os.path.exists(dd) and os.access(dd,os.X_OK) :
                    casa['helpers'][entry] = dd
                    break
            if casa['helpers'][entry] is not None:
                break
