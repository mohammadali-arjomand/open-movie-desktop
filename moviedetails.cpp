// moviedetails.cpp
#include "moviedetails.h"
#include "ui_moviedetails.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QSettings>
#include <QApplication>
#include <QClipboard>

extern QSettings settings;

MovieDetails::MovieDetails(QString title, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MovieDetails)
{
    ui->setupUi(this);
    setWindowTitle(title);
    ui->label->setText(title);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(settings.value("dbPath").toString());

    if (!db.open()) {
        qDebug() << "Error opening DB:" << db.lastError().text();
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT season, episode, quality FROM files WHERE title = :title");
    query.bindValue(":title", title);

    if (!query.exec()) {
        qDebug() << "DB Error:" << query.lastError().text();
        return;
    }

    // Season -> Episode -> Quality
    QMap<int, QTreeWidgetItem*> seasonItems;
    QMap<QString, QTreeWidgetItem*> episodeItems; // key = "season-episode"

    while (query.next()) {
        int season = query.value("season").toInt();
        int episode = query.value("episode").toInt();
        QString quality = query.value("quality").toString();

        // Season
        QTreeWidgetItem *seasonItem;
        if (!seasonItems.contains(season)) {
            seasonItem = new QTreeWidgetItem(ui->treeWidget);
            seasonItem->setText(0, QString("Season %1").arg(season));
            seasonItems[season] = seasonItem;
        } else {
            seasonItem = seasonItems[season];
        }

        // Episode (با lookup)
        QString episodeKey = QString("%1-%2").arg(season).arg(episode);
        QTreeWidgetItem *episodeItem;
        if (!episodeItems.contains(episodeKey)) {
            episodeItem = new QTreeWidgetItem(seasonItem);
            episodeItem->setText(0, QString("Episode %1").arg(episode));
            episodeItems[episodeKey] = episodeItem;
        } else {
            episodeItem = episodeItems[episodeKey];
        }

        // Quality
        QTreeWidgetItem *qualityItem = new QTreeWidgetItem(episodeItem);
        qualityItem->setText(0, quality);
    }

    ui->treeWidget->setHeaderHidden(true);
    ui->treeWidget->collapseAll();


}

MovieDetails::~MovieDetails()
{
    delete ui;
}

void MovieDetails::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    if (!item) return;

    // فرض می‌کنیم ساختار درخت:
    // Season -> Episode -> Quality
    // اسم Quality توی text(0)
    QString quality = item->text(0);

    // فرض می‌کنیم parent دوم اسم Episode و parent سوم اسم Season است
    QString episode, season;
    if (item->parent()) episode = item->parent()->text(0);
    if (item->parent() && item->parent()->parent()) season = item->parent()->parent()->text(0);

    // باز کردن دیتابیس
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "Database not open!";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT url FROM files WHERE season = :season AND episode = :episode AND quality = :quality");
    query.bindValue(":season", season);
    query.bindValue(":episode", episode);
    query.bindValue(":quality", quality);

    if (!query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return;
    }

    if (query.next()) {
        QString url = query.value(0).toString();

        // کپی به Clipboard
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(url);
        qDebug() << "Copied URL to clipboard:" << url;
    }
}
