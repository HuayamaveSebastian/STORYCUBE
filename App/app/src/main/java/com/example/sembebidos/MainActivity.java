package com.example.sembebidos;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.auth.FirebaseAuth;
import com.example.sembebidos.ui.Activities.LoginActivity;
import com.example.sembebidos.ui.Activities.SettingsActivity;
import com.example.sembebidos.network.ESP32Manager;

public class MainActivity extends AppCompatActivity {
    private Handler mainHandler = new Handler(Looper.getMainLooper());
    private static final String TAG = "MainActivity";
    private Button btnLogout;
    private FirebaseAuth mAuth;
    private ESP32Manager esp32Manager;

    // Controles de brillo
    private SeekBar seekBrightness;
    private TextView tvBrightnessValue;

    // Controles de reproducción
    private Button btnPlayPause, btnNext, btnPrevious;

    // Controles de volumen
    private Button btnVolUp, btnVolDown;
    private TextView tvVolumeValue;

    // Mini pantalla de estado
    private TextView tvStatus, tvCurrentTrack, tvCurrentVolume, tvCurrentMode;

    // Selector de modo
    private Spinner spinnerMode;
    private Button btnChangeMode;

    // Variables de estado
    private int currentVolume = 20;
    private int currentTrack = 1;
    private String currentMode = "RFID";
    private boolean isPlaying = false;
    private boolean updatingBrightnessUI = false;
    private boolean updatingVolumeUI = false;  // evitar bucles de volumen

    private static final int NUM_TRACKS = 6;
    private static final int VOL_MIN = 0;
    private static final int VOL_MAX = 50;
    private static final int BRIGHTNESS_MIN = 5;
    private static final int BRIGHTNESS_MAX = 255;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Cargar IP guardada de SharedPreferences
        SharedPreferences prefs = getSharedPreferences("StoryCube", Context.MODE_PRIVATE);
        String ipAddress = prefs.getString("ip_address", "192.168.1.100");
        Constants.BASE_URL = "http://" + ipAddress;

        Log.d(TAG, "Base URL: " + Constants.BASE_URL);

        // Inicializar Firebase Auth
        mAuth = FirebaseAuth.getInstance();

        // Verificar si hay usuario logueado
        if (mAuth.getCurrentUser() == null) {
            goToLogin();
            return;
        }

        // Inicializar ESP32Manager
        esp32Manager = new ESP32Manager();

        // Inicializar vistas
        initViews();
        setupListeners();

        // Obtener estado inicial del ESP32
        updateStatus();
    }

    @Override
    protected void onResume() {
        super.onResume();
        SharedPreferences prefs = getSharedPreferences("StoryCube", Context.MODE_PRIVATE);
        String ipAddress = prefs.getString("ip_address", "192.168.1.100");
        Constants.BASE_URL = "http://" + ipAddress;
        updateStatus();
    }

    private void initViews() {
        btnLogout = findViewById(R.id.btnLogout);
        Button btnSettings = findViewById(R.id.btnSettings);

        //  BRILLO
        seekBrightness = findViewById(R.id.seekBrightness);
        tvBrightnessValue = findViewById(R.id.tvBrightnessValue);

        seekBrightness.setMin(BRIGHTNESS_MIN);
        seekBrightness.setMax(BRIGHTNESS_MAX);
        seekBrightness.setProgress(50);
        tvBrightnessValue.setText("50");

        // Reproducción
        btnPlayPause = findViewById(R.id.btnPlayPause);
        btnNext = findViewById(R.id.btnNext);
        btnPrevious = findViewById(R.id.btnPrevious);

        // Volumen
        btnVolUp = findViewById(R.id.btnVolUp);
        btnVolDown = findViewById(R.id.btnVolDown);
        tvVolumeValue = findViewById(R.id.tvVolumeValue);

        // Estado
        tvStatus = findViewById(R.id.tvStatus);
        tvCurrentTrack = findViewById(R.id.tvCurrentTrack);
        tvCurrentVolume = findViewById(R.id.tvCurrentVolume);
        tvCurrentMode = findViewById(R.id.tvCurrentMode);

        // Modo
        spinnerMode = findViewById(R.id.spinnerMode);
        btnChangeMode = findViewById(R.id.btnChangeMode);
    }

    private void setupListeners() {
        btnLogout.setOnClickListener(v -> logout());

        Button btnSettings = findViewById(R.id.btnSettings);
        btnSettings.setOnClickListener(v -> {
            Intent intent = new Intent(MainActivity.this, SettingsActivity.class);
            startActivity(intent);
        });

        // ✅ BRILLO
        seekBrightness.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    Log.d(TAG, "Usuario cambió brillo a: " + progress);
                    tvBrightnessValue.setText(String.valueOf(progress));
                    changeBrightness(progress);
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {}
        });

        // ✅ REPRODUCCIÓN
        btnPlayPause.setOnClickListener(v -> {
            if (currentMode.equals("Manual")) {
                togglePlayPause();
            } else {
                Toast.makeText(MainActivity.this, "Play/Pausa solo en modo Manual", Toast.LENGTH_SHORT).show();
            }
        });

        btnNext.setOnClickListener(v -> {
            if (currentMode.equals("Manual")) {
                nextTrack();
            } else {
                Toast.makeText(MainActivity.this, "Siguiente solo en modo Manual", Toast.LENGTH_SHORT).show();
            }
        });

        btnPrevious.setOnClickListener(v -> {
            if (currentMode.equals("Manual")) {
                previousTrack();
            } else {
                Toast.makeText(MainActivity.this, "Anterior solo en modo Manual", Toast.LENGTH_SHORT).show();
            }
        });

        // ✅ VOLUMEN
        btnVolUp.setOnClickListener(v -> increaseVolume());
        btnVolDown.setOnClickListener(v -> decreaseVolume());

        // Modo
        btnChangeMode.setOnClickListener(v -> changeMode());
    }

    /**
     * Cambia el brillo del ESP32
     */
    private void changeBrightness(int brightness) {
        Log.d(TAG, "📡 Enviando brillo al ESP32: " + brightness);

        esp32Manager.setBrightness(brightness, new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "✅ Brillo aplicado correctamente");
                mainHandler.post(() -> {
                    tvBrightnessValue.setText(String.valueOf(brightness));
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "❌ Error al cambiar brillo: " + error);
                Toast.makeText(MainActivity.this, "Error brillo: " + error, Toast.LENGTH_SHORT).show();
            }
        });
    }

    /**
     * Alterna entre Play y Pausa
     */
    private void togglePlayPause() {
        Log.d(TAG, "🎯 Toggle Play/Pause - Estado actual: " + (isPlaying ? "Reproduciendo" : "Pausado"));

        // ✅ NO cambiar isPlaying aquí - esperar confirmación del ESP32
        if (isPlaying) {
            pauseTrack();
        } else {
            // ✅ Si está pausado, reanudar (NO reproducir desde el inicio)
            resumeTrack();
        }
    }
    /**
     * Reproduce la pista actual
     */
    private void playTrack() {
        Log.d(TAG, "📡 Reproduciendo pista " + currentTrack + " desde el inicio");

        esp32Manager.playTrack(currentTrack, new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "✅ ESP32 confirmó REPRODUCCIÓN");
                mainHandler.post(() -> {
                    isPlaying = true;
                    updateStatusDisplay();
                    updatePlayPauseButton();
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "❌ Error al reproducir: " + error);
                mainHandler.post(() -> {
                    Toast.makeText(MainActivity.this, "Error al reproducir: " + error, Toast.LENGTH_SHORT).show();
                });
            }
        });
    }

    /**
     * Pausa la reproducción
     */
    private void pauseTrack() {
        Log.d(TAG, "📡 Enviando PAUSE al ESP32...");

        esp32Manager.pause(new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "✅ ESP32 confirmó PAUSA");
                mainHandler.post(() -> {
                    isPlaying = false;  // ✅ Solo cambiar después de confirmación
                    updateStatusDisplay();
                    updatePlayPauseButton();
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "❌ Error al pausar: " + error);
                mainHandler.post(() -> {
                    Toast.makeText(MainActivity.this, "Error al pausar: " + error, Toast.LENGTH_SHORT).show();
                });
            }
        });
    }

    /**
     * Siguiente pista
     */
    private void nextTrack() {
        currentTrack++;
        if (currentTrack > NUM_TRACKS) {
            currentTrack = 1;
        }

        Log.d(TAG, "Siguiente pista: " + currentTrack);

        esp32Manager.next(new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                updateStatusDisplay();
            }

            @Override
            public void onError(String error) {
                currentTrack--;
                if (currentTrack < 1) {
                    currentTrack = NUM_TRACKS;
                }
                Toast.makeText(MainActivity.this, "Error al avanzar: " + error, Toast.LENGTH_SHORT).show();
                Log.e(TAG, "Error en siguiente: " + error);
            }
        });
    }

    /**
     * Pista anterior
     */
    private void previousTrack() {
        currentTrack--;
        if (currentTrack < 1) {
            currentTrack = NUM_TRACKS;
        }

        Log.d(TAG, "Pista anterior: " + currentTrack);

        esp32Manager.previous(new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                updateStatusDisplay();
            }

            @Override
            public void onError(String error) {
                currentTrack++;
                if (currentTrack > NUM_TRACKS) {
                    currentTrack = 1;
                }
                Toast.makeText(MainActivity.this, "Error al retroceder: " + error, Toast.LENGTH_SHORT).show();
                Log.e(TAG, "Error en anterior: " + error);
            }
        });
    }

    /**
     * Aumenta el volumen (0-50)
     */
    private void increaseVolume() {
        if (currentVolume < VOL_MAX) {
            currentVolume++;
            changeVolume(currentVolume);
        }
    }

    /**
     * Disminuye el volumen (0-50)
     */
    private void decreaseVolume() {
        if (currentVolume > VOL_MIN) {
            currentVolume--;
            changeVolume(currentVolume);
        }
    }

    /**
     * Cambia el volumen del ESP32
     * ACTUALIZA AMBAS VISTAS DE VOLUMEN
     */
    private void changeVolume(int volume) {
        Log.d(TAG, "📡 Enviando volumen al ESP32: " + volume);

        esp32Manager.setVolume(volume, new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "✅ Volumen aplicado: " + volume);
                mainHandler.post(() -> {
                    // ✅ ACTUALIZAR AMBAS VISTAS
                    updateVolumeDisplay();
                    updateStatusDisplay();
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "❌ Error al cambiar volumen: " + error);
                Toast.makeText(MainActivity.this, "Error volumen: " + error, Toast.LENGTH_SHORT).show();
            }
        });
    }

    /**
     * Cambia el modo del StoryCube
     */
    private void changeMode() {
        String selectedMode = spinnerMode.getSelectedItem().toString();
        String mode = getModeConstant(selectedMode);

        Log.d(TAG, "Cambiando modo a: " + selectedMode + " (" + mode + ")");

        esp32Manager.setMode(mode, new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                currentMode = selectedMode;
                updateStatusDisplay();
                Log.d(TAG, "Modo cambiado: " + selectedMode);
                Toast.makeText(MainActivity.this, "Modo: " + selectedMode, Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onError(String error) {
                Toast.makeText(MainActivity.this, "Error modo: " + error, Toast.LENGTH_SHORT).show();
                Log.e(TAG, "Error modo: " + error);
            }
        });
    }

    /**
     * Mapea el nombre del modo a la constante
     */
    private String getModeConstant(String modeName) {
        switch (modeName) {
            case "RFID":
                return Constants.MODO_RFID;
            case "Manual":
                return Constants.MODO_MANUAL;
            case "Números":
                return Constants.MODO_NUMEROS;
            case "Colores":
                return Constants.MODO_COLORES;
            default:
                return Constants.MODO_MANUAL;
        }
    }

    /**
     * Actualiza la pantalla de estado
     */
    /**
     * Reanuda la reproducción (NO reinicia)
     */
    private void resumeTrack() {
        Log.d(TAG, "📡 Enviando RESUME al ESP32...");

        esp32Manager.pause(new ESP32Manager.Callback() {  // ✅ Usa el mismo endpoint
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "✅ ESP32 confirmó REANUDACIÓN");
                mainHandler.post(() -> {
                    isPlaying = true;  // ✅ Solo cambiar después de confirmación
                    updateStatusDisplay();
                    updatePlayPauseButton();
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "❌ Error al reanudar: " + error);
                mainHandler.post(() -> {
                    Toast.makeText(MainActivity.this, "Error al reanudar: " + error, Toast.LENGTH_SHORT).show();
                });
            }
        });
    }
    private void updateStatusDisplay() {
        tvCurrentTrack.setText("Pista: " + currentTrack);
        tvCurrentVolume.setText("Volumen: " + currentVolume + "/50");
        tvCurrentMode.setText("Modo: " + currentMode);

        String estado = isPlaying ? "▶ Reproduciendo" : "⏸ Pausado";
        tvStatus.setText(estado);

        updatePlayPauseButton();
    }

    /**
     * Actualiza el botón Play/Pausa
     */
    private void updatePlayPauseButton() {
        if (isPlaying) {
            btnPlayPause.setText("⏸ Pausar");
        } else {
            btnPlayPause.setText("▶ Reanudar");
        }
    }

    /**
     * Actualiza el volumen en pantalla
     */
    private void updateVolumeDisplay() {
        tvVolumeValue.setText(currentVolume + "/50");
        tvCurrentVolume.setText("Volumen: " + currentVolume + "/50");
    }

    /**
     * Obtiene el estado del ESP32
     */
    private void updateStatus() {
        esp32Manager.getStatus(new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "Estado: " + response);
                updateStatusDisplay();
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "Error estado: " + error);
                Toast.makeText(MainActivity.this, "No se pudo obtener estado: " + error, Toast.LENGTH_SHORT).show();
            }
        });
    }

    private void logout() {
        mAuth.signOut();
        Toast.makeText(this, "Sesión cerrada", Toast.LENGTH_SHORT).show();
        goToLogin();
    }

    private void goToLogin() {
        Intent intent = new Intent(MainActivity.this, LoginActivity.class);
        startActivity(intent);
        finish();
    }
}