#ifndef MOVIEDETAILS_H
#define MOVIEDETAILS_H

#include <QWidget>
#include <QTreeWidgetItem>

namespace Ui {
class MovieDetails;
}

class MovieDetails : public QWidget
{
    Q_OBJECT

public:
    explicit MovieDetails(QString title, QWidget *parent = nullptr);
    ~MovieDetails();

private slots:
    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::MovieDetails *ui;
};

#endif // MOVIEDETAILS_H
