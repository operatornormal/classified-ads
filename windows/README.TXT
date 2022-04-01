Here are briefly the steps for setting up dev env for windows:

1. download qt (v5.5) from http://download.qt.io/official_releases/qt/5.5/5.5.0/qt-opensource-windows-x86-mingw492-5.5.0.exe
2. install, also select tools/MinGW4.9.2 from "components" section
3. get openssl, instructions are in http://wiki.qt.io/Compiling_OpenSSL_with_MinGW
3.1 e.g. -> install msys, the 32-bit version from https://msys2.github.io/
3.2 get https://www.openssl.org/source/openssl-1.0.2o.tar.gz
3.3 get necessary goodies into msys, in msys-shell say
     $ sed -ie 's/SigLevel    = Required DatabaseOptional/SigLevel    = Never/g' /etc/pacman.conf
     # And the command above is really bad, it disables gpg signature
     # checking of pacman packages so you can't really trust what
     # you install via pacman next. If someone can show me how to
     # include gpg key 5F944B027F7FE2091985AA2EFA11531AA0AA7F57 into
     # trust chain of pacman then please do so, the key is available
     # from https://pgp.surfnet.nl/pks/lookup?op=get&search=0xFA11531AA0AA7F57
     $ pacmac -Sy
     $ pacman --needed -S bash pacman pacman-mirrors msys2-runtime
    restart the msys shell
     $ pacman --needed -S tar perl make
    add gcc from qt into path:
     $ export PATH=/d/Qt5.11/Tools/mingw530_32/bin:$PATH
    in directory where openssl is extracted into, configure:
     $ ./Configure --prefix=$PWD -I$PWD/include no-idea no-mdc2 no-rc5 shared mingw
    and do not try using mingw32-make, it will resort to g++ and fail due to missing CFLAGS
     $ make depend
     $ make
     $ make install
4. get miniupnpc from http://miniupnp.free.fr/files/download.php?file=miniupnpc-2.1.tar.gz
    compile by saying
     $ export PATH=/d/Qt5.11/Tools/mingw530_32/bin:$PATH
     $ export PATH=.:$PATH
     $ make -f Makefile.mingw CC=gcc
     $ ./wingenminiupnpcstrings.exe  miniupnpcstrings.h.in miniupnpcstrings.h
     $ make -f Makefile.mingw CC=gcc

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
5.1 get lib opus, e.g. get
  http://downloads.xiph.org/releases/opus/opus-1.2.tar.gz
  and un-tar it into some directory, I used one directory up from 
  classified ads source code and the .pro -file is now set up like this. 
  Then using same method as with openssl e.g. from MSYS shell
      $ export PATH=/d/Qt5.11/Tools/mingw530_32/bin:$PATH
      $ sh configure --prefix=`pwd`/binary
      $ make
      $ make install
5.2 get tcl/tk, e.g. get
  tcl8.6.7-src.tar.gz from https://sourceforge.net/projects/tcl/files/Tcl/8.6.7/ 
  and
  http://prdownloads.sourceforge.net/tcl/tk8.6.6-src.tar.gz
  and see instructions at http://wiki.tcl.tk/4424 about compiling. 
  In practice un-tar the sources into directory and say in MSYS shell:
  $ gzip -d < deps/tcl8.6.6-src.tar.gz | tar xfvo -
  $ gzip -d < deps/tk8.6.6-src.tar.gz | tar xfvo -
  $ mkdir tclbuild
  $ cd tclbuild/
  $ export PATH=/d/Qt5.11/Tools/mingw530_32/bin:$PATH
  $ export PATH=.:$PATH
  $ ../tcl8.6.7/win/configure --prefix=/opt/tcl
  $ make
  $ make install
  $ mkdir tk
  $ cd tk
  $ ../../tk8.6.7/win/configure --prefix=/opt/tcl --with-tcl=..
  $ make 
  $ make install 
After classified ads is compiled you need to copy 
C:\msys32\opt\tcl\bin\tcl86.dll
C:\msys32\opt\tcl\bin\tk86.dll
C:\msys32\opt\tcl\bin\zlib1.dll
into same directory with classified-ads.exe. If classified-ads.exe does not
exists yet (see last steps of this README.TXT) then simply create directories
"debug" and "release" under directory where classified ads source
code will be unpacked ; build process will produce the binary into
those directories, depending on debug/release setting. Copy TCL and Tk runtime 
files into path relative to classified-ads.exe ; Tcl library needs to be
found from sub-directory tcl8.6 and tk library (usually found from 
c:\msys32\opt\tcl\lib\tk8.6) needs to be copied to directory tk8.6
in same directory with classified-ads.exe. Inside tk there is file 
pkgIndex.tcl that contains location of tk86.dll and it is now wrong. 
Edit the line 
    package ifneeded Tk 8.6.6 [list load [file normalize [file join $dir .. .. bin tk86.dll]] Tk]
so be
    package ifneeded Tk 8.6.6 [list load [file normalize [file join $dir .. tk86.dll]] Tk]
e.g. remove the ".. bin" and TCL will now look for tk86.dll from same
directory where classified-ads.exe is is. Classified ads will try to adjust
TCL library path (see file main.cpp) at startup but it will not overwrite
existing TCL library path ; if there is existing TCL installation that
sets TCL_LIBRARY or TCLLIBPATH it may or may not interfere with TCL
installation bundled classified ads. If Classified ads starts using
wrong TCL runtime, it may be necessary to unset TCL_LIBRARY and TCLLIBPATH 
first. Also copy from tcl/lib directory contents of directory "tcl8" 
into same directory where directories tcl8.6 and tcl8.6 reside, same
directory where classified-ads.exe is found. 

6. Get classified ads, preferably from  https://github.com/operatornormal/classified-ads/archive/master.zip
    unzip, possibly adjust directories in file classified-ads.pro so that all libraries listed
    above are found from their paths. If you don't want to install
    ImageMagic the get preprocessed graphics by entering same directory
    where classified-ads.pro is located after unzipping and in there
    say
    $ wget https://github.com/operatornormal/classified-ads/raw/graphics/preprocessed.tar.gz
    $ gzip -d < preprocessed.tar.gz | tar xfvo - 
    All library compilation was done inside msys shell  but classified ads
    gets compiled inside Qt shell, so open qt-shell, navigate to directory
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
04.03.2016  16:43    <DIRECTORY>       opus-1.1.2
22.09.2015  00:00             2 750 windows-setup.txt

and programs are compiled in place, gettext and libiconv are also installed
using the "make install" command so they're found from \msys32 path also. 

For producing the installation package, you'll need the nsis installer program. 
Before running the installer generation script, run batch file "deps.bat" in
windows sub-directory, it will copy some Qt dll's in place, they're needed
at run-time. 

Happy hacking. 
