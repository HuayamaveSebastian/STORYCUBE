package com.example.sembebidos.ui.Activities;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.util.Log;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import com.example.sembebidos.Constants;
import com.example.sembebidos.R;
import com.example.sembebidos.network.ESP32Manager;
import com.google.android.material.textfield.TextInputEditText;

import org.json.JSONObject;

public class SettingsActivity extends AppCompatActivity {
    private static final String TAG = "SettingsActivity";
    private static final String PREFS_NAME = "StoryCube";

    private TextInputEditText etWifiSsid;
    private TextInputEditText etWifiPassword;
    private TextInputEditText etIpAddress;
    private TextView tvConnectionStatus;
    private Button btnTestConnection;
    private Button btnSave;
    private Button btnCancel;
    private Button btnNetworkInfo;

    private SharedPreferences prefs;
    private ESP32Manager esp32Manager;
    private Handler mainHandler;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        prefs = getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE);
        esp32Manager = new ESP32Manager();
        mainHandler = new Handler(Looper.getMainLooper());

        initViews();
        loadSavedData();
        setupListeners();
    }

    private void initViews() {
        etWifiSsid = findViewById(R.id.etWifiSsid);
        etWifiPassword = findViewById(R.id.etWifiPassword);
        etIpAddress = findViewById(R.id.etIpAddress);
        tvConnectionStatus = findViewById(R.id.tvConnectionStatus);
        btnTestConnection = findViewById(R.id.btnTestConnection);
        btnSave = findViewById(R.id.btnSave);
        btnCancel = findViewById(R.id.btnCancel);
        btnNetworkInfo = findViewById(R.id.btnNetworkInfo);
    }

    private void loadSavedData() {
        String savedSsid = prefs.getString("wifi_ssid", "");
        String savedPassword = prefs.getString("wifi_password", "");
        String savedIp = prefs.getString("ip_address", "192.168.1.100");

        etWifiSsid.setText(savedSsid);
        etWifiPassword.setText(savedPassword);
        etIpAddress.setText(savedIp);

        Log.d(TAG, "Datos cargados: SSID=" + savedSsid + " | IP=" + savedIp);
    }

    private void setupListeners() {
        btnSave.setOnClickListener(v -> guardarYEnviarConfiguracion());
        btnCancel.setOnClickListener(v -> finish());
        btnTestConnection.setOnClickListener(v -> testConnection());

        // Ver info de red del ESP32
        if (btnNetworkInfo != null) {
            btnNetworkInfo.setOnClickListener(v -> mostrarInfoRed());
        }
    }

    /**
     * Guarda localmente Y envía al ESP32
     */
    private void guardarYEnviarConfiguracion() {
        String ssid = etWifiSsid.getText().toString().trim();
        String password = etWifiPassword.getText().toString().trim();
        String ipAddress = etIpAddress.getText().toString().trim();

        // Validaciones
        if (ssid.isEmpty()) {
            Toast.makeText(this, "Ingresa el SSID", Toast.LENGTH_SHORT).show();
            return;
        }

        if (password.isEmpty()) {
            Toast.makeText(this, "Ingresa la contraseña", Toast.LENGTH_SHORT).show();
            return;
        }

        if (ipAddress.isEmpty()) {
            Toast.makeText(this, "Ingresa la IP del ESP32", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!isValidIpAddress(ipAddress)) {
            Toast.makeText(this, "IP inválida", Toast.LENGTH_SHORT).show();
            return;
        }

        // PASO 1: Actualizar URL con la IP ACTUAL (donde está ahora)
        Constants.BASE_URL = "http://" + ipAddress;

        Log.d(TAG, "📡 ===== CONFIGURANDO WIFI =====");
        Log.d(TAG, "SSID destino: " + ssid);
        Log.d(TAG, "IP actual del ESP32: " + ipAddress);
        Log.d(TAG, "================================");

        //PASO 2: Mostrar diálogo de confirmación
        new AlertDialog.Builder(this)
                .setTitle("⚠️ Confirmar Cambio")
                .setMessage(
                        "Se configurará:\n\n" +
                                "Nueva red WiFi: " + ssid + "\n" +
                                "IP actual: " + ipAddress + "\n\n" +
                                "El ESP32 se reiniciará y obtendrá una NUEVA IP.\n" +
                                "Deberás buscar esa IP para conectarte después.\n\n" +
                                "¿Continuar?"
                )
                .setPositiveButton("Sí, configurar", (dialog, which) -> {
                    enviarConfiguracionAlESP32(ssid, password, ipAddress);
                })
                .setNegativeButton("Cancelar", null)
                .show();
    }

    /**
     * ENVÍA LA CONFIGURACIÓN AL ESP32
     */
    private void enviarConfiguracionAlESP32(String ssid, String password, String ipActual) {
        // Mostrar progreso
        AlertDialog progressDialog = new AlertDialog.Builder(this)
                .setTitle("Configurando WiFi")
                .setMessage("Enviando configuración al ESP32...\n\nEsto tomará unos segundos.")
                .setCancelable(false)
                .create();
        progressDialog.show();

        // ENVIAR AL ESP32
        esp32Manager.configureWiFi(ssid, password, new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, " WiFi enviado al ESP32: " + response);

                mainHandler.post(() -> {
                    progressDialog.dismiss();

                    // GUARDAR LOCALMENTE SOLO SI EL ESP32 RESPONDIÓ OK
                    SharedPreferences.Editor editor = prefs.edit();
                    editor.putString("wifi_ssid", ssid);
                    editor.putString("wifi_password", password);
                    editor.putBoolean("wifi_configured", true);
                    editor.apply();

                    Log.d(TAG, "✓ Configuración guardada localmente");

                    //  MOSTRAR INSTRUCCIONES
                    new AlertDialog.Builder(SettingsActivity.this)
                            .setTitle(" Configuración Enviada")
                            .setMessage(
                                    "El ESP32 se reiniciará y se conectará a:\n" +
                                            "Red: " + ssid + "\n\n" +
                                            "IMPORTANTE:\n" +
                                            "1. El ESP32 obtendrá una NUEVA IP\n" +
                                            "2. Busca esa IP en tu router o consola serial\n" +
                                            "3. Actualiza la IP en esta pantalla\n\n" +
                                            "Espera 30 segundos antes de intentar conectar."
                            )
                            .setPositiveButton("Entendido", (d, w) -> {
                                // Pedir nueva IP
                                pedirNuevaIP(ssid);
                            })
                            .setCancelable(false)
                            .show();
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "❌ Error enviando WiFi: " + error);

                mainHandler.post(() -> {
                    progressDialog.dismiss();

                    new AlertDialog.Builder(SettingsActivity.this)
                            .setTitle("❌ Error")
                            .setMessage(
                                    "No se pudo enviar la configuración:\n" +
                                            error + "\n\n" +
                                            "Verifica:\n" +
                                            "• Que el ESP32 esté encendido\n" +
                                            "• Que la IP actual sea correcta\n" +
                                            "• Que estés en la misma red"
                            )
                            .setPositiveButton("OK", null)
                            .show();
                });
            }
        });
    }

    /**
     * PIDE LA NUEVA IP DESPUÉS DE CONFIGURAR
     */
    private void pedirNuevaIP(String ssid) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);

        final EditText input = new EditText(this);
        input.setHint("192.168.x.xxx");

        builder.setTitle("Nueva IP del ESP32")
                .setMessage(
                        "El ESP32 ya se conectó a: " + ssid + "\n\n" +
                                "Busca su nueva IP en:\n" +
                                "• Consola serial del ESP32\n" +
                                "• Panel de tu router\n" +
                                "• App de escaneo de red\n\n" +
                                "Ingresa la nueva IP:"
                )
                .setView(input)
                .setPositiveButton("Guardar", (dialog, which) -> {
                    String newIp = input.getText().toString().trim();

                    if (isValidIpAddress(newIp)) {
                        // Guardar nueva IP
                        SharedPreferences.Editor editor = prefs.edit();
                        editor.putString("ip_address", newIp);
                        editor.apply();

                        Constants.BASE_URL = "http://" + newIp;
                        etIpAddress.setText(newIp);

                        Toast.makeText(this, "✓ IP actualizada: " + newIp, Toast.LENGTH_SHORT).show();
                        Log.d(TAG, "Nueva IP guardada: " + newIp);

                        // Probar conexión con la nueva IP
                        testConnection();
                    } else {
                        Toast.makeText(this, "IP inválida", Toast.LENGTH_SHORT).show();
                        pedirNuevaIP(ssid);
                    }
                })
                .setNegativeButton("Después", null)
                .show();
    }

    /**
     * MUESTRA INFORMACIÓN DE RED DEL ESP32
     */
    private void mostrarInfoRed() {
        Toast.makeText(this, "📡 Obteniendo información...", Toast.LENGTH_SHORT).show();

        esp32Manager.getNetworkInfo(new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                try {
                    JSONObject json = new JSONObject(response);

                    boolean apMode = json.optBoolean("ap_mode", false);
                    boolean connected = json.optBoolean("connected", false);
                    String currentSsid = json.optString("ssid", "N/A");
                    String currentIp = json.optString("ip_address", "N/A");
                    String mode = json.optString("mode", "N/A");

                    String info;

                    if (apMode) {
                        int clients = json.optInt("clients_connected", 0);
                        info = "🔵 MODO ACCESS POINT\n\n" +
                                "Red: " + currentSsid + "\n" +
                                "IP: " + currentIp + "\n" +
                                "Clientes conectados: " + clients + "\n\n" +
                                "El ESP32 está en modo configuración.\n" +
                                "Configura una red WiFi para que salga de este modo.";
                    } else {
                        String signalStr = json.has("signal_strength") ?
                                json.getInt("signal_strength") + " dBm" : "N/A";

                        String estado = connected ? "✅ Conectado" : "❌ Desconectado";

                        info = "📡 INFORMACIÓN DE RED\n\n" +
                                "Estado: " + estado + "\n" +
                                "Red actual: " + currentSsid + "\n" +
                                "IP: " + currentIp + "\n" +
                                "Señal: " + signalStr + "\n" +
                                "Modo: " + mode;
                    }

                    mainHandler.post(() -> {
                        new AlertDialog.Builder(SettingsActivity.this)
                                .setTitle("📡 Red del ESP32")
                                .setMessage(info)
                                .setPositiveButton("OK", null)
                                .show();
                    });

                } catch (Exception e) {
                    Log.e(TAG, "Error parseando network info: " + e.getMessage());
                    mainHandler.post(() -> {
                        Toast.makeText(SettingsActivity.this,
                                "Error procesando respuesta", Toast.LENGTH_SHORT).show();
                    });
                }
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "Error obteniendo network info: " + error);
                mainHandler.post(() -> {
                    Toast.makeText(SettingsActivity.this,
                            "❌ No se pudo obtener información: " + error,
                            Toast.LENGTH_LONG).show();
                });
            }
        });
    }

    /**
     * Prueba la conexión con el ESP32
     */
    private void testConnection() {
        String ipAddress = etIpAddress.getText().toString().trim();

        if (ipAddress.isEmpty()) {
            Toast.makeText(this, "Ingresa la IP primero", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!isValidIpAddress(ipAddress)) {
            Toast.makeText(this, "IP inválida", Toast.LENGTH_SHORT).show();
            return;
        }

        String oldUrl = Constants.BASE_URL;
        Constants.BASE_URL = "http://" + ipAddress;

        tvConnectionStatus.setText("⏳ Conectando...");
        tvConnectionStatus.setTextColor(getResources().getColor(android.R.color.holo_orange_dark));

        esp32Manager.getStatus(new ESP32Manager.Callback() {
            @Override
            public void onSuccess(String response) {
                Log.d(TAG, "✓ Conexión exitosa");
                mainHandler.post(() -> {
                    tvConnectionStatus.setText(" Conectado correctamente");
                    tvConnectionStatus.setTextColor(getResources().getColor(android.R.color.holo_green_dark));
                    Toast.makeText(SettingsActivity.this, "✓ Conexión exitosa", Toast.LENGTH_SHORT).show();
                });
            }

            @Override
            public void onError(String error) {
                Log.e(TAG, "✗ Error de conexión: " + error);
                Constants.BASE_URL = oldUrl;
                mainHandler.post(() -> {
                    tvConnectionStatus.setText("❌ Error: " + error);
                    tvConnectionStatus.setTextColor(getResources().getColor(android.R.color.holo_red_dark));
                    Toast.makeText(SettingsActivity.this,
                            "✗ Error de conexión: " + error, Toast.LENGTH_LONG).show();
                });
            }
        });
    }

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