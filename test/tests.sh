
which_tests_to_run ()
{
    cat <<EOF
yes  tshark_installed
yes  plugin_recognition
yes  plain_field_recognition
EOF
}

# See if we can even execute TShark
test_tshark_installed ()
{
    if [ "$1" != 'generate' ]
    then
        return 0
    fi

    which "$TSHARK" > /dev/null
    process_test_result 'TShark Installed' $? \
    || \
    {
        # Test failed
        tell_test_status_on_3 1

        echo '\-- You do not even have tshark installed >:('
        if command -v wireshark > /dev/null
        then
            echo '\-- But the strange part is, you do have wireshark!'
        fi
        echo '\-- I will just exit now...'
        return 1
    }
    return 0
} >&2

# See if the plugin is even recognized
test_plugin_recognition ()
{
    stdoutTshark="$outDTshark/plugin_recognition"

    if [ "$1" = 'generate' ]
    then
        run_client 'hello there server!'
        return $?
    fi

    tshark -r "$capturedTshark" 2>> "$stderrTshark" \
    | grep -e 'FAST' > /dev/null

    process_test_result 'Plugin Recognition' $? \
    || \
    {
        if [ -x "$HOME/.wireshark/plugins/packet-fast.so" ]
        then
            echo '\-- But it looks like the plugin is installed.'
        else
            echo '\-- Oh, the plugin is not installed, it should' \
                 "be located at $HOME/.wireshark/plugins/packet-fast.so"
        fi
    } >&2
    return 0
}

# See if the plugin can echo a text message
test_plain_field_recognition ()
{
    local client_text
    client_text="message from client"

    if [ "$1" = 'generate' ]
    then
        run_client "$client_text"
        return $?
    fi

    # See if TShark got client_text
    tshark -r "$capturedTshark" -T fields -e udp.dstport -e fast.text \
           2>> "$stderrTshark" \
    | grep -e '^'"${PORT}.*${client_text}"'$' > /dev/null

    process_test_result 'Plain Field Recognition' $?
    return 0
}

