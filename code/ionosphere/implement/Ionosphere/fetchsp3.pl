#!/usr/bin/perl
# fetchephem.pl: Perl script for resolving/fetching a number of ephemeris files
#
#   Copyright (C) 1996,1997,1998,1999,2000,2001,2002
#   Associated Universities, Inc. Washington DC, USA.
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the Free
#   Software Foundation; either version 2 of the License, or (at your option)
#   any later version.
#
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#   more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
#   Correspondence concerning AIPS++ should be addressed as follows:
#          Internet email: aips2-request@nrao.edu.
#          Postal address: AIPS++ Project Office
#                          National Radio Astronomy Observatory
#                          520 Edgemont Road
#                          Charlottesville, VA 22903-2475 USA
#
#   $Id$

use Net::FTP;

@ARGV or die 
"Set up the environment:
    SP3_PATH=             search path
    SP3_CACHE=            cache directory
    SP3_FILENAME=         template(s) for possible ephemeris filenames (separated by ':')
    SP3_FTPSITE=          remote FTP site
    SP3_FTPDIR=           template(s) for remote RINEX directory
Then use:
    fetchephem.pl <dates as GPSWEEK:WEEKDAY>\n";

# scan arguments and environment to set everything up. Use reasonable
# defaults
@searchpath = map { glob($_) } split /:/,($ENV{SP3_PATH} || ".");
$cachedir = glob($ENV{SP3_CACHE});
@filelist = split /:/,($ENV{SP3_FILENAME} || 'igs@W@d.sp3:igr@W@d.sp3:igp@W@d.sp3');
$ftpsite = $ENV{SP3_FTPSITE} || 'igscb.jpl.nasa.gov';
@pathlist = split /:/,($ENV{SP3_FTPDIR} || '/igscb/product/@W');

foreach( @ARGV ) {
  /^(\d+):(\d+)$/ and do { 
    push @gpsweeks,sprintf("%04d",$1); 
    push @weekdays,$2; 
    next;
  };
}
@gpsweeks or die "No dates specified";
$filelist[0] or die "SP3_FILENAME not specified";
if( $cachedir ) {
  -d $cachedir or die "SP3_CACHE must be a valid directory";
  push @searchpath,$cachedir;
}
if( @pathlist == 1 ) { # only one pathlist? Repeat once per filelist then
  @pathlist = (@pathlist) x @filelist;
} elsif( @pathlist ) { # else check for congruity with filenames
  @pathlist == @filelist or die 
    "Number of elements in SP3_FTPDIR must match SP3_FILENAME";
}

# this hash defines the known compressed formats
%compressors = (".Z" => "uncompress -c", ".gz" => "gunzip -c", "" => "cat" );

# now, start loop over dates to resolve files
for( $i=0; $i<@gpsweeks; $i++ ) {  
  # form filenames
  $week = $gpsweeks[$i];
  $day  = $weekdays[$i];
  @paths = @pathlist;
  @files = @filelist;
  foreach( @paths,@files ) {
    s/\@W/$week/g;
    s/\@d/$day/g;
  }
  $specification = "$week/$day";
  print "Resolving SP3 files for $specification\n";
  undef $flocal;
  undef $ftype;
  # first, see if we can find the file locally
  LOCALFILE: foreach $z ("",".gz",".Z") {  # loop over compressed types
    foreach $dir (@searchpath) {          # loop over search path
      for( $type=0; $type<@files; $type++ ) { # loop over file types
        $filename = "$dir/$files[$type]$z";
        if( -s $filename ) { # does this file exist?
          print "NORMAL: Found local file $filename\n";
          $flocal = 1;
          $ftype = $type;
          $fz = $z;
          last LOCALFILE;
        }
      }
    }
  } # end of loop over local files

  # if unable to find locally, try to find by ftp
  unless( defined $ftype ) {
    # set up ftp connection the first time round
    unless( defined $ftp ) {
      unless( eval {
            $ftpsite or die "SP3_FTPSITE not specified";
            @pathlist or die "SP3_FTPDIR not specified";
            # log in to ftp site
            $ftpuser = "anonymous";
            chop ($ftppassword = "$ENV{USER}@" . `domainname`);
            print "NORMAL: Connecting to $ftpsite as $ftpuser:$ftppassword\n";
            $ftp = Net::FTP->new($ftpsite, Debug => 0);
            defined $ftp or die "connect failed ($@)";
            $ftp->login("anonymous","$ENV{USER}@" . `domainname`) or die "login failed";
            print "Logged in\n";
            $ftp->binary();
            $ftp->hash(\*STDERR,8192);
            1;
          } )
      {
        $ftp = $ftp_failed = $@;
        print "WARN: FTP not available: $@\n";
      }
    }
    # do we have an ftp connection at all?
    unless( $ftp_failed ) {
      print "Trying to find $specification by ftp\n";
      FTPFILE: foreach $z (".Z",".gz","") {
        for( $type=0; $type<@files; $type++ ) {
          $dir = $paths[$type];
          $filename = "$files[$type]$z";
          unlink $filename if -e $filename;  
          $remote = "$dir/$filename";
          print "Trying $remote...\n";
          if( $ftp->get($remote) ) {
            $size = (stat $filename)[7];
            print "NORMAL: Got $filename via FTP ($size bytes)\n";
            $ftype = $type;
            $fz = $z;
            last FTPFILE;
          } 
        }
      } # end of loop over ftp files
    } 
  }
  # have we found any kind of file?
  undef $success;
  if( defined $ftype ) {
    # do we need to decompress or decompact the file?
    $decompress = $compressors{$fz};
    $normal_name = $files[$ftype];
    undef $existing_file;
    if( $fz ne "" ) { # does file require decompression?
      $command = "$decompress $filename >$normal_name";
      print "Decompressing ($command)\n";
      ($success = !system($command) and -e $files[$ftype] )
          or print "WARN: Unable to decompress $filename\n";
    } else {
      $success=$existing_file=$filename;
    }
    # If the file was retrieved by ftp, we'll try to move it to the ephemeris
    # cache. Local files stay in place
    if( $success ) {
      $resolved_count++;
      $ftp_count++ unless $flocal;
      if( defined $cachedir and !$flocal ) {
        if( -w $cachedir ) {
          if( !system("mv -f $filename $cachedir") ) {
            print "NORMAL: Cached file $filename\n";
            # if file was found as-is (i.e. not generated via decompress),
            # we need to modify the filename to reflect the new location
            $filename = "$cachedir/$filename" if $existing_file;
          } else {
            print "WARN: Unable to cache $filename in $cachedir\n";
          }
        } else {
          print "NORMAL: Cache directory not writable. Caching disabled.\n";
          undef $cachedir; 
        }
      } 
      # print out the resulting resolved filename
      if( $existing_file ) {
        print "EXISTING: $filename\n";
      } else {
        print "GENERATED: $normal_name\n";
      }
    } 
  }
  $success or print "WARN: No SP3 files found for $specification\n";
} # end of loop over dates
if( defined $resolved_count ) {
  print "NORMAL: $resolved_count SP3 files were resolved\n";
  print "NORMAL: $ftp_count of them were retrieved by FTP\n" if $ftp_count;
} else {
  print "WARN: No SP3 files were resolved\n";
}

$ftp->quit() if defined $ftp and !defined $ftp_failed;

exit 0;
