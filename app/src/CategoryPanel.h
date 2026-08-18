#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QLabel>
#include <QStringList>
#include <QVector>

class QPushButton;

class CategoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit CategoryPanel(const QStringList &categories, QWidget *parent = nullptr);

public slots:
    void applyTheme();

signals:
    void categorySelected(const QString &category);

protected:
    void paintEvent(QPaintEvent *) override;

private:
    struct Row { QPushButton *btn; QString value; };

    void addRow(const QString &label, const QString &value);
    QString buttonStyleSheet() const;

    QVBoxLayout   *m_layout    = nullptr;
    QButtonGroup  *m_group     = nullptr;
    QVector<Row>   m_rows;
    QLabel        *m_header    = nullptr;
};
