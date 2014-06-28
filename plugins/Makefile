include ../Makefile.common

CFLAGS = $(GTK_INCLUDE) $(GLIB_INCLUDE) $(NLS) -DLOCALEDIR=\"$(LOCALEDIR)\"
LDFLAGS = $(GTK_LIB) $(GLIB_LIB) -shared
CC = gcc -O2 -Wall -fPIC -DPIC

OBJS = for_each.so glob.so rename_ext.so copy_to_clipboard.so sort_by_ext.so \
 clone.so pack.so du.so view.so unpack.so

.SUFFIXES: .so .c

.c.so:
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

all: plugins

plugins: $(OBJS)

$(OBJS): ../emelfm.h ../Makefile.common

install: plugins
	install -d $(PLUGINS_DIR)
	for file in $(OBJS); do \
	  install -c -s -m 755 $$file $(PLUGINS_DIR); \
	done

clean:
	rm -f *.so
