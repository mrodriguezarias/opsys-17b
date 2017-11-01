#!/bin/bash
echo "utnso" | sudo -S su &> /dev/null

echo "Updating packages…"
sudo apt-get update -qq

echo "Installing dependencies…"

echo -n " • so-commons-library: downloading…"
git clone -q https://github.com/sisoputnfrba/so-commons-library.git
echo -ne "\r\e[0K • so-commons-library: installing…"
sudo make install -s -B -C so-commons-library &> /dev/null
sudo rm -rf so-commons-library
echo -e "\r\e[0K • so-commons-library: done."

echo -n " • libreadline6-dev: downloading…"
sudo apt-get download -qq libreadline6-dev libtinfo-dev
echo -ne "\r\e[0K • libreadline6-dev: installing…"
sudo dpkg -i *.deb &> /dev/null
rm -f *.deb
echo -e "\r\e[0K • libreadline6-dev: done."

echo -n " • libssl-dev: downloading…"
sudo apt-get download -qq libssl-dev zlib1g-dev
echo -ne "\r\e[0K • libssl-dev: installing…"
sudo dpkg -i *.deb &> /dev/null
rm -f *.deb
echo -e "\r\e[0K • libssl-dev: done."

echo "Building modules…"

echo -n " • DataNode: building…"
make all -s -B -C DataNode/Debug &> /dev/null
echo -e "\r\e[0K • DataNode: done."
echo -n " • FileSystem: building…"
make all -s -B -C FileSystem/Debug &> /dev/null
echo -e "\r\e[0K • FileSystem: done."
echo -n " • Master: building…"
make all -s -B -C Master/Debug &> /dev/null
echo -e "\r\e[0K • Master: done."
echo -n " • Worker: building…"
make all -s -B -C Worker/Debug &> /dev/null
echo -e "\r\e[0K • Worker: done."
echo -n " • YAMA: building…"
make all -s -B -C YAMA/Debug &> /dev/null
echo -e "\r\e[0K • YAMA: done."

echo "Creating symlinks…"
sudo ln -sf $(readlink -m DataNode/Debug/DataNode) /usr/local/bin/dnode
sudo ln -sf $(readlink -m FileSystem/Debug/FileSystem) /usr/local/bin/fs
sudo ln -sf $(readlink -m Master/Debug/Master) /usr/local/bin/master
sudo ln -sf $(readlink -m Worker/Debug/Worker) /usr/local/bin/worker
sudo ln -sf $(readlink -m YAMA/Debug/YAMA) /usr/local/bin/yama

echo "Ready to run."
