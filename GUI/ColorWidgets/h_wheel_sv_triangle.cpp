#include "h_wheel_sv_triangle.h"
#include "GUI/mainwindow.h"
#include "GUI/ColorWidgets/helpers.h"
#include <QDebug>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QWindow>
#include <QApplication>

qreal sign(qreal x, qreal y, QPointF p2, QPointF p3) {
    return (x - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (y - p3.y());
}

bool pointInTriangle(qreal x, qreal y, QPointF v1, QPointF v2, QPointF v3) {
    bool b1, b2, b3;

    qreal q0 = 0;
    b1 = sign(x, y, v1, v2) < q0;
    b2 = sign(x, y, v2, v3) < q0;
    b3 = sign(x, y, v3, v1) < q0;

    return ((b1 == b2) && (b2 == b3));
}

bool insideCircle(int r, int x_t, int y_t) {
    return x_t*x_t + y_t*y_t < r*r;
}

bool outsideCircle(int r, int x_t, int y_t) {
    return !insideCircle(r, x_t, y_t);
}

H_Wheel_SV_Triangle::H_Wheel_SV_Triangle(QWidget *parent_t) :
    ColorWidget(parent_t) {
    setFixedSize(200, 200);
}

void H_Wheel_SV_Triangle::resizeGL(int w, int h) {
    wheel_dim = static_cast<uint>(std::min(w, h));
    outer_circle_r = wheel_dim*0.5f;
    inner_circle_r = outer_circle_r - wheel_thickness;
    triangle_width = sqrt_3f*inner_circle_r;
    triangle_tex_width = static_cast<GLuint>(ceil(inner_circle_r*2));
    triangle_tex_height = static_cast<GLuint>(ceil(1.5f*inner_circle_r));
    ColorWidget::resizeGL(w, h);
}

void H_Wheel_SV_Triangle::paintGL() {
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glViewport(0, 0, width(), height());

    glClear(GL_COLOR_BUFFER_BIT);

    /*glColor3f(1.0, 0.0, 0.0);
    glRects(50, 50, 50, -50);*/

    glColor4f(1.f, 1.f, 1.f, 1.f);
    drawWheel();

    glPushMatrix();
    glTranslatef(outer_circle_r, outer_circle_r, 0.f);

    //
    glPushMatrix();

    glRotatef(-hue*360 + 180, 0.f, 0.f, 1.f);
    glTranslatef(-outer_circle_r, -0.5f, 0.f);
    if(shouldValPointerBeLightHSV(hue, 1.f, 1.f) ) {
        drawSolidRectCenter(wheel_thickness*0.5f - 1.f, 0.f, wheel_thickness, 2.5f,
                            1.f, 1.f, 1.f, true, true, true, true);
    } else {
        drawSolidRectCenter(wheel_thickness*0.5f - 1.f, 0.f, wheel_thickness, 2.5f,
                            0.f, 0.f, 0.f, true, true, true, true);
    }
    //drawRectCenter(wheel_thickness*0.5, 0, wheel_thickness - 7.f, 2, curr_h_h, curr_h_s, curr_h_v);

    glPopMatrix();
    //

    glRotatef(-hue*360 - 30, 0.0f, 0.0f, 1.0f);
    glTranslatef(-inner_circle_r, -inner_circle_r, 0.f);
    glColor3f(0.f, 0.f, 0.f);

    float center_y = value*inner_circle_r*1.5f;
    float row_width_t = center_y*2/sqrt_3f;
    float row_x_0 = inner_circle_r - row_width_t*0.5f;
    float center_x = row_x_0 + row_width_t*saturation;

    drawTriangle();
    if(shouldValPointerBeLightHSV(hue, saturation, value) ) {
        drawSolidCircle(5, center_x, center_y, 16, 1.f, 1.f, 1.f);
    } else {
        drawSolidCircle(5, center_x, center_y, 16, 0.f, 0.f, 0.f);
    }
    float curr_h = hue;
    float curr_s = saturation;
    float curr_v = value;
    hsv_to_rgb_float(&curr_h, &curr_s, &curr_v);
    drawSolidCircle(3, center_x, center_y, 16, curr_h, curr_s, curr_v);

    //drawRect(inner_circle_r, 0, wheel_thickness, 2, 0.f, 0.f, 0.f);

    glPopMatrix();

    /*glColor3f(1.0, 0.0, 0.0);
    glRects(0, 0, 50, 50);*/
}

void H_Wheel_SV_Triangle::drawTriangle() {
    GLfloat x_mar = (triangle_tex_width - triangle_width)*0.5f;
    glPushMatrix();
    glTranslatef(x_mar, 0.f, 0.f);

    /*glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_TRIANGLES);
        glVertex2f(triangle_width*0.5, 0);
        glVertex2f(0, triangle_tex_height);
        glVertex2f(triangle_width, triangle_tex_height);
    glEnd();*/
    GLfloat s1 = 1.f;
    GLfloat v1 = 0.f;

    GLfloat s2 = 0.f;
    GLfloat v2 = 1.f;

    GLfloat s3 = 1.f;
    GLfloat v3 = 1.f;
    drawSubTris(triangle_width*0.5f, 0.f,
                hue, s1, v1,
                0.f, triangle_tex_height,
                hue, s2, v2,
                triangle_width, triangle_tex_height,
                hue, s3, v3,
                3.f, true, true, true);
    /*drawAATris(triangle_width*0.5, 0,
                   0, triangle_tex_height,
                   triangle_width, triangle_tex_height,
                   1.f, 0.f, 0.f);*/
    glPopMatrix();
}

void H_Wheel_SV_Triangle::drawWheel() {
    float cx = outer_circle_r;
    float cy = outer_circle_r;
    float r = outer_circle_r - 1.f;
    int num_seg = 128;
    float theta = 2.f * PIf/num_seg;
    float c = cosf(theta);//precalculate the sine and cosine
    float s = sinf(theta);
    float t;

    float x = r;//we start at angle = 0
    float y = 0;

    // activate and specify pointer to vertex array

    //glBegin(GL_LINE_LOOP);
    float last_r = 1.f;
    float last_g = 0.f;
    float last_b = 0.f;
    float last_x = x;
    float last_y = y;
    for(int ii = 0; ii < num_seg; ii++) {
        //apply the rotation matrix
        t = x;
        x = c * x - s * y;
        y = s * t + c * y;

        float h2r = 1.f - (ii + 1.f)/num_seg;
        float s2g = 1.f;
        float v2b = 1.f;
        hsv_to_rgb_float(&h2r, &s2g, &v2b);

        drawAACircTris(last_x + cx, last_y + cy,
                       x + cx, y + cy,
                       cx, cy,
                       last_r, last_g, last_b,
                       h2r, s2g, v2b);
        last_x = x;
        last_y = y;
        last_r = h2r;
        last_g = s2g;
        last_b = v2b;
    }


    drawSolidCircle(inner_circle_r, cx, cy,
                    static_cast<GLuint>(num_seg),
                    mBgColor.fR, mBgColor.fG, mBgColor.fB);
}

void H_Wheel_SV_Triangle::wheelEvent(QWheelEvent *e) {
    if(e->delta() > 0) {
        hue += 0.01;
        if(hue > 1)
        {
            hue -= 1;
        }
    } else {
        hue -= 0.01;
        if(hue < 0)
        {
            hue += 1;
        }
    }
    //drawTriangle();
    update();
}

void H_Wheel_SV_Triangle::wheelInteraction(const int &x_t, const int &y_t) {
    double radial_x = x_t - wheel_dim*0.5;
    double radial_y = y_t - wheel_dim*0.5;
    hue = getAngleF(1, 0, -radial_x, radial_y);
    //drawTriangle();
    update();
}

void H_Wheel_SV_Triangle::triangleInteraction(int x_t, int y_t) {
    x_t--;
    y_t--;
    x_t -= wheel_thickness;
    y_t -= wheel_thickness;

    float tr_x_t = x_t - inner_circle_r;
    float tr_y_t = y_t - inner_circle_r;
    float hue_rad = (hue + 1/12.f)*2*PIf;
    rotate(hue_rad, &tr_x_t, &tr_y_t);
    tr_x_t += inner_circle_r;
    tr_y_t += inner_circle_r;
    float row_width_t = tr_y_t*2/sqrt_3f;
    float row_x_0 = inner_circle_r - row_width_t*0.5f;

    saturation = clamp( (tr_x_t - row_x_0)/row_width_t , 0.f, 1.f);
    value = clamp(tr_y_t/(inner_circle_r*1.5f), 0.f, 1.f);

    update();
}

void H_Wheel_SV_Triangle::mousePressEvent(QMouseEvent *e) {
    mValueBlocked = true;

    if(e->button() == Qt::RightButton) {
        return;
    }
    grabMouse();
    if(isInTriangle(e->pos())) {
        value_focus = SV;
        triangleInteraction(e->x(), e->y() );
    } else if(isInWheel(e->pos())) {
        value_focus = H;
        wheelInteraction(e->x(), e->y() );
    } else {
        value_focus = NONE;
        return;
    }
    QApplication::setOverrideCursor(
                QCursor(QPixmap("pixmaps/cursor_crosshair_open.png") ) );
}

void H_Wheel_SV_Triangle::mouseReleaseEvent(QMouseEvent *) {
    releaseMouse();
    QApplication::restoreOverrideCursor();
    value_focus = NONE;
    mValueBlocked = false;
}

void H_Wheel_SV_Triangle::mouseMoveEvent(QMouseEvent *e) {
    if(value_focus == H) {
        wheelInteraction(e->x(), e->y());
    } else if(value_focus == SV) {
        triangleInteraction(e->x(), e->y());
    }
}

bool H_Wheel_SV_Triangle::isInWheel(const QPoint& pos_t) {
    int rel_center_x = pos_t.x() - static_cast<int>(outer_circle_r);
    int rel_center_y = pos_t.y() - static_cast<int>(outer_circle_r);
    if(!insideCircle(static_cast<int>(outer_circle_r),
                     rel_center_x, rel_center_y)) return false;
    return outsideCircle(static_cast<int>(inner_circle_r),
                         rel_center_x, rel_center_y);
}

bool H_Wheel_SV_Triangle::isInTriangle(const QPoint& pos_t) {
    float x_mar = (triangle_tex_width - triangle_width)*0.5f + wheel_thickness;
    float y_mar = wheel_thickness;
    float x1_t = triangle_width*0.5f + x_mar - outer_circle_r;
    float y1_t = y_mar - outer_circle_r;
    QPointF v1 = QPointF(static_cast<qreal>(x1_t),
                         static_cast<qreal>(y1_t));
    float x2_t = x_mar - outer_circle_r;
    float y2_t = triangle_tex_height + y_mar - outer_circle_r;
    QPointF v2 = QPointF(static_cast<qreal>(x2_t),
                         static_cast<qreal>(y2_t));
    float x3_t = triangle_width + x_mar - outer_circle_r;
    float y3_t = triangle_tex_height + y_mar - outer_circle_r;
    QPointF v3 = QPointF(static_cast<qreal>(x3_t),
                         static_cast<qreal>(y3_t));

    float x_t = pos_t.x() - outer_circle_r;
    float y_t = pos_t.y() - outer_circle_r;
    float hue_rad = (hue + 1/12.f)*2*PIf;
    rotate(hue_rad, &x_t, &y_t);
    return pointInTriangle(static_cast<qreal>(x_t),
                           static_cast<qreal>(y_t),
                           v1, v2, v3);
}