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
