/*
*  Copyright 2021 Jose Maria Castelo Ares
*
*  Contact: <jose.maria.castelo@gmail.com>
*  Repository: <https://github.com/jmcastelo/MorphogenGL>
*
*  This file is part of MorphogenGL.
*
*  MorphogenGL is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  MorphogenGL is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with MorphogenGL.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "edge.h"
#include "node.h"
#include "cycle.h"
#include "graphwidget.h"
#include "generator.h"
#include "focuslineedit.h"

#include <QPainter>
#include <QtMath>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QDoubleValidator>
#include <QSlider>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>

Edge::Edge(GraphWidget *graphWidget, Node *sourceNode, Node *destNode)
    : graph { graphWidget },
      source { sourceNode },
      dest { destNode }
{
    setAcceptedMouseButtons(Qt::NoButton);
    source->addEdge(this);
    dest->addEdge(this);

    adjust();

    for (Edge* edge: dest->edges())
        if (edge->destNode() == source)
            edge->adjust();

    connect(this, &Edge::blendFactorWidgetCreated, graph, &GraphWidget::blendFactorWidgetCreated);
    connect(this, &Edge::blendFactorWidgetToggled, graph, &GraphWidget::blendFactorWidgetToggled);

    if (graph->generator->isNode(source->id) && graph->generator->isNode(dest->id))
        constructBlendFactorWidget();
}

Edge::~Edge()
{
    blendFactorWidget->deleteLater();
}

void Edge::remove()
{
    source->removeEdge(this);
    dest->removeEdge(this);

    for (Edge* edge: dest->edges())
        if (edge->destNode() == source)
            edge->adjust();

    graph->removeEdge(this);
}

Node *Edge::sourceNode() const
{
    return source;
}

Node *Edge::destNode() const
{
    return dest;
}

void Edge::setPredge(bool set)
{
    predge = set;

    if (predge)
        graph->generator->setOperationInputType(source->id, dest->id, InputType::Blit);
    else
        graph->generator->setOperationInputType(source->id, dest->id, InputType::Normal);

    update();
}

QPointF Edge::intersectionPoint(Node *node, Node *other, QPointF offset, QLineF line)
{
    QPointF center = node->mapToScene(node->boundingRect().center());

    QPointF nodePoint = node->mapToScene(node->boundingRect().center() + offset);
    QPointF otherPoint = other->mapToScene(other->boundingRect().center() + offset);

    QRectF ellipseRect = node->mapRectToScene(node->boundingRect());
    qreal halfWidth = 0.5 * ellipseRect.width();
    qreal halfHeight = 0.5 * ellipseRect.height();

    if (!qFuzzyIsNull(line.x2() - line.x1()))
    {
        qreal slope = (line.y2() - line.y1()) / (line.x2() - line.x1());

        QLineF vertical(0.0, -10000.0, 0.0, 10000.0);
        QPointF point0;
        line.intersects(vertical, &point0);

        qreal a = halfHeight * halfHeight + halfWidth * halfWidth * slope * slope;
        qreal b = 2.0 * (slope * halfWidth * halfWidth * (point0.y() - center.y()) - halfHeight * halfHeight * center.x());
        qreal c = halfHeight * halfHeight * (center.x() * center.x() - halfWidth * halfWidth) + halfWidth * halfWidth * (point0.y() - center.y()) * (point0.y() - center.y());

        qreal q = b * b - 4.0 * a * c;

        qreal root1 = -(b + qSqrt(q)) / (2.0 * a);
        qreal root2 = (qSqrt(q) - b) / (2.0 * a);

        QPointF intersection1(root1, slope * root1 + point0.y());
        QPointF intersection2(root2, slope * root2 + point0.y());

        if (QLineF(otherPoint, intersection1).length() < QLineF(otherPoint, intersection2).length())
            return intersection1 - nodePoint;
        else
            return intersection2 - nodePoint;
    }
    else
    {
        qreal scaled = (nodePoint.x() - center.x()) / halfWidth;
        qreal yOff = halfHeight * qSqrt(1.0 - scaled * scaled);

        QPointF intersection1(nodePoint.x(), center.y() + yOff);
        QPointF intersection2(nodePoint.x(), center.y() - yOff);

        if (QLineF(otherPoint, intersection1).length() < QLineF(otherPoint, intersection2).length())
            return intersection1 - nodePoint;
        else
            return intersection2 - nodePoint;
    }
}

void Edge::adjust()
{
    if (!source || !dest)
        return;

    setLinkOffset();

    qreal scale = linkOffset * 0.25;
    qreal bWidth = qMin(source->boundingRect().width(), dest->boundingRect().width());

    QPointF offset(scale * bWidth, 0.0);

    QLineF line(mapFromItem(source, source->boundingRect().center() + offset), mapFromItem(dest, dest->boundingRect().center() + offset));

    prepareGeometryChange();

    QPointF sourceEdgeOffset = intersectionPoint(source, dest, offset, line);
    QPointF sp = line.p1() + sourceEdgeOffset;

    QPointF destEdgeOffset = intersectionPoint(dest, source, offset, line);
    QPointF dp = line.p2() + destEdgeOffset;

    if (!source->contains(mapToItem(source, dp)) && !dest->contains(mapToItem(dest, sp)))
    {
        sourcePoint = sp;
        destPoint = dp;
    }
    else
    {
        sourcePoint = destPoint = line.p1();
    }
}

void Edge::setLinkOffset()
{
    linkOffset = 0;

    for (Edge* edge: dest->edges())
    {
        if (edge->destNode() == source)
        {
            if (edge->linkOffset == 0 || edge->linkOffset == 1)
                linkOffset = -1;
            else
                linkOffset = 1;
        }
    }
}

void Edge::setAsPredge()
{
    predge = true;

    graph->generator->setOperationInputType(source->id, dest->id, InputType::Blit);

    update();
}

void Edge::setAsEdge()
{
    predge = false;

    graph->generator->setOperationInputType(source->id, dest->id, InputType::Normal);

    update();
}

void Edge::insertNode(QAction* action)
{
    graph->insertNodeBetween(action, this);
}

void Edge::constructBlendFactorWidget()
{
    FocusLineEdit* blendFactorLineEdit = new FocusLineEdit;
    blendFactorLineEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    QDoubleValidator* validator = new QDoubleValidator(0.0, 1.0, 6, blendFactorLineEdit);
    validator->setLocale(QLocale::English);
    blendFactorLineEdit->setValidator(validator);
    blendFactorLineEdit->setText(QString::number(graph->generator->blendFactor(source->id, dest->id)));

    connect(blendFactorLineEdit, &FocusLineEdit::focusOut, this, [=](){ blendFactorLineEdit->setText(QString::number(graph->generator->blendFactor(source->id, dest->id))); });

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setAlignment(Qt::AlignLeft);
    formLayout->addRow("Blend factor:", blendFactorLineEdit);

    QSlider* blendFactorSlider = new QSlider(Qt::Horizontal);
    blendFactorSlider->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    blendFactorSlider->setRange(0, 100000);
    blendFactorSlider->setSliderPosition(graph->generator->blendFactor(source->id, dest->id) * blendFactorSlider->maximum());

    connect(blendFactorSlider, &QAbstractSlider::valueChanged, this, [=](int value)
    {
        float factor = static_cast<float>(value) / blendFactorSlider->maximum();
        graph->generator->setBlendFactor(source->id, dest->id, factor);
        blendFactorLineEdit->setText(QString::number(factor));
        update();
    });

    connect(blendFactorLineEdit, &FocusLineEdit::returnPressed, this, [=]()
    {
        float factor = blendFactorLineEdit->text().toFloat();
        graph->generator->setBlendFactor(source->id, dest->id, factor);
        int index = factor * blendFactorSlider->maximum();
        blendFactorSlider->setSliderPosition(index);
        update();
    });

    connect(this, &Edge::blendFactorChanged, this, [=]()
    {
        float factor = graph->generator->blendFactor(source->id, dest->id);
        blendFactorLineEdit->setText(QString::number(factor));
        int index = factor * blendFactorSlider->maximum();
        blendFactorSlider->setSliderPosition(index);
    });

    QPushButton* closeButton = new QPushButton;
    closeButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    closeButton->setFixedHeight(10);
    closeButton->setStyleSheet("background-color: rgb(255, 0, 0); color: rgb(255, 255, 255)");

    connect(closeButton, &QPushButton::clicked, this, [&]()
    {
        blendFactorWidget->setVisible(false);
        emit blendFactorWidgetToggled(blendFactorWidget);
    });

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setSizeConstraint(QLayout::SetFixedSize);
    layout->addLayout(formLayout);
    layout->addWidget(blendFactorSlider);
    layout->addWidget(closeButton);

    blendFactorGroupBox = new QGroupBox(source->name + " - " + dest->name);
    blendFactorGroupBox->setLayout(layout);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(blendFactorGroupBox);

    blendFactorWidget = new QWidget;
    blendFactorWidget->setLayout(mainLayout);
    blendFactorWidget->setVisible(false);

    emit blendFactorWidgetCreated(blendFactorWidget);
}

void Edge::setBlendFactorGroupBoxTitle()
{
    blendFactorGroupBox->setTitle(source->name + " - " + dest->name);
}

void Edge::updateBlendFactor()
{
    emit blendFactorChanged();
}

void Edge::setBlendFactor()
{
    blendFactorWidget->setVisible(true);
    emit blendFactorWidgetToggled(blendFactorWidget);
}

void Edge::closeBlendFactorWidget()
{
    if (blendFactorWidget)
        blendFactorWidget->close();
}

QRectF Edge::boundingRect() const
{
    if (!source || !dest)
        return QRectF();

    return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(), destPoint.y() - sourcePoint.y())).normalized().adjusted(-10.0, -10.0, 10.0, 10.0);
}

QPainterPath Edge::shape() const
{
    QLineF line(sourcePoint, destPoint);
    QLineF normal1 = line.normalVector();
    normal1.setLength(5.0);
    QLineF normal2(sourcePoint, 2.0 * sourcePoint - normal1.p2());

    QPainterPath path;
    path.addPolygon(QPolygonF(QVector<QPointF>{ normal2.p2(), normal1.p2(), normal1.p2() + destPoint - sourcePoint, normal2.p2() + destPoint - sourcePoint }));
    path.closeSubpath();
    return path;
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!source || !dest)
        return;

    QLineF line(sourcePoint, destPoint);
    if (qFuzzyCompare(line.length(), qreal(0.0)))
        return;

    // Draw the line itself

    if (predge)
        painter->setPen(QPen(Qt::black, 2.0, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));
    else
        painter->setPen(QPen(Qt::black, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter->drawLine(line);

    if (line.length() > arrowSize)
    {
        // Draw the arrow
        double angle = std::atan2(-line.dy(), line.dx());

        QPointF destArrowP1 = destPoint + QPointF(sin(angle - M_PI / 2.5) * arrowSize, cos(angle - M_PI / 2.5) * arrowSize);
        QPointF destArrowP2 = destPoint + QPointF(sin(angle - M_PI + M_PI / 2.5) * arrowSize, cos(angle - M_PI + M_PI / 2.5) * arrowSize);

        painter->setPen(QPen(Qt::black, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::black);
        painter->drawPolygon(QPolygonF() << line.p2() << destArrowP1 << destArrowP2);
    }

    if (paintBlendFactor)
    {
        QString text = QString::number(graph->generator->blendFactor(source->id, dest->id));

        QFontMetricsF fontMetrics(scene()->font());
        qreal textWidth = fontMetrics.boundingRect(text).width();

        painter->setFont(scene()->font());
        painter->drawText(line.center() - QPointF(0.5 * textWidth, 0.0), text);
    }
}

void Edge::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    if (qgraphicsitem_cast<OperationNode*>(source))
    {
        if (!predge)
            menu.addAction("Set as predge", this, &Edge::setAsPredge);
        else
        {
            bool showAcion = true;

            foreach (Cycle* cycle, cycleList)
            {
                if (cycle->numPredges() == 1)
                {
                    showAcion = false;
                    break;
                }
            }

            if (showAcion)
                menu.addAction("Set as edge", this, &Edge::setAsEdge);
        }
    }

    menu.addAction("Set blend factor", this, &Edge::setBlendFactor);

    QMenu *operationsMenu = menu.addMenu("Insert operation");

    foreach (QString opName, graph->generator->availableOperations)
    {
        QAction* action = operationsMenu->addAction(opName);
        action->setData(QVariant(QCursor::pos()));
    }

    connect(operationsMenu, &QMenu::triggered, this, &Edge::insertNode);

    menu.addAction("Remove", this, &Edge::remove);

    menu.exec(event->screenPos());

    //QGraphicsObject::contextMenuEvent(event);
}
