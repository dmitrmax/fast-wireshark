#!/bin/sh

# Test to see if the dissector actually reads
# the messages it should.

show_usage ()
{
    local name
    name=$(basename "$0")
    cat <<EOF
------ Usage ------
$name (option)* -f file
$name (option)* -p port
    -f file
       Do not generate data, instead get it from a file
    -p port
       Use this port for testing the plugin
    -o file
       Generate test data to this file
    --tshark /path/to/tshark
       Use a TShark install thet is not in the path
EOF
} >&2


###### BEGIN GLOBAL VARIABLES ######

TSHARK=tshark

# Port on which to transfer data
PORT=

# To generate or not to generate. Default is unknown
dopGenerate=2

dir=$(mktemp -d --tmpdir "fast-wireshark.XXXXXX") || exit 1

stderrTshark=$dir/stderrTshark
capturedTshark=$dir/capture.raw

###### END GLOBAL VARIABLES ######

set_globals ()
{
    local opts
    opts=$(getopt -l 'tshark:' -o 'f:p:o:' -- "$@") \
    || { show_usage ; return 1 ; }

    eval set -- $opts

    while true
    do
        case "$1" in
            '-f') dopGenerate=0 ; capturedTshark=$2 ; shift ;;
            '-p') dopGenerate=1 ; PORT=$2           ; shift ;;
            '-o')                 capturedTshark=$2 ; shift ;;
            '--tshark')           TSHARK=$2         ; shift ;;
            '--') break ;;
            ''  ) echo 'Why is there an empty arg?' >&2 ; return 1 ;;
        esac
        shift
    done
    shift

    if [ 2 = $dopGenerate ]
    then
        echo 'You must specify an input file or a port!' >&2
        show_usage
        return 1
    fi

    return 0
}

set_globals "$@" || exit 1



alert_global_failure ()
{
    echo 1 >&3
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
            echo "Problem starting server, see error log in $dir/" >&2
            alert_global_failure
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
    ./client -p $PORT -h localhost "$@" $PORT
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
        alert_global_failure
    fi

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
        echo "\-- See $dir/ for relevant files"
    fi

    return "$statusp"
} >&2

###### Generate test data
generate_test_data ()
{
    local enabledp name statusp
    [ 0 = $dopGenerate ] && return 0 # Skip this step

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
    if [ ! -e "$capturedTshark" ]
    then
        printf 'No capture file: %s\n' "$capturedTshark" >&2
        alert_global_failure
        return 1
    fi

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

