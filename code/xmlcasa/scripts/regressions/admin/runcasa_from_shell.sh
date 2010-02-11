#!/bin/bash

# Run a casapy script from the command line / as cronjob
#
# The CASA environment is setup and a dummy X server is
# started if necessary

if [ $# -lt 2 ]; then
    echo "Usage: $0 virtual_display_no script.py [args ...]"
    exit 1
fi

display=$1
command=${@:2}

echo `date -u`

if [ "x$SHELL" != "x" ] ; then
    echo "SHELL = $SHELL"
    if [ "$SHELL" == "/bin/bash" ] ; then
	setups=($HOME/.bash_profile $HOME/.bashrc)
	for script in ${setups[@]} ; do
	    if [ -f $script ] ; then
		echo "source $script"
		source $script
	    else
		echo $script does not exist
	    fi
	done
	echo "PATH = $PATH"
    else
	echo "Unsupported shell $SHELL"
	# To support more shells, add the above
    fi 
else
    echo "SHELL undefined"
    exit 1
fi

if which casapy > /dev/null ; then
    echo "Using `which casapy`"
else
    echo "No casapy in PATH!"
    exit 1
fi

started_xvfb=0

# Find or create an X display
if [ "x$DISPLAY" == "x" ] ; then
    echo "DISPLAY undefined, launch Xvfb :$display"

    if which Xvfb &> /dev/null ; then
	echo "Using `which Xvfb`"
    else
	# Try our luck
	export PATH=$PATH:/usr/X11R6/bin/
	if which Xvfb > /dev/null ; then
	    echo "Using `which Xvfb`"
	else
	    "Could not find Xvfb, giving up"
	    exit 1
	fi
    fi

    Xvfb :$display &> /dev/null &
    # Suppressing error messages is bad style(tm),
    # but this X server is really noisy!
    pid_xvfb=$!

    export DISPLAY=":$display.0"
    echo "DISPLAY = $DISPLAY"

    # Query.
    # On most implementations waiting for the server
    # to start is not necessary
    xset q > /dev/null
    if test $? -ne 0; then
        echo "X server not responding, wait 5 seconds and try again"
        sleep 5
        xset q > /dev/null
        if test $? -ne 0; then
            echo 1>&2 "Could not connect to $DISPLAY"
            exit 1
        fi
    fi
    echo "X server at $DISPLAY up and running"
    started_xvfb=1
else
    echo "DISPLAY = $DISPLAY"
fi

cmd="casapy --nologger -c $command"
echo $cmd
$cmd &
pid=$!
wait $pid
echo "casapy returned $?"

echo "Kill anything in process group $pid if it exists..."
kill -s TERM -- -$pid
sleep 2
kill -s KILL -- -$pid

if [ $started_xvfb != 0 ]; then
    echo "Kill Xvfb :$display..."
    kill -s TERM $pid_xvfb
fi

exit 0
