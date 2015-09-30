Compiling:
 You need 5, libnatmpm, openssl and miniupnpc . The debian control file is 
made for qt5 but program compiles with qt4 also, then add libmagick and
libqjson into dependencies.  
So, using your favourite package-manager
install 
 qtbase5-dev-tools
 libssl-dev 
 libnatpmp-dev
 libminiupnpc-dev
 dpkg-dev (if going to build with dpkg-buildpackage)
 libgcrypt11-dev (if going to build with dpkg-buildpackage, it checks for this)
 libqt5sql5-sqlite (is needed at runtime, not compile time)
 imagemagick (alternatively download 
 https://github.com/operatornormal/classified-ads/blob/graphics/preprocessed.tar.gz?raw=true 
 and unpack that into same directory with the sources and skip the 
 graphics-highres -part below)
Libgcrypt-dev or libgcrypt11-dev are needed for test suite, some bignum
tests are done against routines in this library but in normal runtime
there is no dependency. 

If compiling in linux or other unix-like system, you do the following:

- but skip this part for debian+derivatives (ubuntu,mint) if building
  with dpkg-buildpackage:
 cd  graphics-highres
 make
 cd ..

Then just say "qmake ; make install" and that should do the trick ; or
if inside debian-ecosystem "dpkg-buildpackage -r" in this directory should
produce a installable package for your environment ; this is the correct
way anyway. 

For compiling under ms-windows, see readme-file in sub-dir windows. 

License: LGPL. Happy hacking. 
