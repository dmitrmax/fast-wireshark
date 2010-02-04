#!/bin/sh

# Test to see if the dissector actually reads
# the messages it should.

if [ $# -ne 2 ]
then
    echo "Usage: $0 PORT TSHARK"
    exit 1
fi

PORT="$1"
TSHARK="$2"

dir=`mktemp -d` || exit 1
textf="$dir/text"
capf="$dir/cap"

#timeout 3 
$TSHARK -i lo -c 2 -T fields -e fast.text -l \
 2> /dev/null \
 > "$textf" \
 &

tshark_pid=$!
sleep 1 # We must wait a sec for tshark to get ready

# Run client and server
server_text="message from server"
client_text="message from client"

./server "$server_text" $PORT > /dev/null &
./client "$client_text" localhost $PORT > /dev/null

# Should wait for tshark to exit
wait $tshark_pid

# See if wireshark got both messages
grep -e "$server_text" "$textf" > /dev/null && grep -e "$client_text" "$textf" > /dev/null
successp=$?

# Clean up temp files
rm -r "$dir"
# TODO: Do we care about cleaning up the temp file wireshark makes?

# Display test result
if [ $successp -eq 0 ]
then
    success_txt="PASSED"
else
    success_txt="FAILED"
fi

echo "Simple text exchange: $success_txt"

# Exit appropriately
exit $successp

