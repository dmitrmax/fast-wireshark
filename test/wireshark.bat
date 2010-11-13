
"C:\Program Files\Wireshark\wireshark" -r "output.pcap" ^
  -o fast.port:"5000" ^
  -o fast.template:"templates.xml" ^
  -o fast.enabled:true ^
  -o fast.show_empty:true

pause

