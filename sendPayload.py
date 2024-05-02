from scapy.all import *

# Defina o endereço IP de destino e a porta
dest_ip = "10.0.0.114"
dest_port = 5555

# Crie o pacote
packet = IP(dst=dest_ip) / TCP(dport=dest_port) / "Hello, Network!"

# Envie o pacote
send(packet)
