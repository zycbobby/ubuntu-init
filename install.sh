sudo cp /etc/apt/sources.list /etc/apt/sources.list
sudo cp ./resources/sources.list /etc/apt/sources.list 

sudo apt-get update

sudo apt-get install -y terminator zsh
#cp ./resources/terminator.config ~/.config/terminator/config

wget --no-check-certificate http://install.ohmyz.sh -O - | sh

chsh -s /bin/zsh

# install gnome
sudo apt-get install gnome-shell gnome-tweak-tool

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

# install maven
# Apache benchmark
sudo apt-get install apache2-utils

# install screen
sudo apt-get install screen
cp ./resources/.screenrc ~/.screenrc
