package com.example.NativeExample;

import android.app.Activity;
import android.app.NativeActivity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.widget.TextView;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.text.InputType;
import android.view.WindowManager;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.content.res.Configuration;
import android.view.WindowInsets;
import android.graphics.Insets;

import android.view.MotionEvent;
import android.util.Log;

import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Locale;

import javax.naming.Context;

public class MyActivity extends NativeActivity {

  public void showKeyboard() {
    NativeActivity context = this;

    runOnUiThread(new Runnable() {
      @Override
      public void run() {
        EditText dummyEditText = new EditText(context);
        dummyEditText.setVisibility(View.VISIBLE); // Or INVISIBLE if you don't want to see it
        dummyEditText.setFocusable(true);
        dummyEditText.setFocusableInTouchMode(true);
        dummyEditText.setInputType(InputType.TYPE_CLASS_TEXT); // Adjust input type if needed
        dummyEditText.requestFocus();
        InputMethodManager imm = (InputMethodManager) getSystemService(context.INPUT_METHOD_SERVICE);
        imm.showSoftInput(dummyEditText, InputMethodManager.SHOW_IMPLICIT);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
      }
    });
  }

  public void hideKeyboard() {
  }

  @Override
  public boolean onTouchEvent(MotionEvent event) {
    switch (event.getAction()) {
      case (MotionEvent.ACTION_DOWN):
        Log.d("NativeExample", "Action was DOWN");
        return true;
      case (MotionEvent.ACTION_MOVE):
        Log.d("NativeExample", "Action was MOVE");
        return true;
      case (MotionEvent.ACTION_UP):
        Log.d("NativeExample", "Action was UP");
        return true;
      case (MotionEvent.ACTION_CANCEL):
        Log.d("NativeExample", "Action was CANCEL");
        return true;
      case (MotionEvent.ACTION_OUTSIDE):
        Log.d("NativeExample", "Movement occurred outside bounds of current screen element");
        return true;
      default:
        return super.onTouchEvent(event);
    }
  }

  public int pressedMessageBoxButtonId = 0;

  public int showYesNoMessageBox(String title, String message) {
    NativeActivity context = this;
    runOnUiThread(new Runnable() {
      @Override
      public void run() {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        builder.setMessage(message);
        builder.setTitle(title);
        builder.setPositiveButton("Yes", new DialogInterface.OnClickListener() {
          public void onClick(DialogInterface dialog, int id) {
            pressedMessageBoxButtonId = 1;
          }
        });
        builder.setNegativeButton("No", new DialogInterface.OnClickListener() {
          public void onClick(DialogInterface dialog, int id) {
            pressedMessageBoxButtonId = -1;
          }
        });

        AlertDialog dialog = builder.show();
      }
    });
    while (pressedMessageBoxButtonId == 0) {
    }
    return pressedMessageBoxButtonId;
  }

  public int getDpi() {
    DisplayMetrics metrics = getResources().getDisplayMetrics();
    int result = metrics.densityDpi;
    return result;
  }

  public float getPixelDensity() {
    DisplayMetrics metrics = getResources().getDisplayMetrics();
    float result = metrics.density;
    return result;
  }

  public float getFontScale() {
    Configuration config = getResources().getConfiguration();
    float result = config.fontScale;
    return result;
  }

  // NOTE(hampus): returns (top, down, left, right)
  public float[] getStatusBarInset() {
    View view = this.getWindow().getDecorView();
    WindowInsets window_insets = view.getRootWindowInsets();
    Insets insets = window_insets.getInsets(WindowInsets.Type.statusBars());
    float result[] = new float[] { insets.top, insets.bottom, insets.left, insets.right };
    return result;
  }

  public float[] getNavigationBarInset() {
    View view = this.getWindow().getDecorView();
    WindowInsets window_insets = view.getRootWindowInsets();
    Insets insets = window_insets.getInsets(WindowInsets.Type.navigationBars());
    float result[] = new float[] { insets.top, insets.bottom, insets.left, insets.right };
    return result;
  }
}