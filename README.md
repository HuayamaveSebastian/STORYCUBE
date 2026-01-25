# STORYCUBE
<p align="center">
<img width="650" height="598" alt="image" src="Imagenes/STORYCUBE.jpg" />
</p>

## Tabla de Contenido
  - [Objetivo](#Objetivo)
  - [Componentes](#Componentes)
  - [Esquemático](#Esquemático)
  - [Autores](#Autores)
## Objetivo
El proyecto tiene como finalidad combinar el aprendizaje con la diversión a través de la
tecnología. Mediante un cubo interactivo educativo, los niños podrán acceder a cuentos
infantiles personalizados al acercar un muñeco con tecnología RFID, que activará efectos
visuales y auditivos. Este dispositivo busca fomentar el hábito de la lectura y la curiosidad
tecnológica en edades tempranas(niños) convirtiéndose en una herramienta interactiva que
estimula la comprensión auditiva, la atención y la imaginación.

## Componentes
   - Placa electrónica ESP32
   - Módulo RFID RC522 y su respectivo TAG
   - Matriz WS2812B(8x8)
   - DFPlayer MP3 Mini
   - Teclado de membrana matricial(3x4)
   - Mini Parlante
   - Pulsador
   - Resistencias
   - Potenciómetro
   - Modulo SM5308
     
## Esquemático
<p align="center">
<img width="827" height="517" alt="image" src="https://github.com/user-attachments/assets/4d95cc4e-7ee7-4667-ac1f-76abca1f6e49" />
</p>
  - Consideraciones: Tener en cuenta que en este esquematico se usa el puerto de carga TP4056 y para la implementacion fisica Se uso el modulo de carga SM5308
  el cual funciona con un pulsador, por ende el porque colocamos un pulsador como activador de todo nuestro proyecto.

## Lista de Audios
En esta sección se explica cómo está organizada la carpeta **"MP3"** de la tarjeta SD utilizada en el módulo **DFPlayer Mini**, donde se almacenan los archivos de audio numerados (001, 002, etc.) y su función  dentro del proyecto.

## Autores
  - Sebastian Huayamave
  - Steven Yari
