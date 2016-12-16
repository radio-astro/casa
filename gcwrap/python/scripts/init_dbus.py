## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##     first try to find executables using casapyinfo...
##            (since system area versions may be incompatible)...
##     next try likely system areas...
## ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
##
##   note:  hosts which have dbus-daemon-1 but not dbus-daemon seem to have a broken dbus-daemon-1...
##
import signal

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

if casa['helpers']['dbus'] is not None :
    dbus_conf= __casapath__ + "/etc/dbus/casa.conf"
    if not os.path.exists(dbus_conf):
        dbus_conf = __casapath__ + "/Resources/dbus/casa.conf"
    argv_0_path = os.path.dirname(os.path.abspath(sys.argv[0]))
    dbus_path = os.path.dirname(os.path.abspath(casa['helpers']['dbus']))

    (r,w) = os.pipe( )

    if os.fork( ) == 0 :
        os.close(r)
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        signal.signal(signal.SIGHUP, signal.SIG_IGN)
        ## close standard input to avoid terminal interrupts
        sys.stdin.close( )
        os.close(0)
        args = [ 'casa-dbus-daemon' ]
        args = args + ['--print-address', str(w)]
        if dbus_conf is not None and os.path.exists(dbus_conf) :
            args = args + ['--config-file',dbus_conf]
        else:
            args = args + ['--session']
        os.execvp(casa['helpers']['dbus'],args)
        sys.exit

    os.close(w)
    dbus_address = os.read(r,200)
    dbus_address = dbus_address.strip( )
    os.close(r)
    if len(dbus_address) > 0 :
        os.putenv('DBUS_SESSION_BUS_ADDRESS',dbus_address)
        os.environ['DBUS_SESSION_BUS_ADDRESS'] = dbus_address
