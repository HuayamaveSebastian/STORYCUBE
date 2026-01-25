# STORYCUBE
<p align="center">
<img width="450" height="398" alt="image" src="Imagenes/STORYCUBE.jpg" />
</p>

## Tabla de Contenido
  - [Objetivo](#Objetivo)
  - [Funcionamiento](#Funcionamiento)
  - [Componentes](#Componentes)
  - [Esquemático](#Esquemático)
  - [Lista de Audios](#Audios)
  - [Instalación App](#APP)
  - [Autores](#Autores)

## Objetivo
El proyecto tiene como finalidad combinar el aprendizaje con la diversión a través de la
tecnología. Mediante un cubo interactivo educativo, los niños podrán acceder a cuentos
infantiles personalizados al acercar un muñeco con tecnología RFID, que activará efectos
visuales y auditivos. Este dispositivo busca fomentar el hábito de la lectura y la curiosidad
tecnológica en edades tempranas(niños) convirtiéndose en una herramienta interactiva que
estimula la comprensión auditiva, la atención y la imaginación.

## Funcionamiento

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
     
## Esquemático
<p align="center">
<img width="827" height="517" alt="image" src="https://github.com/user-attachments/assets/4d95cc4e-7ee7-4667-ac1f-76abca1f6e49" />
</p>
  **Consideraciones:** Tener en cuenta que en este esquemático se utiliza el puerto de carga **TP4056** y que, para la implementación física, se usó el módulo de carga **SM5308**, el cual funciona con un pulsador. Por ende, se colocó un pulsador como activador de todo el proyecto.

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


## Autores
  - Sebastian Huayamave
  - Steven Yari
