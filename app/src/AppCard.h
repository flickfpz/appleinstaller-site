#pragma once
#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include "AppData.h"

class AppCard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal hoverOpacity READ hoverOpacity WRITE setHoverOpacity)
    Q_PROPERTY(qreal pressScale   READ pressScale   WRITE setPressScale)

public:
    explicit AppCard(const AppData &data, QWidget *parent = nullptr);

    bool    isChecked() const { return m_checked; }
    void    setChecked(bool checked);
    void    setAvailable(bool available);   // dims card when OS doesn't support it
    bool    isAvailable() const { return m_available; }
    AppData appData()   const { return m_data; }

public slots:
    void applyTheme();

signals:
    void selectionChanged(const QString &id, bool selected);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    qreal hoverOpacity() const     { return m_hoverOpacity; }
    void  setHoverOpacity(qreal v) { m_hoverOpacity = v; update(); }
    qreal pressScale()   const     { return m_pressScale; }
    void  setPressScale(qreal v)   { m_pressScale = v; update(); }

    static QPixmap makeIconPixmap(const QString &iconName, int size);
    void toggle();

    AppData m_data;
    bool    m_checked      = false;
    bool    m_available    = true;    // false = unavailable on current OS
    qreal   m_hoverOpacity = 0.0;
    qreal   m_pressScale   = 1.0;

    QLabel *m_iconLabel  = nullptr;
    QLabel *m_nameLabel  = nullptr;
    QLabel *m_descLabel  = nullptr;
    QLabel *m_verLabel   = nullptr;

    QPropertyAnimation *m_hoverAnim = nullptr;
    QPropertyAnimation *m_pressAnim = nullptr;
};
