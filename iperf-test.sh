echo "===================== Test UDP-Direct bandwith ====================="
iperf3 -u -c $1 -b 200M -i 1 -t 10 -l 300

sleep 3
echo "===================== Test UDP-over-TUN bandwith ====================="
iperf3 -u -c 10.0.0.11 -b 200M -i 1 -t 10 -l 300

sleep 3
echo "===================== Test TCP-Direct bandwith ====================="
iperf3 -c $1 -b 200M -i 1 -t 10 -l 300

sleep 3
echo "===================== Test TCP-over-TUN bandwith ====================="
iperf3 -c 10.0.0.11 -b 200M -i 1 -t 10 -l 300
