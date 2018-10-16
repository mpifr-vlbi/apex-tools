
echo "## dpkg --get-selections"  > wheezy-dpkg-selections.lst
dpkg --get-selections >> wheezy-dpkg-selections.lst

echo "## apt-mark showmanual" > wheezy-dpkg-manualinstalls.lst
apt-mark showmanual >> wheezy-dpkg-manualinstalls.lst

