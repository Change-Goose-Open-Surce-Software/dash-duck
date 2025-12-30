#Begrüßung
echo Hi,nice that you use Dash Duck a programm from Change Goose
echo We hope that you like it !

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
mkdir -p ~/.dash-duck/assets/icon

#Hole install script
wget -O ~/.dash-duck/bin/update.sh https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/install.sh

#Install script ausführbar machen
chmod +x ~/.dash-duck/bin/update.sh

#Hauptprogramm Instalieren
wget -O ~/.dash-duck/src/main.cpp https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/main.cpp
wget -O ~/.dash-duck/src/AddonLoader.cpp https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/Addonloader.cpp
wget -O ~/.dash-duck/src/AddonLoader.h https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/AddonLoader.h
wget -O ~/.dash-duck/src/MainWindow.cpp https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/MainWindow.cpp
wget -O ~/.dash-duck/src/MainWindow.h https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/MainWindow.h

#Hauptprogramm Ausführbar machen
chomd +x ~/.dash-duck/src/main.cpp
chomd +x ~/.dash-duck/src/AddonLoader.h
chomd +x ~/.dash-duck/src/AddonLoader.cpp
chomd +x ~/.dash-duck/src/MainWindow.cpp
chomd +x ~/.dash-duck/src/MainWindow.h

#Desktop Datei holen und ausführbar machen
wget -O ~/.local/share/applications/dash-duck.desktop https://raw.githubusercontent.com/Change-Goose-Open-Surce-Software/dash-duck/main/dash-duck.desktop
chmod +x ~/.local/share/applications/dash-duck.desktop
update-desktop-database ~/.local/share/applications

#Compliere alle Datein Zusammen
g++ ~/.dash-duck/src/*.cpp -I~/.dash-duck/src -o ~/.dash-duck/bin/dash-duck

#Instaliere Abhänigkeiten
sudo apt update
sudo apt install -y build-essential cmake git qtbase5-dev qtdeclarative5-dev libopencv-dev libopencv-core-dev libopencv-imgproc-dev libopencv-highgui-dev libavcodec-dev libavformat-dev libswscale-dev gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good v4l2loopback-dkms v4l2loopback-utils ffmpeg sox libpulse-dev libasound2-dev pkg-config
sudo apt install v4l2loopback-dkms v4l2loopback-utils
sudo modprobe v4l2loopback video_nr=10 card_label="DashDuckCam" exclusive_caps=1
sudo apt install nlohmann-json3-dev
sudo apt install gstreamer1.0-plugins-ugly gstreamer1.0-libav
