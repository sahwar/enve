#include "edge.h"
#include "MovablePoints/nodepoint.h"
#include "MovablePoints/ctrlpoint.h"
#include "Boxes/boundingbox.h"
#include "global.h"
#include "Animators/PathAnimators/vectorpathanimator.h"
#include "pointhelpers.h"
#include "GUI/mainwindow.h"
#include "Animators/transformanimator.h"

VectorPathEdge::VectorPathEdge(NodePoint *pt1, NodePoint *pt2) {
    setPoint1(pt1);
    setPoint2(pt2);
}

void VectorPathEdge::getNewRelPosForKnotInsertionAtT(const QPointF &P0,
                                                     QPointF *P1_ptr,
                                                     QPointF *P2_ptr,
                                                     const QPointF &P3,
                                                     QPointF *new_p_ptr,
                                                     QPointF *new_p_start_ptr,
                                                     QPointF *new_p_end_ptr,
                                                     const qreal &t) {
    QPointF P1 = *P1_ptr;
    QPointF P2 = *P2_ptr;
    QPointF P0_1 = (1-t)*P0 + t*P1;
    QPointF P1_2 = (1-t)*P1 + t*P2;
    QPointF P2_3 = (1-t)*P2 + t*P3;

    QPointF P01_12 = (1-t)*P0_1 + t*P1_2;
    QPointF P12_23 = (1-t)*P1_2 + t*P2_3;

    QPointF P0112_1223 = (1-t)*P01_12 + t*P12_23;

    *P1_ptr = P0_1;
    *new_p_start_ptr = P01_12;
    *new_p_ptr = P0112_1223;
    *new_p_end_ptr = P12_23;
    *P2_ptr = P2_3;
}


void VectorPathEdge::getNewRelPosForKnotInsertionAtTSk(const SkPoint &P0,
                                                     SkPoint *P1_ptr,
                                                     SkPoint *P2_ptr,
                                                     SkPoint P3,
                                                     SkPoint *new_p_ptr,
                                                     SkPoint *new_p_start_ptr,
                                                     SkPoint *new_p_end_ptr,
                                                     const SkScalar &t) {
    SkPoint P1 = *P1_ptr;
    SkPoint P2 = *P2_ptr;
    SkPoint P0_1 = P0*(1-t) + P1*t;
    SkPoint P1_2 = P1*(1-t) + P2*t;
    SkPoint P2_3 = P2*(1-t) + P3*t;

    SkPoint P01_12 = P0_1*(1-t) + P1_2*t;
    SkPoint P12_23 = P1_2*(1-t) + P2_3*t;

    SkPoint P0112_1223 = P01_12*(1-t) + P12_23*t;

    *P1_ptr = P0_1;
    *new_p_start_ptr = P01_12;
    *new_p_ptr = P0112_1223;
    *new_p_end_ptr = P12_23;
    *P2_ptr = P2_3;
}

qreal VectorPathEdge::getLength(const QPointF &p0Pos,
                      const QPointF &p1EndPos,
                      const QPointF &p2StartPos,
                      const QPointF &p3Pos) {
    qreal length = 0.;

    qreal tInc = 0.01;
    qreal t = 0.;
    QPointF lastPoint = p0Pos;
    while(true) {
        QPointF currentPoint = getPosBetweenPointsAtT(t + tInc,
                                                      p0Pos,
                                                      p1EndPos,
                                                      p2StartPos,
                                                      p3Pos);
        qreal lenInc = pointToLen(currentPoint - lastPoint);
        if(lenInc > 5) {
            tInc = tInc * 4 / lenInc;
            continue;
        }
        if(lenInc < 3) {
            t += tInc;
            tInc = tInc * 4 / lenInc;
        } else {
            t += tInc;
        }
        length += lenInc;
        lastPoint = currentPoint;
        if(t > 0.999) {
            break;
        }
    }

    return length;
}

qreal VectorPathEdge::getLength(const QPointF &p0Pos,
                        const QPointF &p1EndPos,
                        const QPointF &p2StartPos,
                        const QPointF &p3Pos,
                        int divisions) {
    divisions++; // so that 'i' starts at 1, not 0
    qreal length = 0.;

    QPointF lastPoint = p0Pos;
    for(int i = 1; i < divisions; i++) {
        qreal t = i/static_cast<qreal>(divisions);
        QPointF currentPoint = getPosBetweenPointsAtT(t,
                                                      p0Pos,
                                                      p1EndPos,
                                                      p2StartPos,
                                                      p3Pos);
        length += pointToLen(currentPoint - lastPoint);
        lastPoint = currentPoint;
    }
    length += pointToLen(p3Pos - lastPoint);

    return length;
}

QPointF VectorPathEdge::getPosBetweenPointsAtT(const qreal &t,
                                     const QPointF &p0Pos,
                                     const QPointF &p1EndPos,
                                     const QPointF &p2StartPos,
                                     const QPointF &p3Pos) {
    qreal x0 = p0Pos.x();
    qreal y0 = p0Pos.y();
    qreal x1 = p1EndPos.x();
    qreal y1 = p1EndPos.y();
    qreal x2 = p2StartPos.x();
    qreal y2 = p2StartPos.y();
    qreal x3 = p3Pos.x();
    qreal y3 = p3Pos.y();

    return QPointF(calcCubicBezierVal(x0, x1, x2, x3, t),
                   calcCubicBezierVal(y0, y1, y2, y3, t) );
}

QPointF VectorPathEdge::getRelPosBetweenPointsAtT(const qreal &t,
                                        NodePoint *point1,
                                        NodePoint *point2) {
    if(point1 == nullptr) return point2->getRelativePos();
    if(point2 == nullptr) return point1->getRelativePos();

    CtrlPoint *point1EndPt = point1->getEndCtrlPt();
    CtrlPoint *point2StartPt = point2->getStartCtrlPt();
    QPointF p0Pos = point1->getRelativePos();
    QPointF p1Pos = point1EndPt->getRelativePos();
    QPointF p2Pos = point2StartPt->getRelativePos();
    QPointF p3Pos = point2->getRelativePos();

    return getPosBetweenPointsAtT(t, p0Pos, p1Pos, p2Pos, p3Pos);
}

QPointF VectorPathEdge::getAbsPosBetweenPointsAtT(const qreal &t,
                                        NodePoint *point1,
                                        NodePoint *point2) {
    if(point1 == nullptr) return point2->getAbsolutePos();
    if(point2 == nullptr) return point1->getAbsolutePos();

    CtrlPoint *point1EndPt = point1->getEndCtrlPt();
    CtrlPoint *point2StartPt = point2->getStartCtrlPt();
    QPointF p0Pos = point1->getAbsolutePos();
    QPointF p1Pos = point1EndPt->getAbsolutePos();
    QPointF p2Pos = point2StartPt->getAbsolutePos();
    QPointF p3Pos = point2->getAbsolutePos();

    return getPosBetweenPointsAtT(t, p0Pos, p1Pos, p2Pos, p3Pos);
}

QPointF VectorPathEdge::getRelPosAtT(const qreal &t) {
    return getRelPosBetweenPointsAtT(t, mPoint1, mPoint2);
}

QPointF VectorPathEdge::getAbsPosAtT(const qreal &t) {
    return getAbsPosBetweenPointsAtT(t, mPoint1, mPoint2);
}

void VectorPathEdge::makePassThrough(const QPointF &absPos) {
    if(!mPoint2->isStartCtrlPtEnabled() ) {
        mPoint2->setStartCtrlPtEnabled(true);
    }
    if(!mPoint1->isEndCtrlPtEnabled() ) {
        mPoint1->setEndCtrlPtEnabled(true);
    }


    QPointF p0Pos = mPoint1->getAbsolutePos();
    QPointF p1Pos = mPoint1EndPt->getAbsolutePos();
    QPointF p2Pos = mPoint2StartPt->getAbsolutePos();
    QPointF p3Pos = mPoint2->getAbsolutePos();

    if(!mEditPath) {
        auto parentTransform =
                GetAsPtr(mPoint1->getParentTransform(),
                         BoxTransformAnimator);
        BoundingBox* parentBox = parentTransform->getParentBox();
        NodePointValues p1Values = mPoint1->getPointValues();
        p0Pos = parentBox->getCombinedTransform().map(
                    p1Values.pointRelPos);
        p1Pos = parentBox->getCombinedTransform().map(
                    p1Values.endRelPos);
        NodePointValues p2Values = mPoint2->getPointValues();
        p2Pos = parentBox->getCombinedTransform().map(
                    p2Values.startRelPos);
        p3Pos = parentBox->getCombinedTransform().map(
                    p2Values.pointRelPos);
    }

    qreal x0 = p0Pos.x();
    qreal y0 = p0Pos.y();
    qreal x1 = p1Pos.x();
    qreal y1 = p1Pos.y();
    qreal x2 = p2Pos.x();
    qreal y2 = p2Pos.y();
    qreal x3 = p3Pos.x();
    qreal y3 = p3Pos.y();

    qreal dx = absPos.x() - calcCubicBezierVal(x0, x1, x2, x3, mPressedT);
    qreal dy = absPos.y() - calcCubicBezierVal(y0, y1, y2, y3, mPressedT);
    while(dx*dx + dy*dy > 1.) {
        x1 += (1. - mPressedT)*dx;
        y1 += (1. - mPressedT)*dy;
        x2 += mPressedT*dx;
        y2 += mPressedT*dy;

        dx = absPos.x() - calcCubicBezierVal(x0, x1, x2, x3, mPressedT);
        dy = absPos.y() - calcCubicBezierVal(y0, y1, y2, y3, mPressedT);
    }


    mPoint1EndPt->moveToAbs(QPointF(x1, y1) );
    mPoint2StartPt->moveToAbs(QPointF(x2, y2) );
}

void VectorPathEdge::finishPassThroughTransform() {
    mPoint1EndPt->finishTransform();
    mPoint2StartPt->finishTransform();
}

void VectorPathEdge::startPassThroughTransform() {
    mPoint1EndPt->startTransform();
    mPoint2StartPt->startTransform();
}

void VectorPathEdge::cancelPassThroughTransform() {
    mPoint1EndPt->cancelTransform();
    mPoint2StartPt->cancelTransform();
}

void VectorPathEdge::setEditPath(const bool &bT) {
    mEditPath = bT;
}

void VectorPathEdge::generatePainterPath() {
    mSkPath = SkPath();
    mSkPath.moveTo(QPointFToSkPoint(mPoint1->getAbsolutePos()));
    mSkPath.cubicTo(QPointFToSkPoint(mPoint1->getEndCtrlPtAbsPos()),
                    QPointFToSkPoint(mPoint2->getStartCtrlPtAbsPos()),
                    QPointFToSkPoint(mPoint2->getAbsolutePos()));
}

void VectorPathEdge::drawHoveredSk(SkCanvas *canvas,
                                   const SkScalar &invScale) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(2.5f*invScale);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(mSkPath, paint);

    paint.setColor(SK_ColorRED);
    paint.setStrokeWidth(1.25f*invScale);
    canvas->drawPath(mSkPath, paint);
}

NodePoint *VectorPathEdge::getPoint1() const {
    return mPoint1;
}

NodePoint *VectorPathEdge::getPoint2() const {
    return mPoint2;
}

void VectorPathEdge::setPoint1(NodePoint *point1) {
    mPoint1 = point1;
    mPoint1EndPt = mPoint1->getEndCtrlPt();
}

void VectorPathEdge::setPoint2(NodePoint *point2) {
    mPoint2 = point2;
    mPoint2StartPt = mPoint2->getStartCtrlPt();
}

void VectorPathEdge::setPressedT(const qreal &t) {
    mPressedT = t;
}

void VectorPathEdge::getNearestAbsPosAndT(const QPointF &absPos,
                                QPointF *nearestPoint,
                                qreal *t) {
    *t = getClosestTValueBezier2D(mPoint1->getAbsolutePos(),
                                  mPoint1->getEndCtrlPtAbsPos(),
                                  mPoint2->getStartCtrlPtAbsPos(),
                                  mPoint2->getAbsolutePos(),
                                  absPos,
                                  nearestPoint);
}

void VectorPathEdge::getNearestRelPosAndT(const QPointF &relPos,
                                QPointF *nearestPoint,
                                qreal *t,
                                qreal *error) {
    *t = getClosestTValueBezier2D(mPoint1->getRelativePos(),
                            mPoint1->getEndCtrlPtValue(),
                            mPoint2->getStartCtrlPtValue(),
                            mPoint2->getRelativePos(),
                            relPos,
                            nearestPoint,
                            error);
}

QPointF VectorPathEdge::getSlopeVector(const qreal &t) {
    QPointF posAtT = getRelPosAtT(t);
    QPointF posAtTPlus = getRelPosAtT(t + 0.01);
    return scalePointToNewLen(posAtTPlus - posAtT, 1.);
}