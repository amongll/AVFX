package com.youku.cloud.vedit;

import java.io.File;
import android.view.Surface;

public class MltRuntime {
	MltRuntime(){}
	MltRuntime(File json_path) {
		createNative(json_path);
	}
	
	MltRuntime(String json_text) {
		createNative(json_text); 
	}
	
	public void close() {
		if (native_obj == 0)
			destroyNative();
	}
	
	public void finalize() {
		close();
	}
	
	public native String lastErrorNative();
	public native boolean addRuntimeEntry(String json_text, String json_path);
	public native boolean replaceRuntimeEntryUUID(String json_text, String replaced_uuid);
	public native boolean replaceRuntimeEntryJsonPath(String json_text, String replaced_jspath);
	public native boolean delRuntimeEntryUUID(String uuid);
	public native boolean delRuntimeEntryJsonPath(String json_path);
	public native boolean prepareNative();
	public native boolean runNative(Surface nativeWindow);
	public native boolean outputNative(File outpath);
	public native boolean seekNative(long frame_position);
	public native boolean setSpeedNative(double speed);
	public native long getFramePositionNative();
	public native long getFrameLengthNative();
	public native boolean runningNative();
	public native void stopNative();
	public native String getRuntimeEntryJsonPath(String uuid);
	
	public native boolean createNative(String json_text);
	public native boolean createNative(File json_path);
	public native void destroyNative();

	private long native_obj = 0;
}
