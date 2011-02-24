
______________________________________________________________________________
--- Building


>-> Get dependencies.
Last time I checked, this is all you need from a bare debian-based system.
sudo apt-get install build-essential bison flex \
                  libgtk2.0-dev libpcap-dev \
                  libxml2-dev cmake

You may want to get Wireshark too.
  apt-get install wireshark

These are pretty much the same packages you'll need for Fedora
sudo yum install build-essential bison flex \
                  glib2-devel libpcap-devel \
                  libxml2-devel cmake \
                  gtk+

>-> Setup Wireshark source.
Download the Wireshark source and move it to ../wireshark relative to this
directory. Next, build you need to generate a correct config.h from the
Wireshark source directory.
  ./configure


>-> Run CMake.
Poll the system for dependent libraries.
  cmake .

If all went well, you will never need to do that again. It will
automatically be rerun as needed if the CMakeLists.txt file
changes.


>-> Build and install.
Make the plugin, install it in $HOME/.wireshark/plugins.
  make
  make install

Of course, you can just always run 'make install' and it'll do the 'make' part
for you.


>-> Fix your permissions.
On Ubuntu, and other Debian-based systems I assume, this must be done to allow
a user to run Wireshark. If root (or a sudo'd user) runs Wireshark, user
plugins WILL NOT LOAD. The following allows dumpcap to listen on network
interfaces without being run as root.
  setcap 'CAP_NET_RAW+eip CAP_NET_ADMIN+eip' `which dumpcap`

You may need to install "setcap".
  apt-get install libcap2-bin
  or
  yum install libcap2-bin

For more information on this, see:
http://wiki.wireshark.org/CaptureSetup/CapturePrivileges


______________________________________________________________________________
--- Notes for maintainers

  cmake -DCMAKE_BUILD_TYPE=Release -DCPACK_GENERATOR=DEB
  make package

This will create a debian package, distribute it to the masses.
To change back into regular development mode you must rerun cmake.
  cmake -DCMAKE_BUILD_TYPE=

______________________________________________________________________________
--- EOF

