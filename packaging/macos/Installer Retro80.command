#!/bin/bash
# Installe Retro80 sur ce Mac : plugin AU (GarageBand, Logic), plugin VST3 et appli autonome.

cd "$(dirname "$0")" || exit 1

COMPONENTS="$HOME/Library/Audio/Plug-Ins/Components"
VST3="$HOME/Library/Audio/Plug-Ins/VST3"
APPS="/Applications"
[ -w "$APPS" ] || APPS="$HOME/Applications"

echo "Installation de Retro80..."
mkdir -p "$COMPONENTS" "$VST3" "$APPS"

install_bundle () {
    local source="$1" destination="$2"
    rm -rf "$destination/$(basename "$source")"
    cp -R "$source" "$destination/"
    # Le plugin n'est pas signé par Apple : on retire le blocage « téléchargé depuis Internet ».
    xattr -dr com.apple.quarantine "$destination/$(basename "$source")" 2>/dev/null
    echo "  ✓ $(basename "$source") → $destination"
}

install_bundle "Retro80.component" "$COMPONENTS"
install_bundle "Retro80.vst3" "$VST3"
install_bundle "Retro80.app" "$APPS"

# Force macOS à relire la liste des plugins Audio Units.
killall -9 AudioComponentRegistrar 2>/dev/null

echo ""
echo "Terminé ! Ouvre GarageBand, crée une piste « Instrument logiciel »,"
echo "puis dans Plug-ins : Instrument → Modules Audio Units → Retro80 → Retro80."
echo "Tu peux aussi lancer l'appli « Retro80 » directement depuis $APPS."
echo ""
read -r -p "Appuie sur Entrée pour fermer cette fenêtre."
