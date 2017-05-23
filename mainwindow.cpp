#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "menuproxystyle.h"

#include <QTextCodec>
#include <QDesktopServices>
#include <QMessageBox>
#include <QLabel>

#include <QDataStream>
#include <QFileDialog>
#include <QFile>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());

    ui->path->setText(QDir::toNativeSeparators("C:/"));
    ui->key->setText("Password");

    //Сигнально-слотовые соединения
    connect(ui->selectDir, SIGNAL(clicked()), this, SLOT(seletctOutPathDir()));
    connect(ui->addItem, SIGNAL(clicked()), this, SLOT(addFile()));
    connect(ui->deleteItem, SIGNAL(clicked()), this, SLOT(deleteFile()));
    connect(ui->start, SIGNAL(clicked()), this, SLOT(execute()));
    connect(ui->listfile, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(systemShow(QModelIndex)));
    connect(ui->listfile, SIGNAL(clicked(QModelIndex)), this, SLOT(selectItemList(QModelIndex)));
    connect(ui->aboutMenu, SIGNAL(triggered()), this, SLOT(showAbout()));

    //Начальная инициализация
    ui->path->setReadOnly(true);
    ui->encryptButton->setChecked(true);
    ui->openDir->setChecked(true);
    ui->listfile->setEnabled(false);
    ui->deleteItem->setEnabled(false);
    ui->start->setEnabled(false);

    label = new QLabel(trUtf8("\t Блочный алгоритм шифрования Blowfish"));
    label->setFont(QFont("Tahoma", 10, QFont::Bold));

    menuBar()->setCornerWidget(label, Qt::TopLeftCorner);
    menuBar()->setStyle(new MenuProxyStyle());
    menuBar()->insertSeparator(ui->help->menuAction());
}

MainWindow::~MainWindow()
{
    delete label;
    delete ui;
}

void MainWindow::addFile()
{
   QStringList list;
   list << QFileDialog::getOpenFileNames(this, tr("Выбрать файл:"), QDir::currentPath(),
            trUtf8("All files (*.*);;Файл (*.txt);;Файл (*.fish);;"));

   for(int i = 0; i < list.count(); ++i)
       ui->listfile->addItem(QDir::toNativeSeparators(list.at(i)));

   if( ui->listfile->count() > 0) {
       ui->listfile->setEnabled(true);
       ui->start->setEnabled(true);
   }

   statusBar()->showMessage(tr("\t Корличество файлов в списке: %0").arg(list.count()));
}
void MainWindow::deleteFile()
{
  delete ui->listfile->currentItem();

  ui->listfile->setFocus();

  if( ui->listfile->count() == 0) {
      ui->listfile->setEnabled(false);
      ui->deleteItem->setEnabled(false);
      ui->start->setEnabled(false);
  }

  statusBar()->showMessage(tr("\t Корличество файлов в списке: %0").arg(ui->listfile->count()));
}
void MainWindow::execute()
{
    if(ui->key->text().isEmpty()){
        QMessageBox::warning(this, trUtf8("Внимание"), trUtf8("Введите ключ!"));
        return;
    }
    if(ui->path->text().isEmpty()){
        QMessageBox::warning(this, trUtf8("Внимание"), trUtf8("Не указана директория назначения!"));
        return;
    }

    fish.SetKey(ui->key->text().toStdString());
    int size = ui->listfile->count();

    if(ui->encryptButton->isChecked())
    {
       for(int i = 0; i < size; ++i)
       {
          file.setFile(ui->listfile->item(i)->text());
          QByteArray *bytes = readFile(file.absoluteFilePath());
          QByteArray encrypted;
          fish.Encrypt(&encrypted, *bytes);
          this->writeFile(&encrypted, QDir::fromNativeSeparators(ui->path->text())+tr("/") + file.fileName() + tr(".fish"));
          delete bytes;
       }
    }
    else if(ui->decryptButton->isChecked())
    {
       for(int i = 0; i < size; ++i)
       {
          file.setFile(ui->listfile->item(i)->text());
          QByteArray *bytes = readFile(file.absoluteFilePath());
          QByteArray decrypted;
          fish.Decrypt(&decrypted, *bytes);
          QString name = file.fileName();
          if(file.suffix() == "fish")
              name = name.left(name.length() - 5);
          this->writeFile(&decrypted, QDir::fromNativeSeparators(ui->path->text())+tr("/") + name);
          delete bytes;
       }
    }

    ui->listfile->clear();

    statusBar()->showMessage(tr("\t Корличество файлов в списке: %0").arg(ui->listfile->count()));
    QMessageBox::information(this, trUtf8("Информация"), trUtf8("Список обработан."));

    ui->listfile->setEnabled(false);
    ui->deleteItem->setEnabled(false);
    ui->start->setEnabled(false);
    this->setFocus();
    if(ui->openDir->isChecked())
        QDesktopServices::openUrl(QUrl("file:///" + ui->path->text()));

}
void MainWindow::seletctOutPathDir()
{
   QString path =  QDir::toNativeSeparators(QFileDialog::getExistingDirectory(this, trUtf8("Выберите директорию:"), "/home",
                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
   if(path.isEmpty())
       ui->path->setText(QDir::toNativeSeparators("C:/"));
   else
       ui->path->setText(path);
}

QByteArray* MainWindow::readFile(const QString &str)
{
    QFile file(str);
    QByteArray *qbytes = new QByteArray();

    if(file.open(QIODevice::ReadOnly)) {
        int size = file.bytesAvailable();

        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setVersion(QDataStream::Qt_5_1);

        char *bytes = new char[size];
        stream.readRawData(bytes, size);

        file.close();
        qbytes->append(bytes, size);

    }
    return qbytes;
}

void MainWindow::writeFile(QByteArray *bytes, const QString &str)
{
    QFile file(str);
    if(file.open(QIODevice::WriteOnly)) {
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian);
        stream.setVersion(QDataStream::Qt_5_1);
        stream.writeRawData(bytes->data(), bytes->size());
        file.close();
    }
}

void MainWindow::systemShow(QModelIndex)
{
   file.setFile(ui->listfile->currentItem()->text());
   QDesktopServices::openUrl(QUrl("file:///" + file.path()));
}

void MainWindow::showAbout()
{
    AboutDialog Aboutdlg;
    Aboutdlg.exec();
}

void MainWindow::selectItemList(QModelIndex )
{
  ui->deleteItem->setEnabled(true);
  file.setFile(ui->listfile->currentItem()->text());
  statusBar()->showMessage(tr("\t Всего файлов в списке: %0, размер выбраного файла: %1, дата последнего изменения: %2").
                           arg(ui->listfile->count()).
                           arg(QString::number((double)file.size()/1024).left(5).append(" КБ")).
                           arg(file.lastModified().toString("dd MM yyyy")));

}
