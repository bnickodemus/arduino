package com.brocnickodemus.embedsys;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity {
    private final String DEVICE_ADDRESS = "00:14:03:05:58:FC"; // required MAC Address of Bluetooth Module
    //private final String DEVICE_ADDRESS = "98:D3:31:FB:84:27"; // required MAC Address of Bluetooth Module
    private final UUID PORT_UUID = UUID.fromString("00001101-0000-1000-8000-00805f9b34fb"); // bluetooth serial board well-known SPP UUID

    private static final String TAG = "MainActivity";
    private ConnectedThread mConnectedThread;
    private BluetoothDevice device;
    private BluetoothSocket socket;
    private InputStream inStream = null;
    private OutputStream outStream = null;
    private boolean running = true;
    private boolean verbose = true;
    private String buff = "";

    StringBuilder total = new StringBuilder();
    String incomingMessage;
    Button saveBtn, bluetoothConnBtn, leftBtn, centerBtn, rightBtn, verbBtn;
    TextView sensorView;
    int bytes; // bytes returned from read()
    String command = "1";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        sensorView = (TextView) findViewById(R.id.textView);
        leftBtn = (Button) findViewById(R.id.left);
        centerBtn = (Button) findViewById(R.id.center);
        rightBtn = (Button) findViewById(R.id.right);
        verbBtn = (Button) findViewById(R.id.start);
        saveBtn = (Button) findViewById(R.id.save);
        bluetoothConnBtn = (Button) findViewById(R.id.bluetooth_conn_btn);

        sensorView.setMovementMethod(new ScrollingMovementMethod());

        int REQUEST_CODE = 1;
        ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, REQUEST_CODE);

        leftBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                command = "0";
                /*try {
                    outStream.write(command.getBytes()); //transmits the value of command to the bluetooth module
                } catch (IOException e) {
                    e.printStackTrace();
                }*/
                return false;
            }
        });

        centerBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                command = "1";
                /*try {
                    //outStream.write(command.getBytes()); //transmits the value of command to the bluetooth module
                } catch (IOException e) {
                    e.printStackTrace();
                }*/
                return false;
            }
        });

        rightBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                command = "2";
                /*try {
                    outStream.write(command.getBytes()); //transmits the value of command to the bluetooth module
                } catch (IOException e) {
                    e.printStackTrace();
                }*/
                return false;
            }
        });

        verbBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //running = false;
                sensorView.setText("");
                //mConnectedThread = new ConnectedThread(socket);
                verbose = !verbose;
                if(verbose){
                    verbBtn.setText("Basic");
                }
                else{
                    verbBtn.setText("Verbose");
                }
            }
        });

        saveBtn.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                running = false;
                mConnectedThread.interrupt();
                String data = sensorView.getText().toString();
                Date myDate = Calendar.getInstance().getTime();
                SimpleDateFormat frmt = new SimpleDateFormat("HH:mm-dd.MM.yy");
                SimpleDateFormat precisefrmt = new SimpleDateFormat("HH:mm:ss.SSS-dd.MM.yy");
                String currDate = frmt.format(myDate);
                String timeStamp = precisefrmt.format(myDate);
                String title = "flightData-" + currDate + ".txt";
                File file = new File(Environment.getExternalStorageDirectory(), title);

                try {
                    FileOutputStream os = new FileOutputStream(file);
                    os.write(data.getBytes());
                    timeStamp = "\n\n" + timeStamp;
                    os.write(timeStamp.getBytes());
                    os.close();
                    Toast.makeText(getApplicationContext(), "Saving to /sdcard/" + title, Toast.LENGTH_SHORT).show();
                    //sensorView.setText(""); // this clears the data before it can be written to the file
                } catch (IOException e) {
                    Toast.makeText(getApplicationContext(), e.toString(), Toast.LENGTH_SHORT).show();
                }

                running = true;
                mConnectedThread = new ConnectedThread(socket);
                mConnectedThread.start();
                return false;
            }
        });

        bluetoothConnBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Boolean connected = false;
                if (BTinit()) {
                    try {
                        socket = device.createRfcommSocketToServiceRecord(PORT_UUID);
                        socket.connect();
                        Toast.makeText(getApplicationContext(), "Connection to bluetooth device successful", Toast.LENGTH_SHORT).show();
                        connected = true;
                    } catch (IOException e) {
                        e.printStackTrace();
                        connected = false;
                    }
                    if (connected) {
                        mConnectedThread = new ConnectedThread(socket);
                        mConnectedThread.start();
                    }
                }
            }
        });
    }

    public boolean BTinit() {
        boolean found = false;
        BluetoothAdapter bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (bluetoothAdapter == null) { // does phone support bluetooth
            Toast.makeText(getApplicationContext(), "Device doesn't support bluetooth", Toast.LENGTH_SHORT).show();
        }
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableAdapter = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableAdapter, 0);
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        Set<BluetoothDevice> bondedDevices = bluetoothAdapter.getBondedDevices();
        if (bondedDevices.isEmpty()) { // check for paired bluetooth devices
            Toast.makeText(getApplicationContext(), "Please pair the device first", Toast.LENGTH_SHORT).show();
        } else {
            for (BluetoothDevice iterator : bondedDevices) {
                Toast.makeText(getApplicationContext(), iterator.getAddress().toString(), Toast.LENGTH_SHORT).show();
                if (iterator.getAddress().equals(DEVICE_ADDRESS)) {
                    device = iterator;
                    found = true;
                    break;
                }
            }
        }
        return found;
    }

    private class ConnectedThread extends Thread {

        public ConnectedThread(BluetoothSocket socket) {
            Log.d(TAG, "ConnectedThread: Starting.");
            try {
                inStream = socket.getInputStream();
                outStream = socket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        public void run() {
            byte[] buffer = new byte[2048];  // buffer store for the stream
            try {
                sleep(1100); //This is mainly to give "save" a time buffer before clearing the textView
            }catch (Exception e) {
                Log.e(TAG, "Sleep error");
            }
            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    sensorView = (TextView) findViewById(R.id.textView);
                    sensorView.setText("");
                    buff = "";
                }
            });

            while (running) { // Keep listening to the InputStream until an exception occurs
                try {
                    //if (inStream.available() > 0) {
                        bytes = inStream.read(buffer);

                        incomingMessage = new String(buffer, 0, bytes);
                        buff += incomingMessage.toString();

                        if (buff.length() >= 16) { //Once the message is greater than the minimal length, extract the data and format the message
                            buff = buff.replace("\n", "");
                            String[] vals = buff.split(", ", 4);
                            //vals[2] = vals[2].replace("\n", "");
                            if (vals.length > 2) {
                                if (vals[0].length() >= 4 || vals[0].length() > 6) {
                                    if (verbose){
                                        buff = "Pitch: " + vals[0] + ", Roll: " + vals[1] + ",  " + vals[2] + " ms\n";
                                    }
                                    else{
                                        buff = vals[0] + ", " + vals[1] + ", " + vals[2] + "\n";
                                    }

                                    Log.d(TAG, "vals length: " + vals.length);
                                }
                                else {
                                    Log.d(TAG, "buff issue inner: " + vals[0] + ", " + vals[1] + ", " + vals[2]);
                                    Log.d(TAG, "vals[0] length: " + vals[0].length());
                                    buff = "";
                                    continue;
                                }
                            }
                            else{
                                Log.d(TAG, "buff issue outer: " + buff);
                                Log.d(TAG, "vals length: " + vals.length);
                                continue;
                            }
                            runOnUiThread(new Runnable() {
                                @Override
                                public void run() {
                                    Log.d(TAG, "InputStream: " + buff); // incoming bytes arent sent at same time
                                    sensorView = (TextView) findViewById(R.id.textView);
                                    try {
                                        outStream.write(command.getBytes());
                                    } catch (IOException e) {
                                        e.printStackTrace();
                                    }

                                    sensorView.append(buff);
                                    buff = "";
                                    try {
                                        sleep(50);
                                    }catch (Exception e) {
                                        Log.e(TAG, "Sleep error");
                                    }
                                }
                            });
                        }
                    //}
                } catch (IOException e) {
                    Log.e(TAG, "write: Error reading Input Stream. " + e.getMessage());
                    Log.e(TAG, "dropped data");
                    break;
                }
                try {
                    sleep(300);
                }catch (InterruptedException  e) {
                    Log.d(TAG, "***************Thread Ended***************");
                    return;
                }
            }
        }

    }
}




















