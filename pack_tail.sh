if [ -e $2 ];then
dd conv=sync bs=64K if=$1 of=$1".tmp"
cat $1".tmp" >> $2
rm $1".tmp"
else
dd conv=sync bs=64K if=$1 of=$2
fi
cat tailtmp >> $2
rm tailtmp
