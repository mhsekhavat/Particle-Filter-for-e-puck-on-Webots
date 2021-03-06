#include <cstdlib>
#include "SensorModel.h"

SensorModel::SensorModel(const char *meanFilePath, const char *varFilePath) {
    readSensorModelFile(varFilePath, meanFilePath);
}

void SensorModel::readSensorModelFile(const char *s, const char *meanFilePath) {
    ifstream meanFile;
    meanFile.open(meanFilePath);
    cout << "reading mean file ..." << endl;
    if (meanFile.is_open()) {
        for (int i = 0; i < 8; i++) {
            string line;
            getline(meanFile, line);
            int index = 0;
            string temp = "";
            vector<double> tempVector;
            meanSensorVector.push_back(tempVector);
            while (index < (int) (line.length())) {
                if (line.at(index) == ',') {
                    double x = atof(temp.c_str());
                    meanSensorVector[i].push_back(x);
                    temp = "";
                } else if (line.at(index) == '\n') {

                    break;
                } else {
                    temp += line.at(index);
                }
                index += 1;
            }
            double x = atof(temp.c_str());
            // cout << x << endl;
            meanSensorVector[i].push_back(x);
            temp = "";

        }
        meanFile.close();
        cout << "read mean file finished" << endl;
        for (int i = 0; i < meanSensorVector.size(); i++) {
            cout << "sensor " << i << ": ";
            for (int j = 0; j < meanSensorVector[i].size(); j++) {

                cout << meanSensorVector[i][j] << " ";
            }
            cout << endl;
        }
        cout << endl;
    } else {
        cout << "can not open mean file" << endl;
    }
    ifstream varianceFile;
    varianceFile.open(s);
    cout << "reading variance file ..." << endl;
    if (varianceFile.is_open()) {
        for (int i = 0; i < 8; i++) {
            string line;
            getline(varianceFile, line);
            int index = 0;
            string temp = "";
            vector<double> tempVector;
            stdDeviationSensorVector.push_back(tempVector);
            while (index < line.length()) {
                if (line.at(index) == ',') {
                    double x = atof(temp.c_str());
                    stdDeviationSensorVector[i].push_back(pow(x, 2.0));
                    temp = "";
                } else if (line.at(index) == '\n') {

                    break;
                } else {
                    temp += line.at(index);
                }
                index += 1;
            }
            double x = atof(temp.c_str());
            cout << x << endl;
            stdDeviationSensorVector[i].push_back(x);
            temp = "";

        }
        varianceFile.close();
        cout << "read variance file finished" << endl;
        for (int i = 0; i < stdDeviationSensorVector.size(); i++) {
            cout << "sensor " << i << ": ";
            for (int j = 0; j < stdDeviationSensorVector[i].size(); j++) {
                cout << stdDeviationSensorVector[i][j] << "\t";
            }
            cout << endl;
        }
        cout << endl;
    } else {
        cout << "can not open variance file" << endl;
    }
}

SensorModel::~SensorModel(void) {
}

Observation SensorModel::sensorValuesToObservation(double *sensorValues) {
    Observation result(SENSORS, 0.0);
    for (int i = 0; i < SENSORS; i++) {
        result[i] = convertSingleSensorValue(i, sensorValues[i]);
    }
    return result;
}

double interpolate(double x1, double y1, double x2, double y2, double xQuery) {
    // cout << x1 << ' ' << y1 << ';' << x2 << ' ' << y2 << ';' << xQuery << endl;
    double dx = x2 - x1;
    return (xQuery - x1) / dx * y2 + (x2 - xQuery) / dx * y1;
}

inline double regularize(double distance) {
    return min(8.0, max(0.0, distance));
}

double SensorModel::convertSingleSensorValue(int sensorId, double sensorValue) {
    vector<double> &means = meanSensorVector[sensorId];

    int slopeBegin = 0;

    for (; slopeBegin < DISTANCES - 2; slopeBegin++) {
        if (means[slopeBegin + 1] <= sensorValue) {
            break;
        }
    }

    double result = interpolate(means[slopeBegin], slopeBegin,
                                means[slopeBegin + 1], slopeBegin + 1,
                                sensorValue);
    return regularize(result);
}

Gaussian SensorModel::getSensorGaussian(int sensorId, double distance) {
    Gaussian gaussian;
    gaussian.mean = distance;
    if (distance <= 0.5) {
        gaussian.sigma2 = stdDeviationSensorVector[sensorId][0];
    } else if (distance >= 7 - 1e9) {
        gaussian.sigma2 = stdDeviationSensorVector[sensorId][7];
    } else {
        gaussian.sigma2 = stdDeviationSensorVector[sensorId][(int) round(distance)];
    }
    if (gaussian.sigma2 < 0.0001) {
        cout << "===================================== " << distance << endl;
    }
//    cout << "gausian s" << sensorId << " , " << distance << " = N(" << gaussian.mean << "," << gaussian.sigma2 << ")"
//         << endl;
    return gaussian;
}

/**
double SensorModel::getObservationProbability(Particle *particle, Observation *observation, Map *world) {
    Observation expectedObservation = getExpectedObservation(particle, world);
    double prob = 1;
    for (int i = 0; i < SENSORS; i++) {
        prob *= getSensorGaussian(i, expectedObservation[i]).getProbability(observation->at(i));
    }
    return prob;
}
/**/
#include "tinyexpr.h"

double funcReduceFirst, funcMean, funcVar, funcX, funcPrev, funcCurr;
te_variable funcMapVars[] = {{"mean", &funcMean},
                             {"var",  &funcVar},
                             {"x",    &funcX}},
        funcReduceVars[] = {{"prev", &funcPrev},
                            {"curr", &funcCurr}};
te_expr *funcMapExpr, *funcReduceExpr;
bool funcLoaded = false;

void funcLoad() {
    if (funcLoaded)
        return;
    else
        funcLoaded = true;
    ifstream funcFin("params-func.txt");
    string line;
    int err;
    getline(funcFin, line);
    funcMapExpr = te_compile(line.c_str(), funcMapVars, 3, &err);
    if (err) {
        cout << "error parsing map" << endl;
    }
    getline(funcFin, line);
    funcFin >> funcReduceFirst;
    funcReduceExpr = te_compile(line.c_str(), funcReduceVars, 2, &err);
    if (err) {
        cout << "error parsing reduce" << endl;
    }
}

double SensorModel::getObservationProbability(Particle *particle, Observation *observation, Map *world) {
    funcLoad();
    funcPrev = funcReduceFirst;
    Observation expectedObservation = getExpectedObservation(particle, world);
    for (int i = 0; i < SENSORS; i++) {
        const Gaussian &gaussian = getSensorGaussian(i, expectedObservation[i]);
        funcMean = gaussian.mean;
        funcVar = gaussian.sigma2;
        funcX = observation->at(i);
        funcCurr = te_eval(funcMapExpr);
        funcPrev = te_eval(funcReduceExpr);
    }
    return funcPrev;
}

/**/

Observation SensorModel::getExpectedObservation(const Particle *particle, Map *world) const {
    Observation expectedObservation;
    for (int i = 0; i < SENSORS; i++) {
        double distance = world->distanceToNearestObstacle(particle->position,
                                                           particle->angle - M_PI_2 + SENSOR_ORIENTATION[i]);
        distance -= RADIUS_ROBOT;
        expectedObservation.push_back(regularize(distance));
    }
    return expectedObservation;
}