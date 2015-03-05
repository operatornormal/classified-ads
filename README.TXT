Compiling:
 You need qt4 or 5, libnatmpm, openssl and qjson. The debian control file is 
made for qt4 but program compiles with qt5 also. If using qt5 make sure that
the libqjson in use is linked against qt5 and not qt4, in most linux
distributions shipping both qt4 and 5 the libqjson is linked against qt4
and will crash if used with qt5. So, using your favourite package-manager
install 
 libqt4-dev  
 libssl-dev 
 libnatpmp-dev
 libqjson-dev
 dpkg-dev (if going to build with dpkg-buildpackage)
 libgcrypt11-dev (if going to build with dpkg-buildpackage, it checks for this)
 libqt4-sql-sqlite (is needed at runtime, not compile time)
Libgcrypt-dev or libgcrypt11-dev are needed for test suite, some bignum
tests are done against routines in this library but in normal runtime
there is no dependency. 

Then just say "qmake-qt4 ; make install" and that should do the trick ; or
if inside debian-ecosystem "dpkg-buildpackage -r" in this directory should
produce a installable package for your environment ; this is the correct
way anyway. 

In windows program compiles in MinGW ; attempt with MSVC2010 resulted in
unresolvable symbols from moc-files that seems weird to me but the MinGW-build
is all right and it is possible to compile all required libs with MinGW. 
Qjson from  version control did not work, there is patch included in 
directory called patch to make it behave. 

License: LGPL. Happy hacking. 
