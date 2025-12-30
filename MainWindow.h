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
