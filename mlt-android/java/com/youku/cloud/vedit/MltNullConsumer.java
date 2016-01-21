package com.youku.cloud.vedit;

import java.io.File;

/**
 * Created by li.lei on 2016/1/20.
 */
public class MltNullConsumer {
    private File frameInfoPath;

    public boolean connect(MltAVFProducer source)
    {
        return true;
    }

    public boolean run()
    {
        return true;
    }
}
