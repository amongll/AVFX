package com.youku.cloud.vedit;

import java.io.File;
import android.content.res.AssetManager;

public class MltTemplate {
	static final String VideoType="video";
	static final String AudioType="audio";
	static final String ImageType="image";
	static final String GifType="gif";
	static final String PlaylistType="playlist";
	static final String MultitrackType="multitrack";
	static final String FilterType="filter";
	static final String TransitionType="transition";
	static final String MltAsisProducerType="asis_producer";
	
	static final String Profile480PLandscape="android_480p_25fps_16:9";
	static final String Profile560PLandscape="android_560p_25fps_16:9";
	static final String Profile640PLandscape="android_640p_25fps_16:9";
	static final String Profile720PLandscape="android_720p_25fps_16:9";
	static final String Profile1080PLandscape="android_1080p_25fps_16:9";
	
	static boolean default_init()
	{
		initNative(Profile480PLandscape);
	}
	
	static native boolean initNative(String profile_name,String log_tag, int log_level);
	static native boolean enginePrepare(AssetManager assets_mgr, String files_root, String[] plugins);
	static native String lastErrorNative();
	static native boolean loadFileDir(File template_dir);
	static native boolean loadAssetDir(AssetManager assmgr, String asset_dir);
	static native boolean loadFile(File template_file);
	static native String[] expand(String template_name, String args_json);
	static native String[] expandType(String template_name, String type_checked, String args_json);
}
