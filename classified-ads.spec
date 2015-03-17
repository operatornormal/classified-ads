Name:		classified-ads
Version:	0.04
Release:	2%{?dist}
Summary:	Classified ads is a program for posting ads online

Group:		Applications/Internet
License:	LGPLv2
URL:		http://katiska.org/classified_ads/
Source0:	https://github.com/operatornormal/%{name}/archive/%{version}.tar.gz#/%{name}-%{version}.tar.gz
BuildRequires:	qt-devel >= 4
BuildRequires:	openssl-devel, libnatpmp-devel, qjson-devel, gcc-c++, miniupnpc-devel, file-devel, libappstream-glib
%description
Classified ads is an attempt to re-produce parts of the functionality
that went away when Usenet news ceased to exist. This attempt tries to
fix the problem of disappearing news-servers so that there is no servers
required and no service providers needed; data storage is implemented
inside client applications that you and me are running.
%prep
%setup -q

%build
qmake-qt4 
make

%install
INSTALL_ROOT=$RPM_BUILD_ROOT make install 
appstream-util validate-relax --nonet %{buildroot}/%{_datadir}/appdata/classified-ads.appdata.xml
%files
%doc README.TXT
%{_bindir}/classified-ads
%{_datadir}/applications/classified-ads.desktop
%{_datadir}/app-install/icons/turt-transparent-128x128.png
%{_datarootdir}/classified-ads/classified_ads_fi.qm
%{_datarootdir}/classified-ads/classified_ads_sv.qm
%{_mandir}/man1/classified-ads.1.gz
%{_datadir}/appdata/classified-ads.appdata.xml
%license LICENSE
%changelog
* Tue Mar 17 2015 Antti Jarvinen <classified-ads.questions@katiska.org> - 0.04-2
- Changed packaging to happen in more civilized way.
  Lot of changes into spec file. 
- Package name has changed classified_ads -> classified-ads.
- Added appdata, re-wrote the small manpage in less personal tone. 
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
