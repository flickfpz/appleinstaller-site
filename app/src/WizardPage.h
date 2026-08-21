#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVector>
#include <QMap>

#include "AppData.h"
#include "AppCard.h"

class WizardPage : public QWidget
{
    Q_OBJECT

public:
    WizardPage(const QString &title, const QVector<AppData> &apps,
               QWidget *parent = nullptr);

    QVector<QString> selectedIds() const;
    void setBackVisible(bool v);
    void setNextVisible(bool v);

public slots:
    void applyTheme();
    void selectAll();
    void deselectAll();

signals:
    void selectionChanged();
    void backClicked();
    void nextClicked();
    void installClicked();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void buildGrid(const QVector<AppData> &apps);

    QString m_title;
    QMap<QString, AppCard *> m_cards;

    QLabel      *m_titleLabel   = nullptr;
    QLabel      *m_countLabel   = nullptr;
    QPushButton *m_backBtn      = nullptr;
    QPushButton *m_nextBtn      = nullptr;
    QPushButton *m_installBtn   = nullptr;
    QPushButton *m_selectAllBtn = nullptr;
    QPushButton *m_deselectBtn  = nullptr;
    QScrollArea *m_scrollArea   = nullptr;
    QWidget     *m_gridContainer = nullptr;
    QGridLayout *m_grid         = nullptr;
};
