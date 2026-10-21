// Copyright Epic Games, Inc. All Rights Reserved.

package com.epicgames.unreal;

import android.app.Activity;
import android.os.Bundle;
import android.view.Window;
import android.view.WindowManager;
import android.util.Log;

public class PICOSpatialActivity extends Activity {

    public static native void JavaToCOnSpatialContainerStateChangeCallback(Object activity);

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		Window window = getWindow();
		WindowManager.LayoutParams params = window.getAttributes();
		params.alpha = 0;
		window.setAttributes(params);
		Log.d("LogPICOSpatial", "PICOSpatialActivity onCreate.");
		JavaToCOnSpatialContainerStateChangeCallback(this);
    }
}
