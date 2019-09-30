#include "query.h"
#include <QApplication>
#include <QDebug>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Query w;

//    qDebug() << QDir::currentPath();

    w.setWindowTitle("EasySearch");
    QIcon win_ico(":/icon/search.png");
    w.setWindowIcon(win_ico);
    w.set_db_path(".\\database\\etc_projects.db");
    w.init();
    w.show();

    return a.exec();
}
