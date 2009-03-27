#!/usr/bin/perl -Wl

# Run a program, then reap its children. The given program either
# - exists normally, or
# - is killed after the specified timeout, or
# - is killed when this process becomes an orphan, or
# - is killed when this process is terminated
#
# Returns: the exit status from the program to run.
#
# Note: The given program and any leftover subprocesses are reaped by sending
# SIGHUP, SIGTERM and SIGKILL (in that order) to all processes in that program's
# process group id. Therefore, any created subprocess should not change its process
# group id from that of its parent. Secondly, this process must be terminated cleanly
# (i.e., not SIGKILLed or SIGSTOPped) to ensure proper cleanup.

$#ARGV >= 1 or die "Usage: $0 TIMEOUT_IN_SECONDS command [args ...]";

$ppid = getppid();

$child = fork;
defined $child or die "Could not fork()";

if (!$child) {
    # child
    setpgrp();       # process group for child processes
    shift @ARGV;
    $cmd = join(' ', @ARGV);
    exec $cmd;
    exit(1);
}
else {
    # parent
    $SIG{TERM} = $SIG{HUP} = $SIG{INT} = $SIG{QUIT} = 
    $SIG{TRAP} = $SIG{PIPE} = $SIG{ABRT} =
    sub {
	terminate(1);
    };
    $SIG{ALRM} = \&alrm;
    
    $interval = 5;         # wall clock time (seconds)
    $timeout  = $ARGV[0];
    $elapsed  = 0;
    
    alarm($interval);

    #shift @ARGV;
    #$cmd = join(' ', @ARGV);
    #print "$child: $cmd";
    
    #print "parent=$$, child=$child, timeout=$timeout";
    $pid = wait();
    $retval = $?;
    if ($pid != -1) {
	    #print "[$pid] terminated with exit code $?";
    }
    terminate($retval);
}

sub alrm
{
    $elapsed += $interval;
    #print "handle SIGALRM, elapsed = $elapsed, ppid = ", getppid();
    if (getppid() <= 1) {
	print STDERR "Parent $ppid disappeared, going down...";
	terminate(1);
    }
    elsif ($elapsed >= $timeout) {
	print STDERR "[$child] $timeout seconds TIMEOUT";
	terminate(1);
    }
    else {
	alarm($interval);
    }
}

sub terminate
{
    $SIG{TERM} = $SIG{HUP} = $SIG{INT} = $SIG{QUIT} = 
    $SIG{TRAP} = $SIG{PIPE} = $SIG{ABRT} = $SIG{ALRM} = 'IGNORE';

    $return_value = $_[0];

    $n = kill 0, -$child;
    if ($n > 0) {
	print "$0: kill $n leftover child" . (($n != 1) ? "ren": "") . " in PGID=$child";
	kill SIGHUP , -$child;
	sleep(2);
	if (kill SIGTERM, -$child) {
	    sleep(2);
	    kill SIGKILL, -$child;
	}
    }
    
    exit $return_value;
}
