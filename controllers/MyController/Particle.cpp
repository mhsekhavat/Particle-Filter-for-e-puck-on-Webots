#include "Particle.h"
#include "Map.h"

Particle::~Particle(void) {

}

void Particle::doAction(Action *action, Map *world) {

    double step = 0.1;
    double nextX, nextY;
    nextX = position.real();
    nextY = position.imag();

    int iteration = (int) (action->distance / step);
    for (int j = 0; j < iteration; ++j) {
        cout << "distanceToNearestObstacle: nextX=" << nextX << "\tnextY=" << nextY << endl;
        nextX = nextX + cos(angle) * step;
        nextY = nextY + sin(angle) * step;

        if (nextX < 0 || nextX > world->realWidth || nextY < 0 ||
            nextY > world->realHeight) {
            break;
        }
        if (world->canRobotBeAt(nextX, nextY)) {
            break;
        }
        position = Point(nextX, nextY);
    }
    angle = angle + action->rotateRadian;
}

Particle::Particle(void) {

}
