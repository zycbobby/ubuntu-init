# Mount the disk on /dev/sdb2

# Mount disk
if [ ! -d /mnt/windowsc ]; then
	sudo mkdir /mnt/windowsc
	sudo mount /dev/sdb1 /mnt/windowsc
else
	echo '/mnt/windowsc alerady exists'
fi


