#!/usr/bin/bash

sudo ip tuntap add dev tap0 mode tap
sudo ip link set tap0 up
sudo ip a add 10.0.0.2/31 dev tap0

sudo ip link add name br0 type bridge
sudo ip link set br0 up
sudo ip link set tap0 master br0
