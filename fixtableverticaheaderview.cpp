/*
Fix8logviewer is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8logviewer Open Source FIX Log Viewer.
Copyright (C) 2010-14 David N Boosalis dboosalis@fix8.org, David L. Dight <fix@fix8.org>

Fix8logviewer is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8logviewer is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
#include "fixtableverticaheaderview.h"
#include "worksheetmodel.h"
#include "proxyFilter.h"
#include <QDebug>
#include <QLinearGradient>
#include <QPainter>
#include <QPen>
FixTableVerticaHeaderView::FixTableVerticaHeaderView(QWidget *parent) :
    QHeaderView(Qt::Vertical,parent),highLightOn(false),proxyFilter(0),proxyFilterOn(false),hightLightItems(0)
{
    //setClickable(true);
    _model = false;
    setSectionsClickable(true);
}
void  FixTableVerticaHeaderView::paintSection(QPainter * painter,
                                              const QRect &rect,
                                              int logicalIndex) const
{

    QStyleOptionHeader option;
    option.position = QStyleOptionHeader::Beginning;
    bool wasHighlighted = false;
    int proxyIndex;
    if (!highLightOn || !hightLightItems) {
        QHeaderView::paintSection(painter, rect, logicalIndex);
        return;
    }
    initStyleOption( &option );
    qint64 oldIndex;
    QStandardItem *item;
    QStandardItem *currentItem;
    currentItem  = _model->item(logicalIndex,0);
    QModelIndex oldmi;
    if (proxyFilterOn) {
        QListIterator <QStandardItem *> iter2(*hightLightItems);
        while(iter2.hasNext()) {
            item = iter2.next();
            QModelIndex m2 = item->index();
            QModelIndex l2 = proxyFilter->mapFromSource(m2);
            int lrow = l2.row();
            if (lrow == logicalIndex) {
                painter->save();
                painter->restore();
                QLinearGradient gradient(rect.topLeft(),rect.topRight());
                gradient.setColorAt(0,QColor(40,98,231,100));
                gradient.setColorAt(1,QColor(87,166,231,100));
                painter->fillRect(rect,QBrush(gradient));
                QRect lineRect = rect;
                lineRect.setRight(rect.right()-1);
                //lineRect.setBottom(rect.bottom()-1);
                QRect newRect(rect);
                newRect.setLeft(rect.left()+ 3);
                painter->drawText(newRect,Qt::AlignVCenter|Qt::AlignLeft,QString::number(logicalIndex+1));
                painter->drawRect(lineRect);
                wasHighlighted = true;
                break;
            }

        }
    }
    else {
        QListIterator <QStandardItem *> iter(*hightLightItems);
        while(iter.hasNext()) {
            item = iter.next();
            if (item == currentItem) {
                painter->save();
                painter->restore();
                QLinearGradient gradient(rect.topLeft(),rect.topRight());
                gradient.setColorAt(0,QColor(40,98,231,100));
                gradient.setColorAt(1,QColor(87,166,231,100));
                painter->fillRect(rect,QBrush(gradient));
                QRect lineRect = rect;
                lineRect.setRight(rect.right()-1);
                //lineRect.setBottom(rect.bottom()-1);
                QRect newRect(rect);
                newRect.setLeft(rect.left()+ 3);
                painter->drawText(newRect,Qt::AlignVCenter|Qt::AlignLeft,QString::number(logicalIndex+1));
                painter->drawRect(lineRect);
                wasHighlighted = true;
                break;
            }
        }
    }
    if (!wasHighlighted)
        QHeaderView::paintSection(painter, rect, logicalIndex);
}
void FixTableVerticaHeaderView::setHighlightList(QVector <qint32>list,QList<QStandardItem *> *items,bool turnOn)
{
    qint32 row;
    QModelIndex mi;
    QModelIndex newMi;
    highLightOn = turnOn;
    hightlightRows = list;
    hightLightItems = items;
    viewport()->repaint();
    //repaint();
}
void FixTableVerticaHeaderView::turnOnSearchHighLight(bool on)
{
    highLightOn  = on;

    viewport()->repaint();
}
void FixTableVerticaHeaderView::redoSearch()
{
    viewport()->repaint();
}
void FixTableVerticaHeaderView::setWorkModel(WorkSheetModel *m)
{
    _model = m;
}

void FixTableVerticaHeaderView::setProxyFilter(ProxyFilter *pf)
{
    proxyFilter = pf;
    if (!proxyFilter )
        proxyFilterOn = false;
    viewport()->repaint();
}
void FixTableVerticaHeaderView::setProxyFilterOn(bool b)
{
    proxyFilterOn = b;

    viewport()->repaint();

}
