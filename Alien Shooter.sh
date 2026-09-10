#!/bin/bash
# PortMaster / ArkOS / dArkOS / NextOS launcher for Alien Shooter v1.2.9
# Target: R36S (RK3326 ARM64)

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}

if [ -d "/opt/system/Tools/PortMaster/" ]; then
  CONTROL_FOLDER="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  CONTROL_FOLDER="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  CONTROL_FOLDER="$XDG_DATA_HOME/PortMaster"
else
  CONTROL_FOLDER="/roms/ports/PortMaster"
fi

[ -f "${CONTROL_FOLDER}/control.txt" ] && source "${CONTROL_FOLDER}/control.txt"
[ -f "${CONTROL_FOLDER}/mod_downloader.txt" ] && source "${CONTROL_FOLDER}/mod_downloader.txt"

[ -n "$(type -t get_controls)" ] && get_controls

GAMEDIR="/$directory/ports/alienport"
[ ! -d "$GAMEDIR" ] && GAMEDIR="$(cd "$(dirname "$0")"/alienport && pwd)"
[ ! -d "$GAMEDIR" ] && GAMEDIR="$(cd "$(dirname "$0")" && pwd)"

cd "$GAMEDIR"

# 1. Verificação / Instalação Híbrida de Dados (APK ou Pasta Pronta)
if [ ! -f "lib/arm64-v8a/libalien_shooter.so" ] || [ ! -d "assets" ]; then
    APK_FILE=$(ls -1 *.apk 2>/dev/null | head -n 1)
    if [ -n "$APK_FILE" ] && [ -f "$APK_FILE" ]; then
        echo "Extraindo dados do jogo a partir de $APK_FILE..."
        if command -v unzip >/dev/null 2>&1; then
            unzip -o -q "$APK_FILE" "assets/*" "lib/arm64-v8a/*" -d .
        fi
    fi
fi

# Validação final dos dados necessários
if [ ! -f "lib/arm64-v8a/libalien_shooter.so" ]; then
    if [ -n "$ESUDO" ] && [ -n "$CUR_TTY" ]; then
        $ESUDO $CUR_TTY ./tools/dialog --title "Erro Alien Shooter" \
            --msgbox "Arquivos do jogo não encontrados!\nColoque a pasta 'assets' e 'lib' ou o arquivo .apk dentro de:\n$GAMEDIR" 0 0
    else
        echo "ERRO: lib/arm64-v8a/libalien_shooter.so não encontrado em $GAMEDIR"
    fi
    exit 1
fi

# Cria diretório de saves portátil se não existir
mkdir -p "$GAMEDIR/savedata"

# 2. Variáveis de ambiente
export LD_LIBRARY_PATH="$GAMEDIR/lib:$LD_LIBRARY_PATH"
[ -n "$sdl_controllerconfig" ] && export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

# 3. Execução
if [ -n "$GPTOKEYB" ]; then
    $GPTOKEYB "alienport" &
fi

./alienport "$GAMEDIR"

# 4. Finalização e limpeza
if [ -n "$ESUDO" ]; then
    $ESUDO killall -9 gptokeyb 2>/dev/null || true
fi

unset LD_LIBRARY_PATH
printf "\033c" > /dev/tty0 2>/dev/null || true
