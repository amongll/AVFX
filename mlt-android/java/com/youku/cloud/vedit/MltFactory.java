package com.youku.cloud.vedit;

import android.app.Application;
import android.content.Context;
import android.content.res.AssetManager;

public class MltFactory {
	static {
		System.loadLibrary("mlt_jni");
	}
	public static final String MLT_CONSUMER="android_consumer";
	public static final String MLT_PROFILE="android_profile";

	public static boolean init(String[] plugins, String logTag, int logLevel,Context context)
	{
		AssetManager mgr = context.getAssets();
		String filesRoot = context.getFilesDir().getAbsolutePath();
		filesRoot = filesRoot.substring(0, filesRoot.length() - "files".length());

		return init(plugins, logTag, logLevel, filesRoot, mgr);
	}

	public static native boolean init(String[] plugins,String logTag, int logLevel,String filesRoot,AssetManager mgr);
	public static native void close();
}
