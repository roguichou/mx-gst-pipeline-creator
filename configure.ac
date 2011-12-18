m4_define([mx_gpc_major], [0])
m4_define([mx_gpc_minor], [0])
m4_define([mx_gpc_micro], [1])

m4_define([mx_gpc_version], [mx_gpc_major.mx_gpc_minor.mx_gpc_micro])

AC_INIT([mx_gpc], [mx_gpc_version])
AC_CONFIG_MACRO_DIR([build/autotools])

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CONFIG_HEADERS([config.h])

AC_ISC_POSIX
AC_HEADER_STDC
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
AM_PATH_GLIB_2_0
AC_LIBTOOL_WIN32_DLL
AC_PROG_CXX

MX_GPC_MAJOR_VERSION=mx_gpc_major
MX_GPC_MINOR_VERSION=mx_gpc_minor
MX_GPC_MICRO_VERSION=mx_gpc_micro
MX_GPC_VERSION=mx_gpc_version
AC_SUBST(MX_GPC_MAJOR_VERSION)
AC_SUBST(MX_GPC_MINOR_VERSION)
AC_SUBST(MX_GPC_MICRO_VERSION)
AC_SUBST(MX_GPC_VERSION)

AC_ARG_ENABLE([debug],
        [AC_HELP_STRING([--enable-debug=@<:@no/warning/log/debug@:>@],
            [Control mx_gpc debugging level @<:@default=no@:>@])],
        [],
        [enable_debug=no])

AS_CASE([$enable_debug],
        [debug],
        [ AC_DEFINE(ENABLE_DEBUG,3, [Debug level]) ],
	
        [log],
        [ AC_DEFINE(ENABLE_DEBUG,2, [Debug level]) ],
	
        [warning],
        [ AC_DEFINE(ENABLE_DEBUG,1, [Debug level]) ],

        [no],
        [ AC_DEFINE(ENABLE_DEBUG,0, [Debug level]) ],

        [AC_MSG_ERROR([Unknown argument for --enable-debug])]
       )

PKG_CHECK_MODULES(DEPS, "cogl-1.0 clutter-1.0 mx-1.0 gstreamer-0.10")

AM_PROG_LIBTOOL


AC_DEFINE_UNQUOTED(FL3D_PREFIX, "$prefix", [prefix])

AC_CONFIG_FILES([
        Makefile
        src/Makefile
        src/mx-graph/Makefile
])

AC_OUTPUT
echo "=======    MX_Gst Pipeline Creator   ======="
echo ""
echo "          debug : ${enable_debug}"
echo ""
echo "==========   CONFIGURE END   =========="

