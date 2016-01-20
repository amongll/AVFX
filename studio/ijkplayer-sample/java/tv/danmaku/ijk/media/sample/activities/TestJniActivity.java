package tv.danmaku.ijk.media.sample.activities;

import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

import org.lilei.testjni;
import com.youku.cloud.vedit.*;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import tv.danmaku.ijk.media.sample.R;

public class TestJniActivity extends AppCompatActivity {

    public static Intent newIntent(Context context) {
        Intent intent = new Intent(context, TestJniActivity.class);
        return intent;
    }
    public static void intentTo(Context context) {
        context.startActivity(newIntent(context));
    }

    private static void traversedir(String path) {
        File check = new File(path);

        //if ( check.exists() == true ) {
            if (check.isDirectory() == false) {
                Log.e("IJKMEDIA","entry: "+ check.getAbsolutePath() + " read:"+check.canRead() + " exec:"+check.canExecute()
                        + " write:"+check.canWrite());
                return;
            }
            else  {
                Log.e("IJKMEDIA", "Dir: " + check.getAbsolutePath() + " read:"+check.canRead() + " exec:"+check.canExecute()
                        + " write:"+check.canWrite());
                if (check.canRead() == true) {
                    String[] entries = check.list();
                    for(String e : entries) {
                        traversedir(path+ "/" +  e);
                    }
                }
                else {
                    if ( true == check.setReadable(true) ) {
                        String[] entries = check.list();
                        for(String e : entries) {
                            traversedir(path+ "/" + e);
                        }
                    }
                }
            }
        //}
        //else  {
        //    Log.e("IJKMEDIA","dir:"+check.getAbsolutePath()+" not exists");
       // }
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_jni);

        testjni obj= new testjni();
        String call1ret = obj.call1();
        String call2ret = obj.call2();
        String call3ret = obj.call_dlopened_module2();
        String pwd = obj.getPwd();

        String filsDir = getFilesDir().getAbsolutePath();
        Log.e("IJKMEDIA", "filesRoot:" + filsDir);

        File checkdir = new File(filsDir);
        String[] filsEs = checkdir.list();
        for ( String f : filsEs ) {
            Log.e("IJKMEDIA", "files entry:" + f);
        }

        File myDir = new File(filsDir + "/mylib/mlt" );
        if (myDir.exists() == false) {
            myDir.mkdirs();
        }
        else if ( false == myDir.isDirectory() ) {
            Log.e("IJKMEDIA", "dir is not directory:"+myDir.getAbsolutePath());
            myDir = null;
        }

        File assetDirTry = getDir("", Context.MODE_PRIVATE);
        if ( assetDirTry.exists() == true ) {
            if (assetDirTry.canRead() ) {
                String[] assetTries = assetDirTry.list();
                for (String e : assetTries) {
                    Log.e("IJKMEDIA", "filesEntry:" + e);
                }
            }
            else {
                Log.e("IJKMEDIA","Dir:"+assetDirTry.getAbsolutePath() + " not readable");
            }
        }
        else {
            Log.e("IJKMEDIA","Dir:"+assetDirTry.getAbsolutePath() + " do not exists");
        }

        AssetManager assMgr = getAssets();
        try {
            String[] assEntries = assMgr.list("");
            for (String assEntry : assEntries) {
                Log.e("IJKMEDIA", "assEntry:" + assEntry);
            }

            InputStream inStream = assMgr.open("test.txt",AssetManager.ACCESS_RANDOM);
            if (myDir != null) {
                File copyPath = new File(myDir,"test.txt");
                OutputStream o = new FileOutputStream(copyPath);

                byte[] copyBuf = new byte[1024];
                while( true) {
                    int rb = inStream.read(copyBuf);
                    if ( rb == -1 ) {
                        o.flush();
                        break;
                    }
                    else {
                        o.write(copyBuf);
                    }
                }
                o.close();
                obj.catfile(copyPath.getAbsolutePath());
            }
            inStream.close();
        }
        catch(IOException e) {
            e.printStackTrace();
        }

        filsDir = filsDir.substring(0, filsDir.length() - "files".length());
        traversedir(filsDir);

        /*
        checkdir = new File(filsDir);
        Log.e("IJKMEDIA", "files dir: "+filsDir+" exists:"+ checkdir.exists() + "  dir:"+ checkdir.isDirectory() +
                "  read:"+checkdir.canRead() + " exec:"+checkdir.canExecute());
        if (checkdir.exists()) {
            filsEs = checkdir.list();
            if (filsEs != null) {
                for ( String f : filsEs ) {
                    Log.e("IJKMEDIA", "lib entry:" + f);
                }
            }
        }

        checkdir = new File(filsDir + "lib");
        Log.e("IJKMEDIA", "lib dir "+filsDir + "lib" +" exists:"+ checkdir.exists() + "  dir:"+ checkdir.isDirectory() +
                "  read:"+checkdir.canRead() + " exec:"+checkdir.canExecute());
        if (checkdir.exists()) {
            filsEs = checkdir.list();
            if (filsEs != null) {
                for ( String f : filsEs ) {
                    Log.e("IJKMEDIA", "lib entry:" + f);
                }
            }
        }

        checkdir = new File(filsDir + "lib64");
        Log.e("IJKMEDIA", "lib dir "+filsDir + "lib64" +" exists:"+ checkdir.exists() + "  dir:"+ checkdir.isDirectory() +
                "  read:"+checkdir.canRead() + " exec:"+checkdir.canExecute());
        if (checkdir.exists()) {
            filsEs = checkdir.list();
            if (filsEs != null) {
                for ( String f : filsEs ) {
                    Log.e("IJKMEDIA", "lib entry:" + f);
                }
            }
        }*/
     }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_test_jni, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }
}
