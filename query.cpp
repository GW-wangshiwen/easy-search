#include "query.h"
#include "ui_query.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <QMessageBox>

using namespace std;

Query::Query(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Query)
{
    ui->setupUi(this);

    connect(ui->pushButton_query, &QPushButton::clicked, this, &Query::start);
    connect(ui->pushButton_clear, &QPushButton::clicked, this, &Query::set_clear);
}

Query::~Query()
{
    delete ui;
}

void Query::set_db_path(const char *path)
{
    db_path = path;
}

void Query::set_clear()
{
    ui->comboBox_cap->setCurrentText("");
    ui->comboBox_date->setCurrentText("");
    ui->comboBox_reportor->setCurrentText("");
    ui->lineEdit_keyword->clear();
    /*ui->tableWidget_result->setRowCount(0);
    ui->tableWidget_result->setColumnCount(0);*/
}

char const* Query::get_db_path()
{
    return db_path;
}

bool Query::is_connect()
{
    return connect_status;
}

void Query::init()
{
    int rc;
    char const* sql;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int nrow = 0;       //查询到数据的行数
    int ncolumn = 0;   //查询到数据的列数
    char **azResult;   //二维数组存放结果
    char *zErrMsg = nullptr; //错误信息

    rc = sqlite3_open(get_db_path(), &db);
    msg = get_msg(rc, sqlite3_errmsg(db));

    sql = "SELECT 项目名,执行人,日期,容量 from 工程技术中心归档";

    rc = sqlite3_prepare_v2(db, sql, int(strlen(sql)), &stmt, nullptr);
    msg = get_msg(rc, sqlite3_errmsg(db));

    rc = sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg ); //查询数据库
    msg = get_msg(rc, sqlite3_errmsg(db));

    ui->comboBox_reportor->addItem("");
    ui->comboBox_date->addItem("");
    ui->comboBox_cap->addItem("");
    for (int i = 0; i <= nrow; i++)
    {
        for (int j = 0; j < ncolumn; j++)
        {
            if (i != 0)
            {
                switch (j) {
                case 1:
                    if (-1 == ui->comboBox_reportor->findText(azResult[i*ncolumn+j]))
                        ui->comboBox_reportor->addItem(azResult[i*ncolumn+j]);
                    break;
                case 2:
                    if (-1 == ui->comboBox_date->findText(azResult[i*ncolumn+j]))
                        ui->comboBox_date->addItem(azResult[i*ncolumn+j]);
                    break;
                case 3:
                    if (bool(azResult[i*ncolumn+j]) && -1 == ui->comboBox_cap->findText(azResult[i*ncolumn+j]))
                        ui->comboBox_cap->addItem(azResult[i*ncolumn+j]);
                    break;
                default:
                    break;
                }
            }
        }
    }
    sqlite3_free_table(azResult);     //释放掉 azResult的内存空间

    sqlite3_finalize(stmt);     //销毁一个SQL语句对象

    /*关闭数据库连接对象*/
    sqlite3_close(db);
    ui->statusBar->showMessage(msg);
}

const char *Query::get_dynamic_sql(QString project, QString reportor, QString date, QString capacity)
{
    string where_stmt("SELECT 项目名,执行人,日期,容量 FROM 工程技术中心归档 WHERE");
    string logic_word(" AND");

    for (int i = 0; i < 4; i++)
    {
        logic_word = " AND";
        switch (i) {
        case 0:
            if (!project.isEmpty())
                where_stmt += " 项目名 LIKE '%" + project.toStdString() + "%'";
            break;
        case 1:
            if (!reportor.isEmpty()){
                if (project.isEmpty())
                    logic_word.clear();
                where_stmt += logic_word +" 执行人='" + reportor.toStdString() + "'";
            }
            break;
        case 2:
            if (!date.isEmpty()){
                if (project.isEmpty() && reportor.isEmpty())
                    logic_word.clear();
                where_stmt += logic_word + " 日期='" + date.toStdString() + "'";
            }
            break;
        case 3:
            if (!capacity.isEmpty()){
                if (project.isEmpty() && reportor.isEmpty() && date.isEmpty())
                    logic_word.clear();
                where_stmt += logic_word + " 容量='" + capacity.toStdString() + "'";
            }
            break;
        }
    }
    const char *sql_where = where_stmt.c_str();
    return sql_where;
}

QString Query::get_msg(int rc, QString error)
{
    if (rc != SQLITE_OK){
        msg = QString(error);
        connect_status = false;
    }
    else {
        connect_status = true;
        msg = "ready";
    }

    return msg;
}

void Query::start()
{
    int rc;
    const char* sql;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int nrow = 0;       //查询到数据的行数
    int ncolumn = 0;   //查询到数据的列数
    char **azResult;   //二维数组存放结果
    char *zErrMsg = nullptr; //错误信息

    rc = sqlite3_open(get_db_path(), &db);
    msg = get_msg(rc, sqlite3_errmsg(db));
    if (!is_connect())
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Info");
        msgBox.setText("数据库未连接");
        msgBox.exec();
    }
    else {
        QString project = ui->lineEdit_keyword->text();
        QString reportor = ui->comboBox_reportor->currentText();
        QString date = ui->comboBox_date->currentText();
        QString capacity = ui->comboBox_cap->currentText();

        sql = get_dynamic_sql(project, reportor, date, capacity);

        rc = sqlite3_prepare_v2(db, sql, int(strlen(sql)), &stmt, nullptr);
        msg = get_msg(rc, sqlite3_errmsg(db));

        rc = sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg ); //查询数据库
        msg = get_msg(rc, sqlite3_errmsg(db));

//        cout << "nrow: " << nrow << endl;
//        cout << "ncolumn: " << ncolumn << endl;
        if (nrow == 0)
        {
            QString info_msg = (rc == 0) ? "未找到" : QString(zErrMsg);
            QMessageBox msgBox;
            msgBox.setWindowTitle("Info");
            msgBox.setText(info_msg);
            msgBox.exec();
        }
        else
        {
            ui->tableWidget_result->setRowCount(nrow);
            ui->tableWidget_result->setColumnCount(ncolumn);
            QTableWidgetItem *header_project = new QTableWidgetItem("项目");
            ui->tableWidget_result->setHorizontalHeaderItem(0, header_project);
            QTableWidgetItem *header_reportor = new QTableWidgetItem("报告人");
            ui->tableWidget_result->setHorizontalHeaderItem(1, header_reportor);
            QTableWidgetItem *header_date = new QTableWidgetItem("日期");
            ui->tableWidget_result->setHorizontalHeaderItem(2, header_date);
            QTableWidgetItem *header_capacity = new QTableWidgetItem("容量");
            ui->tableWidget_result->setHorizontalHeaderItem(3, header_capacity);
            ui->tableWidget_result->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
            for (int i = 0; i <= nrow; i++)
            {
                for (int j = 0; j < ncolumn; j++)
                {
                    if (i != 0)
                    {
                        QTableWidgetItem *tw = new QTableWidgetItem(azResult[i*ncolumn+j]);
                        ui->tableWidget_result->setItem(i-1, j, tw);
                    }
                }
            }
        }

        sqlite3_free_table(azResult);     //释放掉 azResult的内存空间

        sqlite3_finalize(stmt);     //销毁一个SQL语句对象

        /*关闭数据库连接对象*/
        sqlite3_close(db);
        ui->statusBar->showMessage(msg);
    }
}
