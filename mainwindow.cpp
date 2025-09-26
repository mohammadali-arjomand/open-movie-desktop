#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QSortFilterProxyModel>
#include "moviedetails.h"

QSettings settings;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Open Movie");

    if (settings.value("dbPath").isValid()) {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(settings.value("dbPath").toString());

        if (!db.open()) {
            qDebug() << "Error opening DB:" << db.lastError().text();
            return;
        }

        QSqlQueryModel *model = new QSqlQueryModel;
        model->setQuery("SELECT DISTINCT title FROM files");

        QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
        proxy->setSourceModel(model);
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy->setFilterKeyColumn(0);

        ui->listView->setModel(proxy);

        connect(ui->lineEdit, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);

        db.close();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionOpen_new_database_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Choose Database",
        "",
        "Sqlite Database (*.db);;All files (*)"
    );

    if (!fileName.isEmpty()) {
        settings.setValue("dbPath", fileName);
    }

}


void MainWindow::on_actionClose_database_triggered()
{
    settings.remove("dbPath");
}


void MainWindow::on_actionQuit_triggered()
{
    QApplication::quit();
}


void MainWindow::on_pushButton_clicked()
{
    qDebug() << settings.value("dbPath").toString();
}


void MainWindow::on_listView_clicked(const QModelIndex &index)
{
    QSortFilterProxyModel *proxy = qobject_cast<QSortFilterProxyModel*>(ui->listView->model());
    if (!proxy) return;

    QModelIndex sourceIndex = proxy->mapToSource(index);

    QSqlQueryModel *model = qobject_cast<QSqlQueryModel*>(proxy->sourceModel());
    if (!model) return;

    QString title = model->data(sourceIndex).toString();
    qDebug() << "Clicked title:" << title;

    MovieDetails *details = new MovieDetails(title, nullptr);
    details->show();


}

