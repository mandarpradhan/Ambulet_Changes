package seamo.android;


import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Bundle;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ViewFlipper;
import android.view.View;
import android.view.View.OnClickListener;

public class Main extends Activity implements OnClickListener{
	TextView textStatus, name;
	RelativeLayout relative, relative_child;
	RadioButton wifib, threegb;
	Button save, back, startb, stopb;
	int WIFI = 1, _3G = 2;
	ViewFlipper flipper; 
	static int i = 0;
	public static final String PREFS_CONF = "SeaMo.confData";
	
	SeaMo_AndroidActivity seamoObj = new SeaMo_AndroidActivity();

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        textStatus= (TextView)findViewById(R.id.TextStatus);
        Button add_network = (Button)findViewById(R.id.add_networkb);
       
        startb = (Button)findViewById(R.id.start);
        stopb = (Button)findViewById(R.id.stop);
        add_network = (Button)findViewById(R.id.add_networkb);
        
        
        add_network.setOnClickListener(this);
        startb.setOnClickListener(this);
        stopb.setOnClickListener(this);  
     
        loadData();
        //restore previous entered conf data
        readConf();
        
        //startb.setEnabled(true);
    }
    
	void readConf(){
    	SharedPreferences          confData;
    
    	confData = getSharedPreferences(PREFS_CONF, 0);
    	
    	String str = confData.getString("CONF", null);
    	
    	/* If using the SeaMo application for the first time */
    	if(str == null){
    		textStatus.append("Configure the network\n");
    		startb.setEnabled(false);
    		return;	
    	}
    	if (str.startsWith("["))
        {
            str = str.substring(1, str.length());
        }
        if (str.endsWith("]"))
        {
            str = str.substring(0, str.length() - 1);
        }
    	
    	String[] resultEntry = str.split(",");
    	
    	ArrayList<conf>  temp = new ArrayList<conf>();
    	
    	
    	for (int x=0; x < resultEntry.length; x++){
    	   String[] resultData = resultEntry[x].trim().split("#");
    	   int z = 0;
    	   conf element = new conf();
    	   
    	   
    		  while(z < resultData.length){    			  
    			  element.net = Integer.parseInt(resultData[z]);
    			  z++;
    			  element.essid = resultData[z];
    			  z++;
    			  element.cost = Integer.parseInt(resultData[z]);
    			  z++;
    			  element.pref = Integer.parseInt(resultData[z]);
    			  z++;    			  
    		  }
    		 temp.add(element);
    	  }//for loop
    	NetworkAdder.data = temp;
    }
    
    void saveConf(){
        String s1 = objectToString(NetworkAdder.data);
        textStatus.append("string: " + s1);
        
        SharedPreferences confData;
        
        confData = getSharedPreferences(PREFS_CONF, 0);
		final SharedPreferences.Editor editor = confData.edit();
		
		editor.putString("CONF", s1);
		editor.commit();
   }
    
    public static String objectToString(ArrayList<conf> object){
    	if (NetworkAdder.data.size() == 0){
    		return null;
    	}
    	return object.toString();
    }  
    
    /* Storing the scan_only module to the device */
    private void loadData(){
    	//Read from raw resource
    	InputStream ins = getResources().openRawResource(R.raw.scan_only);
       	int size = 0;
    	byte[] buffer = null, scriptB = null; 
    	PackageManager m = getPackageManager();
        String dir = getPackageName(), dirM, dirS;
        PackageInfo p = null;
        FileOutputStream fos;
    	
		try {
			size = ins.available();
			buffer = new byte[size];
			
			// Read the entire resource into a local byte buffer.
	    	ins.read(buffer);
	    	ins.close();
	    	
		} catch (IOException e1) {
			e1.printStackTrace();
		}

		//Get current persistant application's data area       
		try {
			p = m.getPackageInfo(dir, 0);
		} catch (NameNotFoundException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		dir = p.applicationInfo.dataDir;       
        
		dirS = dir + "/scan-only";
		
		try {
			fos = new FileOutputStream(dirS);
			fos.write(buffer);
			fos.close();
			
			String temp = "chmod 777 " + dirS;
			Runtime.getRuntime().exec(temp);
		} catch (IOException e) {
			e.printStackTrace();
		} 		
		
		 /* Motorola device */
		//String script = "insmod /system/lib/modules/tiwlan_drv.ko\nsleep 1\nstart wlan_loader\nsleep 1\n/system/bin/ifconfig tiwlan0 up";
		
		/* Samsung device */
		String script = "insmod /lib/modules/dhd.ko \"firmware_path=/system/etc/wifi/bcm4330_sta.bin nvram_path=/system/etc/wifi/nvram_net.txt\"\nsleep 1\n/system/bin/ifconfig eth0 up";
		
		scriptB = new byte[size];
		scriptB =  script.getBytes();
		dirM = dir + "/modules.sh";		
		try {
			fos = new FileOutputStream(dirM);
			fos.write(scriptB);
			fos.close();
			
			String temp = "chmod 777 " + dirM;
			Runtime.getRuntime().exec(temp);
		} catch (IOException e) {
			e.printStackTrace();
		} 		
    }
    
        
    public void onClick(View v){
    	
    	switch(v.getId()){
    	case R.id.add_networkb: Intent i = new Intent(this, GuiTestActivity.class);
                                startActivity(i);                                
                                startb.setEnabled(true);
    	                        break;
    	case R.id.start       : Intent dialogIntent = new Intent(this, SeaMo_AndroidActivity.class);
    	                        //dialogIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    	                        startActivity(dialogIntent);    		                        		                        	                        
    	                        break; 
    	case R.id.stop        : saveConf();  
    	                        finish();  
    	                        System.exit(0);
    		                    break;
       }//end switch
    }//end of onClick
}//end of class
