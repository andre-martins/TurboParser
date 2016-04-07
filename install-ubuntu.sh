#!/bin/bash                                                                                                                                                                                                        
sudo apt-get install -y build-essential automake autoconf

./install_deps.sh
rm missing
aclocal
autoconf
automake --add-missing
./configure && make && make install

sudo rm -rf /opt/TurboParser
sudo cp -r "$PWD" /opt/TurboParser
sudo ln -s /opt/TurboParser/Turbo* /usr/local/bin/

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/TurboParser/deps/local/lib:
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/TurboParser/deps/local/lib:"  >> ~/.bashrc
echo
echo "Installation finished"
echo
