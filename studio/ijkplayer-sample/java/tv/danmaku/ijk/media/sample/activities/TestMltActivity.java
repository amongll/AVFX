package tv.danmaku.ijk.media.sample.activities;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import tv.danmaku.ijk.media.sample.R;

import com.youku.cloud.vedit.*;

import java.io.File;
import java.util.Timer;
import java.util.TimerTask;

public class TestMltActivity extends AppCompatActivity {

    public static Intent newIntent(Context context) {
        Intent intent = new Intent(context, TestMltActivity.class);
        return intent;
    }
    public static void intentTo(Context context) {
        context.startActivity(newIntent(context));
    }

    Timer checkStatusTimer = null;
    String statusPerStr;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_test_mlt);

        String[] plugins = {"libmltcore.so", "libmltavformat.so"};
        Log.e("IJKMEDIA", "mltfactory init:" + MltFactory.init(plugins, "IJKMEDIA", Log.VERBOSE, this));

        TextView selectMedia = (TextView)findViewById(R.id.mediaSelectText);
        selectMedia.setTextIsSelectable(true);
        selectMedia.setOnLongClickListener(new View.OnLongClickListener() {
                                               @Override
                                               public boolean onLongClick(View v) {
                                                   FileExplorerActivity.intentTo(TestMltActivity.this, 1010);
                                                   return true;
                                               }
                                           }
        );

        progressView = (TextView)findViewById(R.id.processStatusTextView);
    }

    protected void onStop()
    {
        super.onStop();
        _stopTestAvFormat();
    }

    public void onClickStartTest(View view)
    {
        if (testPath == null) return;
        _startTestAvFormat(testMediaFile);
        checkStatusTimer = new Timer(true);

        final Runnable redraw = new Runnable() {
           @Override
            public void run() {
               statusPerStr = MltFactory._statusTestAvformat();
               char[] tt = statusPerStr.toCharArray();
               progressView.setText(tt,0,tt.length);
           }
        };

        checkStatusTimer.scheduleAtFixedRate(
                new TimerTask(){
                    public void run() {
                        runOnUiThread(redraw);
                    }
                },
                1000,
                1000
            );
    }

    public void onClickStopTest(View view)
    {

        _stopTestAvFormat();
    }

    private TextView progressView;
    private String testPath = null;
    private File testMediaFile;
    @Override
    protected void onActivityResult(int reqCode, int resCode, Intent data)
    {
        if ( reqCode == 1010) {
            if (resCode == Activity.RESULT_OK ) {
                TextView view = (TextView)findViewById(R.id.mediaSelectText);
                testPath = data.getStringExtra("path");
                char[] tmp = testPath.toCharArray();
                view.setText(testPath.toCharArray(), 0, tmp.length);
                testMediaFile = new File(testPath);
            }
        }
    }

    private Thread runThread;
    private Double progress;

    private void _startTestAvFormat(File media)
    {
        String mediaLog = "test_avformat_" + media.getName() + ".log";
        File logPath = new File(getDir("log",MODE_PRIVATE),mediaLog);
        //MltFactory._startTestAvformat(media.getAbsolutePath(), logPath.getAbsolutePath());
        MltFactory._startTestAvformat(media.getAbsolutePath(), null);
    }

    private void _stopTestAvFormat()
    {
        if (checkStatusTimer != null) {
            checkStatusTimer.purge();
            checkStatusTimer.cancel();
        }
        MltFactory._stopTestAvformat();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_test_mlt, menu);
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
