package com.youku.cloud.vedit;

import android.app.Application;
import android.content.Context;
import android.content.res.AssetManager;
import android.view.Surface;
import android.view.SurfaceView;

import java.io.File;

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

	public static String getMltConsumer() {
		return MLT_CONSUMER;
	}

	public static void cleanup()
	{
		close();
	}

	private static native boolean init(String[] plugins,String logTag, int logLevel,String filesRoot,AssetManager mgr);
	private static native void close();
	//protected static native boolean regist_surface(Surface sur, String id);
	//protected static native boolean detach_surface(Surface sur, String id);

	public static native void _initTestAvformat(String mediaFile, String infoPath, String consumerId);
	public static native void _startTestAvformat();
	public static native void _stopTestAvformat();
	public static native String _statusTestAvformat();
	public static native void _setTestSurface(Surface surface);
}
