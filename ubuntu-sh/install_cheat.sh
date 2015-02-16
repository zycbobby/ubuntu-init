wget https://www.python.org/ftp/python/3.4.2/Python-3.4.2.tar.xz -O  Python-3.4.2.tar.xz
tar xJf Python-3.4.2.tar.xz
cd Python-3.4.2
sudo ./configure --prefix=/opt/python3.4
make && sudo make install
echo 'alias python =python3' >> ~/.zshrc


sudo apt-get install python-pip

sudo pip install docopt pygments

git clone https://github.com/chrisallenlane/cheat.git

cd cheat

sudo python setup.py install

echo 'export EDITOR =/usr/bin/vim' >> ~/.zshrc

wget https://github.com/chrisallenlane/cheat/raw/master/cheat/autocompletion/cheat.bash 
sudo mv cheat.bash /etc/bash_completion.d/

