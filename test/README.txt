PLAN RUNNER
______________________________________________________________________________
--- Description

Provides a simple way to generate predetermined sequences of FAST messages.

Allows output for UDP, UDP pcap file, TCP, raw bytes, and ascii binary.

Based on the FAST 1.1 specification
  http://www.fixprotocol.org/fastspec

______________________________________________________________________________
--- Building

>-> Get dependencies
Requires:
  JDK of atleast version 1.6
  openFAST
Optional:
  ant

JDK can be obtained from:
  http://www.oracle.com/

openFAST can be obtained from:
  http://www.openfast.org/

ant can be obtained from:
  http://ant.apache.org/

There is also an ant build rule fetchlib which will download the dependencies
automatically.

The dependencies need to be located in lib/ relative to this directory.

>-> Run Ant.
Run the ant build script and it will generate an executeable jar file named
"OpenFastPlanRunner.jar" in the current directory.


______________________________________________________________________________
--- Running

>-> Starting

The Plan Runner can be started with:
java -jar OpenFastPlanRunner.jar
  -t <file name> Specifies the template file to use (REQUIRED)
  -p <file name> Specifies the plan file to use (REQUIRED)
  
  OUTPUT CONTROL
  By default ascii binary is output
  -b Output raw bytes of the encoding
  -n <port> Specifies the port to use, will send UDP packets by default
  -P <file name> Outputs a pcap file, requires -n, doesn't create any network
    traffic
  -T Use TCP instead of UDP, requires -n


______________________________________________________________________________
--- Template and Plan Files

>-> Template Files

Template files are the standard template files as defined in the FAST 1.1
specification.

>-> Plan Files

Plan files are simple XML documents that contain a sequence of fast packets to
send.

The general format is:
<plan>
  <message value="<tid>"
    <<field type> value="<value>" />
    <<optional field> />
  </message>
</plan>

The plan runner will attempt to convert values to the correct type based on
the conversion rules in the FAST specification.

These plan files will support sending single message packets with correct data.

If a plan file needs to contain multiple messages, error data, or a specific
format of data a bytemessage plan file should be used instead.

These plan files specify in ascii binary the bytes to send as written.  The
general format is:
<plan>
  <bytemessage>
    11000000
    10000000
    10000000
  </bytemessage>
</plan>

These bytes will be sent as written to the output stream specified by the.
command line switches.

Example plan files can be found in the plans/ and byteplans/ directories.


____________________________________________________________________________
--- EOF