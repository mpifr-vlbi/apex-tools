
source /usr/local/src/difx-281/setup_difx

# ./install-difx --reconf --ipp --withhops --withpolconvert --withpython  |& tee install-difx.log
./install-difx --noconf --ipp --withpolconvert --withpython  |& tee install-difx.log
