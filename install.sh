#!/bin/bash
echo "utnso" | sudo -S su &> /dev/null

base="/home/utnso/git/tp-2017-2c-YATPOS"
yatpos="/home/utnso/yatpos"
declare -A procs=([dnode]="DataNode" [fs]="FileSystem" [master]="Master" [worker]="Worker" [yama]="YAMA")
[[ "$#" -eq 1 && ("$1" == "node" || -n "${procs[$1]}") ]] && proc="$1" || proc="all"
[ -n "${procs[$proc]}" ] && n=1 || n=0

[ -f "/usr/lib/libcommons.so" ]; libcommons_installed=$?
dpkg-query -W -f='${Status}' libreadline6-dev 2>/dev/null | grep -q "install ok"; libreadline_installed=$?
dpkg-query -W -f='${Status}' libssl-dev 2>/dev/null | grep -q "install ok"; libssl_installed=$?

if [ $libreadline_installed -ne 0 -o $libssl_installed -ne 0 -o $libcommons_installed -ne 0 ] ; then
	echo "Installing dependencies…"
fi

if [ $libcommons_installed -ne 0 ] ; then
echo -n " • libcommons: downloading…"
git clone -q https://github.com/sisoputnfrba/so-commons-library.git
echo -ne "\r\e[0K • libcommons: installing…"
sudo make install -s -B -C so-commons-library &> /dev/null
sudo rm -rf so-commons-library
echo -e "\r\e[0K • libcommons: done."
fi

if [ $libreadline_installed -ne 0 ] ; then
echo -n " • libreadline6-dev: downloading…"
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/n/ncurses/libtinfo-dev_5.9+20140118-1ubuntu1_i386.deb
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/r/readline6/libreadline6-dev_6.3-4ubuntu2_i386.deb
echo -ne "\r\e[0K • libreadline6-dev: installing…"
sudo dpkg -i *.deb &> /dev/null
rm -f *.deb
echo -e "\r\e[0K • libreadline6-dev: done."
fi

if [ $libssl_installed -ne 0 ] ; then
echo -n " • libssl-dev: downloading…"
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/z/zlib/zlib1g-dev_1.2.8.dfsg-1ubuntu1_i386.deb
curl -sO http://ubuntu.unc.edu.ar/ubuntu/pool/main/o/openssl/libssl-dev_1.0.1f-1ubuntu2.23_i386.deb
echo -ne "\r\e[0K • libssl-dev: installing…"
sudo dpkg -i *.deb &> /dev/null
rm -f *.deb
echo -e "\r\e[0K • libssl-dev: done."
fi

if [ ! -d $base ]; then
echo "Reticulating splines…"
sleep 0.5

echo "Installing shared library…"
mkdir -p $base
cp -r Shared $base
fi

printf "Building module%.*s…\n" $((n != 1)) "s"

for kcur in "${!procs[@]}"; do
	[[ "$proc" != "all" ]] && [[ "$kcur" != "$proc" ]] && [[ "$proc" != "node" || "$kcur" != "dnode" && "$kcur" != "worker" ]] && continue
	vcur="${procs[$kcur]}"
	echo -n " • $vcur: building…"
	cp -r $vcur $base
	make all -s -B -C $base/$vcur/Debug &> /dev/null
	echo -e "\r\e[0K • $vcur: done."
done

if [ ! -L /usr/local/bin/log ]; then
printf "Creating symlink%.*s…\n" $((n != 1)) "s"

for kcur in "${!procs[@]}"; do
	[[ "$proc" != "all" ]] && [[ "$kcur" != "$proc" ]] && [[ "$proc" != "node" || "$kcur" != "dnode" && "$kcur" != "worker" ]] && continue
	vcur="${procs[$kcur]}"
	sudo ln -sf $base/$vcur/Debug/$vcur /usr/local/bin/$kcur
done

echo "Installing log utility…"
sudo ln -sf $base/Shared/scripts/log /usr/local/bin/log
fi

if [ ! -d $yatpos/config ]; then
printf "Creating configuration file%.*s…\n" $((n != 1)) "s"
mkdir -p $yatpos/config

for kcur in "${!procs[@]}"; do
	[[ "$proc" != "all" ]] && [[ "$kcur" != "$proc" ]] && [[ "$proc" != "node" || "$kcur" != "dnode" ]] && continue
	[[ "$kcur" == "dnode" || "$kcur" == "worker" ]] && vcur="Node" || vcur="${procs[$kcur]}"
	file="$yatpos/config/$vcur.cnf"
	[ -f $file ] && continue
	cp $base/Shared/rsc/config/$vcur.cnf $file
	case "$vcur" in
	"Node")
		ip_fs_key="IP_FILESYSTEM"
		printf "$ip_fs_key="; read ip_fs_val < /dev/tty
		sed -i "s/^\($ip_fs_key=\).*/\1$ip_fs_val/" $file
		node_name_key="NOMBRE_NODO"
		printf "$node_name_key="; read node_name_val < /dev/tty
		sed -i "s/^\($node_name_key=\).*/\1$node_name_val/" $file
		data_size_key="DATABIN_SIZE"
		printf "$data_size_key="; read size < /dev/tty
		data_size_val=$(echo "$size" | bc)
		sed -i "s/^\($data_size_key=\).*/\1$data_size_val/" $file
		;;
	"Master")
		ip_master_key="YAMA_IP"
		printf "$ip_master_key="; read ip_master_val < /dev/tty
		sed -i "s/^\($ip_master_key=\).*/\1$ip_master_val/" $file
		;;
	"YAMA")
		ip_fs_key="FS_IP"
		printf "$ip_fs_key="; read ip_fs_val < /dev/tty
		sed -i "s/^\($ip_fs_key=\).*/\1$ip_fs_val/" $file
		;;
	esac
done
fi

LC_ALL=C
LANG=C
export LC_ALL LANG

echo "Ready to run."
