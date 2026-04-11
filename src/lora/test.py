import sys

port = None

# allow argument for port
if len(sys.argv) > 1:
    port =  sys.argv[1]

# try to guess port
if not port:
    import serial.tools.list_ports
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if port.description == 'CP2102 USB to UART Bridge Controller':
            port = port.device

if not port:
    print("Usage: test.py <RNODE_PORT>")
    sys.exit(1)

from RNode import RNodeInterface

def gotPacket(data, rnode):
    message = data.decode("utf-8")
    print("Received a packet: "+message)
    print("RSSI: "+str(rnode.r_stat_rssi)+" dBm")
    print("SNR:  "+str(rnode.r_stat_snr)+" dB")

rnode = RNodeInterface(
    callback = gotPacket,
    name = "My RNode",
    port = port,
    frequency = 915000000,
    bandwidth = 125000,
    txpower = 2,
    sf = 7,
    cr = 5,
    loglevel = RNodeInterface.LOG_DEBUG
)
rnode.setPromiscuousMode(True)

try:
    print("Waiting for packets, hit enter to send a packet, Ctrl-C to exit")
    while True:
        message = input("Enter your message: ")
        data = message.encode("utf-8")
        print("sending", data.hex())
        rnode.send(data)
except KeyboardInterrupt as e:
    print("")
    exit()