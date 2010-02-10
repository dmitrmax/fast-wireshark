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

client_text="message from client"

cleanup_script ()
{
    local pids
    pids=$(cut -d ' ' -f '2' "$dir/pids")

    echo "killing: $pids"
    kill $pids

} 2>&1 >> "$dir/logScript"

#timeout 3 
{
    $TSHARK -i lo -T fields -e fast.text -l \
     > "$dir/stdoutTshark" \
     2>&3 \
     &
    # Grap TShark's pid
    echo "tshark $!" >> "$dir/pids"
} 3>&1 \
| \
{ # IN: TShark's stderr 
    
    # Wait for TShark to be ready
    read -r useless || exit 1

    ./server $PORT localhost 2> "$dir/stderrServ" &
    echo "server $!" >> "$dir/pids"
} \
| \
{ # IN: Server's stdout
    read -r listenerc anon || exit 1
    echo "we have $listenerc listeners" >> "$dir/logScript"
    # Server is now listening
    # So run client
    ./client "$client_text" $PORT localhost 2> "$dir/stderrClient"
}

sleep 2
statusp=0

# See if TShark got client_text
grep -e "$client_text" "$dir/stdoutTshark" \
 > /dev/null \
&&   echo "Simple exchange PASSED" >&2 \
|| { echo "Simple exchange FAILED" >&2 ; statusp=1 ; }

# Display test result
if [ $statusp -eq 0 ]
then
    echo "ALL TESTS PASSED"
    # TODO: Do we care about cleaning up the temp file wireshark makes?
else
    echo "SOME TESTS FAILED"
    echo "check $dir for relevant files"
fi

cleanup_script
exit $statusp

