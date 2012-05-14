package br.com.bb.pw3270;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;

public class PW3270Activity extends Activity {
	
	lib3270 host;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        TextView text = (TextView) findViewById(R.id.text);
        Button btn = (Button) findViewById(R.id.connect);
        
      
        
        host = new lib3270();
        
        text.setText(host.getVersion());
    }
}