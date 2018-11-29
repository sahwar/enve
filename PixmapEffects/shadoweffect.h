#ifndef SHADOWEFFECT_H
#define SHADOWEFFECT_H
#include "pixmapeffect.h"

struct ShadowEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys;
    bool highQuality;
    qreal blurRadius;
    qreal opacity;
    QColor color;
    QPointF translation;
protected:
    ShadowEffectRenderData() {}
};

class ShadowEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);
protected:
    ShadowEffect(qreal radius = 10.);
private:
//    QrealAnimator mScale;
    BoolPropertyQSPtr mHighQuality;
    QrealAnimatorQSPtr mBlurRadius;
    QrealAnimatorQSPtr mOpacity;
    ColorAnimatorQSPtr mColor;
    QPointFAnimatorQSPtr mTranslation;
};

#endif // SHADOWEFFECT_H