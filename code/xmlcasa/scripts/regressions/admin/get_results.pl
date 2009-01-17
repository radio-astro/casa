#!/usr/bin/perl -W

# This script collects regression test results from remote hosts
# and generates email notifications about failures.
#
# (Note: runs only on one machine but that would be simple
#  to change when that need arises)
#



#
# First some global definitions
#

# Remove hosts, and where to get tests results from
%dir = (
	"el4tst" => "/casa/state/",
	"ballista" => "http://www.aoc.nrao.edu/~jmlarsen/casa/",
	"onager" => "http://www.aoc.nrao.edu/~jmlarsen/casa_onager/",
	"casa-dev-15" => "http://www.aoc.nrao.edu/~jmlarsen/casa_casa-dev-15/",
	"nb014496.ads.eso.org" => "/home/jmlarsen/ALMA/Regression",
	"ma014655.hq.eso.org" => "/Users/jmlarsen/casa_test/result/"
);

# Orphan test failures go to this email
$maintainer_email = "jmlarsen\@eso.org";

# This file keeps track of which test failures were already reported
$notifications_sent = "/home/jmlarsen/public_html/ALMA/notifications_sent.txt";

# Paths to SVN:
$svn_path = "/diska/jmlarsen/gnuactive/code";
$tests_file = "xmlcasa/scripts/regressions/admin/tests_list.txt";


# Start processing...
system("date -u");

foreach $host (keys %dir) {
    mkdir("/scratch/tmp/Regression/$host/");
    chdir("/scratch/tmp/Regression/$host/")or die;
    
    print "Retrieving $host, ${dir{$host}}... ";

    # Lots of special cases...
    if ($dir{$host} =~ /^http/) {
	$cmd = "wget --quiet --output-document=result-all.tgz \"${dir{$host}}/result-all.tgz\"";
    }
    else {
	if ($dir{$host} eq "/casa/regression/01/result/" or
	    $dir{$host} eq "/casa/state/")
	{
	    $ssh_cmd = "ssh 2>/dev/null -T -A -l casagate ssh.cv.nrao.edu --";
	    # login messages are written to stderr, suppress that
	}
	else {
	    $ssh_cmd = "ssh -T";
	}

	if ($dir{$host} eq "/casa/state/") {
	    $files_to_get = "--after-date '5 hours ago' Result/ Log/";
	}
	else {
	    $files_to_get = "result-all.tgz";
	}
	$cmd =
	    'echo -e "cd ' . ${dir{$host}} . ' \n tar zc ' . $files_to_get . ' " | ' .
	    $ssh_cmd . ' ' . $host . ' | tar zxf -';
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
	    if (mysystem("echo touch ${dir{$host}}/retrieved | $ssh_cmd $host")) {
		warn $!;
		next;
	    }
	}

	if ($dir{$host} ne "/casa/state/") {
	    if (mysystem("tar zxf result-all.tgz")) {
		warn "$host: $!";
		next;
	    }

	    # Wanted to use in the following system call
	    # tar's --keep-newer-files option
	    # to overwrite previous Log/* files with later. But the
	    # option is buggy in the available tar-1.14
	    #
	    # Instead, sort the tar.gz files by date and extract
	    # in that order.
	    if (mysystem("/bin/ls -1tr result-*tar.gz | xargs -i tar zxf {}")) {
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
	    mysystem("cp -p ./Log/* ../Log/") == 0 or warn $!;
	    #mysystem("rm -rf ./Log/") == 0 or die $!;
	}
	if (-d "./Result") {
	    mysystem("find ./Result/ -type d | grep -Ev '^./Result/\$' | xargs -i cp -pR {} ../Result") == 0 or warn $!;
	    # Used to be "cp -pR ./Result/* ../Result" but too many arguments

	    mysystem("rm -rf ./Result/") == 0 or warn $!;
	}
	print "OK\n";
    }
}

mysystem('grep       "Unexpected error" /scratch/tmp/Regression/Log/* | tee /home/jmlarsen/public_html/ALMA/error.log 1>&2');
mysystem('grep -B200 "Unexpected error" /scratch/tmp/Regression/Log/* >> /home/jmlarsen/public_html/ALMA/error.log');

exit 0;




sub email_notify 
{
    $svn_base = `svn info $svn_path | grep URL | awk '{print \$2}'`; $? == 0 or die $!;
    chomp($svn_base);
    $svn_rev = `svn -v status $svn_path/$tests_file | awk '{print \$2}'`; $? == 0 or die $!;
    chomp($svn_rev);
    
    @fail_logs = `find ./Result/ -name \\*.txt | xargs grep -lE \"^status .\*fail\"`;

    #print "fail logs are @fail_logs\n";

    foreach $fl (@fail_logs) {
	$_ = $fl;
	chomp();
	$testid = `cat $_ | grep -E "^testid" | awk '{print \$3}'`;  $? == 0 or die $!;
	$type   = `cat $_ | grep -E "^type" | awk '{print \$3}'`;    $? == 0 or die $!;
	$casa   = `cat $_ | grep -E "^CASA" | grep -oE "'.*'"`;      $? == 0 or die $!;
	$host   = `cat $_ | grep -E "^host" | awk '{print \$3}'`;    $? == 0 or die $!;
	chomp($testid);
	chomp($type);
	chomp($casa); $casa =~ s/\'//g;
	chomp($host);
	$email  = `cat $svn_path/$tests_file | grep -wE "\\s*$testid" | awk '{print \$2}'`;  $? == 0 or die $!;
	chomp($email);

	if ($type ne "exec") {
	    $image = `cat $_ | grep -E "^image" | awk '{print \$3}'`; $? == 0 or die $!;
	    chomp($image);
	}
	else {
	    $image = "";
	}

	# If no email address is given then...
	if ($email !~ /\S/) {
	    $email = $maintainer_email;
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
        print FILE "http://www.eso.org/~jmlarsen/ALMA/CASA/test-report.html";
	if ($type eq "exec") {
	    print FILE " and\nhttp://www.eso.org/~jmlarsen/ALMA/CASA/summary_$testid.html\n";
	}
	else {
	    print FILE "\n";
	}
	print FILE "to confirm and take remedial action. Be aware that it may take a few hours\n";
	print FILE "until the HTML is updated with these results.\n";
	print FILE "\n";
	print FILE "-- \n";
	print FILE "This is an automated message, you can reply.\n";
	print FILE "You have received this message because your email is listed on\n";
	print FILE "$svn_base/$tests_file r$svn_rev\n";
	print FILE "as the contact address for $testid.\n";
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
	    
	    (-e $notifications_sent) or die;   
	    # This is a spam-reducing check (in case something is wrong with the given path)
	    # If the script died here and you think $notifications_sent
	    # points to the correct place, then create an empty file at that location
	    if (mysystem("grep \"$testid $image $type $casa\" $notifications_sent")) {
		open FILE, ">>$notifications_sent" or die;
		$ddd = `date -u`; chomp($ddd);
		print FILE "$ddd: $testid $image $type $casa - $fl\n";
		close FILE or die;
		mysystem("cat $message | mail -s \"$testid failure, $host, $casa\" -b $maintainer_email $email");
		#mysystem("cat $message | mail -s \"$testid failure, $host, $casa\" -b $maintainer_email jmlarsen\@eso.org");
		print "Sent notification to $email\n";
	    }
	    else {
		print "Don't repeat this notification\n";
	    }
	}
    }
}

sub mysystem
{
    my $cmd = $_[0];
    print "+ $cmd\n";
    return system($cmd);
}


