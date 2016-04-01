
PREFIX=/usr/local
CFLAGS+=-DAVDATADIR=\"/usr/local/share/ffmpeg/\"
CFLAGS+=-I/usr/local/include  
LDFLAGS+=-L/usr/local/lib -lavcodec  
CFLAGS+=-I/usr/local/include  
LDFLAGS+=-L/usr/local/lib -lavutil  
CFLAGS+=-I/usr/local/include  
LDFLAGS+=-L/usr/local/lib -lavformat  
CFLAGS+=-I/usr/local/include  
LDFLAGS+=-L/usr/local/lib -lswscale  
CFLAGS+=-I/usr/local/include  
LDFLAGS+=-L/usr/local/lib -lavdevice  
EXTRA_LIBS=
CODECS=1
FILTERS=1
DEVICES=1
