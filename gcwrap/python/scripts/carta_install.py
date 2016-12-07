import re
import sys
import os
import argparse
import urllib
import urllib2
from HTMLParser import HTMLParser
from subprocess import check_output
import platform

# Supported operating system versions
osVersions = ["el6", "el7", "osx1011", "osx1012"]
script_dir = os.path.dirname(os.path.realpath(__file__))


class MyHTMLParser(HTMLParser):

    links = []

    def handle_starttag(self, tag, attrs):
        # Only parse the 'anchor' tag.
        if tag == "a":
            # Check the list of defined attributes.
            for name, value in attrs:
                # If href is defined, print it.
                if name == "href":
                    print name, "=", value
                    if value.lower().startswith("carta") and ".md5" not in value:
                        self.links.append(value)

    def get_links(self):
        return self.links

# sampleUrl=https://svn.cv.nrao.edu/casa/distro/osx/cartadev/master/
base_url = "https://svn.cv.nrao.edu/casa/distro/"
url_tail = "/cartadev/dev/"
package_name = ""


def splitext(path):
    for ext in ['.tar.gz', '.tar.bz2']:
        if path.endswith(ext):
            return path[:-len(ext)], path[-len(ext):]
    return os.path.splitext(path)


def resolve_os():
    print "Resolving OS"
    detected_os = "linux"
    if "el7" in platform.uname()[2]:
        detected_os = 'el7'
    if "el6" in platform.uname()[2]:
        detected_os = 'el6'
    if "Darwin Kernel Version 15" in platform.uname()[3]:
            # os = 'osx1011'
            detected_os = 'osx'
    if "Darwin Kernel Version 16" in platform.uname()[3]:
            # os= 'osx1012'
            detected_os = 'osx'
    print "Found " + detected_os + " " + detected_os
    return detected_os


def resolve_url():
    print "Resolving package URL"
    # OSX URL
    stitched_url = base_url + curr_os + url_tail

    # Linux URL
    if curr_os.startswith("el"):
        stitched_url = base_url + "linux" + url_tail + curr_os + "/"
    return stitched_url


def get_latest_link(html):
    print "Getting the latest link"
    print html
    link_parser = MyHTMLParser()
    link_parser.feed(html)
    result = link_parser.get_links()
    split_packages = []
    for pkg in result:
        print pkg
        split_pkg = re.split("[\.|-]", pkg)
        print split_pkg
        split_packages.append(split_pkg)

    if curr_os == "osx":
        sorted_packages = sorted(split_packages,
                                 key=lambda x: (int(x[-6]), int(x[-5]), int(x[-4]), int(x[-3])), reverse=True)
    else:
        sorted_packages = sorted(split_packages,
                                 key=lambda x: (int(x[-7]), int(x[-6]), int(x[-5]), int (x[-4])), reverse=True)
    lead = "-".join(sorted_packages[0][0:2])
    version = ".".join(sorted_packages[0][2:5])
    rev = sorted_packages[0][5]
    tail = ".".join(sorted_packages[0][6:len(sorted_packages[0])])
    print lead
    print version
    print rev
    print tail

    current_version = lead + "-" + version + "-" + rev + "." + tail

    return current_version


# Read the arguments
parser = argparse.ArgumentParser(description="Download and install Carta.")
parser.add_argument("--os", help="Choose a specific operating system. Valid options" + str(osVersions))
parser.add_argument("--url", help="Specific url for the package")
parser.add_argument("--installationtype", help="Type of installation. Valid values: dev, package", default="dev")
parser.add_argument("--installationdir", help="The directory to install Carta to.")
args = parser.parse_args()

installation_directory=""
# Resolve operating system
curr_os = resolve_os()

if args.os is not None:
    curr_os = args.os


# Resolve the latest package url
url = resolve_url()

packageUrl = ""

if args.url is not None:
    packageUrl = args.url
else:
    print "Downloading package for " + curr_os
    print "url: " + url

    response = urllib2.urlopen(url)
    download_page_html = response.read()
    latest_link = get_latest_link(download_page_html)
    packageUrl = url + latest_link


# Figure out the correct installation directory
installation_type = ""
if args.installationtype is not None:
    installation_type = args.installationtype

if args.installationdir is not None:
    installation_directory = args.installationdir

if installation_type == "package" and installation_directory == "":
    print "Installing Carta in packaging environment"
    if curr_os == "osx":
        installation_directory = script_dir + "/../../Resources"
        print "Installing casa into " + installation_directory
    elif curr_os.startswith("el") :
        installation_directory = script_dir + "/../../etc"
        print "Installing casa into " + installation_directory
    else:
        print "Couldn't resolve installation directory for the package build."
elif installation_directory != "":
    print "Installing Carta to " + installation_directory
else:
    print "No valid installation type or directory provided. Exiting..."
    sys.exit()


# Fetch the package
print "Download link: " + packageUrl
urllib.urlretrieve(packageUrl, latest_link)

# Extract package
if curr_os.startswith("osx"):
    out = check_output(["hdiutil", "attach", latest_link])
    package_dir = "/Volumes/"
    m = re.search('/Volumes/(Carta.*)', out)
    if m:
        # package_dir = package_dir + m.group(1).strip()
        package_dir += m.group(1).strip()
    print "Package dir: " + package_dir

    # Copy to the right place here
    check_output(["cp", "-r", package_dir + "/Carta.app", installation_directory])

    out = check_output(["hdiutil", "detach", package_dir])
    print out
    out = check_output(["rm", latest_link])
    print out
elif curr_os.startswith("el"):
    urllib.urlretrieve (packageUrl)
    install_dir_mod = installation_directory + "/carta"
    print "Installing Carta to " +  install_dir_mod
    print "Extracting " + latest_link
    out = check_output(["tar", "-xvzf", latest_link])
    print out
    out = check_output(["mv",  splitext(latest_link)[0], install_dir_mod])
    print out
    out = check_output(["rm", latest_link])
    print out
else:
    print "Unsupported operating system"
