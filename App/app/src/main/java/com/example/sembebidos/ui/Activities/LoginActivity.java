package com.example.sembebidos.ui.Activities;

import static android.content.ContentValues.TAG;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.example.sembebidos.MainActivity;
import com.example.sembebidos.R;

public class LoginActivity extends AppCompatActivity {

    // Vistas
    private EditText etEmail, etPassword;
    private Button btnLogin, btnRegister;
    private ProgressBar progressBar;
    private TextView tvForgotPassword;

    // Firebase
    private FirebaseAuth mAuth;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_login);

        // Inicializar Firebase Auth
        mAuth = FirebaseAuth.getInstance();

        // Verificar si ya hay usuario logueado
        FirebaseUser currentUser = mAuth.getCurrentUser();
        if (currentUser != null) {
            // Ya está logueado, ir a la siguiente pantalla
            goToNextActivity();
            return;
        }

        // Inicializar vistas
        initViews();

        // Configurar listeners
        setupListeners();
    }

    private void initViews() {
        etEmail = findViewById(R.id.etEmail);
        etPassword = findViewById(R.id.etPassword);
        btnLogin = findViewById(R.id.btnLogin);
        btnRegister = findViewById(R.id.btnRegister);
        progressBar = findViewById(R.id.progressBar);
        tvForgotPassword = findViewById(R.id.tvForgotPassword);
    }

    private void setupListeners() {
        btnLogin.setOnClickListener(v -> loginUser());
        btnRegister.setOnClickListener(v -> registerUser());
        tvForgotPassword.setOnClickListener(v -> resetPassword());
    }

    /**
     * Login del usuario con email y contraseña
     */
    private void loginUser() {
        String email = etEmail.getText().toString().trim();
        String password = etPassword.getText().toString().trim();

        // Validaciones
        if (TextUtils.isEmpty(email)) {
            etEmail.setError("El email es requerido");
            return;
        }

        if (TextUtils.isEmpty(password)) {
            etPassword.setError("La contraseña es requerida");
            return;
        }

        if (password.length() < 6) {
            etPassword.setError("La contraseña debe tener al menos 6 caracteres");
            return;
        }

        // Mostrar loading
        showLoading(true);

        // Login con Firebase
        mAuth.signInWithEmailAndPassword(email, password)
                .addOnCompleteListener(this, task -> {
                    showLoading(false);

                    if (task.isSuccessful()) {
                        // Login exitoso
                        Toast.makeText(LoginActivity.this,
                                "¡Bienvenido!", Toast.LENGTH_SHORT).show();

                        goToNextActivity();
                    } else {
                        // Error en login
                        Toast.makeText(LoginActivity.this,
                                "Error: " + task.getException().getMessage(),
                                Toast.LENGTH_LONG).show();
                    }
                });
    }

    /**
     * Registro del usuario con email y contraseña
     */
    private void registerUser() {
        String email = etEmail.getText().toString().trim();
        String password = etPassword.getText().toString().trim();

        // Validaciones
        if (TextUtils.isEmpty(email)) {
            etEmail.setError("El email es requerido");
            return;
        }

        if (TextUtils.isEmpty(password)) {
            etPassword.setError("La contraseña es requerida");
            return;
        }

        if (password.length() < 6) {
            etPassword.setError("La contraseña debe tener al menos 6 caracteres");
            return;
        }

        // Mostrar loading
        showLoading(true);

        // Registro con Firebase
        mAuth.createUserWithEmailAndPassword(email, password)
                .addOnCompleteListener(this, task -> {
                    showLoading(false);

                    if (task.isSuccessful()) {
                        // Registro exitoso
                        Toast.makeText(LoginActivity.this,
                                "¡Cuenta creada exitosamente!", Toast.LENGTH_SHORT).show();

                        goToNextActivity();
                    } else {
                        // Error en registro
                        Toast.makeText(LoginActivity.this,
                                "Error: " + task.getException().getMessage(),
                                Toast.LENGTH_LONG).show();
                    }
                });
    }

    /**
     * Envía email para resetear contraseña
     */
    private void resetPassword() {
        String email = etEmail.getText().toString().trim();

        if (TextUtils.isEmpty(email)) {
            etEmail.setError("Ingresa tu email");
            return;
        }

        showLoading(true);

        mAuth.sendPasswordResetEmail(email)
                .addOnCompleteListener(task -> {
                    showLoading(false);

                    if (task.isSuccessful()) {
                        Toast.makeText(LoginActivity.this,
                                "Email de recuperación enviado", Toast.LENGTH_SHORT).show();
                    } else {
                        Toast.makeText(LoginActivity.this,
                                "Error: " + task.getException().getMessage(),
                                Toast.LENGTH_LONG).show();
                    }
                });
    }

    /**
     * Muestra o oculta el loading
     */
    private void showLoading(boolean show) {
        if (show) {
            progressBar.setVisibility(View.VISIBLE);
            btnLogin.setEnabled(false);
            btnRegister.setEnabled(false);
        } else {
            progressBar.setVisibility(View.GONE);
            btnLogin.setEnabled(true);
            btnRegister.setEnabled(true);
        }
    }

    /**
     * Navega según el estado de configuración WiFi
     */
    private void goToNextActivity() {
        SharedPreferences prefs = getSharedPreferences("StoryCube", Context.MODE_PRIVATE);

        // Verificar si WiFi fue configurado (no solo mostrado)
        boolean wifiConfigurado = prefs.getBoolean("wifi_configured", false);

        if (!wifiConfigurado) {
            // Primera vez o WiFi no configurado: mostrar WifiSetup
            Log.d(TAG, "WiFi no configurado - mostrando WifiSetup");
            try {
                Intent intent = new Intent(LoginActivity.this, WifiSetup.class);
                startActivity(intent);
            } catch (Exception e) {
                // Si WifiSetup no existe, ir a MainActivity
                Log.e(TAG, "WifiSetup no encontrado: " + e.getMessage());
                Intent intent = new Intent(LoginActivity.this, MainActivity.class);
                startActivity(intent);
            }
        } else {
            // WiFi ya configurado: ir directo a MainActivity
            Log.d(TAG, "WiFi configurado - ir a MainActivity");
            Intent intent = new Intent(LoginActivity.this, MainActivity.class);
            startActivity(intent);
        }
        finish();
    }
}