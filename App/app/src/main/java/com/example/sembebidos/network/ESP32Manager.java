package com.example.sembebidos.network;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import com.example.sembebidos.Constants;

import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;

/**
 * Manager para comunicarse con el ESP32
 */
public class ESP32Manager {

    private static final String TAG = "ESP32Manager";
    private Handler mainHandler;

    /**
     * Interface para callbacks de respuesta
     */
    public interface Callback {
        void onSuccess(String response);
        void onError(String error);
    }

    public ESP32Manager() {
        this.mainHandler = new Handler(Looper.getMainLooper());
    }

    /**
     * Cambia el brillo del dispositivo
     */
    public void setBrightness(int brightness, Callback callback) {
        String url = Constants.BASE_URL + "/api/brightness";
        String body = "value=" + brightness;
        Log.d(TAG, "setBrightness URL: " + url + " | Body: " + body);
        makeRequest(url, "POST", body, callback);
    }

    /**
     * Cambia el volumen del dispositivo
     */
    public void setVolume(int volume, Callback callback) {
        String url = Constants.BASE_URL + "/api/volume";
        String body = "value=" + volume;
        Log.d(TAG, "setVolume URL: " + url + " | Body: " + body);
        makeRequest(url, "POST", body, callback);
    }

    /**
     * Cambia el modo del dispositivo
     */
    public void setMode(String mode, Callback callback) {
        String url = Constants.BASE_URL + "/api/mode";
        String body = "mode=" + mode;
        Log.d(TAG, "setMode URL: " + url + " | Body: " + body);
        makeRequest(url, "POST", body, callback);
    }

    /**
     * Obtiene el estado del dispositivo
     */
    public void getStatus(Callback callback) {
        String url = Constants.BASE_URL + "/api/status";
        Log.d(TAG, "getStatus URL: " + url);
        makeRequest(url, "GET", "", callback);
    }

    /**
     * Reproduce una pista
     */
    public void playTrack(int track, Callback callback) {
        String url = Constants.BASE_URL + "/api/play";
        String body = "track=" + track;
        Log.d(TAG, "playTrack URL: " + url + " | Body: " + body);
        makeRequest(url, "POST", body, callback);
    }

    /**
     * Pausa la reproducción
     */
    public void pause(Callback callback) {
        String url = Constants.BASE_URL + "/api/pause";
        Log.d(TAG, "pause URL: " + url);
        makeRequest(url, "POST", "", callback);
    }

    /**
     * Siguiente pista
     */
    public void next(Callback callback) {
        String url = Constants.BASE_URL + "/api/next";
        Log.d(TAG, "next URL: " + url);
        makeRequest(url, "POST", "", callback);
    }

    /**
     * Pista anterior
     */
    public void previous(Callback callback) {
        String url = Constants.BASE_URL + "/api/previous";
        Log.d(TAG, "previous URL: " + url);
        makeRequest(url, "POST", "", callback);
    }

    /**
     * Cambia el color
     */
    public void setColor(int colorNumber, Callback callback) {
        String url = Constants.BASE_URL + "/api/color";
        String body = "number=" + colorNumber;
        Log.d(TAG, "setColor URL: " + url + " | Body: " + body);
        makeRequest(url, "POST", body, callback);
    }

    /**
     * Obtiene información del dispositivo
     */
    public void getDeviceInfo(Callback callback) {
        String url = Constants.BASE_URL + "/api/device-info";
        Log.d(TAG, "getDeviceInfo URL: " + url);
        makeRequest(url, "GET", "", callback);
    }

    /**
     *  CONFIGURA LA RED WIFI DEL ESP32
     */
    public void configureWiFi(String ssid, String password, Callback callback) {
        try {
            String url = Constants.BASE_URL + "/api/wifi-config";
            String body = "ssid=" + URLEncoder.encode(ssid, "UTF-8") +
                    "&password=" + URLEncoder.encode(password, "UTF-8");

            Log.d(TAG, "configureWiFi URL: " + url);
            Log.d(TAG, "configureWiFi SSID: " + ssid);
            Log.d(TAG, "configureWiFi Body: " + body);

            makeRequest(url, "POST", body, callback);

        } catch (Exception e) {
            Log.e(TAG, "Error encoding WiFi params: " + e.getMessage());
            mainHandler.post(() -> callback.onError("Error: " + e.getMessage()));
        }
    }

    /**
     *  OBTIENE INFORMACIÓN DE RED DEL ESP32
     */
    public void getNetworkInfo(Callback callback) {
        String url = Constants.BASE_URL + "/api/network-info";
        Log.d(TAG, "getNetworkInfo URL: " + url);
        makeRequest(url, "GET", "", callback);
    }

    /**
     * Realiza una petición HTTP al ESP32
     */
    private void makeRequest(String urlString, String method, String body, Callback callback) {
        new Thread(() -> {
            HttpURLConnection connection = null;
            try {
                Log.d(TAG, "Realizando petición: " + method + " " + urlString);

                URL url = new URL(urlString);
                connection = (HttpURLConnection) url.openConnection();
                connection.setRequestMethod(method);
                connection.setConnectTimeout(5000);
                connection.setReadTimeout(5000);
                connection.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
                connection.setRequestProperty("Accept", "application/json");

                // Si es POST con body, enviarlo
                if ("POST".equals(method) && body != null && !body.isEmpty()) {
                    connection.setDoOutput(true);
                    try (OutputStream os = connection.getOutputStream()) {
                        byte[] input = body.getBytes("utf-8");
                        os.write(input, 0, input.length);
                    }
                }

                int responseCode = connection.getResponseCode();
                Log.d(TAG, "Response Code: " + responseCode);

                if (responseCode == HttpURLConnection.HTTP_OK ||
                        responseCode == HttpURLConnection.HTTP_CREATED) {

                    BufferedReader reader = new BufferedReader(
                            new InputStreamReader(connection.getInputStream()));
                    StringBuilder response = new StringBuilder();
                    String line;

                    while ((line = reader.readLine()) != null) {
                        response.append(line);
                    }
                    reader.close();

                    String result = response.toString();
                    Log.d(TAG, "Response: " + result);
                    mainHandler.post(() -> callback.onSuccess(result));

                } else {
                    String errorMsg = "Error HTTP: " + responseCode;
                    Log.e(TAG, errorMsg);
                    mainHandler.post(() -> callback.onError(errorMsg));
                }

            } catch (Exception e) {
                String errorMsg = "Conexión fallida: " + e.getMessage();
                Log.e(TAG, errorMsg, e);
                mainHandler.post(() -> callback.onError(errorMsg));

            } finally {
                if (connection != null) {
                    connection.disconnect();
                }
            }
        }).start();
    }
}