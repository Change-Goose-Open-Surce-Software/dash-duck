# Dash Duck — Addon‑Spezifikation & Entwickler‑Guide

**Kurzbeschreibung (Deutsch)**
Dash Duck ist eine modulare Desktop‑App, die Video‑/Streaming‑Nutzern erlaubt, ihr Gesicht durch tierische Avatare zu ersetzen. Tiere, Rassen (Skins), Sounds, Animationen und Abläufe (Flows) werden als **Addons** verwaltet — vollständig erweiterbar ohne Quellcode‑Änderung.

---

## Architektur‑Entscheidung (Empfehlung)

* **Programmiersprache / Framework:** C++ mit **Qt** für die GUI (stabil, performant, plattformfähig) und **OpenCV** für Bildverarbeitung. Gründe: native Performance, direkte Kamera‑Anbindung, einfache Erzeugung eines virtuellen Kamerasignals über `v4l2loopback`/GStreamer, keine Abhängigkeit auf pip/npm.
* **Warum C++/Qt statt Python/Electron:** Der Nutzer wünscht *APT‑only* Abhängigkeiten. Implementierung in C++/Qt erlaubt alle Systempakete via `apt` bereitzustellen (keine pip/npm erforderlich). Außerdem ist Qt ideal für moderne Desktop‑UIs.

---

## Notwendige apt‑Pakete (Beispiel für Debian/Ubuntu)

```
sudo apt update
sudo apt install -y build-essential cmake git qtbase5-dev qtdeclarative5-dev libopencv-dev libopencv-core-dev libopencv-imgproc-dev libopencv-highgui-dev libavcodec-dev libavformat-dev libswscale-dev gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good v4l2loopback-dkms v4l2loopback-utils ffmpeg sox libpulse-dev libasound2-dev pkg-config
```

*Erläuterung:*

* `qtbase5-dev`, `qtdeclarative5-dev` — GUI (QML/Widgets)
* `libopencv-dev` — Bildverarbeitung, Tracking, Masking
* `gstreamer*`, `ffmpeg` — Video‑Pipeline, Encoding
* `v4l2loopback-dkms` — virtuelle Kamera erzeugen (`/dev/videoX`)
* `sox`, `libpulse-dev` — Sound‑Wiedergabe/Manipulation

---

## Addon‑Konzept (Grundregeln)

* Jedes Addon ist ein Verzeichnis unter `~/.dash-duck/addons/`.
* Jede Addon‑Ordner muss eine Metadatei `addon.json` enthalten.
* Addons dürfen ausführbare `install.sh` oder `uninstall.sh` Skripte enthalten, die lokal ausführbar sind.
* Die App listet Addons automatisch, indem sie das Addons‑Verzeichnis rekursiv scannt.

Pfadbeispiel:

```
~/.dash-duck/addons/
├─ add-animal-flamingo/
│  ├─ addon.json
│  ├─ thumbnail.png
│  ├─ install.sh
│  ├─ uninstall.sh
│  ├─ meta/
│  │  ├─ breeds/
│  │  │  ├─ pink/
│  │  │  │  ├─ skin.png
│  │  │  │  └─ skin.conf
│  │  │  └─ albino/
│  │  ├─ sounds/
│  │  │  ├─ squawk.wav
│  │  │  └─ flap.wav
│  │  ├─ animations/
│  │  │  ├─ wave.animation.json
│  │  │  └─ wing-flap.animation.json
│  │  └─ flows/
│  │     └─ intro.flow.json
│  └─ docs/
│     └─ readme.md
```

---

## `addon.json` — Metadaten (Schema)

```json
{
  "id": "add-animal-flamingo",
  "title": "Flamingo",
  "developer": "Max Mustermann",
  "version": "1.0.0",
  "release_date": "2025-12-15",
  "last_update": "2025-12-20",
  "thumbnail": "thumbnail.png",
  "description": "Ein fröhlicher Flamingo‑Avatar mit mehreren Rassen.",
  "categories": ["animal"],
  "files": {
    "breeds": "meta/breeds/",
    "sounds": "meta/sounds/",
    "animations": "meta/animations/",
    "flows": "meta/flows/"
  }
}
```

* Die App läd `thumbnail` relativ zum Addon‑Ordner.
* `files` hilft dem UI die Inhalte zu gruppieren.

---

## Dateien‑Konventionen

### Sounds

* Format: `.wav` oder `.ogg` (PCM, 16 bit empfohlen).
* Metadaten optional: `sounds.json` in `meta/sounds/` mit Feldern: `id`, `title`, `duration_ms`, `tags`.

**Beispiel `meta/sounds/sounds.json`:**

```json
[
  { "id":"squawk","file":"squawk.wav","title":"Kreisch"},
  { "id":"flap","file":"flap.wav","title":"Flügelschlag"}
]
```

### Animationen

Animationen sind deklarative JSON‑Dateien, die Transformationen, Timing und ggf. Frame‑Sequenzen beschreiben.
Zwei erlaubte Formen:

1. **Skeletal / Parametrische Animation** — JSON beschreibt Parameteränderungen über Zeit (z. B. Kopfneigung, Augenblinzeln).
2. **Frame‑Sequenz** — Pfad zu einer Bildfolge oder zu einer Spritesheet.

**Beispiel `wing-flap.animation.json`:**

```json
{
  "id": "wing-flap",
  "type": "parametric",
  "duration_ms": 800,
  "steps": [
    {"t":0, "params": {"wing_angle": 0}},
    {"t":200, "params": {"wing_angle": 45}},
    {"t":400, "params": {"wing_angle": 0}}
  ]
}
```

Parameter‑Namen (z. B. `wing_angle`, `eye_blink`) sind Konventionen, die von der Runtime‑Engine interpretiert werden. Dokumentiere neue Parameter in `docs/` des Addons.

### Flows (Abläufe)

Flows sind Skripte in JSON, die Sounds und Animationen zeitlich verknüpfen.

**Beispiel `intro.flow.json`:**

```json
{
  "id": "intro",
  "title": "Begrüßung",
  "sequence": [
    {"when_ms": 0, "action": {"type":"animation","id":"wing-flap"}},
    {"when_ms": 100, "action": {"type":"sound","id":"squawk"}},
    {"when_ms": 900, "action": {"type":"animation","id":"wave"}}
  ]
}
```

---

## Wie die App Addons lädt (Technik)

1. Beim Start scannt die App `~/.dash-duck/addons/` nach Verzeichnissen.
2. Für jedes Verzeichnis wird `addon.json` geladen — Fehlerhafte Addons werden in der UI markiert.
3. Addon‑Banner (Liste) wird in chronologischer Reihenfolge angezeigt: neue Addons oben oder unten (konfigurierbar in Settings).
4. Klick auf Banner öffnet Addon‑Detailseite, welche folgende Felder anzeigt: Titel, Entwickler, Version, Release‑Datum, Last Update, Changelog (falls `changelog.md` vorhanden), Screenshots (aus `meta/screenshots/`), Featureliste (aus `addon.json` oder `docs/readme.md`), und ein Install‑Terminal‑Befehl (z. B. `sudo ~/.dash-duck/addons/add-animal-flamingo/install.sh`).

---

## UI: Addon‑Store (Display‑Regeln)

* **Liste‑Layout:** Lang gestreckte Liste, für jedes Addon ein Banner/Row.
* **Links:** Thumbnail.
* **Rechts oben:** Titel.
* **Darunter:** Veröffentlichungsdatum + Entwicklername.
* Klick auf Row → Detail‑Modal (zeigt alle Metadaten und Install‑Button).
* Install‑Button kopiert Terminal‑Befehl in die Zwischenablage und kann optional das `install.sh` per `exec` ausführen (Button in Settings aktivierbar — vorsichtig, Sicherheitsrisiko!).

---

## Install‑Skript (`install.sh`) — Template

```bash
#!/usr/bin/env bash
set -e
# Beispiel Install Script für add-animal-flamingo
ADDON_DIR="$HOME/.dash-duck/addons/add-animal-flamingo"
mkdir -p "$ADDON_DIR"
cp -r * "$ADDON_DIR/"
chmod +x "$ADDON_DIR"/*.sh
# Optional: Registriere Version in lokaler DB (zB sqlite)
# echo "installed" >> "$HOME/.dash-duck/installed.log"

echo "Addon installiert in $ADDON_DIR"
```

* Hinweis: Da Addon‑Store lokal ist, kann die App das Skript per `QProcess`/`system()` aufrufen. Achte auf Sicherheitsüberprüfungen — das Ausführen fremder Skripte ist riskant!

---

## Preview → virtuelle Kamera (Zoom/Discord/Twitch)

**Vorgehensweise (vereinfachter Ablauf):**

1. App liest Webcam (z. B. `/dev/video0`) via OpenCV.
2. Rendert Avatar‑Overlay/Masking in einer Render‑Pipeline (OpenCV + ggf. OpenGL für Performance).
3. Ausgabe wird an einen `v4l2loopback`‑Device geschrieben (z. B. `/dev/video10`) mittels GStreamer oder direktem V4L2‑Write. Beispiel GStreamer‑Pipeline:

```
app -> BGR frames -> convert -> videoconvert ! v4l2sink device=/dev/video10
```

4. In Zoom/Discord/Twitch wählst du `Dash Duck Virtual Camera` als Kameraquelle.

Wichtig: `v4l2loopback` muss mit passenden Rechten geladen werden. Installation:

```
sudo apt install v4l2loopback-dkms v4l2loopback-utils
sudo modprobe v4l2loopback video_nr=10 card_label="DashDuckCam" exclusive_caps=1
```

---

## Beispiel: Minimaler Addon‑Erstellungs‑Leitfaden (Schritt für Schritt)

1. **Ordner erstellen:** `mkdir -p ~/.dash-duck/addons/add-animal-example/meta/{breeds,sounds,animations,flows}`
2. **Metadatei:** Lege `addon.json` im Addon‑Ordner an (siehe Schema oben).
3. **Thumbnail:** Kopiere `thumbnail.png` in den Addon‑Ordner.
4. **Sounds:** Füge WAVs in `meta/sounds/` ein und erstelle `meta/sounds/sounds.json`.
5. **Animationen:** Erstelle `meta/animations/*.animation.json` wie oben beschrieben.
6. **Flows:** Erstelle `meta/flows/*.flow.json`.
7. **install.sh:** Optional: lege ein `install.sh` bei, mache es ausführbar (`chmod +x install.sh`).
8. **Testen:** Starte Dash Duck → Einstellungen → Scan Addons → Addon sollte in der Liste erscheinen.

---

## Wie man Animationen erstellt (Praktisch)

* **Konzept:** Animationen beschreiben Parameter‑Keyframes (z. B. `eye_blink`, `jaw_open`, `head_yaw`) über Zeit. Die Runtime implementiert eine kleine Interpolations‑Engine (linear oder ease‑in/out).
* **Erstellung:** Du kannst einfache Editor‑Tools schreiben (z. B. ein kleines QML‑Tool) oder Animationen manuell als JSON anlegen.
* **Alternative (Frame‑based):** Eine Sequenz von PNGs `frame000.png, frame001.png, ...` + `animation.metadata.json` mit `fps` und `loop`.

---

## Sicherheits‑Hinweise

* Addons enthalten ausführbare Skripte → **Signatur / Whitelist / Sandbox** empfohlen.
* Empfehlung: App fragt vor Ausführung des `install.sh` nach Bestätigung. Es kann wählbar sein, ob die App automatisch Skripte ausführt oder nur die Install‑Befehle zum Kopieren anbietet.

---

## Beispiel: Vollständiges kleines Addon (Flamingo) — Checkliste

* [x] `addon.json`
* [x] `thumbnail.png`
* [x] `meta/sounds/squawk.wav` + `meta/sounds/sounds.json`
* [x] `meta/animations/wing-flap.animation.json`
* [x] `meta/flows/intro.flow.json`
* [x] `install.sh` (kopiert Dateien in `~/.dash-duck/addons`)
* [x] `docs/readme.md` (Kurz‑Anleitung)

---

## UI‑Funktionen, die du in der App brauchst

* Addon‑Store (Liste + Detail‑Modal)
* Addon‑Installer (copy vs. exec install.sh) + Button zum Kopieren des Terminalbefehls
* Avatar‑Auswahl (Tiere, Rassen/Skins)
* Action‑Panel (Sounds, Animationen, Flows) mit Vorschau
* Preview‑Fenster mit Virtual Camera Output Control (Device auswählen / neu laden)
* Settings: Reihenfolge Addon‑Liste (neu/alt), Sicherheitseinstellungen (Auto‑Exec), Virtual Cam Device

---

## Weiteres / Empfehlung für spätere Erweiterungen

* Digitales Signieren von Addons (GPG‑Signatur in `addon.json` prüfen)
* Online Addon‑Store (optional) — initial lokal, später Mirror/Repo
* Plugin SDK in C++ mit Header‑Dateien, damit Addons komplexe Erweiterungen in nativen Modulen bereitstellen können

---

Wenn du möchtest, kann ich jetzt eine **Konkrete Dateistruktur** und ein **Beispiel‑`addon.json` + `install.sh`** als separate Datei im Canvas anlegen, oder ein **Minimal‑C++/Qt‑Projekt‑Skelett** (z. B. `CMakeLists.txt`, `main.cpp`, `AppMainWindow`) erstellen — sag mir einfach, welche der Optionen du direkt als Datei im Canvas willst.

Viel Erfolg beim Entwickeln — Dash Duck klingt nach einer tollen Idee! 🦆

---

## Schnellstart: C++/Qt Minimal‑Projekt‑Skelett (Beispiel)

Die folgenden Dateien sind ein minimales Gerüst, das du als Ausgangspunkt verwenden kannst. Es nutzt CMake + Qt5 (Widgets) und zeigt, wie Addons beim Start gescannt und in einer Liste angezeigt werden können. Dies ist **nur ein Beispiel** – erweitere es nach Bedarf.

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(dash_duck)
set(CMAKE_CXX_STANDARD 17)
find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
find_package(OpenCV REQUIRED)
add_executable(dash-duck
  src/main.cpp
  src/MainWindow.cpp
  src/AddonLoader.cpp
)
target_include_directories(dash-duck PRIVATE src)
target_link_libraries(dash-duck PRIVATE Qt5::Widgets Qt5::Core Qt5::Gui ${OpenCV_LIBS})
```

### src/main.cpp

```cpp
#include <QApplication>
#include "MainWindow.h"
int main(int argc, char** argv){
  QApplication app(argc, argv);
  MainWindow w;
  w.show();
  return app.exec();
}
```

### src/MainWindow.h

```cpp
#pragma once
#include <QMainWindow>
#include <QListWidget>
class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget* parent=nullptr);
private:
  QListWidget* addonList;
  void setupUi();
};
```

### src/MainWindow.cpp

```cpp
#include "MainWindow.h"
#include "AddonLoader.h"
#include <QVBoxLayout>
#include <QLabel>
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent){
  setupUi();
  AddonLoader loader;
  auto addons = loader.scanAddons();
  for(auto &a : addons){
    QListWidgetItem* it = new QListWidgetItem(QString::fromStdString(a.title));
    addonList->addItem(it);
  }
}
void MainWindow::setupUi(){
  QWidget* central = new QWidget(this);
  QVBoxLayout* v = new QVBoxLayout(central);
  QLabel* header = new QLabel("Dash Duck — Addon Store");
  addonList = new QListWidget();
  v->addWidget(header);
  v->addWidget(addonList);
  setCentralWidget(central);
}
```

### src/AddonLoader.h

```cpp
#pragma once
#include <string>
#include <vector>
struct AddonMeta{ std::string id; std::string title; std::string developer; std::string thumbnail; };
class AddonLoader{
public:
  std::vector<AddonMeta> scanAddons();
};
```

### src/AddonLoader.cpp

```cpp
#include "AddonLoader.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp> // optional, include single header nlohmann json in third_party
using json = nlohmann::json;
std::vector<AddonMeta> AddonLoader::scanAddons(){
  std::vector<AddonMeta> out;
  std::filesystem::path base = std::filesystem::path(std::getenv("HOME"))/".dash-duck"/"addons";
  if(!std::filesystem::exists(base)) return out;
  for(auto &d : std::filesystem::directory_iterator(base)){
    if(!d.is_directory()) continue;
    auto metaFile = d.path()/"addon.json";
    if(!std::filesystem::exists(metaFile)) continue;
    std::ifstream f(metaFile);
    try{
      json j; f >> j;
      AddonMeta m;
      m.id = j.value("id", "");
      m.title = j.value("title", "(no title)");
      m.developer = j.value("developer", "");
      m.thumbnail = (d.path()/j.value("thumbnail", "thumbnail.png")).string();
      out.push_back(m);
    }catch(...){ /* fehlerhafte addon.json überspringen */ }
  }
  return out;
}
```

> **Hinweis:** Ich verwende oben `nlohmann/json.hpp` als bequemen JSON‑Parser (ein Single‑Header C++ JSON Parser). Du kannst ihn aus APT beziehen (`sudo apt install nlohmann-json3-dev`) oder als vendored header beilegen.

---

## Antworten auf deine Fragen (präzise)

**1) Was heißt „Die App lädt `thumbnail` relativ zum Addon‑Ordner“?**
Das bedeutet: Wenn in `addon.json` der Wert `"thumbnail": "thumbnail.png"` steht, interpretiert die App diesen Pfad im Zusammenhang mit dem Addon‑Verzeichnis. Beispiel:

* Addon‑Ordner: `~/.dash-duck/addons/add-animal-flamingo/`
* `thumbnail`-Eintrag: `thumbnail.png`
* Tatsächlicher Pfad, den die App öffnet: `~/.dash-duck/addons/add-animal-flamingo/thumbnail.png`

Du kannst im `addon.json` auch relative Unterordner angeben, z. B. `"thumbnail": "meta/screenshots/thumb.png"` — die App addiert diesen Pfad an das Addon‑Root.

**2) Warum .wav oder .ogg? Ich will .mp3 (oder .opus)**
Gute Frage — ich habe WAV/OGG als *empfohlen* erwähnt, weil sie oft lizenzfrei und leicht zu dekodieren sind. Wenn du MP3 bevorzugst, ist das völlig in Ordnung — wir machen MP3 zur primären Anforderung. OPUS OK als Fallback.

Technisch bedeutet das:

* Die App muss einen Audio‑Decoder zur Laufzeit haben, der MP3 und OPUS lesen kann. Unter Linux sind `ffmpeg`/`gstreamer` übliche Tools/Stacks, die MP3 und OPUS unterstützen (achte auf Lizenzpakete). In der `apt`‑Liste oben sollten `gstreamer1.0-plugins-ugly` und evtl. `libmp3lame` ergänzt werden, damit MP3 funktioniert.

Empfohlene apt‑Erweiterung:

```
sudo apt install gstreamer1.0-plugins-ugly gstreamer1.0-libav
```

Damit deckst du MP3/OPUS ab.

**3) Wallpaper unabhängig vom Tier & Tiere als PNG im Vordergrund**
Ja — richtig verstanden. Implementation:

* Addon‑Avatar stellt immer eine PNG mit transparentem Hintergrund (RGBA) bereit — das ist die Vordergrund‑Grafik.
* In der UI wählst du optional ein Wallpaper/Background (eigenes Bild oder ein Addon‑übergreifendes Background‑Set). Das Wallpaper wird hinter dem PNG gerendert.

**4) Habe ich es richtig verstanden, dass Animationen eine Abfolge von PNG sind?**
Ja — genau. Frame‑basierte Animationen sind unterstützt: eine Sequenz `frame000.png, frame001.png, ...` + eine Metadatei `animation.json` mit `fps`, `loop` und evtl. Hotspot/offset. Zusätzlich unterstützen wir auch parametergesteuerte Animationen (JSON Keyframes) — aber die PNG‑Sequenz ist eine erste‑klassige, verpflichtende Option (wie du wolltest).

---

## Nächste Schritte — ich fange mit dem eigentlichen Code‑Gerüst an

Ich habe das Projekt‑Gerüst in das Entwickler‑Dokument eingefügt (siehe oben). Wenn du möchtest, kann ich jetzt eines der folgenden Dinge **direkt** ins Canvas schreiben (ich mache das ohne weitere Rückfrage):

A) Ein **vollständiges Beispiel‑Addon** (Flamingo) mit realistischen `addon.json`, `install.sh`, Beispiel PNG‑Frames (als Namenskonventionen, nicht echte Bilder), MP3‑Soundeintrag.
B) Ein **erweiterteres C++/Qt‑Skelett** mit UI‑Komponenten für Avatar‑Auswahl, Wallpaper, Action‑Panel und virtuelle Kamera Steuerung (CMake + Quelldateien).
C) Beide A+B.

Ich werde jetzt **B** (das erweiterte C++/Qt‑Skelett) in den Canvas schreiben, damit du sofort mit dem Hauptprogramm loslegen kannst. Wenn du lieber A möchtest statt B oder zusätzlich A brauchst, sag nichts — ich füge beides an (da du "kannst mit dem Coden loslegen" gesagt hast, gehe ich proaktiv vor). Bitte beachte: Ich kann die Bilder nicht hochladen, aber ich lege die Pfad‑Konventionen und Beispieldateien an.

Ich beginne jetzt und ergänze das Canvas mit dem erweiterten Projekt‑Skelett (CMakeLists, main.cpp, UI‑Klassen, AddonLoader, Beispiel `addon.json`).
