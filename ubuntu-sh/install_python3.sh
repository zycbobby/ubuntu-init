wget https://www.python.org/ftp/python/3.4.3/Python-3.4.3.tgz
tar -xvf Python-3.4.3.tgz
cd Python-3.4.3
./configure
make && sudo make install

ln -sf /usr/bin/python3 /usr/bin/python 

sudo apt-get install python3-setuptools
sudo easy_install3 pip

# run pip3 install XXX
