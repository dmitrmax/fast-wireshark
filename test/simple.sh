#!/bin/sh

# Test to see if the dissector actually reads
# the messages it should.

if [ $# -ne 2 ]
then
    echo "Usage: $0 port [tshark]" >&2
    exit 1
fi

###### BEGIN GLOBAL VARIABLES ######

PORT="$1"

TSHARK=tshark

if [ -n "$2" ]
then
    TSHARK="$2"
fi

dir=$(mktemp -d --tmpdir "fast-wireshark.XXXXXX") || exit 1

outDTshark="$dir/tshark_output"
mkdir "$outDTshark" || exit 1

# Will turn to 1 if a test fails
# This is our exit status
STATUSP=0

###### END GLOBAL VARIABLES ######

start_tshark ()
{
    $TSHARK -i lo -l $@ \
     2>> "$dir/stderrTshark" \
     &

    # Grab TShark's pid
    printf 'tshark\t%d\n' $! >> "$dir/pids"

    # Wait for TShark to be ready
    sleep 1
}

stop_tshark ()
{
    # Wait for TShark to be ready
    sleep 1
    stop_process tshark

    # TShark needs time to flush buffers
    sleep .2
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

run_client ()
{
    ./client "$@" $PORT localhost
} 2>> "$dir/stderrClient"

process_test_result ()
{
    local nameTest status
    nameTest=$1
    status=$2

    if [ "$status" -eq 0 ]
    then
        printf 'TEST: %s PASS\n' "$nameTest"
    else
        printf 'TEST: %s FAIL\n' "$nameTest"
        STATUSP=1
    fi
    return $status
} >&2

script_is_done ()
{
    # Stop spawned server
    stop_process server

    # Display test result
    if [ $STATUSP -eq 0 ]
    then
        echo 'ALL TESTS PASSED'
        rm -r "$dir"
        # TODO: Do we care about cleaning up the temp file wireshark makes?
    else
        echo 'SOME TESTS FAILED'
        echo '\-- Hint: Check the first failure'
        echo "\-- See $dir for relevant files"
    fi

    return $STATUSP
} >&2

start_server || exit 1

##### TEST: TShark Installed
# See if we can even execute TShark
command -v "$TSHARK" > /dev/null
process_test_result 'TShark Installed' $? \
|| \
{
    STATUSP=1
    echo '\-- You do not even have tshark installed >:('
    if command -v wireshark > /dev/null
    then
        echo '\-- But the strange part is, you do have wireshark!'
    fi
    echo '\-- I will just exit now...'
    script_is_done
    exit $?
}


###### TEST: Plugin Recognition
# See if the plugin is even recognized
{
    stdoutTshark="$outDTshark/plugin_recognition"

    start_tshark > "$stdoutTshark"
    run_client 'hello there server!'
    stop_tshark

    # TODO: this combo is ideal but doesn't work! why?
    # start_tshark -T fields -e udp > "$stdoutTshark"
    # sed -i -e 's/^.*Dst Port:\([^(]*\)(\([^)]*\)).*$/\1 \2/' "$stdoutTshark"
    # grep -e "$PORT" "$stdoutTshark"

    grep -e 'FAST' "$stdoutTshark" > /dev/null

    process_test_result 'Plugin Recognition' $? \
    || \
    {
        if [ -x "$HOME/.wireshark/plugins/packet-fast.so" ]
        then
            echo '\-- But it looks like the plugin is installed'

            grep -e '.' "$stdoutTshark" > /dev/null \
            && echo '\-- ... but tshark did pick something else up in its'  \
                    'place so there is definitely a problem with the plugin.' \
            || echo '\-- ... but tshark did not pick up anything' \
                    'so perhaps we killed it prematurely.'
        else
            echo '\-- Oh, the plugin is not installed, it should' \
                 "be located at $HOME/.wireshark/plugins/packet-fast.so"
        fi
    } >&2
}


###### TEST: Plain Field Recognition
# See if the plugin can echo a text message
{
    stdoutTshark="$outDTshark/plain_field_recognition"
    client_text="message from client"

    start_tshark -T fields -e udp.dstport -e fast.text > "$stdoutTshark"
    run_client "$client_text"
    stop_tshark

    # See if TShark got client_text
    grep -e '^'"${PORT}.*${client_text}"'$' "$stdoutTshark" > /dev/null

    process_test_result 'Plain Field Recognition' $? \
    || \
    {
        grep -e "$PORT" "$stdoutTshark" > /dev/null \
        || echo '\-- ... but it looks like we killed tshark prematurely'
    } >&2
}

script_is_done
exit $?

