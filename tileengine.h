#ifndef TILEENGINE_H
#define TILEENGINE_H
//#include <SFML/Graphics.hpp>
#include "memory.h"
#include "qrenderwindow.h"
#include <QAtomicInt>

class TileEngine: public QRenderWindow
{
private:
    Memory* memory;
    Clock clk;
    Time time;
    int framesPerSecond;

    Vector2u screenSize;
    int *horizontalScroll, *verticalScroll;

    QAtomicInt shouldRender;
public:
    TileEngine(QWidget*, const QPoint&, const QSize&, Memory *, QVector<int> *mRegisters);
    ~TileEngine();
    void initialize();
    void update();
    void renderFrame();
    void setMemory(Memory*);

    void stopRendering();
public slots:
    void updateNow();


};

#endif // TILEENGINE_H
