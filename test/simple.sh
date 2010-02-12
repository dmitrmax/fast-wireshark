#!/bin/sh

# Test to see if the dissector actually reads
# the messages it should.

if [ $# -lt 1 ]
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

stderrTshark="$dir/stderrTshark"
capturedTshark="$dir/captured.raw"

###### END GLOBAL VARIABLES ######

tell_test_status_on_3 ()
{
    echo $1 >&3
}

start_tshark ()
{
    $TSHARK -i lo -w "$capturedTshark" 2>> "$stderrTshark" &

    # Grab TShark's pid
    printf 'tshark\t%d\n' $! >> "$dir/pids"

    # Wait for TShark to be ready
    sleep 1
}

stop_tshark ()
{
    # Wait for TShark to be ready
    sleep 1
    stop_process 'tshark'
}

start_server ()
{
    {
        ./server $PORT localhost 2>> "$dir/stderrServ" &

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
    local name pid
    name=$1
    pid=$(grep -e "$name" < "$dir/pids" | cut -f 2)
    echo "killing $name $pid"

    # Kill the specified process
    kill "$pid"

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
    fi

    tell_test_status_on_3 $status
    return $status
} >&2

script_is_done ()
{
    local statusp
    statusp=$1
    # Display test result
    if [ "$statusp" -eq 0 ]
    then
        echo 'ALL TESTS PASSED'
        rm -r "$dir"
    else
        echo 'SOME TESTS FAILED'
        echo '\-- Hint: Check the first failure'
        echo "\-- See $dir for relevant files"
    fi

    return "$statusp"
} >&2

###### Generate test data
generate_test_data ()
{
    local enabledp name statusp
    start_server || return 1
    start_tshark

    which_tests_to_run \
    | \
    while read enabledp name
    do
        if [ "$enabledp" = 'yes' ]
        then
            "test_$name" 'generate' || return 1
        fi
    done

    statusp=$?

    stop_process 'server'
    stop_tshark
    wait

    return $statusp
}

###### Evaluate test data
evaluate_test_data ()
{
    local enabledp name
    which_tests_to_run \
    | \
    while read enabledp name
    do
        if [ "$enabledp" = 'yes' ]
        then
            "test_$name" 'evaluate' || return 1
        fi
    done
}

. './tests.sh'

{
    generate_test_data && evaluate_test_data
} 3>&1 \
| \
{
    # Check for failures
    ! grep -e '1' >/dev/null
    script_is_done $?
}

# Exit 1 if failed

