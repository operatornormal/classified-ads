Here are briefly the steps for setting up dev env for windows:

1. download qt (v5.5) from http://download.qt.io/official_releases/qt/5.5/5.5.0/qt-opensource-windows-x86-mingw492-5.5.0.exe
2. install, also select tools/MinGW4.9.2 from "components" section
3. get openssl, instructions are in http://wiki.qt.io/Compiling_OpenSSL_with_MinGW
3.1 e.g. -> install msys, the 32-bit version from https://msys2.github.io/
3.2 get https://www.openssl.org/source/openssl-1.0.2d.tar.gz
3.3 get necessary goodies into msys, in msys-shell say
     $ pacmac -Sy
     $ pacman --needed -S bash pacman pacman-mirrors msys2-runtime
    restart the msys shell
     $ pacman --needed -S tar perl make
    add gcc from qt into path:
     $ export PATH=$PATH:/c/qt/Qt5.5.0/Tools/mingw492_32/bin
    in directory where openssl is extracted into, configure:
     $ ./Configure --prefix=$PWD -I$PWD/include no-idea no-mdc2 no-rc5 shared mingw
    and do not try using mingw32-make, it will resort to g++ and fail due to missing CFLAGS
     $ make
4. get miniupnpc from http://miniupnp.free.fr/files/download.php?file=miniupnpc-1.9.tar.gz
    compile by saying
     $ export PATH=$PATH:/c/qt/Qt5.5.0/Tools/mingw492_32/bin
     $ export PATH=.:$PATH
     $ make -f Makefile.mingw
5. get libiconv+gnu gettext, e.g. get 
     http://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.14.tar.gz
     http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.6.tar.xz
     and compile like this:
     $ tar xfvz libiconv-1.14.tar.gz
     $ cd libiconv-1.14
     $ ./configure --host=i586-pc-mingw32 --build=i586-pc-mingw32
     $ make
     $ make install
    again, make sure to use make, not mingw32-make
     $ xz -d < gettext-0.19.6.tar.xz | tar xfvo -
     $ cd gettext-0.19.6
     $ ./configure --host=i586-pc-mingw32 --build=i586-pc-mingw32
     $ make
     $ make install
    after this the libraries needed at runtime libiconv-2.dll and libintl-8.dll 
    are at \msys32\usr\local\bin\
6. Get classified ads, preferably from  https://github.com/operatornormal/classified-ads/archive/master.zip
    unzip, possibly adjust directories in file classified-ads.pro so that all libraries listed
    above are found from their paths. All library compilation was done inside msys shell
    but classified ads gets compiled inside Qt shell, so open qt-shell, navigate to directory
    and say
      qmake
      mingw32-make 
    the resulting binary goes either to sub-directory debug or release depending on your
    settings in .pro file. Before running you must copy following .dll files into same
    directory with classified-ads.exe:  
     libeay32.dll and  ssleay32.dll from openssl directory
     libiconv-2.dll and libintl-8.dll from \msys32\usr\local\bin\
     miniupnpc.dll from miniupnpc directory

Currently the classified-ads.pro is set up so that all libraries that
classified ads depends on are unpacked at same directory with classified
ads, like this:
C:\foobar>dir

 Kansio C:\foobar

24.09.2015  18:24    <DIRECTORY>       .
24.09.2015  18:24    <DIRECTORY>       ..
24.09.2015  16:07    <DIRECTORY>       classified_ads
21.09.2015  23:31    <DIRECTORY>       debugview
21.09.2015  21:31    <DIRECTORY>       deps
21.09.2015  22:24    <DIRECTORY>       gettext-0.19.6
21.09.2015  22:10    <DIRECTORY>       libiconv-1.14
15.09.2015  20:33    <DIRECTORY>       miniupnpc-1.9
15.09.2015  20:24    <DIRECTORY>       openssl-1.0.2d
22.09.2015  00:00             2 750 windows-setup.txt

and programs are compiled in place, gettext and libiconv are also installed
using the "make install" command so they're found from \msys32 path also. 

For producing the installation package, you'll need the nsis installer program. 
Before running the installer generation script, run batch file "deps.bat" in
windows sub-directory, it will copy some Qt dll's in place, they're needed
at run-time. 

Happy hacking. 
