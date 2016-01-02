
#include <QToolButton>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QPainter>
#include <QDebug>

#include "imagelineedit.h"

ImageLineEdit::ImageLineEdit (QWidget * parent):
  QLineEdit (parent),
  m_clearBtn (0),
  m_progress(0)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
  const QIcon icon = QIcon(":/images/images/light/appbar.clear.inverse.reflect.horizontal.png");
  const int iconSize = style ()->pixelMetric (QStyle::PM_SmallIconSize);

  m_clearBtn = new QToolButton (this);
  m_clearBtn->setStyleSheet (QLatin1String ("QToolButton{border:0;padding:0;margin:0}"));
  m_clearBtn->setToolButtonStyle (Qt::ToolButtonIconOnly);
  m_clearBtn->setFocusPolicy (Qt::NoFocus);
  m_clearBtn->setCursor (Qt::ArrowCursor);
  m_clearBtn->setIconSize (QSize (iconSize, iconSize));
  m_clearBtn->setIcon (icon);
  m_clearBtn->setFixedSize (m_clearBtn->iconSize () + QSize (2, 2));
  m_clearBtn->hide ();

  const int frameWidth = style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
  const QSize minSizeHint = minimumSizeHint ();
  const QSize buttonSize = m_clearBtn->size ();

  setStyleSheet (QString::fromLatin1 ("QLineEdit{padding-right:%1px}").arg (buttonSize.width () + frameWidth + 1));
  setMinimumSize (qMax (minSizeHint.width (), buttonSize.width () + frameWidth * 2 + 2),
                  qMax (minSizeHint.height (), buttonSize.height () + frameWidth * 2 + 2));

  connect (this, SIGNAL (textChanged (QString)), this, SLOT (updateClearButtonVisibility ()));
  connect (m_clearBtn, SIGNAL (clicked ()), this, SLOT (clear ()));
#else
  setClearButtonEnabled (true);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(4, 7, 0)
  setPlaceholderText (tr ("<binary file>"));
#endif
}

void ImageLineEdit::setProgress(int progress)
{
    if(progress > 100)
        progress = 100;
    if(progress < 0)
        progress = 0;

    m_progress = progress;

    repaint();
}

void
ImageLineEdit::resizeEvent (QResizeEvent * event)
{
  QLineEdit::resizeEvent (event);

#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
  const int frameWidth = style ()->pixelMetric (QStyle::PM_DefaultFrameWidth);
  const QRect editRect = rect();
  const QSize buttonSize = m_clearBtn->size ();

  m_clearBtn->move (editRect.right () - frameWidth - buttonSize.width (),
                       editRect.top () + (editRect.height () - buttonSize.height ()) / 2);
#endif
}

void
ImageLineEdit::updateClearButtonVisibility ()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 2, 0)
  m_clearBtn->setVisible (!text ().isEmpty ());
#endif
}

void ImageLineEdit::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);
    QStyleOptionFrame panel;
    initStyleOption(&panel);
    style()->drawPrimitive(QStyle::PE_PanelLineEdit, &panel, &painter, this);
    QRect backgroundRect = style()->subElementRect(QStyle::SE_LineEditContents, &panel, this);

    if(hasFocus()) QLineEdit::paintEvent(event);

    if(!hasFocus() && m_progress <= 100)
    {
        QPen oldPen = painter.pen();
        painter.setBrush(Qt::darkGreen);
        painter.setPen(Qt::transparent);
        int mid = (backgroundRect.width() / 100.0) * m_progress;
        QRect progressRect(backgroundRect.x(), backgroundRect.y(), mid, backgroundRect.height());
        painter.drawRect(progressRect);

        painter.setPen(oldPen);
        painter.drawText(backgroundRect,Qt::AlignLeft|Qt::AlignVCenter, this->text());
    }
}
