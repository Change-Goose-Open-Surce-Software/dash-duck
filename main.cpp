// Project: TierWallpaperApp — vollständige Implementierung (Minimal-V1)
// Hinweis: Dieses Projekt ist lauffähig mit Qt 6, CMake und GStreamer/FFmpeg-Unterstützung für MP3/Opus.
// Struktur (Vorschlag):
//  - CMakeLists.txt
//  - src/main.cpp
//  - src/app/AddonLoader.h/.cpp
//  - src/app/AnimationPlayer.h/.cpp
//  - src/app/WallpaperManager.h/.cpp
//  - src/ui/MainWindow.h/.cpp
//  - assets/
//  - addons/<addon>/...

// ========================= CMakeLists.txt =========================
cmake_minimum_required(VERSION 3.21)
project(TierWallpaperApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Multimedia MultimediaWidgets)

add_executable(TierWallpaperApp
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/app/AddonLoader.cpp
    src/app/AddonLoader.h
    src/app/AnimationPlayer.cpp
    src/app/AnimationPlayer.h
    src/app/WallpaperManager.cpp
    src/app/WallpaperManager.h
)

target_include_directories(TierWallpaperApp PRIVATE src)

target_link_libraries(TierWallpaperApp PRIVATE
    Qt6::Widgets
    Qt6::Gui
    Qt6::Multimedia
    Qt6::MultimediaWidgets
)

install(TARGETS TierWallpaperApp RUNTIME DESTINATION bin)

// ========================= src/main.cpp =========================
#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

// ========================= src/app/AddonLoader.h =========================
#pragma once
#include <QObject>
#include <QDir>
#include <QJsonObject>
#include <QVector>
#include <QString>

struct AnimationFrame {
    QString pngPath;    // absolut
    int durationMs{100};
};

struct AnimalAnimation {
    QString id;
    QVector<AnimationFrame> frames; // PNG-Sequenz
    QString soundPath; // mp3 bevorzugt, opus erlaubt
};

struct AddonMeta {
    QString id;
    QString name;
    QString thumbnailPath; // relativ zum Addon-Ordner (wird zu absolut aufgelöst)
    QVector<AnimalAnimation> animals;
};

class AddonLoader : public QObject {
    Q_OBJECT
public:
    explicit AddonLoader(QObject* parent=nullptr);

    void setAddonsRoot(const QString& root);
    QVector<AddonMeta> scan();

signals:
    void addonsChanged(const QVector<AddonMeta>& addons);

private:
    QString m_root;
    AddonMeta loadAddon(const QDir& dir) const;
    AnimalAnimation loadAnimal(const QDir& dir, const QString& id, const QJsonObject& obj) const;
};

// ========================= src/app/AddonLoader.cpp =========================
#include "AddonLoader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

AddonLoader::AddonLoader(QObject* parent) : QObject(parent) {}

void AddonLoader::setAddonsRoot(const QString& root) { m_root = root; }

QVector<AddonMeta> AddonLoader::scan() {
    QVector<AddonMeta> list;
    QDir root(m_root);
    for (const auto& entry : root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir addonDir(entry.absoluteFilePath());
        try {
            list.push_back(loadAddon(addonDir));
        } catch (...) {
            qWarning() << "Skipping addon" << addonDir.absolutePath();
        }
    }
    emit addonsChanged(list);
    return list;
}

AddonMeta AddonLoader::loadAddon(const QDir& dir) const {
    QFile file(dir.filePath("addon.json"));
    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("addon.json missing");

    const auto json = QJsonDocument::fromJson(file.readAll()).object();

    AddonMeta meta;
    meta.id = json.value("id").toString(dir.dirName());
    meta.name = json.value("name").toString(meta.id);

    const QString thumbRel = json.value("thumbnail").toString();
    meta.thumbnailPath = dir.filePath(thumbRel);

    const auto animalsArr = json.value("animals").toArray();
    for (const auto& v : animalsArr) {
        const auto obj = v.toObject();
        const QString id = obj.value("id").toString();
        meta.animals.push_back(loadAnimal(dir, id, obj));
    }

    return meta;
}

AnimalAnimation AddonLoader::loadAnimal(const QDir& dir, const QString& id, const QJsonObject& obj) const {
    AnimalAnimation a;
    a.id = id;

    const QString framesDirRel = obj.value("frames").toString();
    QDir framesDir(dir.filePath(framesDirRel));

    const auto framesArr = obj.value("sequence").toArray();
    if (!framesArr.isEmpty()) {
        for (const auto& fv : framesArr) {
            auto fo = fv.toObject();
            AnimationFrame f;
            f.pngPath = framesDir.filePath(fo.value("file").toString());
            f.durationMs = fo.value("duration").toInt(100);
            a.frames.push_back(f);
        }
    } else {
        // fallback: load all png in directory alphabetically
        for (const auto& fi : framesDir.entryInfoList({"*.png"}, QDir::Files, QDir::Name)) {
            AnimationFrame f{fi.absoluteFilePath(), 100};
            a.frames.push_back(f);
        }
    }

    const QString soundRel = obj.value("sound").toString();
    if (!soundRel.isEmpty())
        a.soundPath = dir.filePath(soundRel); // mp3 bevorzugt, opus erlaubt

    return a;
}

// ========================= src/app/AnimationPlayer.h =========================
#pragma once
#include <QObject>
#include <QPixmap>
#include <QTimer>
#include <QVector>
#include "AddonLoader.h"
#include <QSoundEffect>

class AnimationPlayer : public QObject {
    Q_OBJECT
public:
    explicit AnimationPlayer(QObject* parent=nullptr);
    void setAnimation(const AnimalAnimation& anim);
    QPixmap currentFrame() const;

signals:
    void frameChanged();

private:
    QVector<AnimationFrame> m_frames;
    int m_index{0};
    QPixmap m_pix;
    QTimer m_timer;
    QSoundEffect m_sound;
};

// ========================= src/app/AnimationPlayer.cpp =========================
#include "AnimationPlayer.h"

AnimationPlayer::AnimationPlayer(QObject* parent): QObject(parent) {
    connect(&m_timer, &QTimer::timeout, this, [this]{
        if (m_frames.isEmpty()) return;
        m_index = (m_index + 1) % m_frames.size();
        m_pix = QPixmap(m_frames[m_index].pngPath);
        m_timer.start(m_frames[m_index].durationMs);
        emit frameChanged();
    });
}

void AnimationPlayer::setAnimation(const AnimalAnimation& anim) {
    m_frames = anim.frames;
    m_index = 0;
    if (!m_frames.isEmpty()) {
        m_pix = QPixmap(m_frames[0].pngPath);
        m_timer.start(m_frames[0].durationMs);
    }

    if (!anim.soundPath.isEmpty()) {
        m_sound.setSource(QUrl::fromLocalFile(anim.soundPath));
        m_sound.setLoopCount(QSoundEffect::Infinite);
        m_sound.play();
    }
}

QPixmap AnimationPlayer::currentFrame() const { return m_pix; }

// ========================= src/app/WallpaperManager.h =========================
#pragma once
#include <QObject>
#include <QPixmap>

class WallpaperManager : public QObject {
    Q_OBJECT
public:
    explicit WallpaperManager(QObject* parent=nullptr);

    void setWallpaper(const QString& path); // optionales Wallpaper (kann leer sein)
    QPixmap wallpaper() const;

private:
    QPixmap m_wallpaper;
};

// ========================= src/app/WallpaperManager.cpp =========================
#include "WallpaperManager.h"

WallpaperManager::WallpaperManager(QObject* parent): QObject(parent) {}

void WallpaperManager::setWallpaper(const QString& path) {
    if (path.isEmpty()) { m_wallpaper = QPixmap(); return; }
    m_wallpaper = QPixmap(path);
}

QPixmap WallpaperManager::wallpaper() const { return m_wallpaper; }

// ========================= src/ui/MainWindow.h =========================
#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include "app/AddonLoader.h"
#include "app/AnimationPlayer.h"
#include "app/WallpaperManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=nullptr);

private:
    AddonLoader m_loader;
    AnimationPlayer m_player;
    WallpaperManager m_wall;

    QListWidget* m_addonList{nullptr};
    QListWidget* m_animalList{nullptr};
    QLabel* m_preview{nullptr};
    QPushButton* m_chooseWallpaper{nullptr};

    QVector<AddonMeta> m_addons;

    void refreshPreview();
};

// ========================= src/ui/MainWindow.cpp =========================
#include "MainWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPainter>

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    resize(960, 640);

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);

    m_addonList = new QListWidget(central);
    m_animalList = new QListWidget(central);
    m_preview = new QLabel(central);
    m_preview->setMinimumSize(480, 360);
    m_preview->setAlignment(Qt::AlignCenter);

    m_chooseWallpaper = new QPushButton("Wallpaper auswählen (optional)", central);

    auto* left = new QVBoxLayout();
    left->addWidget(new QLabel("Addons"));
    left->addWidget(m_addonList);
    left->addWidget(new QLabel("Tiere"));
    left->addWidget(m_animalList);
    left->addWidget(m_chooseWallpaper);

    layout->addLayout(left, 1);
    layout->addWidget(m_preview, 2);

    setCentralWidget(central);

    m_loader.setAddonsRoot("addons");
    m_addons = m_loader.scan();

    for (const auto& a : m_addons)
        m_addonList->addItem(a.name);

    connect(m_addonList, &QListWidget::currentRowChanged, this, [this](int row){
        m_animalList->clear();
        if (row < 0 || row >= m_addons.size()) return;
        for (const auto& an : m_addons[row].animals)
            m_animalList->addItem(an.id);
    });

    connect(m_animalList, &QListWidget::currentRowChanged, this, [this](int row){
        int addonIdx = m_addonList->currentRow();
        if (addonIdx < 0 || addonIdx >= m_addons.size()) return;
        if (row < 0 || row >= m_addons[addonIdx].animals.size()) return;
        m_player.setAnimation(m_addons[addonIdx].animals[row]);
        refreshPreview();
    });

    connect(&m_player, &AnimationPlayer::frameChanged, this, [this]{ refreshPreview(); });

    connect(m_chooseWallpaper, &QPushButton::clicked, this, [this]{
        const auto path = QFileDialog::getOpenFileName(this, "Wallpaper wählen", {}, "Images (*.png *.jpg *.jpeg)");
        if (!path.isEmpty()) m_wall.setWallpaper(path);
        else m_wall.setWallpaper("");
        refreshPreview();
    });
}

void MainWindow::refreshPreview() {
    QPixmap composed(m_preview->size());
    composed.fill(Qt::transparent);

    QPainter p(&composed);

    if (!m_wall.wallpaper().isNull())
        p.drawPixmap(0,0, composed.width(), composed.height(), m_wall.wallpaper().scaled(composed.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    auto frame = m_player.currentFrame();
    if (!frame.isNull()) {
        auto fg = frame.scaled(composed.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        int x = (composed.width() - fg.width())/2;
        int y = (composed.height() - fg.height())/2;
        p.drawPixmap(x, y, fg);
    }

    m_preview->setPixmap(composed);
}
