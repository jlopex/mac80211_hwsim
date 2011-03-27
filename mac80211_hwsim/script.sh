sudo modprobe mac80211
sudo insmod mac80211_hwsim.ko

sudo iw dev wlan0 interface add mesh0 type mp mesh_id prueba
sudo iw dev wlan1 interface add mesh1 type mp mesh_id prueba
sudo ip link set dev mesh0 up
sudo ip link set dev mesh1 up

sudo iw dev mesh0 set channel 1
sudo iw dev mesh1 set channel 1

#sudo ifconfig mesh0 192.168.1.1
#sudo ifconfig mesh1 192.168.1.2

#sudo brctl addbr br0
#sudo brctl addif br0 eth1-eth2
#sudo brctl addif br0 mesh0
#sudo brctl addbr br1
#sudo brctl addif br1 eth2
#sudo brctl addif br1 mesh1

