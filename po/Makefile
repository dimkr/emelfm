include ../Makefile.common

all:
	for i in `ls *.po`; do \
		msgfmt -o `echo $$i | sed -e s/.po//`.gmo $$i; \
	done

install:
	for i in `ls *.gmo` ; do \
		mkdir -p $(LOCALEDIR)/`echo $$i|sed -e s/.gmo//`/LC_MESSAGES;\
		install -c -m644 $$i $(LOCALEDIR)/`echo $$i | sed -e s/.gmo//`/LC_MESSAGES/emelfm.mo ; \
	done

uninstall:
	for i in `ls *.po` ; do \
		rm -f $(LOCALEDIR)/`echo $$i | sed -e s/.po//`/LC_MESSAGES/emelfm.mo || test -z "" ; \
	done

clean:
	rm -f *~ *.gmo messages
