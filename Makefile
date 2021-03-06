# This software is distributed under the following license:
# http://host-sflow.sourceforge.net/license.html

# note - shell invocation with `` quotes is portable
# between GNU and BSD make

PROG=hsflowd
MY_RPM_TOP=/tmp/HSFLOWD_RPM_TOP
MY_RPM_BUILDROOT=/tmp/HSFLOWD_RPM_BUILD_ROOT

all: $(PROG)

$(PROG):
	cd src/sflow; $(MAKE)
	cd src/json; $(MAKE)
	PLATFORM=`uname`; \
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
        cd src/$$PLATFORM; $(MAKE) VERSION=$$MYVER RELEASE=$$MYREL

clean:
	cd src/sflow; $(MAKE) clean
	cd src/json; $(MAKE) clean
	PLATFORM=`uname`; \
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
        cd src/$$PLATFORM; $(MAKE) VERSION=$$MYVER RELEASE=$$MYREL clean

install:
	PLATFORM=`uname`; \
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
        cd src/$$PLATFORM; $(MAKE) VERSION=$$MYVER RELEASE=$$MYREL install

schedule:
	PLATFORM=`uname`; \
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
        cd src/$$PLATFORM; $(MAKE) VERSION=$$MYVER RELEASE=$$MYREL schedule

dist:
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
	MYTARBALL=$(PROG)-$$MYVER.tar.gz; \
	git archive HEAD --prefix=$(PROG)-$$MYVER/ | gzip >$$MYTARBALL;

rpm:
	MYARCH=`uname -m`; \
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
	MYTARBALL=$(PROG)-$$MYVER.tar.gz; \
	git archive HEAD --prefix=$(PROG)-$$MYVER/ | gzip >$$MYTARBALL; \
	MYSRCDIR=$(MY_RPM_TOP)/SOURCES; \
	rm -rf $$MYSRCDIR; \
	mkdir -p $$MYSRCDIR; \
	cp $$MYTARBALL $$MYSRCDIR; \
	rpmbuild --define "_topdir $(MY_RPM_TOP)" --buildroot=$(MY_RPM_BUILDROOT) -ba $(PROG).spec; \
	echo "==============="; \
	MYRPM="$(MY_RPM_TOP)/RPMS/$$MYARCH/$(PROG)-$$MYVER-$$MYREL.$$MYARCH.rpm"; \
	MYSRPM="$(MY_RPM_TOP)/SRPMS/$(PROG)-$$MYVER-$$MYREL.src.rpm"; \
	echo "copying new RPMs $$MYRPM and $$MYSRPM back to current directory"; \
	cp $$MYRPM $$MYSRPM .

aixrpm:
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
	SOURCES=/opt/freeware/src/packages/SOURCES; \
	MYSRCDIR=$$SOURCES/$(PROG)-$$MYVER; \
	rm -rf $$MYSRCDIR; \
	cp -r . $$MYSRCDIR; \
	tar cf $$MYSRCDIR.tar -C $$SOURCES $(PROG)-$$MYVER; \
        gzip -f $$MYSRCDIR.tar; \
	rpm -ba $(PROG).spec.aix

pkg:
	PLATFORM=`uname`; \
	MYVER=`./getVersion`; \
	MYREL=`./getRelease`; \
	MYSRCDIR=src/$$PLATFORM/scripts/$(PROG)-$$MYVER; \
	rm -rf $$MYSRCDIR; \
	mkdir -p $$MYSRCDIR; \
	mkdir -p $$MYSRCDIR/etc; cp src/$$PLATFORM/scripts/hsflowd.conf $$MYSRCDIR/etc; \
	mkdir -p $$MYSRCDIR/usr/sbin; cp src/$$PLATFORM/hsflowd $$MYSRCDIR/usr/sbin; \
	mkdir -p $$MYSRCDIR/lib/svc/method; cp src/$$PLATFORM/scripts/svc-hsflowd $$MYSRCDIR/lib/svc/method; \
	mkdir -p $$MYSRCDIR/var/svc/manifest/site; cp src/$$PLATFORM/scripts/hsflowd.xml $$MYSRCDIR/var/svc/manifest/site; \
	pkgmk -o -r $$MYSRCDIR -f src/$$PLATFORM/scripts/Prototype ; \
	pkgtrans /var/spool/pkg /tmp/$(PROG)-$$MYVER-$$MYREL hsflowd; \
	mv /tmp/$(PROG)-$$MYVER-$$MYREL .

deb: $(PROG)
	MYARCH=`uname -m|sed 's/x86_64/amd64/'`; \
	MYVER=`./getVersion`; \
        MYREL=`./getRelease`; \
	mkdir -p debian/DEBIAN; \
        mkdir -p debian/usr/sbin; \
	mkdir -p debian/etc/init.d; \
	install DEBIAN_build/control debian/DEBIAN; \
	sed -i -e s/_PACKAGE_/$(PROG)/g debian/DEBIAN/control; \
	sed -i -e s/_VERSION_/$${MYVER}-$${MYREL}/g debian/DEBIAN/control; \
	chmod 644 debian/DEBIAN/control; \
	install -m 555 DEBIAN_build/conffiles debian/DEBIAN; \
	install -m 555 DEBIAN_build/preinst debian/DEBIAN; \
	install -m 555 DEBIAN_build/postinst debian/DEBIAN; \
	install -m 555 DEBIAN_build/prerm debian/DEBIAN; \
	install -m 700 src/Linux/hsflowd debian/usr/sbin; \
	install -m 755 src/Linux/scripts/hsflowd.deb debian/etc/init.d/hsflowd; \
	test -e src/Linux/sflowovsd && install -m 700 src/Linux/sflowovsd debian/usr/sbin; \
	test -e src/Linux/sflowovsd && install -m 755 src/Linux/scripts/sflowovsd.deb debian/etc/init.d/sflowovsd; \
	install -m 644 src/Linux/scripts/hsflowd.conf debian/etc; \
        cd debian; \
	find . -type d | xargs chmod 755; \
        md5sum `find usr etc -type f` > DEBIAN/md5sums; \
        cd ..; \
	dpkg-deb --build debian hsflowd_$${MYVER}-$${MYREL}_$$MYARCH.deb

xenserver: rpm
	cd xenserver-ddk; $(MAKE) clean; $(MAKE)

.PHONY: $(PROG) clean install schedule rpm xenserver

