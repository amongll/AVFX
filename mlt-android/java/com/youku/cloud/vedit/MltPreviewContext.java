package com.youku.cloud.vedit;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by li.lei on 2016/1/27.
 */
public class MltPreviewContext implements SurfaceHolder.Callback
{
    private Object audio_renderer;
    private Surface video_renderer;
    private long gen;
    private long seq;
    private String surface_id;
    private String usage_id;

    public MltPreviewContext(SurfaceView view, String usage)
    {
        SurfaceHolder holder = view.getHolder();
        surface_id = usage + "-" + Integer.toString(view.getId());
        holder.addCallback(this);
        usage_id = usage;
    }

    public void surfaceCreated(SurfaceHolder holder)
    {
        video_renderer = holder.getSurface();
        seq = 0;
        //MltFactory.regist_surface(video_renderer, surface_id);
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int width,
                   int height)
    {
        seq += 1;
    }

    public void surfaceDestroyed(SurfaceHolder holder)
    {
        //MltFactory.detach_surface(video_renderer, surface_id);
        video_renderer = null;
    }
}
