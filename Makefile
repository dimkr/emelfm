include Makefile.common

CFLAGS = $(GTK_INCLUDE) $(NLS) -DLOCALEDIR=\"$(LOCALEDIR)\" -DPLUGINS_DIR=\"$(PLUGINS_DIR)\" -DDOC_DIR=\"$(DOC_DIR)\"
LDFLAGS = $(GTK_LIB) $(GLIB_LIB)

OBJS = emelfm.o window.o filelist.o menu.o fileops.o permissions_dialog.o \
  config_files.o utils.o config_dialog.o filetype_dialog.o callbacks.o \
  filetype.o widget_utils.o add_ext_dialog.o init_filetype_dialog.o \
  file_info_dialog.o ownership_dialog.o view_dialog.o size_filter_dialog.o \
  filename_filter_dialog.o date_filter_dialog.o user_prompt.o fileview.o \
  user_command_dialog.o confirm_dialog.o toolbar_button_dialog.o \
  key_binding_dialog.o button_dialog.o filetype_action_dialog.o \
  open_query_dialog.o interface_callbacks.o list_utils.o \
  command_panel.o

.c.o:
	$(CC) $(CFLAGS) -c $<

all: emelfm nls

emelfm: $(OBJS)
	$(CC) $(OBJS) -o emelfm $(LDFLAGS)

static: $(OBJS)
	$(CC) $(OBJS) -o emelfm $(LDFLAGS) -static

$(OBJS): emelfm.h Makefile.common

nls: emelfm
	[ -z $(NLS) ] || (cd po; make)

install: emelfm nls
	install -c -s -m 755 emelfm $(BIN_DIR)/emelfm
	install -d $(DOC_DIR)
	for file in `ls docs/`; do \
		install -c -m 644 docs/$$file $(DOC_DIR); \
	done
	[ -z $(NLS) ] || LOCALEDIR=$(LOCALEDIR) cd po; make install

uninstall:
	rm -f $(BIN_DIR)/emelfm
	rm -rf $(PREFIX)/share/emelfm
	[ -z $(NLS) ] || LOCALEDIR=$(LOCALEDIR) cd po; make uninstall

clean:
	rm -f core tags *.o .*swp *.out emelfm
	cd po; make clean

