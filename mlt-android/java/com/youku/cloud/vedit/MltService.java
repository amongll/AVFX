package com.youku.cloud.vedit;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Created by L-F000000-PC on 2016/1/20.
 */
abstract public class MltService {
    private long implNative;
    private String serviceName;
    private String resource;
    private LinkedHashMap<String,String> properties;

    private long frameIn;
    private long frameOut;

    public int attachFilter(MltFilter obj)
    {
        //todo
        return 0;
    }

    protected void init()
    {

    }

    native protected long nativeInit(String service, String resource,  Map<String,String> propertes);
    native protected void nativeDestory();
}
