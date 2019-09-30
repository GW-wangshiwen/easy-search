#ifndef QUERY_H
#define QUERY_H

#include <QMainWindow>
#include "sqlite3.h"


namespace Ui {
class Query;
}

class Query : public QMainWindow
{
    Q_OBJECT

public:
    explicit Query(QWidget *parent = nullptr);
    ~Query();

    void set_db_path(char const* path);
    void init();
    void start();

private:
    Ui::Query *ui;
    const char * db_path;
    QString msg;
    bool connect_status;

    const char *get_dynamic_sql(QString keyword, QString reportor, QString date, QString capacity);
    QString get_msg(int rc, QString error_prefix);
    const char * get_db_path();
    void set_clear();
    bool is_connect();
};

#endif // QUERY_H
