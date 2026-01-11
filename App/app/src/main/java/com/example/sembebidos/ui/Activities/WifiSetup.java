package com.example.sembebidos.ui.Activities;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import androidx.appcompat.app.AppCompatActivity;

import com.example.sembebidos.Constants;
import com.example.sembebidos.MainActivity;
import com.example.sembebidos.R;
import com.example.sembebidos.network.ESP32Manager;
import com.google.android.material.textfield.TextInputEditText;

public class WifiSetup extends AppCompatActivity {

    private static final String TAG = "WifiSetup";
    private static final String PREFS_NAME = "StoryCube";

    private TextInputEditText etWifiSsid;
    private TextInputEditText etWifiPassword;
    private TextInputEditText etIpAddress;
    private TextView tvConnectionStatus;
    private Button btnTestConnection;
    private Button btnContinue;
    private Button btnSkip;

    private SharedPreferences prefs;
    private ESP32Manager esp32Manager;
    private Handler mainHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_wifi_setup);

        // Inicializar componentes
        prefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE);
        esp32Manager = new ESP32Manager();
        mainHandler = new Handler(Looper.getMainLooper());

        initViews();
        loadSavedDataIfExists();
        setupListeners();
    }

    private void initViews() {
        etWifiSsid = findViewById(R.id.etWifiSsid);
        etWifiPassword = findViewById(R.id.etWifiPassword);
        etIpAddress = findViewById(R.id.etIpAddress);
        tvConnectionStatus = findViewById(R.id.tvConnectionStatus);
        btnTestConnection = findViewById(R.id.btnTestConnection);
        btnContinue = findViewById(R.id.btnContinue);
        btnSkip = findViewById(R.id.btnSkip);
    }

    /**
     * Carga datos guardados si existen
     */
    private void loadSavedDataIfExists() {
        String savedSsid = prefs.getString("wifi_ssid", "");
        String savedPassword = prefs.getString("wifi_password", "");
        String savedIp = prefs.getString("ip_address", "");

        if (!savedSsid.isEmpty()) {
            etWifiSsid.setText(savedSsid);
        }
        if (!savedPassword.isEmpty()) {
            etWifiPassword.setText(savedPassword);
        }
        if (!savedIp.isEmpty()) {
            etIpAddress.setText(savedIp);
        }
    }

    private void setupListeners() {
        btnTestConnection.setOnClickListener(v -> testConnection());
        btnContinue.setOnClickListener(v -> saveAndContinue());
        btnSkip.setOnClickListener(v -> skipWiFiSetup());
    }

    /**
     * Prueba la conexión con el ESP32
     */
    private void testConnection() {
        String ipAddress = etIpAddress.getText().toString().trim();

        if (ipAddress.isEmpty()) {
            Toast.makeText(this, "Ingresa la dirección IP", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!isValidIpAddress(ipAddress)) {
            Toast.makeText(this, "Dirección IP inválida", Toast.LENGTH_SHORT).show();
            return;
        }

        // Actualizar URL temporalmente
        String oldUrl = Constants.BASE_URL;
        Constants.BASE_URL = "http://" + ipAddress;

        tvConnectionStatus.setText("⏳ Conectando...");
        tvConnectionStatus.setTextColor(getResources().getColor(android.R.color.holo_orange_dark));

        esp32Manager.getStatus(new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "✓ Conexión exitosa");
                mainHandler.post(() -> {
                    tvConnectionStatus.setText("✅ Conectado correctamente");
                    tvConnectionStatus.setTextColor(getResources().getColor(android.R.color.holo_green_dark));
                    Toast.makeText(WifiSetup.this, "✓ Conexión exitosa", Toast.LENGTH_SHORT).show();
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "✗ Error de conexión: " + error);
                Constants.BASE_URL = oldUrl;
                mainHandler.post(() -> {
                    tvConnectionStatus.setText("❌ Error: " + error);
                    tvConnectionStatus.setTextColor(getResources().getColor(android.R.color.holo_red_dark));
                    Toast.makeText(WifiSetup.this, "✗ No se pudo conectar", Toast.LENGTH_SHORT).show();
                });
            }
        });
    }

    /**
     * Guarda la configuración y marca como configurado
     */
    private void saveAndContinue() {
        String ssid = etWifiSsid.getText().toString().trim();
        String password = etWifiPassword.getText().toString().trim();
        String ipAddress = etIpAddress.getText().toString().trim();

        // Validar que no estén vacíos
        if (ssid.isEmpty() || password.isEmpty() || ipAddress.isEmpty()) {
            Toast.makeText(this, "Completa todos los campos", Toast.LENGTH_SHORT).show();
            return;
        }

        // Validar IP
        if (!isValidIpAddress(ipAddress)) {
            Toast.makeText(this, "Dirección IP inválida", Toast.LENGTH_SHORT).show();
            return;
        }

        // GUARDAR EN SharedPreferences
        SharedPreferences.Editor editor = prefs.edit();
        editor.putString("wifi_ssid", ssid);
        editor.putString("wifi_password", password);
        editor.putString("ip_address", ipAddress);

        // MARCAR COMO CONFIGURADO (esto es clave)
        editor.putBoolean("wifi_configured", true);
        editor.apply();

        // Actualizar Constants
        Constants.BASE_URL = "http://" + ipAddress;

        Log.d(TAG, "✓ WiFi configurado: SSID=" + ssid + " | IP=" + ipAddress);
        Toast.makeText(this, "✓ Configuración guardada", Toast.LENGTH_SHORT).show();

        goToMainActivity();
    }

    /**
     * Marca como no configurado al saltar
     */
    private void skipWiFiSetup() {
        // NO marcar como configurado, solo marcar que se mostró la pantalla
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean("wifi_configured", false); // Explícitamente NO configurado
        editor.apply();

        Toast.makeText(this, "Configuración saltada. Puedes configurarlo después en Ajustes", Toast.LENGTH_SHORT).show();
        goToMainActivity();
    }

    private void goToMainActivity() {
        Intent intent = new Intent(WifiSetup.this, MainActivity.class);
        startActivity(intent);
        finish();
    }

    /**
     * Valida el formato de una dirección IP
     */
    private boolean isValidIpAddress(String ip) {
        String[] parts = ip.split("\\.");
        if (parts.length != 4) return false;

        for (String part : parts) {
            try {
                int num = Integer.parseInt(part);
                if (num < 0 || num > 255) return false;
            } catch (NumberFormatException e) {
                return false;
            }
        }
        return true;
    }
}