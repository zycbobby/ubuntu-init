sudo cp /etc/apt/sources.list /etc/apt/sources.list
sudo cp ./resources/sources.list /etc/apt/sources.list 

sudo apt-get update

sudo apt-get install -y terminator zsh curl
#cp ./resources/terminator.config ~/.config/terminator/config

wget --no-check-certificate http://install.ohmyz.sh -O - | sh

chsh -s /bin/zsh

# install gnome
sudo apt-get install gnome-shell gnome-tweak-tool tint2

# install i3 ref: https://i3wm.org/docs/repositories.html
apt-get install i3


# sysv-rc-conf
sudo apt-get install sysv-rc-conf

#install docker
sudo apt-get install docker.io

# install JDK
sudo apt-get install openjdk-7-jdk
# install maven
sudo apt-get install maven

# install tomcat
sudo apt-get install tomcat7 tomcat7-admin tomcat7-docs tomcat7-common
TOMCAT__CONF_HOME='/var/lib/tomcat7/conf/'
TOMCAT_USER_FILENAME='tomcat-users.xml'
sudo cp $TOMCAT_CONF_HOME+$TOMCAT_USER_FILENAME $TOMCAT_CONF_HOME+$TOMCAT_USER_FILENAME+'.bak'
sudo cp ./resources/$TOMCAT_USER_FILENAME $TOMCAT_CONF_HOME+$TOMCAT_USER_FILENAME

# Apache benchmark
sudo apt-get install apache2-utils

# install screen
# sudo apt-get install screen
# cp ./resources/.screenrc ~/.screenrc

# Install tmux
sudo apt-get install tmux

# install fcitx-sunpinyin

# how to install:  http://freetstar.com/ubuntu-most-use-friendly-fcitx-sunpinyin/
# About bug of yue http://forum.ubuntu.org.cn/viewtopic.php?f=180&t=457956&sid=b0980e8a6d2dfd4f0c33c4b562b5cb95&start=15


# install aria2c, it was a good download tool in terminal
sudo apt-get instal aria2c
