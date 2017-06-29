//
// Created by mohammad on 6/22/2017.
//

#ifndef MYCONTROLLER_VISUALIZER_H
#define MYCONTROLLER_VISUALIZER_H

#include "MainController.h"
#include "ParticleFilter.h"

class Visualizer {
public:
    MainController *controller;
    ParticleFilter *particleFilter;
    bool doDisplay;
    Map *map;
    int cellSize;

    void runController(MainController *controller);

    Visualizer(int argc, char **argv);

    void display();

    void idle();

    void visualize(ParticleFilter *particleFilter);

    virtual ~Visualizer();

    void runWithoutController();

    Point convert(const Point &p);

    void vertexPoint(const Point &p);

    void drawCell(int row, int col);
};


#endif //MYCONTROLLER_VISUALIZER_H
