#!/bin/bash

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

source $CONTROL_FOLDER/control.txt
[ -f "${CONTROL_FOLDER}/mod_downloader.txt" ] && source "${CONTROL_FOLDER}/mod_downloader.txt"

get_controls

GAMEDIR="/$directory/ports/alienport"
cd $GAMEDIR

# Check for assets
if [ ! -f "lib/arm64-v8a/libalien_shooter.so" ]; then
    $ESUDO $CUR_TTY ./tools/dialog --title "Erro" --msgbox "Arquivos do jogo não encontrados em lib/arm64-v8a/" 0 0
    exit 1
fi

export LD_LIBRARY_PATH="$GAMEDIR/lib:$LD_LIBRARY_PATH"
export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

$GPTOKEYB "alienport" &
./alienport
$ESUDO killall -9 gptokeyb
unset LD_LIBRARY_PATH
printf "\033c" > /dev/tty0
