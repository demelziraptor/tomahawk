/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2012 Teo Mrnjavac <teo@kde.org>
 *   Copyright 2012 Leo Franchi <lfranchi@kde.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SLIDESWITCHBUTTON_H
#define SLIDESWITCHBUTTON_H

#include <QPushButton>
#include <QColor>
#include <QPixmap>
#include <QFont>
#include <QWeakPointer>

class QMouseEvent;
class QMouseEvent;
class QPropertyAnimation;

class SlideSwitchButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY( qreal knobX READ knobX WRITE setKnobX )
    Q_PROPERTY( QColor baseColorTop READ baseColorTop WRITE setBaseColorTop )
    Q_PROPERTY( QColor baseColorBottom READ baseColorBottom WRITE setBaseColorBottom )
public:
    explicit SlideSwitchButton( QWidget* parent = 0 );
    explicit SlideSwitchButton( const QString& checkedText,
                                const QString& uncheckedText,
                                QWidget* parent = 0 );

    void init();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const { return sizeHint(); }

    //the back check-state cannot be changed by the user, only programmatically
    //to notify that the user-requested operation has completed
    void setBackChecked( bool state );
    bool backChecked() const;

    void setKnobX( qreal x ) { m_knobX = x; repaint(); }
    qreal knobX() const { return m_knobX; }

    void setBaseColorTop( const QColor& color ) { m_baseColorTop = color; repaint(); }
    QColor baseColorTop() const { return m_baseColorTop; }

    void setBaseColorBottom( const QColor& color ) { m_baseColorBottom = color; }
    QColor baseColorBottom() const { return m_baseColorBottom; }

protected:
    void paintEvent( QPaintEvent* event );

    virtual void mousePressEvent( QMouseEvent* e );
    virtual void mouseReleaseEvent( QMouseEvent* e );
    virtual void mouseMoveEvent(QMouseEvent* e);
private slots:
    void onCheckedStateChanged();

private:
    void createKnob();

    QPixmap m_knob;

    QString m_checkedText;
    QString m_uncheckedText;
    QColor m_baseColorTop, m_baseColorBottom;
    QColor m_textColor;
    QColor m_backUncheckedColorTop, m_backUncheckedColorBottom;
    QColor m_backCheckedColorTop, m_backCheckedColorBottom;
    QFont m_textFont; //needed for sizeHint
    bool m_backChecked;
    qreal m_knobX;

    QPoint m_mouseDownPos;

    QWeakPointer<QPropertyAnimation> m_backTopAnimation;
    QWeakPointer<QPropertyAnimation> m_backBottomAnimation;
    QWeakPointer<QPropertyAnimation> m_knobAnimation;
};

#endif // SLIDESWITCHBUTTON_H
