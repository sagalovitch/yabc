AC_INIT([yabc], [1.0], [ssagalov@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CC
AC_CONFIG_HEADERS([config.h])


# Check if readline support is requested
AC_MSG_CHECKING(if readline support is requested)
AC_ARG_WITH(readline,
	[  --with-readline         support readline input (yes by default)],
	[with_readline="$withval"], [with_readline="yes"])
AC_MSG_RESULT($with_readline)

# Is readline available?
if test $with_readline != "no"; then
	AC_CHECK_HEADER([readline/readline.h], 
		[AC_CHECK_LIB(readline, readline, [
				AC_DEFINE(HAVE_READLINE, [],
					[Is readline available?])
				LIBS="$LIBS -lreadline"
		], [])], [])
fi


AC_CONFIG_FILES([
 Makefile
 src/Makefile
])
AC_OUTPUT

