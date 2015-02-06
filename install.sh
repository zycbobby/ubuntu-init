sudo cp /etc/apt/sources.list /etc/apt/sources.list
sudo cp ./resources/sources.list /etc/apt/sources.list 

sudo apt-get update

sudo apt-get install terminator zsh
cp ./resources/terminator.config ~/.config/terminator/config

wget --no-check-certificate http://install.ohmyz.sh -O - | sh

chsh -s /bin/zsh

# install gnome
sudo apt-get install gnome-shell

# install JDK

# install tomcat

# Apache benchmark
sudo apt-get install apache2-utils
