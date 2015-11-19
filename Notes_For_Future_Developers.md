# Folders #

### main directory ###
This directory contains most of the code base for the plugin.  The basic boilerplate Wireshark plugin code is found in packet-fast.c.  The bulk of the dissection occurs in dissect.c and basic-dissect.c.  dictionaries.c keeps track of the state.
### bin ###
This folder contains executable files for our developer tools.  svngdiff is useful for comparing your current sandbox to the committed repository (to use, simply type "bin/svngdiff" in the command line).  README's for the other tools can be found in the util directory.
### CMakeFiles ###
This folder contains auto-generated files that CMake uses.  There is no need to read these files or modify them.
### test ###
This folder contains some sample test plans and the template file we use to do regression testing.  It also contains the OpenFASTPlanRunner, which is a java tool we made for automatically sending FAST packets on lo, so that we can test the plugin in wireshark.  See the README in this directory for more information.
### Testing ###
This folder contains log files generated during testing.
### util ###
This folder contains the code for our developer tools, CMake and rwcompare.  For more information on these tools, see the README files in their respective directories.