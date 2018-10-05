bin_PROGRAMS = inodeum
inodeum_SOURCES = \
		about.c             \
		cairo_chart.c       \
		cairo_chart.h       \
		cairo_util.c        \
		calendar_ui.c       \
		callbacks.c         \
		css.c               \
		date_util.c         \
		defs.h              \
		history.c           \
		isp.h               \
		main.h              \
		main_ui.c           \
		monitor.c           \
		overview.c          \
		prefs.c             \
		service.c           \
		services.h          \
		socket.c            \
		ssl_socket.c        \
		um_main.c           \
		user_login_ui.c     \
		utility.c           \
		version.h           \
		view_file_ui.c

inodeum_CFLAGS=$(GTK_CFLAGS) $(KEYR_CFLAGS) $(SSL_CFLAGS) $(CAIRO_CFLAGS) -Wno-deprecated-declarations
inodeum_LDADD=$(GTK_LIBS) $(KEYR_LIBS) $(SSL_LIBS) $(CAIRO_LIBS)
