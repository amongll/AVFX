noinst_PROGRAMS=test_video_script test_run_serialized

test_video_script_SOURCES=test_video_script.cpp
test_video_script_LDADD=../src/.libs/libvedit_script.la @jansson_LIBS@ @mlt_LIBS@
test_video_script_CPPFLAGS=-I$(top_srcdir)/src @jansson_CFLAGS@ @mlt_CFLAGS@
test_video_script_CXXFLAGS=-Wno-deprecated

test_run_serialized_SOURCES=test_run_serialized.cpp
test_run_serialized_LDADD=../src/.libs/libvedit_script.la @jansson_LIBS@ @mlt_LIBS@
test_run_serialized_CPPFLAGS=-I$(top_srcdir)/src @jansson_CFLAGS@ @mlt_CFLAGS@
test_run_serialized_CXXFLAGS=-Wno-deprecated


