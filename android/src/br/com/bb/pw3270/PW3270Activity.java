package br.com.bb.pw3270;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;
import android.widget.Button;
import android.widget.EditText;
import android.view.View;
import android.content.res.*;
import android.app.AlertDialog;
import android.app.Dialog;

public class PW3270Activity extends Activity implements View.OnClickListener
{
	private class terminal extends lib3270
	{
		TextView view;
		Activity Main;
	
		
		terminal(TextView view, Activity Main)
		{
			this.view = view;
			this.Main = Main;
			
		}

		protected void updateProgramMessage(int id)
		{
			 this.view.setText(message[id]);
		}

		public void popupMessage(int type, String title, String text, String info)
		{
			AlertDialog d = new AlertDialog.Builder(Main).create();
			d.setTitle(title);
			d.setMessage(text);

	
			/*
			d.setButton("OK", new DialogInterface.OnClickListener() 
			{
			      public void onClick(DialogInterface dialog, int which) 
			      {
			 
			       //here you can add functions
			 
			      }
			});
			*/			
			
			d.show();
//			this.view.setText(title + "\n" + text + "\n" + info);
		}

		
	};
	
	private terminal 	host;
	private EditText 	uri;
	private Resources	res;
	private String[] 	message;
	
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) 
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        res 	= getResources();
        message = res.getStringArray(R.array.program_msg);
        uri 	= (EditText) findViewById(R.id.hostname);
        
        // Set button
        Button btn = (Button) findViewById(R.id.connect);
        btn.setOnClickListener((View.OnClickListener) this);        
        
        host = new terminal((TextView) findViewById(R.id.text),this);
        
        
    }

    public void onClick(View v) 
    {
        // Perform action on click
    	// host.setHost(uri.getText().toString());
    	host.connect();
    }

}