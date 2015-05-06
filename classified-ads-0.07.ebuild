# Copyright 1999-2015 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI=5
PLOCALES="en fi sv"
PLOCALE_BACKUP="en"
inherit qt4-r2

DESCRIPTION="Program for displaying classified advertisement items"
HOMEPAGE="http://katiska.org/classified-ads/"
SRC_URI="https://github.com/operatornormal/classified-ads/archive/${PV}.tar.gz https://github.com/operatornormal/classified-ads/blob/graphics/preprocessed.tar.gz?raw=true -> classified-ads-graphics-${PV}.tar.gz"

LICENSE="LGPL-2.1"
SLOT="0"
KEYWORDS="~amd64 ~arm ~x86"

IUSE="debug"

DEPEND="dev-libs/openssl
	dev-libs/qjson
	net-libs/libnatpmp
	net-libs/miniupnpc
	sys-apps/file
        dev-qt/qtgui:4[debug?]"
RDEPEND="${DEPEND}"

src_prepare() {
	# preprocessed graphics are unpacked into wrong directory
	# so lets move them into correct location:
	mv ../ui/turt-transparent-128x128.png ui/
	mv ../ui/turt558.png ui/
	# then just run qmake
        qt4-r2_src_prepare
}
