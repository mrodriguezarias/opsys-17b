#!/bin/bash
echo "utnso" | sudo -S su &> /dev/null

base="/home/utnso/git/tp-2017-2c-YATPOS"
yatpos="/home/utnso/yatpos"
declare -A procs=([dnode]="DataNode" [fs]="FileSystem" [master]="Master" [worker]="Worker" [yama]="YAMA")

[ -f "/usr/lib/libcommons.so" ]; libcommons_installed=$?
dpkg-query -W -f='${Status}' libreadline6-dev 2>/dev/null | grep -q "install ok"; libreadline_installed=$?
dpkg-query -W -f='${Status}' libssl-dev 2>/dev/null | grep -q "install ok"; libssl_installed=$?

if [ $libreadline_installed -ne 0 -o $libssl_installed -ne 0 -o $libcommons_installed -ne 0 ] ; then
	echo "Installing dependencies…"
fi

if [ $libcommons_installed -ne 0 ] ; then
echo -ne " • \e[1mlibcommons\e[0m: downloading…"
git clone -q https://github.com/sisoputnfrba/so-commons-library.git
echo -ne "\r\e[0K • \e[1mlibcommons\e[0m: installing…"
sudo make install -s -B -C so-commons-library &> /dev/null
sudo rm -rf so-commons-library
echo -e "\r\e[0K • \e[1mlibcommons\e[0m: done."
fi

if [ $libreadline_installed -ne 0 ] ; then
echo -ne " • \e[1mlibreadline6-dev\e[0m: downloading…"
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/n/ncurses/libtinfo-dev_5.9+20140118-1ubuntu1_i386.deb
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/r/readline6/libreadline6-dev_6.3-4ubuntu2_i386.deb
echo -ne "\r\e[0K • \e[1mlibreadline6-dev\e[0m: installing…"
sudo dpkg -i *.deb &> /dev/null
rm -f *.deb
echo -e "\r\e[0K • \e[1mlibreadline6-dev\e[0m: done."
fi

if [ $libssl_installed -ne 0 ] ; then
echo -ne " • \e[1mlibssl-dev\e[0m: downloading…"
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/z/zlib/zlib1g-dev_1.2.8.dfsg-1ubuntu1_i386.deb
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/o/openssl/libssl-dev_1.0.1f-1ubuntu2.23_i386.deb
echo -ne "\r\e[0K • \e[1mlibssl-dev\e[0m: installing…"
sudo dpkg -i *.deb &> /dev/null
rm -f *.deb
echo -e "\r\e[0K • \e[1mlibssl-dev\e[0m: done."
fi

if [ ! -d $base ]; then
echo "Reticulating splines…"
sleep 0.5

echo "Copying files…"
mkdir -p $base
cp -r . $base
fi

if [ ! -L /usr/local/bin/yatpos ]; then
echo "Creating symlinks…"

for kcur in "${!procs[@]}"; do
	vcur="${procs[$kcur]}"
	sudo ln -sf $base/$vcur/Debug/$vcur /usr/local/bin/$kcur
done

echo "Installing helper script…"
sudo ln -sf $base/Shared/scripts/yatpos /usr/local/bin/yatpos
fi

echo "Ready. Run:"
echo -e " • \e[1myatpos build\e[0m to build modules"
echo -e " • \e[1myatpos config\e[0m to config modules"
echo -e " • \e[1mdnode\e[0m|\e[1mfs\e[0m|\e[1mmaster\e[0m|\e[1mworker\e[0m|\e[1myama\e[0m to run modules"
echo -e " • \e[1myatpos log\e[0m to see logs"
echo -e " • \e[1myatpos uninstall\e[0m to uninstall"
