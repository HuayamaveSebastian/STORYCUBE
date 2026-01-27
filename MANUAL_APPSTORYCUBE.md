Descargando y accediendo con las credenciales

Correo: steven@example.com

Password : steven

## Configuración de red del dispositivo

Al encender el dispositivo, la ESP32 realiza una de las siguientes acciones:

1. **Búsqueda de red guardada**  
   La ESP32 intenta conectarse automáticamente a una red WiFi previamente almacenada.

2. **Creación de red propia (modo configuración)**  
   Si no encuentra una red guardada, la ESP32 crea una red WiFi propia llamada:  
   **`StoryCube-set-up`**

3. **Conexión desde la aplicación**  
   - El usuario se conecta a la red `StoryCube-set-up` desde el teléfono.
   - La ESP32 muestra la **dirección IP** que debe ingresarse en el apartado de configuraciones de la aplicación.
   - Se puede verificar el estado de la conexión utilizando la opción **“PROBAR CONEXIÓN”**.

4. **Uso del dispositivo con diferentes redes**  
   Desde la aplicación se puede guardar una red WiFi del domicilio o del teléfono para que la ESP32 se conecte automáticamente en futuros encendidos.

5. **Uso del dispositivo sin red externa**  
   En caso de no configurar una red WiFi externa:
   - El usuario se conecta directamente a la red creada por la ESP32.
   - Se ingresa la IP del dispositivo en la aplicación.
   - Se guarda la configuración y el dispositivo puede usarse correctamente **sin conexión a internet**.

> Esta configuración se realiza principalmente durante el primer uso del dispositivo o cuando no se cambia de ubicación, permitiendo una conexión rápida y sencilla.

