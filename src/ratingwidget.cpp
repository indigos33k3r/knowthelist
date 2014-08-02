/*
    Copyright 2010, David Sansome <me@davidsansome.com>
    Copyright (C) 2005-2014 Mario Stephan <mstephan@shared-files.de>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ratingwidget.h"

#include <QMouseEvent>
#include <QStylePainter>
#include <QtDebug>

const int RatingPainter::kStarCount;
const int RatingPainter::kStarSize;

RatingPainter::RatingPainter() {
  // Load the base pixmaps
  QPixmap on(":/star-on.png");
  QPixmap off(":/star-off.png");

  // Generate the 10 states, better to do it now than on the fly
  for (int i=0 ; i<kStarCount*2+1 ; ++i) {
    const float rating = float(i) / 2.0;

    // Clear the pixmap
    stars_[i] = QPixmap(kStarSize * kStarCount, kStarSize);
    stars_[i].fill(Qt::transparent);
    QPainter p(&stars_[i]);

    // Draw the stars
    int x = 0;
    for (int i=0 ; i<kStarCount ; ++i, x+=kStarSize) {
      const QRect rect(x, 0, kStarSize, kStarSize);

      if (rating - 0.25 <= i) {
        // Totally empty
        p.drawPixmap(rect, off);
      } else if (rating - 0.75 <= i) {
        // Half full
        const QRect target_left(rect.x(), rect.y(), kStarSize/2, kStarSize);
        const QRect target_right(rect.x() + kStarSize/2, rect.y(), kStarSize/2, kStarSize);
        const QRect source_left(0, 0, kStarSize/2, kStarSize);
        const QRect source_right(kStarSize/2, 0, kStarSize/2, kStarSize);
        p.drawPixmap(target_left, on, source_left);
        p.drawPixmap(target_right, off, source_right);
      } else {
        // Totally full
        p.drawPixmap(rect, on);
      }
    }
  }
}

QRect RatingPainter::Contents(const QRect& rect) {
  const int width = kStarSize * kStarCount;
  return QRect(rect.x(), rect.y(), width, rect.height());
}

double RatingPainter::RatingForPos(const QPoint& pos, const QRect& rect) {
  const QRect contents = Contents(rect);
  const double raw = double(pos.x() - contents.left()) / contents.width();

  // Round to the nearest 0.1
  return double(int(raw * kStarCount * 2 + 0.5)) / (kStarCount * 2);
}

void RatingPainter::Paint(QPainter* painter, const QRect& rect, float rating) const {
  QSize size(qMin(kStarSize*kStarCount, rect.width()),
             qMin(kStarSize, rect.height()));
  QPoint pos(rect.x(), rect.y());

  rating *= kStarCount;

  // Draw the stars
  const int star = qBound(0, int(rating*2.0 + 0.5), kStarCount*2);
  painter->drawPixmap(QRect(pos, size), stars_[star], QRect(QPoint(0,0), size));
}


RatingWidget::RatingWidget(QWidget* parent)
  : QWidget(parent),
    rating_(0.0),
    hover_rating_(-1.0)
{
  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  setMouseTracking(true);
}

QSize RatingWidget::sizeHint() const {
  return QSize(RatingPainter::kStarSize * (RatingPainter::kStarCount+2),
               RatingPainter::kStarSize);
}

void RatingWidget::setRating(float rating) {
  rating_ = rating;
  update();
}

void RatingWidget::paintEvent(QPaintEvent* e) {
  QStylePainter p(this);

  // Draw the stars
  painter_.Paint(&p, rect(), hover_rating_ == -1.0 ? rating_ : hover_rating_);
}

void RatingWidget::mousePressEvent(QMouseEvent* e) {
  rating_ = RatingPainter::RatingForPos(e->pos(), rect());
  emit RatingChanged(rating_);
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e) {
  hover_rating_ = RatingPainter::RatingForPos(e->pos(), rect());
  update();
}

void RatingWidget::leaveEvent(QEvent*) {
  hover_rating_ = -1.0;
  update();
}