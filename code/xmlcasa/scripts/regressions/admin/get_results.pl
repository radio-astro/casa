#!/usr/bin/perl -Ws

# This script collects regression test results from remote hosts
# and generates email notifications about failures.


#
# First some global definitions
#

# Remove hosts, and where to get tests results from
%dir = ( "el5tst" => "/casa/state/",
	 "casa-dev-15.aoc.nrao.edu" => "/export/home/casa-dev-15-2/casa-regressions/sresult",
);

if (! $prefix ) {
    $prefix = "/export/data/casa-regressions";
}
if (! $mail_dir ) {
    $mail_dir = "$prefix/mail";
    die "no mail directory" unless -d $mail_dir;
}
if (! $notifications_sent ) {
    $notifications_sent = "$prefix/state/notification-sent.txt";
    die "no state directory" unless -d "$prefix/state";
}
if (! $tests_file ) {
    $tests_file = "$prefix/admin/tests_list.txt";
    die "no notification list" unless -e $tests_file;
}
if (! $data_base ) {
    $data_base = "$prefix/archive";
    die "no archive directory" unless -d $data_base;
}
if (! $log_dir ) {
    $log_dir = "$prefix/log";
    die "no log directory" unless -d $log_dir;
}

# Orphan test failures go to this email
$default_email = "drs\@nrao.edu";

$mail_no = 0;  # global counter

$tests_file or die;
#= "xmlcasa/scripts/regressions/admin/tests_list.txt";

$data_base = "" if (0); $data_base or die;

$log_dir = "" if (0); $log_dir or die;

# Start processing...
print "---------------------------------------------------------------------------\n";
system("date -u");

$files_to_get = "result-all.tgz";

foreach $host (keys %dir) {
    mkdir("$data_base/$host/");
    chdir("$data_base/$host/")or die;
    
    print "Retrieving $host, ${dir{$host}}...\n";

    # Lots of special cases...
    if ($dir{$host} =~ /^http/) {
	$cmd = "curl \"${dir{$host}}/result-all.tgz\" >result-all.tgz 2>/dev/null";
    }
    else {
	if ($dir{$host} eq "/casa/regression/01/result/" or
	    $dir{$host} eq "/casa/state/")
	{
	    ####
	    #### command used from ESO to fetch results using casagate ssh forwarder
	    ####
	    #$ssh_cmd = "ssh 2>/dev/null -T -A -l casagate ssh.cv.nrao.edu --";
	    ## login messages are written to stderr, suppress that
	    $ssh_cmd = "ssh -l casatest -T";
	}
	else {
	    $ssh_cmd = "ssh -T";
	}

	if ($dir{$host} eq "/casa/state/" or
	    $dir{$host} =~ m|/casa-regressions/sresult$|) {
	    #$files_to_get = "--after-date '5 hours ago' Result/ Log/";
	    $files_to_get = "result-*tar.gz";
	}
	else {
	    $files_to_get = "result-all.tgz";
	}
	$cmd =
	    'echo "cd ' . ${dir{$host}} . ' ; tar zc ' . $files_to_get . ' " | ' .
	    $ssh_cmd . ' ' . $host . ' 2> /dev/null | tar zxf -';
    }
    if (mysystem($cmd) != 0) {
	warn $cmd . ":" . $!;
    }
    else {
	# Notify remote host when the results
	# were latest collected
	if ($dir{$host} =~ /^http/) {
	    # ???
	}
	else {
	    if (mysystem("echo touch ${dir{$host}}/retrieved | $ssh_cmd $host 2> /dev/null")) {
		warn $!;
		next;
	    }
	}

	if (1 or $dir{$host} ne "/casa/state/")  {
	    if ($dir{$host} ne "/casa/state/" and $dir{$host} !~ m|/casa-regressions/sresult$| and mysystem("tar zxf result-all.tgz")) {
		warn "$host: $!";
		next;
	    }

            $unextracted_size = `gunzip -l result-*tar.gz | awk '{SUM += \$1} END {print SUM}'`;
            chomp($unextracted_size);

            if ($unextracted_size > 100*1024*1024) {
                warn "$host: data is way too big!";
                system("/bin/ls -1 *tar.gz 1>&2");
                next;
            }

	    # Wanted to use in the following system call
	    # tar's --keep-newer-files option
	    # to overwrite previous Log/* files with later. But the
	    # option is buggy in the available tar-1.14
	    #
	    # Instead, sort the tar.gz files by date and extract
	    # in that order.

	    if (mysystem("/bin/ls -1tr result-*tar.gz | xargs -n 1 tar zxf")) {
		warn "$host: $!";
		# tolerate if one tgz file is invalid   next;
	    }
	    if (mysystem("rm -f *tar.gz")) {
		warn "$host: $!";
		next;
	    }
	    if (mysystem("rm -f *tgz")) {
		warn "$host: $!";
		next;
	    }
	}

	if (-d "./Result") {
	    email_notify();
	}

	# Preserve time when copying
	if (-d "./Log") {
	    mysystem("cp -pr ./Log/ ../Log/") == 0 or warn $!;
	    #mysystem("rm -rf ./Log/") == 0 or die $!;
	}
	if (-d "./Result") {
	    mysystem("find ./Result/ -type d | grep -Ev '^./Result/\$' | xargs -n 1 -I {} cp -pR {} ../Result") == 0 or warn $!;
	    # Used to be "cp -pR ./Result/* ../Result" but too many arguments

	    mysystem("rm -rf ./Result/") == 0 or warn $!;
	}
	print "OK\n";
    }
}

mysystem("find $data_base/Log/ -type f | xargs grep       \"Unexpected error\" | tee $log_dir/error.log 1>&2");
mysystem("find $data_base/Log/ -type f | xargs grep -B200 \"Unexpected error\" >> $log_dir/error.log");

exit 0;




sub email_notify 
{
    $svn_rev = `svn -v status $tests_file | awk '{print \$2}'`; $? == 0 or die $!;
    chomp($svn_rev);
    
    @fail_logs = `find ./Result/ -name \\*.txt | xargs grep -lE \"^status .\*fail\"`;

    #print "fail logs are @fail_logs\n";

    foreach $fl (@fail_logs) {
	chomp($fl);
	$_ = $fl;
	$testid = `cat $_ | grep -E "^testid" | awk '{print \$3}'`;  $? == 0 or die $!;
	$type   = `cat $_ | grep -E "^type" | awk '{print \$3}'`;    $? == 0 or die $!;
	$casa   = `cat $_ | grep -E "^CASA" | grep -oE "'.*'"`;      $? == 0 or die $!;
	$host   = `cat $_ | grep -E "^host" | awk '{print \$3}'`;    $? == 0 or die $!;
	chomp($testid);
	chomp($type);
	chomp($casa); $casa =~ s/\'//g;
	chomp($host);
	$email  = `cat $tests_file | grep -w "^$testid" | awk '{print \$2 \"\@\" \$3}'`;  $? == 0 or die $!;
	chomp($email);

	if ($type ne "exec") {
	    $image = `cat $_ | grep -Ew "^image" | awk '{print \$3}'`; $? == 0 or die $!;
	    chomp($image);
	}
	else {
	    $image = "";
	}

	# If no email address is given then...
	if ($email !~ /\S/) {
	    $email = $default_email;
	}

	#print "Address = '$email'\n";

	$message = "/tmp/message-$$.txt";
	open FILE, ">$message" or die;
	
	print FILE "Dear $email,\n";
	print FILE "\n";
	print FILE "The regression test $testid";
	if ($type ne "exec") {
	    print FILE " - $image - $type";
	}
	print FILE " failed on host $host using\n$casa.\n\n";
	print FILE "Please check on\n";
        print FILE "http://www.eso.org/~jmlarsen/ALMA/CASA_latest/test-report.html#$testid";
	if ($type eq "exec") {
	    print FILE " and\nhttp://www.eso.org/~jmlarsen/ALMA/CASA/summary_$testid.html\n";
	}
	else {
	    print FILE "\n";
	}
	print FILE "to confirm and take remedial action.\n";
	print FILE "\n";
	print FILE "-- \n";
	print FILE "This is an automated message, you can reply.\n";
	print FILE "You have received this message as contact for $testid according to\n";
	print FILE "$tests_file r$svn_rev.\n";
	close FILE or die;


	# Filter out here pol* tests where component not found in reference image
	# (maybe these should not be inserted in the database at all.
	if ($type =~ /^pol/ and mysystem("grep -E \"^ref_\" $fl")) {
	    print "Skipping $fl where reference pol. component was not found\n";
	    next;
	}

	# Define here which hosts to tell about
	# (better would be to check on the SVN branch but
	#  that's not available)
	if ($host =~ /tst/ or $host =~ /ma014655/) {

	    (-e $notifications_sent) or die $notifications_sent;
	    # This is a spam-preventing check (in case something is wrong with the given path)
	    # If the script died here and you think $notifications_sent
	    # points to the correct place, then create an empty file at that location
	    if (mysystem("grep \"$casa $testid\" $notifications_sent")) {
#	    if (mysystem("grep \"$casa $testid $image $type\" $notifications_sent")) {
		open FILE, ">>$notifications_sent" or die;
		$ddd = `date -u`; chomp($ddd);
		print FILE "$ddd: $casa $testid $image $type - $fl\n";
		close FILE or die;

		$subject = $testid;
		if ($type ne "exec") {
		    $subject = "$testid-$image-$type";
		}

		$email_file = $mail_dir . "/" . $mail_no . ".email";
		$mail_no++;
		open FILE, ">$email_file";

		#print FILE "$email\n";
		print FILE "drs\@nrao.edu\n";   # for testing without spamming

		print FILE "CASA regression failure: $subject, $casa, $host\n";
		close FILE or die;
		mysystem("cat $message >> $email_file");
		print "Dumped notification to $email_file\n";
	    }
	    else {
		print "Don't repeat this notification\n";
	    }
	}
	
	system("/bin/rm $message");
    }
}

sub mysystem
{
    my $cmd = $_[0];
    print "+ $cmd\n";
    return system($cmd);
}


