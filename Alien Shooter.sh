#!/bin/bash
# ==============================================================================
# PortMaster / ArkOS / dArkOS / NextOS launcher para Alien Shooter v1.2.9
# Dispositivo Alvo: R36S (RK3326 ARM64)
# ==============================================================================

# 1. Determina diretório real do launcher
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CUR_TTY="/dev/tty1"
[ -w "/dev/tty0" ] && CUR_TTY="/dev/tty0"

# 2. Inicia LOG IMEDIATAMENTE (antes de qualquer include ou comando arriscado)
LOG_ROOT="$SCRIPT_DIR/alienport_log.txt"
LOG_TMP="/tmp/alienport.log"
exec > >(tee -a "$LOG_ROOT" "$LOG_TMP") 2>&1

echo "=============================================="
echo "Início do Launcher Alien Shooter: $(date)"
echo "SCRIPT_DIR: $SCRIPT_DIR"
echo "Linha de comando: $0 $@"
echo "Terminal atual: $CUR_TTY"
echo "=============================================="

# 3. PortMaster Control Setup (apenas o essencial)
XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}
controlfolder=""

for cand in \
  "/opt/system/Tools/PortMaster" \
  "/opt/tools/PortMaster" \
  "$XDG_DATA_HOME/PortMaster" \
  "/roms/ports/PortMaster" \
  "/roms2/ports/PortMaster" \
  "/storage/roms/ports/PortMaster" \
  "$SCRIPT_DIR/PortMaster"; do
  if [ -f "$cand/control.txt" ]; then
    controlfolder="$cand"
    break
  fi
done

if [ -n "$controlfolder" ]; then
  echo "PortMaster encontrado em: $controlfolder"
  source "$controlfolder/control.txt"
  [ -n "$(type -t get_controls)" ] && get_controls
else
  echo "AVISO: PortMaster control.txt não encontrado, usando padrões locais."
fi

# 4. Localização flexível e robusta da pasta do jogo
GAMEDIR=""
for cand in \
  "$SCRIPT_DIR/alienport" \
  "$SCRIPT_DIR/Alien-Shooter-v1-2-9-patched" \
  "/$directory/ports/alienport" \
  "/roms/ports/alienport" \
  "/roms2/ports/alienport" \
  "/storage/roms/ports/alienport" \
  "$SCRIPT_DIR"; do
  if [ -d "$cand" ] && { [ -f "$cand/alienport" ] || [ -f "$cand/lib/arm64-v8a/libalien_shooter.so" ] || [ -f "$cand/Alien-Shooter-v1-2-9-patched.apk" ]; }; then
    GAMEDIR="$(cd "$cand" && pwd)"
    break
  fi
done

if [ -z "$GAMEDIR" ]; then
  GAMEDIR="$SCRIPT_DIR/alienport"
fi

echo "GAMEDIR selecionado: $GAMEDIR"

if [ ! -d "$GAMEDIR" ]; then
  echo "ERRO FATAL: Pasta do jogo não existe: $GAMEDIR"
  echo "Estrutura atual em $SCRIPT_DIR:"
  ls -la "$SCRIPT_DIR"
  echo ""
  echo "Pressione qualquer botão ou aguarde 10 segundos..."
  if [ -w "$CUR_TTY" ]; then
    cat "$LOG_ROOT" > "$CUR_TTY" 2>/dev/null
  fi
  sleep 10
  exit 1
fi

cd "$GAMEDIR" || exit 1
cp -f "$LOG_ROOT" "$GAMEDIR/log.txt" 2>/dev/null || true

# 5. Garante permissão de execução no binário
chmod +x "$GAMEDIR/alienport" 2>/dev/null || true

echo "Conteúdo de $GAMEDIR:"
ls -la "$GAMEDIR"

# 6. Extração automática caso o usuário tenha colocado apenas o APK
if [ ! -f "lib/arm64-v8a/libalien_shooter.so" ] || [ ! -d "assets" ]; then
  APK_FILE=$(ls -1 *.apk 2>/dev/null | head -n 1)
  if [ -n "$APK_FILE" ] && [ -f "$APK_FILE" ]; then
    echo "Extraindo dados do jogo a partir de $APK_FILE..."
    if command -v unzip >/dev/null 2>&1; then
      unzip -o -q "$APK_FILE" "assets/*" "lib/arm64-v8a/*" -d .
    fi
  fi
fi

# 7. Validação do executável e da biblioteca nativa
MISSING=0
if [ ! -f "$GAMEDIR/alienport" ]; then
  echo "ERRO: Executável 'alienport' não encontrado em $GAMEDIR!"
  MISSING=1
fi

if [ ! -f "$GAMEDIR/lib/arm64-v8a/libalien_shooter.so" ]; then
  echo "ERRO: lib/arm64-v8a/libalien_shooter.so não encontrado em $GAMEDIR!"
  echo "Você precisa colocar a pasta 'assets' e a pasta 'lib' (ou o arquivo .apk) dentro de $GAMEDIR."
  MISSING=1
fi

if [ $MISSING -eq 1 ]; then
  echo ""
  echo "ABORTANDO: Arquivos essenciais não encontrados."
  echo "Consulte o arquivo $GAMEDIR/log.txt ou $LOG_ROOT"
  if [ -w "$CUR_TTY" ]; then
    cat "$LOG_ROOT" > "$CUR_TTY" 2>/dev/null
  fi
  sleep 10
  exit 1
fi

# 8. Ambiente de Execução
mkdir -p "$GAMEDIR/savedata"
export LD_LIBRARY_PATH="$GAMEDIR/lib:$GAMEDIR:$LD_LIBRARY_PATH"
[ -n "$sdl_controllerconfig" ] && export SDL_GAMECONTROLLERCONFIG="$sdl_controllerconfig"

# Teste de dependências (ldd)
echo "--- Verificação de bibliotecas dinâmicas do alienport ---"
ldd "$GAMEDIR/alienport" 2>&1 || true
echo "--- Verificação dos arquivos binários principais ---"
ls -lh "$GAMEDIR/alienport" 2>&1 || true
ls -lh "$GAMEDIR/lib/arm64-v8a/libalien_shooter.so" 2>&1 || true
echo "--------------------------------------------------------"

# 9. Inicializa mapeador de controle se disponível
if [ -n "$GPTOKEYB" ]; then
  $GPTOKEYB "alienport" &
fi

# 10. Executa o jogo
echo "Executando: ./alienport $GAMEDIR"
./alienport "$GAMEDIR"
EXIT_CODE=$?

echo "=============================================="
echo "Jogo finalizou em $(date) com código: $EXIT_CODE"
echo "=============================================="

# 11. Limpeza
if [ -n "$ESUDO" ]; then
  $ESUDO killall -9 gptokeyb 2>/dev/null || true
fi

unset LD_LIBRARY_PATH
cp -f "$LOG_ROOT" "$GAMEDIR/log.txt" 2>/dev/null || true

# Se saiu com erro (crash), mostra na tela do console antes de voltar
if [ $EXIT_CODE -ne 0 ]; then
  echo "O jogo fechou com erro! Mostrando log na tela por 10 segundos..."
  if [ -w "$CUR_TTY" ]; then
    tail -n 25 "$LOG_ROOT" > "$CUR_TTY" 2>/dev/null
  fi
  sleep 10
fi

printf "\033c" > /dev/tty0 2>/dev/null || true
exit $EXIT_CODE
