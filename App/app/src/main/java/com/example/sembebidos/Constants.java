package com.example.sembebidos;

/**
 * Constantes de la aplicación
 */
public class Constants {

    // Base URL del ESP32 (se actualiza dinámicamente desde SharedPreferences)
    public static String BASE_URL = "http://192.168.1.100";

    // ==================== ENDPOINTS ====================
    // Status
    public static final String ENDPOINT_STATUS = "/api/status";

    // Control de volumen
    public static final String ENDPOINT_VOLUME = "/api/volume";

    // Control de brillo
    public static final String ENDPOINT_BRIGHTNESS = "/api/brightness";

    // Reproducción
    public static final String ENDPOINT_PLAY = "/api/play";
    public static final String ENDPOINT_PAUSE = "/api/pause";
    public static final String ENDPOINT_NEXT = "/api/next";
    public static final String ENDPOINT_PREV = "/api/previous";

    // Modo
    public static final String ENDPOINT_MODE = "/api/mode";

    // Color
    public static final String ENDPOINT_COLOR = "/api/color";

    // Información del dispositivo
    public static final String ENDPOINT_DEVICE_INFO = "/api/device-info";

    // ==================== MODOS ====================
    public static final String MODO_RFID = "RFID";
    public static final String MODO_MANUAL = "Manual";
    public static final String MODO_NUMEROS = "Números";
    public static final String MODO_COLORES = "Colores";
}