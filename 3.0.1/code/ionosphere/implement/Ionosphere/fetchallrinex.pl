#!/usr/bin/perl
# fetchallrinex.pl: Perl script for resolving/fetching a number of RINEX files
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
    RINEX_PATH=             search path
    RINEX_CACHE=            cache directory
    RINEX_FILENAME=         template for RINEX filenames
    RINEX_FILENAME_COMPACT= template for compacted-RINEX filenames
    RINEX_FTPSITE=          remote FTP site
    RINEX_FTPDIR=           template for remote RINEX directory
    RINEX_FTPDIR_COMPACT=   template for remote compacted-RINEX directory
Then use:
    fetchrinex.pl <stations> <dates as YYYY:DOY:MM:DD>\n";

# scan arguments and environment to set everything up. Use reasonable
# defaults
@searchpath = map { glob($_) } split /:/,($ENV{RINEX_PATH} || ".");
$cachedir = glob($ENV{RINEX_CACHE});
$filelist[0] = $ENV{RINEX_FILENAME} || '@S@D0.@yo';
$filelist[1] = $ENV{RINEX_FILENAME_COMPACT} || '@S@D0.@yd';
$ftpsite = $ENV{RINEX_FTPSITE} || 'igs.ensg.ign.fr';
$pathlist[0] = $ENV{RINEX_FTPDIR} || '/pub/igs/data/@Y/@D';
$pathlist[1] = $ENV{RINEX_FTPDIR_COMPACT};

foreach( @ARGV ) {
#   /^CACHE=(.*)$/ and do { $cachedir=$1;  next; };
#   /^PATH=(.*)$/ and do  { @searchpath=split /:/,$1; next; };
#   /^SITE=(.*)$/ and do  { $ftpsite=$1;     next; };
#   /^DIR=(.*)$/ and do   { $pathlist[0]=$1; next; };
#   /^DIRC=(.*)$/ and do  { $pathlist[1]=$1; next; };
#   /^FILE=(.*)$/ and do  { $filelist[0]=$1; next; };
#   /^FILEC=(.*)$/ and do { $filelist[1]=$1; next; };
  /^....$/ and do { 
    push @stations,$_; 
    next; 
  };
  /^(\d+):(\d+):(\d+):(\d+)$/ and do { 
    push @years,sprintf("%04d",$1); 
    push @yys,sprintf("%02d",$1%100); 
    push @yeardays,sprintf("%03d",$2); 
    push @months,sprintf("%02d",$3); 
    push @monthdays,sprintf("%02d",$4); 
    next;
  };
}
@stations or die "No stations specified";
@years or die "No dates specified";
$filelist[0] or die "RINEX_FILENAME not specified";
$pathlist[1] or $pathlist[1]=$pathlist[0];
if( $cachedir ) {
  -d $cachedir or die "RINEX_CACHE must be a valid directory";
  push @searchpath,$cachedir;
}

# this hash defines the known compressed formats
%compressors = (".Z" => "uncompress -c", ".gz" => "gunzip -c", "" => "cat" );

# now, start loops over stations and dates to resolve files
foreach $station (@stations) {     # loop over stations...
  for( $i=0; $i<@years; $i++ ) {   # ...and dates
    # form filenames
    @path = @pathlist;
    @file = @filelist;
    foreach( @path,@file ) {
      s/\@S/$station/g;
      s/\@Y/$years[$i]/g;
      s/\@y/$yys[$i]/g;
      s/\@D/$yeardays[$i]/g;
      s/\@m/$months[$i]/g;
      s/\@d/$monthdays[$i]/g;
    }
    $specification = "$station $years[$i]/$months[$i]/$monthdays[$i]";
    print "Resolving RINEX file for $specification\n";
    undef $ftype;
    undef $flocal;
    # first, see if we can find the file locally
    LOCALFILE: foreach $z ("",".gz",".Z") {  # loop over compressed types
      foreach $dir (@searchpath) {          # loop over search path
        foreach $type (0,1) {               # loop over file types
          $filename = "$dir/$file[$type]$z";
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
              $ftpsite or die "RINEX_FTPSITE not specified";
              $pathlist[0] or $pathlist[1] or die "RINEX_FTPDIR or RINEX_FTPDIRC not specified";
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
          # try the two kinds of files, with compacted files having priority
          # because of their smaller size
          foreach $type (1,0) {
            $dir = $path[$type];
            $filename = "$file[$type]$z";
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
      undef $existing_file;
      if( $ftype ) { # The file is a compacted RINEX, so we need to run crx2rnx on it
        $command = "$decompress $filename|crx2rnx - >$file[0]";
        print "Decompacting ($command)\n";
        ($success = !system($command) and -e $file[0] )
            or print "WARN: Unable to decompact $filename\n";
      } else { # the file is not compacted 
        if( $fz ne "" ) { # does file require decompression?
          $command = "$decompress -c $filename >$file[0]";
          print "Decompressing ($command)\n";
          ($success = !system($command) and -e $file[0] )
              or print "WARN: Unable to decompress $filename\n";
        } else {
          $success=$existing_file=$filename;
        }
      }
      # If the file was retrieved by ftp, we'll try to move it to the RINEX 
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
          print "GENERATED: $file[0]\n";
        }
      } 
    }
    $success or print "WARN: no data found for $specification\n";
  } # end of loop over dates
} # end of loop over stations
if( defined $resolved_count ) {
  print "NORMAL: $resolved_count RINEX files were resolved\n";
  print "NORMAL: $ftp_count of them were retrieved by FTP\n" if $ftp_count;
} else {
  print "WARN: No RINEX files were resolved\n";
}

$ftp->quit() if defined $ftp and !defined $ftp_failed;

exit 0;
