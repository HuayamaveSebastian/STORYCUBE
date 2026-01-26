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
  - [Lista de Audios](#Audios)
  - [Instalación App](#APP)
  - [Funcionamiento/Uso](#Funcionamiento/Uso)
  - [Autores](#Autores)

## Objetivo
Desarrollar un cubo interactivo educativo que fomente el aprendizaje y la lectura en niños mediante tecnología RFID y estímulos visuales y auditivos.

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

### Estados
*Revisar* 
<img width="614" height="402" alt="image" src="https://github.com/user-attachments/assets/245d8415-36b8-480b-a2d1-1ff906e08ab1" />

### Esquemático
<p align="center">
<img width="627" height="317" alt="image" src="https://github.com/user-attachments/assets/4d95cc4e-7ee7-4667-ac1f-76abca1f6e49" />
</p>
  
  **Consideraciones:**  Tener en cuenta que en este esquemático se utiliza el puerto de carga **TP4056** y que, para la implementación física, se usó el módulo de carga **SM5308**, el cual funciona con un pulsador. Por ende, se colocó un pulsador como activador de todo el proyecto.

## Audios
En esta sección se explica cómo está organizada la carpeta **"MP3"** de la tarjeta SD utilizada en el módulo **DFPlayer Mini**, donde se almacenan los archivos de audio numerados (001, 002, etc.).

  **Cuentos:**
   - 001 - Pinocho
   - 002 - Peter Pan
   - 003 - Ricitos de Oro
   - 004 - Los tres Cerditos
   - 005 - La tortuga y el conejo
   - 006 - Caperucita Roja

**Modos del Cubo**  
   - 007 - Modo RFID
   - 008 - Modo Números
   - 009 - Modo Colores
   - 027 - Modo Manual
   - 028 - Cambio a sub-modo Español en el modo números
   - 029 - Cambio a sub-modo Inglés en el modo números

**Números**
   - 010 - Número 0
   - 011 - Número 1
   - 012 - Número 2
   - 013 - Número 3
   - 014 - Número 4
   - 015 - Número 5
   - 016 - Número 6
   - 017 - Número 7
   - 018 - Número 8
   - 019 - Número 9

**Colores**
   - 021 - Rojo
   - 022 - Verde
   - 023 - Azul
   - 024 - Amarillo
   - 025 - Morado
   - 026 - Cyan

   **Números en Ingles**
   - 030 - Zero
   - 031 - One
   - 032 - Two
   - 033 - Three
   - 034 - Four
   - 035 - Five
   - 036 - Six
   - 037 - Seven
   - 038 - Eight
   - 039 - Nine

## APP

## Funcionamiento/Uso
El cubo interactivo cuenta con un boton en parte trasera el cual es el botón de ON/OFF, para encenderlo presionamos una vez y para poder apagarlo cuando ya esta en operacion debemos pulsar 2 veces.
Al estar activo el cubo tarda un poco en comenzar a operar inmediatamente, ya que esta conectandose a una red wifi o por su defecto, esta creando su propia red Wifi.

Cuando se muestre en la matriz led el mensaje de "STORYCUBE" iniciara directamente con el **MODO RFID** , por ende al colocar los muñequitos en su respectiva posición, reproducira el audio respectivo del cuento. 
Ahora, si se quiere cambiar del **MODO RFID** a los otros modos, nos apoyamos del teclado Matricial(3x4), nos dirigimos al simbolo "*", el cual al presionar 1 vez, nos cambia al siguiente Modo, el cual es **MODO MANUAL** (Sabremos que cambiamos de modo ya que nuestro cubo nos mencionara en que modo estamos). 

En el **MODO MANUAL** podremos elegir el cuento preferido mediante los botones de "Siguiente" y "Atrás". Al tocar por segunda vez el boton "*" del teclado matricial nos dirigimos al siguiente Modo: **MODO NUMEROS**.

En el **MODO NUMEROS** disponemos del Sub-Modo: "Números en Ingles" el cual para poder acceder a este Sub-modo se presiona el boton "#" del teclado Matricial, y para volver a retornar al Sub-modo Español volvemos a presionar de vuelta, dependiendo en que Sub-modo este, al presionar cualquier número del teclado matricial se mostrara en la pantalla el número y a su vez se reproducira el audio repectivo de dicho número. 

Para acceder al **MODO COLORES** presionamos por tercera vez el "*", en este modo tendremos que usar el mismo teclado matricial para mostrarnos los colores en pantalla, estos colores podran ser selccionados presionando los numeros del 1 al 6, al presionar cualquiera de esos numeros, se observara el color en la matriz y a su vez se reproducira un audio diciendo cual es el color. 

Y para finalizar, al presionar por una cuarta vez "*" regresamos al primer modo el **MODO RFID** .

## Autores
  - Sebastian Huayamave
  - Steven Yari
