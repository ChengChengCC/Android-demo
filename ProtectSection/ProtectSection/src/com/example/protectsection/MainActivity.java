package com.example.protectsection;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends Activity {


	TextView tv = null;
	Button btn = null;
	
	private native String getString();
	private native void  initSo();
	static{
		try {
			
			System.loadLibrary("protect_section");
		} catch (Exception e) {
			// TODO: handle exception
			Log.d("CCDebug", "load lib exception");
		}
		
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		tv = (TextView)findViewById(R.id.tv_show);
		btn =(Button)findViewById(R.id.btn_load);
		btn.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				//loadLibrary("protect_section");
				//tv.setText(getString());
			//	initSo();
				
				getString();
			}
		});
		 
	}
	
	
	public static void loadLibrary(String libName) {
		System.loadLibrary(libName);
		
	}
}
