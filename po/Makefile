#
# Classified Ads is Copyright (c) Antti Järvinen 2013.
#
# This file is part of Classified Ads.
#
# Classified Ads is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# Classified Ads is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with Classified Ads; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

# If adding a new translation, list it here. Also see the list 
# in "install" target.
translations = fi.mo sv.mo da.mo

.SUFFIXES: .mo .po

CLEAN_CMD ?= rm -f

all:	$(translations)

.po.mo:
	$(MSGFMT_PATH)msgfmt $*.po -o $*.mo

clean:
	$(CLEAN_CMD) $(translations)

install:	all
	install -m 644 -D -T fi.mo \
	$(DESTDIR)/usr/share/locale/fi/LC_MESSAGES/classified-ads.mo
	install -m 644 -D -T sv.mo \
	$(DESTDIR)/usr/share/locale/sv/LC_MESSAGES/classified-ads.mo
	install -m 644 -D -T da.mo \
	$(DESTDIR)/usr/share/locale/da/LC_MESSAGES/classified-ads.mo
