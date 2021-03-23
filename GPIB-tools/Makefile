
all:
	make -C src all
	make -C src install

clean:
	make -C src clean

install:
	make -C src install

tar: clean
	rm -f apex_gpib_tools_*.tar.gz
	tar czf apex_gpib_tools_v104.tar.gz *
