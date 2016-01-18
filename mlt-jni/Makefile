.PHONY: all jni data clean
all: jni data

jni:
	ndk-build;

clean:
	ndk-build clean;
	rm -rf assets/mlt

data:
	bash ./gen_data_android.sh;



