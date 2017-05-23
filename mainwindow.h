#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QFileInfo>

#include "blowfish.h"

class QLabel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:

    Ui::MainWindow *ui;
    QLabel *label;


    Blowfish fish;
    QFileInfo file;
    //Функция чтения файла возвращает массив прочитанных байт
    QByteArray *readFile(const QString &str);
    //функция записи шифрованных/дешифрованных байт в файл
    void writeFile(QByteArray *bytes, const QString &str);

private slots:

    //Обработчик (слот) добовления файла всписок
    void addFile();
    //Обработчик (слот) удаления файла из списка
    void deleteFile();
    //Обработчик (слот) выполнения выбранной операции
    void execute();
    //Обработчик (слот) выбора дериктории назначения
    void seletctOutPathDir();
    //Обработчик (слот) открывает каталок в проваднике Windows
    void systemShow(QModelIndex);
    //Обработчик (слот) о программе
    void showAbout();
    //Обработчик (слот) выбор файла в списке
    void selectItemList(QModelIndex);
};

#endif // MAINWINDOW_H
