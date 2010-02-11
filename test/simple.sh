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

dir=$(mktemp -d --tmpdir "fast-wireshark.XXXXXX") || exit 1
textf="$dir/text"

# Will turn to 1 if a test fails
# This is our exit status
STATUSP=0

start_tshark ()
{
    $TSHARK -i lo -T fields -e fast.text -l \
     2>> "$dir/stderrTshark" \
     &

    # Grab TShark's pid
    printf 'tshark\t%d\n' $! >> "$dir/pids"

    # Wait for TShark to be ready
    sleep 1
}

start_server ()
{
    {
        ./server $PORT localhost \
         2>> "$dir/stderrServ" \
         &

        # Grab server's pid
        printf 'server\t%d\n' $! >> "$dir/pids"
    } \
    | \
    {
        # Wait for server to be ready
        read -r listenerc anon \
        || \
        {
            echo "Problem starting server, see error log in $dir" >&2
            return 1
        }

        echo "Server started, we have $listenerc listeners"
        return 0
    }
} >> "$dir/logScript"

stop_process ()
{
    local name
    name=$1
    echo "killing $name"

    # Kill the specified process
    kill $(grep -e "$name" < "$dir/pids" | cut -f 2)

    # Remove its corresponding record
    sed -i -e "/$name/d" "$dir/pids"

} 2>&1 >> "$dir/logScript"


client_text="message from client"

start_server || exit 1
start_tshark >> "$dir/stdoutTshark"
    
# So run client
./client "$client_text" $PORT localhost 2>> "$dir/stderrClient"

# Give TShark some time to collect the traffic
sleep 2

# See if TShark got client_text
grep -e '^'"${client_text}"'$' "$dir/stdoutTshark" \
 > /dev/null \
&&   echo "Simple exchange PASSED" >&2 \
|| { echo "Simple exchange FAILED" >&2 ; STATUSP=1 ; }

# Stop spawned processes
stop_process tshark
stop_process server

# Display test result
if [ $STATUSP -eq 0 ]
then
    echo "ALL TESTS PASSED"
    rm -r "$dir"
    # TODO: Do we care about cleaning up the temp file wireshark makes?
else
    echo "SOME TESTS FAILED"
    echo "check $dir for relevant files"
fi

exit $STATUSP

