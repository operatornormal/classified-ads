Name:		classified_ads
Version:	0.04
Release:	1%{?dist}
Summary:	Classified ads is a program for posting ads online

Group:		Applications/Internet
License:	LGPL
URL:		https://github.com/operatornormal/classified_ads/releases/tag/0.04
Source0:	classified_ads-0.04.tar.gz

BuildRequires:	qt-devel >= 4
BuildRequires:	openssl-devel, libnatpmp-devel, qjson-devel, gcc-c++, miniupnpc-devel, file-devel
Requires:	qt >= 4
Requires:	expat, fontconfig, freetype,
Requires:	pcre, qjson
BuildRoot:	%{_tmppath}/%{name}-%{version}-build
%description
Classified ads is an attempt to re-produce parts of the functionality
that went away when Usenet news ceased to exist. This attempt tries to
fix the problem of disappearing news-servers so that there is no servers
required ; data storage is implemented inside client applications that
you and me are running.
%prep
%setup -q

%build
PREFIX="/tmp/foo"
export CXXFLAGS=-I/usr/include/miniupnpc
QMAKE_ARGS+="INCLUDEPATH+=${LOCALBASE}/include/miniupnpc/ LIBS+=-L${LOCALBASE}/lib"; qmake-qt4 PREFIX=$RPM_BUILD_ROOT
make

%install
INSTALL_ROOT=$RPM_BUILD_ROOT make install 

%clean
rm -rf $RPM_BUILD_ROOT


%files
%doc README.TXT
%{_bindir}/classified-ads
%{_datadir}/applications/classified_ads.desktop
%{_datadir}/app-install/icons/turt-transparent-128x128.png
%{_datarootdir}/classified-ads/classified_ads_fi.qm
%{_datarootdir}/classified-ads/classified_ads_sv.qm
%{_mandir}/man1/classified-ads.1.gz
%changelog
* Sat Mar 14 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.04-1
- License change GPL->LGPL due to OpenSSL license incompatibility.
- Minor UI changes as some bitmaps removed due to licensing issues
* Tue Feb 24 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.03-1
- Rpm build fixes for fedora linux
- Slower connection attempts to unreachable nodes
- Better file extenstion handling
- UI tweaks for small vertical screen resolutions, affects at least 
   users of some mac models
- Classifications entered by user now actually work too 
- If no local search avail, still offer network search
- Possible crash case fix. Still having random crashes inside
  json serialize..
- Links between documents, still does not work with profile comments
- Try miniupnpc also in unix build
- Basic support for trust-tree based on trust lists
- Translations
* Sun Jan 04 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.02-1
- Libnatpmp is not used in windows build,
- Profile comment document width setting was missing
- Made main window slightly smaller vertically.
- Now compiles also under Qt5
- Fix for grave bug: only message sender (not recipient) can read binary attachments
- Error messages for some file operations
- Fix for situation where published private profile breaks every node in network
- Require c++ compiler for compilation of c++, when building rpm.
* Wed Dec 31 2014 Tuomas Haarala <tuoppi@hieno.net> - 0.01-1
- initial spec file scribbled together
