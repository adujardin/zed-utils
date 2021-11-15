mkdir build ; cd build ; cmake .. ; make -j4 ; cd ..

sudo cp build/zed_stream /usr/local/bin/
sudo cp zedstreamd.service /etc/systemd/system/
sudo systemctl start zedstreamd
sudo systemctl enable zedstreamd