import sys
import time

port = None

# allow argument for port
if len(sys.argv) > 1:
    port = sys.argv[1]

# try to guess port
if not port:
    import serial.tools.list_ports

    for p in serial.tools.list_ports.comports():
        # I could put more tests here...
        if "CP210" in (p.description or ""):
            port = p.device
            break

if not port:
    print("Usage: test.py <RNODE_PORT>")
    sys.exit(1)

from RNode import RNodeInterface

rx_count = 0


def gotPacket(data, rnode):
    global rx_count
    rx_count += 1
    try:
        message = data.decode("utf-8")
    except:
        message = data.hex()
    print(
        f"[RX #{rx_count}] {message}  (RSSI:{rnode.r_stat_rssi} dBm  SNR:{rnode.r_stat_snr} dB)"
    )


rnode = RNodeInterface(
    callback=gotPacket,
    name="My RNode",
    port=port,
    frequency=915000000,
    bandwidth=125000,
    txpower=0,
    sf=7,
    cr=5,
    loglevel=RNodeInterface.LOG_DEBUG,
)

print(
    f"Validated: freq={rnode.r_frequency} bw={rnode.r_bandwidth} sf={rnode.r_sf} cr={rnode.r_cr} txp={rnode.r_txpower}"
)

rnode.setPromiscuousMode(True)
print("Promiscuous mode enabled")
print("Continuously sending 'rnode <n>' every 5 seconds. Ctrl-C to stop.\n")

tx_count = 0
try:
    while True:
        msg = f"rnode {tx_count}"
        data = msg.encode("utf-8")
        rnode.send(data)
        print(f"[TX #{tx_count}] {msg}")
        tx_count += 1
        time.sleep(5)
except KeyboardInterrupt:
    print("\nStopped.")
    exit()
