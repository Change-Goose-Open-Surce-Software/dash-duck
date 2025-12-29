#Begrüßung
echo Hi,nice that you use Dash Duck a programm from Change Goose
echo We hope that you like it !

#Instaliere Abhänigkeiten
sudo apt list *qtbase5-dev* *qtdeclarative5-dev* *libopencv-dev* *gstreamer* *ffmpeg* *v4l2loopback* *sox* *libpulse*

#Erstelle fehlende verzeichnisse
mkdir -p ~/.dash-duck/
mkdir -p ~/.dash-duck/addons
mkdir -p ~/.dash-duck/bin
mkdir -p ~/.dash-duck/build
mkdir -p ~/.dash-duck/config
mkdir -p ~/.dash-duck/log
mkdir -p ~/.dash-duck/wallpapers
mkdir -p ~/.dash-duck/assets/ui
mkdir -p ~/.dash-duck/assets/sounds
mkdir -p ~/.dash-duck/cache/preview
mkdir -p ~/.dash-duck/cache/runtime
mkdir -p ~/.dash-duck/src/ui
mkdir -p ~/.dash-duck/app

#Hole install script
wget -O ~/.dash-duck/bin/update.sh https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/install.sh

#Install script ausführbar machen
chmod +x ~/.dash-duck/bin/update.sh

#Hauptprogramm Instalieren
wget -O ~/.dash-duck/src/main.cpp https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/main.cpp

#Mehr folgt noch !
