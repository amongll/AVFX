ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES=libvedit_script.la

libvedit_script_la_LDFLAGS=-avoid-version
libvedit_script_la_SOURCES=\
	VEditCallable.cpp \
	VEditCallable.h \
	VEditCommon.cpp \
	VEditCommonDefine.h \
	VEditCommon.h \
	VEditCompilable.h \
	VEditDefinable.cpp \
	VEditDefinable.h \
	VEditEnumExpandable.cpp \
	VEditEnumExpandable.h \
	VEditError.h \
	VEditException.h \
	VEditExport.h \
	VEditFilterScript.h \
	VEditFilterScript.cpp \
	VEditMacroExpandable.cpp \
	VEditMacroExpandable.h \
	VEditMltRun.cpp \
	VEditMltRun.h \
	VEditMultiTrackScript.h \
	VEditMultiTrackScript.cpp \
	VEditPlaylistScript.h \
	VEditPlaylistScript.cpp \
	VEditProducerScript.cpp \
	VEditProducerScript.h \
	VEditScript.cpp \
	VEditScriptEnums.cpp \
	VEditScriptEnums.h \
	VEditScript.h \
	VEditScriptMacros.cpp \
	VEditScriptMacros.h \
	VEditScriptParams.cpp \
	VEditScriptParams.h \
	VEditScriptProps.cpp \
	VEditScriptProps.h \
	VEditTransitionScript.h \
	VEditTransitionScript.cpp \
	VEditUtil.cpp \
	VEditUtil.h \
	VEditValuable.cpp \
	VEditValuable.h \
	VEditVM.cpp \
	VEditVM.h
    
libvedit_script_la_CPPFLAGS=@jansson_CFLAGS@ @mlt_CFLAGS@
libvedit_script_la_CXXFLAGS=-Wno-deprecated
libvedit_script_la_LIBADD=@jansson_LIBS@ @mlt_LIBS@

