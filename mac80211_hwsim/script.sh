sudo modprobe mac80211
sudo insmod mac80211_hwsim.ko radios=4

sudo iw dev wlan0 interface add mesh0 type mp mesh_id prueba
sudo iw dev wlan1 interface add mesh1 type mp mesh_id prueba
sudo iw dev wlan2 interface add mesh2 type mp mesh_id prueba
#sudo iw dev wlan3 interface add mesh3 type mp mesh_id prueba
sudo ip link set dev mesh0 up
sudo ip link set dev mesh1 up
sudo ip link set dev mesh2 up
#sudo ip link set dev mesh3 up

sudo iw dev mesh0 set channel 1
sudo iw dev mesh1 set channel 1
sudo iw dev mesh2 set channel 1
#sudo iw dev mesh3 set channel 1

sudo ifconfig mesh0 192.168.40.10
sudo ifconfig mesh1 192.168.40.20
sudo ifconfig mesh2 192.168.40.30
#sudo ifconfig mesh3 192.168.40.40

