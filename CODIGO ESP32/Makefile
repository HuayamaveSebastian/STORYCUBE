# ==================== MAKEFILE STORYCUBE ====================
# Proyecto: StoryCube - Cuéntacuentos Interactivo
# Placa: MH-ET LIVE MiniKit ESP32
# Propósito: Compilar y cargar código en ESP32
# ============================================================

# ==================== CONFIGURACIÓN ====================

# Puerto serial (ajusta según tu sistema)
# Linux:   /dev/ttyUSB0, /dev/ttyUSB1
# macOS:   /dev/cu.usbserial-XXXXX
# Windows: COM3, COM4, etc.
PORT ?= /dev/ttyUSB0

# Velocidad de baudrate para monitoreo
BAUD_RATE = 115200

# Entorno de PlatformIO
ENV = mhetesp32devkit

# ==================== COMANDOS PRINCIPALES ====================

.PHONY: all build upload monitor clean help upload-monitor check-port check-pio

# Verificar si PlatformIO está instalado
check-pio:
	@command -v pio >/dev/null 2>&1 || { \
		echo "❌ PlatformIO no está instalado."; \
		echo "📦 Instálalo desde VS Code o ejecuta:"; \
		echo "   pip install platformio"; \
		exit 1; \
	}

# Comando por defecto: compilar y cargar
all: check-pio build upload
	@echo "✓ ¡Compilación y carga completadas!"

# Compilar el proyecto
build: check-pio
	@echo "🔨 Compilando StoryCube para $(ENV)..."
	@pio run -e $(ENV)
	@echo "✓ Compilación exitosa"

# Cargar en el ESP32
upload: check-pio build
	@echo "📤 Cargando firmware en ESP32 (puerto: $(PORT))..."
	@pio run -e $(ENV) --target upload --upload-port $(PORT)
	@echo "✓ Firmware cargado correctamente"

# Monitorear puerto serial
monitor: check-pio
	@echo "📡 Monitoreando puerto serial ($(PORT)) a $(BAUD_RATE) baud..."
	@echo "   Presiona Ctrl+C para salir"
	@pio device monitor -p $(PORT) -b $(BAUD_RATE) -e $(ENV)

# Cargar y monitorear en una sola acción
upload-monitor:
	@make upload
	@echo ""
	@echo "Abriendo monitor serial..."
	@sleep 2
	@make monitor

# Compilar sin cargar
build-only: check-pio
	@echo "🔨 Compilando solo (sin cargar)..."
	@pio run -e $(ENV)

# Limpiar archivos compilados
clean: check-pio
	@echo "🧹 Limpiando archivos compilados..."
	@pio run -e $(ENV) --target clean
	@echo "✓ Limpieza completada"

# Verificar puerto serial disponible
check-port: check-pio
	@echo "🔍 Puertos seriales disponibles:"
	@pio device list
	@echo ""
	@echo "Si tu puerto es diferente, usa:"
	@echo "  make upload PORT=/dev/ttyUSB1"
	@echo "  make upload PORT=COM4"

# Mostrar ayuda
help:
	@echo ""
	@echo "╔════════════════════════════════════════════════════════╗"
	@echo "║     STORYCUBE - Makefile para ESP32 MH-ET DevKit       ║"
	@echo "╚════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "📋 COMANDOS DISPONIBLES:"
	@echo ""
	@echo "  make              → Compilar y cargar (predeterminado)"
	@echo "  make all          → Compilar y cargar"
	@echo "  make build        → Solo compilar"
	@echo "  make build-only   → Compilar sin cargar"
	@echo "  make upload       → Compilar y cargar"
	@echo "  make monitor      → Abrir monitor serial"
	@echo "  make upload-monitor → Cargar y monitorear"
	@echo "  make clean        → Limpiar archivos compilados"
	@echo "  make check-port   → Ver puertos seriales disponibles"
	@echo "  make help         → Mostrar esta ayuda"
	@echo ""
	@echo "⚙️  CONFIGURACIÓN ACTUAL:"
	@echo ""
	@echo "  Puerto:     $(PORT)"
	@echo "  Baudrate:   $(BAUD_RATE)"
	@echo "  Entorno:    $(ENV)"
	@echo "  Placa:      MH-ET LIVE MiniKit ESP32"
	@echo ""
	@echo "📝 EJEMPLOS DE USO:"
	@echo ""
	@echo "  # Compilar y cargar automáticamente"
	@echo "  $$ make"
	@echo ""
	@echo "  # Cargar en puerto específico"
	@echo "  $$ make upload PORT=/dev/ttyUSB1"
	@echo ""
	@echo "  # Monitorear salida"
	@echo "  $$ make monitor"
	@echo ""
	@echo "  # Cargar y ver output en tiempo real"
	@echo "  $$ make upload-monitor"
	@echo ""
	@echo "  # Limpiar y recompilar desde cero"
	@echo "  $$ make clean && make build"
	@echo ""
	@echo "🔧 SOLUCIONAR PROBLEMAS:"
	@echo ""
	@echo "  Puerto no reconocido:"
	@echo "    $$ make check-port"
	@echo ""
	@echo "  Librerías no encontradas:"
	@echo "    $$ pio run -e $(ENV)  (descarga automáticamente)"
	@echo ""

# ==================== COMANDOS ADICIONALES ====================

# Actualizar librerías
update-libs: check-pio
	@echo "📚 Actualizando librerías..."
	@pio lib update
	@echo "✓ Librerías actualizadas"

# Ver información del proyecto
info: check-pio
	@echo "📊 Información del proyecto:"
	@pio project config

# Reinstalar librerías
reinstall-libs: check-pio
	@echo "🔄 Reinstalando librerías..."
	@pio run -e $(ENV) --target clean
	@pio lib install --force
	@echo "✓ Librerías reinstaladas"

.PHONY: update-libs info reinstall-libs
