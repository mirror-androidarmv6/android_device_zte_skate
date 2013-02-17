package com.cyanogenmod.settings.device;

import com.cyanogenmod.settings.device.R;

import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;

public class DeviceSettings extends PreferenceActivity {
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        addPreferencesFromResource(R.xml.skateparts);
    }

    @Override
    public void onPause() {
        super.onPause();
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        // USB charging
        if(prefs.getBoolean("usb_charging", true))
            Utils.writeValue("/sys/module/msm_battery/parameters/usb_chg_enable", 1);
        else
            Utils.writeValue("/sys/module/msm_battery/parameters/usb_chg_enable", 0);
    }
}
