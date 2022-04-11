project=autoinput
parser=cmdline
version=$(shell ./version.sh)
debrelease=20
#architecture=i386
#architecture=amd64
architecture=$(shell dpkg-architecture -qDEB_BUILD_ARCH)
config=xmlconfig

package=$(project)_$(version)-$(debrelease)_$(architecture).deb

CLIBS=-lpthread -lbluetooth # -lhid
CFLAGS=-Wall -g

debianbin=debian/usr/bin
debianproject=$(debianbin)/$(project)
# debian parts that can be erased and rebuilt
debianparts=$(debianproject) debian/DEBIAN/control debian/usr/share/doc/$(project)/changelog.gz
xml2cflags=`xml2-config --cflags`
xml2libs=`xml2-config --libs`

keypress=keypress
beep=beep
threads=threads
objects=$(project).o $(parser).o $(keypress).o $(beep).o $(threads).o
includes=$(parser).h

all: $(project) xmlconfig_test

$(parser).c: $(parser).ggo
	gengetopt < $< --file-name=$(parser) # --unamed-opts

$(parser).h: $(parser).ggo
	gengetopt < $< --file-name=$(parser) # --unamed-opts

$(parser).o: $(parser).c $(parser).h
	gcc $(CFLAGS) -c $(parser).c # -DHAVE_CONFIG_H

$(config).o: $(config).c
	gcc ${xml2cflags} -c $(config).c

$(config)_test: $(config).o
	gcc $(config).o $(xml2libs) -o $@

$(keypress).o: $(keypress).c $(keypress).h
	gcc $(CFLAGS) -c $(keypress).c

$(beep).o: $(beep).c $(beep).h
	gcc $(CFLAGS) -c $(beep).c

$(threads).o: $(threads).c $(threads).h
	gcc $(CFLAGS) -c $(threads).c

$(project).o: $(project).c $(includes)
	gcc $(CFLAGS) -c $(project).c

$(project): $(objects)
	gcc $(CFLAGS) $(objects) $(CLIBS) -o $@

$(debianproject): $(project)
	mkdir -p $(debianbin)
	strip $< -o $@
	chmod og+rx $@

debian/usr/share/doc/$(project)/changelog.gz: changelog
	gzip -9 < $< > $@
	chmod og+r $@

#debian/control: control cmdline.ggo
#	sed \
#	   -e "s/VERSION/$(version)-$(debrelease)/" \
#	   -e "s/ARCHITECTURE/$(architecture)/" \
#	   control > $@

debian/DEBIAN/control: control cmdline.ggo
	sed \
	   -e "s/VERSION/$(version)-$(debrelease)/" \
	   -e "s/ARCHITECTURE/$(architecture)/" \
	   control > $@
	chmod 0644 $@

$(package): $(debianparts)
	#find debian -type f -exec chmod 0644 '{}' \;
	#find debian -type d -exec chmod 0755 '{}' \;
	#find debian/etc/bluenear -type f -exec chmod a+x '{}' \;
	find debian -name "*~" -exec rm -f {} \;
	#find debian/usr/bin -type f -exec chmod +x '{}' \;
	find debian/usr/share/doc -type f -exec chmod 0644 '{}' \;
	fakeroot dpkg-deb --build debian $(package)

package: $(package)
	lintian $<

run: $(project)
	./$< --device /dev/rfcomm23

clean:
	rm -f $(project) $(objects) $(parser).c $(parser).h $(config).o $(config)_test $(package) $(debianparts) *~
