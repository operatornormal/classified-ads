Compiling:
 You need qt4 or 5, libnatmpm, openssl and qjson. The debian control file is 
made for qt4 but program compiles with qt5 also. If using qt5 internal
json routines of Qt are used so libqjson that is listed in
dependencies is actually not used if qt5 is selected. 
So, using your favourite package-manager
install 
 libqt4-dev  
 libssl-dev 
 libmagic-dev
 libnatpmp-dev
 libminiupnpc-dev
 libqjson-dev
 dpkg-dev (if going to build with dpkg-buildpackage)
 libgcrypt11-dev (if going to build with dpkg-buildpackage, it checks for this)
 libqt4-sql-sqlite (is needed at runtime, not compile time)
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

Then just say "qmake-qt4 ; make install" and that should do the trick ; or
if inside debian-ecosystem "dpkg-buildpackage -r" in this directory should
produce a installable package for your environment ; this is the correct
way anyway. 

In windows program compiles in MinGW ; attempt with MSVC2010 resulted in
unresolvable symbols from moc-files that seems weird to me but the MinGW-build
is all right and it is possible to compile all required libs with MinGW. 
Qjson from  version control did not work, there is patch included in 
directory called patch to make it behave. In windows you can do the
graphics-highres part if you have imagemagick. For windows build you
naturally have the same dependencies as with unix build. For openssl
build instructions follow the build instructions that came with
openssl, you'll want to do the msys-based build, using same version 
of MinGW that you use with Qt compile process. Resulting static
libraries should be in c:\msys\1.0\local\lib\. For miniupnpc the 
path that should lead to library is ..\miniupnpc\miniupnpc-1.9\miniupnpc.lib
and that compiles under mingw. Libnatpmp is not used win32 build because 
development team was unable to produce a working version of natpmp. 
Build process for win32 has been tried only using Qt 5.4 so qjson is not 
dependency there. If these paths do not suit your configuration, see 
classified-ads.pro file, remember to modify both include paths and 
library paths.  ; for windows
the normal Qt compile 
qmake
mingw32-make -j 4
should produce a binary. Before usage it was required to copy miniupnpc.dll
from miniupnpc build directory into same directory with the classified
ads executable. If you have intention of making the windows installer
too (nsis file in windows-subdirectory) then run the deps.bat before
launching the nsis installer.

License: LGPL. Happy hacking. 
