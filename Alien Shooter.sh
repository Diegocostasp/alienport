#!/bin/bash
CURR_TTY="/dev/tty1"
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
GAME_DIR="$SCRIPT_DIR/alienport"
BINARY="$GAME_DIR/alienport"
LOG="$GAME_DIR/log.txt"

# Garante que o log existe e limpa ele para nova sessão
mkdir -p "$GAME_DIR"
echo "=== Script iniciado: $(date) ===" > "$LOG"
echo "GAME_DIR=$GAME_DIR" >> "$LOG"
echo "BINARY=$BINARY" >> "$LOG"

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

if [ -f "$controlfolder/control.txt" ]; then
  source "$controlfolder/control.txt"
  get_controls
fi

exec < "$CURR_TTY"

if [ ! -f "$BINARY" ]; then
  echo "ERRO: binario nao encontrado em $BINARY" >> "$LOG"
  printf "\033c" > "$CURR_TTY"
  printf "Error: alienport binary not found at %s\n" "$BINARY" > "$CURR_TTY"
  sleep 5
  exit 1
fi

chmod +x "$BINARY"
mkdir -p "$GAME_DIR/data"

export LD_LIBRARY_PATH="$GAME_DIR/lib/arm64-v8a:$GAME_DIR:$LD_LIBRARY_PATH"
export SDL_VIDEODRIVER=kmsdrm
export SDL_AUDIODRIVER=alsa
export ALSOFT_DRIVERS=alsa
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"
export MALLOC_CHECK_=0

export MESA_GL_VERSION_OVERRIDE=2.1
export MESA_GLES_VERSION_OVERRIDE=2.0
export SDL_VIDEO_GL_DRIVER=libGLESv2.so
export SDL_VIDEO_EGL_DRIVER=libEGL.so

printf "\033c" > "$CURR_TTY"
printf "\e[?25l" > "$CURR_TTY"
echo 0 > /sys/class/vtconsole/vtcon0/bind 2>/dev/null || true
echo 0 > /sys/class/vtconsole/vtcon1/bind 2>/dev/null || true

sync
echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null 2>&1 || true

cd "$GAME_DIR"
echo "=== Iniciando binario ===" >> "$LOG"

# Carrega o shim de pthread apenas se existir
if [ -f "$GAME_DIR/libpthread_preload.so" ]; then
  echo "Carregando LD_PRELOAD=$GAME_DIR/libpthread_preload.so" >> "$LOG"
  export LD_PRELOAD="$GAME_DIR/libpthread_preload.so"
else
  echo "libpthread_preload.so NAO encontrado, rodando sem preload" >> "$LOG"
fi

"$BINARY" >> "$LOG" 2>&1
EXIT_CODE=$?
echo "=== Saiu com codigo: $EXIT_CODE ===" >> "$LOG"

echo 1 > /sys/class/vtconsole/vtcon0/bind 2>/dev/null || true
echo 1 > /sys/class/vtconsole/vtcon1/bind 2>/dev/null || true
printf "\033c" > "$CURR_TTY"
printf "\e[?25h" > "$CURR_TTY"
exit $EXIT_CODE
