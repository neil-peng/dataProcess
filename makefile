CC = g++

EXE = processImg

OBJ = main.o mmysql.o  mmc.o MyImage.o util.o adjust.o   specialPush.o typeProcess.o

INCLUDEDIR =-I ./ -I./lib/opencv/opencv/  -I./lib/lightConf/  -I./lib/mysql/include/ -I./lib/memcache/include/ 

CPPFLAGS =  -g -Wall -pipe -fPIC -O2 -std=c++0x -pthread -fpermissive -gdwarf-2 

LIB = -Xlinker "-(" ./lib/opencv/libcv.a  ./lib/opencv/libcxcore.a ./lib/opencv/libhighgui.a  ./lib/lightConf/libconf.a ./lib/mysql/lib/libdbug.a ./lib/mysql/lib/libheap.a  ./lib/mysql/lib/libmyisam.a  ./lib/mysql/lib/libmyisammrg.a  ./lib/mysql/lib/libmysqlclient.a  ./lib/mysql/lib/libmysqlclient_r.a  ./lib/mysql/lib/libmystrings.a  ./lib/mysql/lib/libmysys.a  ./lib/memcache/lib/libmemcached.a    ./lib/memcache/lib/libmemcachedutil.a    ./lib/memcache/lib/libhashkit.a     ./lib/memcache/lib/libmemcachedutil.a    -lpng12  -ljpeg -lz -lm -ltiff  -lcrypto  -lsasl2 -lcrypto  -lrt  -Xlinker "-)" 


${EXE} : ${OBJ}
	${CC} $(CPPFLAGS) -o $@ $^  $(LIB) -Wl,--rpath=./lib
%.o:%.cpp
	${CC} $(CPPFLAGS) -o $@ -c  $^  ${INCLUDEDIR}
clean:
	rm ${EXE} *.o
