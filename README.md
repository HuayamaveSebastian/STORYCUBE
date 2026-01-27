# STORYCUBE
<p align="center">
<img width="350" height="298" alt="image" src="Imagenes/STORYCUBE.jpg" />
</p>

## Software
  - Visual Studio Code con extensión PlatformIO
  - Android Studio (para desarrollo de app)


## Tabla de Contenido
  - [Objetivo](#Objetivo)
  - [Componentes](#Componentes)
  - [Diagramas](#Diagramas)
      - [Bloques](#Bloques)
      - [Estados](#Estados)
      - [Esquemático](#Esquemático)
  - [Estructura de la carpeta MP3](##Carpeta_MP3)
  - [Instalación App](#APP)
  - [Funcionamiento/Uso](#Funcionamiento/Uso)
  - [Autores](#Autores)

## Objetivo
Este repositorio tiene los archivos y la información detallada para poder desarrollar un cubo interactivo educativo que fomente el aprendizaje y la lectura en niños mediante tecnología RFID y estímulos visuales y auditivos.

## Componentes
   - Placa electrónica ESP32
   - Módulo RFID RC522
   - Llaveros RFID
   - Matriz WS2812B(8x8)
   - DFPlayer MP3 Mini
   - Micro SD 16GB
   - Teclado de membrana matricial(3x4)
   - Mini Parlante 8 ohmios
   - Boton Pulsador
   - Borneras
   - Espadines macho y hembra
   - Resistencias
   - Potenciómetro 10K
   - Modulo SM5308
   - Baterias Litio 3.7V Recargables 
     
## Diagramas

### Bloques
<p align="center">
<img width="756" height="378" alt="image" src="https://github.com/user-attachments/assets/2c16c149-39e0-4db7-b904-a33820e4356c" />
</p>

### Estados
<p align="center">
<img width="500" height="307" alt="image" src="https://github.com/user-attachments/assets/11372bf9-558b-4379-856e-52c16e529ecf" />
</p>

### Sistema
<p align="center">
<img width="627" height="317" alt="image" src="https://github.com/user-attachments/assets/4d95cc4e-7ee7-4667-ac1f-76abca1f6e49" />
</p>
  
  **Consideraciones:**  Tener en cuenta que en este esquemático se utiliza el puerto de carga **TP4056**; sin embargo, para la implementación física se empleó el módulo de carga **SM5308**, el cual opera mediante un pulsador. Por esta razón, se incorporó un pulsador como elemento activador de todo el proyecto.

## Carpeta_MP3
En esta sección se explica cómo está organizada la carpeta "MP3" de la tarjeta SD utilizada en el módulo DFPlayer Mini.

Para ver la lista completa de archivos de audio y su organización, consulta el documento:[Estructura detallada de MP3](./MP3_STRUCTURE.md) 

## APP
- [Descargar la aplicación](./STORYCUBE.apk) - Descarga la app para instalar en tu dispositivo movil. 
- [Manual de uso](./MANUAL_APPSTORYCUBE.md) - Guía completa sobre cómo utilizar la aplicación
## Funcionamiento/Uso
El cubo interactivo cuenta con un botón ON/OFF en la parte trasera. Para encenderlo se presiona una vez y, para apagarlo durante la operación, se presiona dos veces. Al activarse, el cubo tarda unos segundos en iniciar, ya que se conecta a una red WiFi o crea una propia.

Al mostrarse el mensaje “STORYCUBE” en la matriz LED, el sistema inicia en MODO RFID, donde al colocar los muñecos en su posición correspondiente se reproduce el audio del cuento.

El cambio de modos se realiza mediante el teclado matricial (3x4), presionando el símbolo “*”. El segundo modo es el MODO MANUAL, en el cual se seleccionan los cuentos con los botones de “Siguiente” y “Atrás”.

Al presionar nuevamente “*” se accede al MODO NÚMEROS, que incluye los submodos Español e Inglés, alternables con el botón “#”. Al presionar un número, este se muestra en la matriz LED y se reproduce su audio correspondiente.

Con una tercera pulsación de “*” se ingresa al MODO COLORES, donde los colores se seleccionan con los números del 1 al 6, mostrando el color y reproduciendo su audio.

Finalmente, al presionar “*” por cuarta vez, el sistema regresa al MODO  inicial RFID.

## Autores
  - Sebastian Huayamave
  - Steven Yari
