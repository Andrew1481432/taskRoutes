#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "nlohmann/json.hpp"

#include <algorithm>

using namespace std;

using json = nlohmann::json;

const string DIR_CONFIG = "/Users/andrew1481432/CLionProjects/TaskRoutes/resources/routes.json";

struct Route;
vector<Route> routes = {};

const unsigned COUNT_TRANSPLANTS = 3;

// region Station
struct Station{
    unsigned idLocal;
    unsigned stationId;
    string name;
    int travelTime;
    vector<int> transplants = {};
};

void to_json(json& j, const Station& s) {
    j = json{
            { "idLocal", s.idLocal },
            { "stationId", s.stationId },
            { "name", s.name },
            { "transplants", s.transplants },
            { "travel_time", s.travelTime }
    };
}

void from_json(const json& j, Station& s) {
    j.at("idLocal").get_to(s.idLocal);
    j.at("stationId").get_to(s.stationId);
    j.at("name").get_to(s.name);
    if(j.contains("transplants")) {
        j.at("transplants").get_to(s.transplants);
    } else {
        s.transplants = {};
    }
    j.at("travel_time").get_to(s.travelTime);
}

// endregion

// region Route
struct Route{
    string name;
    unsigned id;
    vector<Station> stops;
};

void to_json(json& j, const Route& r) {
    j = json{
            { "name", r.name },
            { "id", r.id },
            { "stops", r.stops },
    };
}

void from_json(const json& j, Route& r) {
    j.at("name").get_to(r.name);
    j.at("id").get_to(r.id);
    j.at("stops").get_to(r.stops);
}

// endregion

struct containerResultMove{
    int time = -1;
    vector<Station> stations = {};

    void addContainer(containerResultMove obj) {
        this->time += obj.time;
        stations.insert(stations.end(), obj.stations.begin(), obj.stations.end());
    }
};

vector<Station> getStationTransplants(Route* route) {
    vector<Station> allStationTransplants = {};
    auto stops = route->stops;
    for(auto& stop: stops) {
        auto transplants = stop.transplants;
        if(!transplants.empty()) {
            allStationTransplants.push_back(stop);
        }
    }
    return allStationTransplants;
}

Route* getRouteByStation(Station* station) {
    for(auto &route: routes) {
        for (auto &stop: route.stops) {
            if(station->stationId == stop.stationId) {
                return &route;
            }
        }
    }
    return nullptr;
}

Station* getStationByName(string name) {
    for(auto &route: routes) {
        for (auto &stop: route.stops) {
            if(stop.name == name) {
                return &stop;
            }
        }
    }
    return nullptr;
}

Station* getStationByStationId(int stationId) {
    for(auto &route: routes) {
        for (auto &stop: route.stops) {
            if(stop.stationId == stationId) {
                return &stop;
            }
        }
    }
    return nullptr;
}

bool checkValidateJson() {
    if(routes.empty()) {
        cout << "Маршруты не обнаружены!" << endl;
        return true;
    }

    int itRoutesInc = 0;
    for(auto itRoutes = routes.begin(); itRoutes != routes.end(); itRoutes++, itRoutesInc++) {
        for(int i = 1; i < (routes.size()-itRoutesInc); i++) {
            if(itRoutes->id == (itRoutes+i)->id) {
                cout << "Обнаружены маршруты с одинаковым id(" << itRoutes->id << ")!" << endl;
                return true;
            }
        }

        int itStationInc = 0;
        auto stations = itRoutes->stops;
        if(stations.size() < 2) {
            cout << "Станций должно быть меньше 1-ой!" << endl;
            return true;
        }

        for(auto itStops = stations.begin(); itStops != stations.end(); itStops++, itStationInc++) {
            for(int i = 1; i < (stations.size()-itStationInc); i++) {
                if(itStops->idLocal == (itStops+i)->idLocal) {
                    cout << "Обнаружены остановки с одинаковым id Local(" << itStops->idLocal << ")!" << endl;
                    return true;
                }
            }
        }
    }

    std::vector<Station> allStations = {};
    for(auto& route: routes) {
        auto stations = route.stops;
        allStations.insert(allStations.end(), stations.begin(), stations.end());
    }

    int inc = 0;
    for(auto it = allStations.begin(); it != allStations.end(); it++, inc++) {
        for(int i = 1; i < (allStations.size()-inc); i++) {
            if (it->stationId == (it+i)->stationId) {
                cout << "Обнаружены остановки с одинаковым id Station(" << it->stationId << ")!" << endl;
                return true;
            }
        }
    }

    for(auto& route: routes) {
        for(auto& sourceStation: route.stops) {
            auto sourceTransplants = sourceStation.transplants; // получение пересадок у станции источника
            if(!sourceTransplants.empty()) {
                for(int idSourceTransplant: sourceTransplants) {
                    Station *targetStation = getStationByStationId(idSourceTransplant);
                    auto targetTransplants = targetStation->transplants;

                    if(!(std::find(targetTransplants.begin(), targetTransplants.end(), sourceStation.stationId) != targetTransplants.end())) {
                        cout << "Обнаружена однонаправленная пересадка. Станция со stationId(" << sourceStation.stationId << ") создает пересадку на станцию со stationId(" << idSourceTransplant << "), но наоборот пересадка не создана!" << endl;
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

void printStations(vector<Station> stations) {
    auto endStation = stations[stations.size() - 1];
    for (auto& station: stations) {
        cout << station.name;
        if(endStation.stationId != station.stationId) {
            cout << " -> ";
        }
    }
    cout << endl;
}

// рекурсивная функция
containerResultMove moving(Station* stationA, Station* stationB, vector<unsigned> routesTransplants = {}) {
    Route *routeA = getRouteByStation(stationA);
    Route *routeB = getRouteByStation(stationB);

    containerResultMove allMoving = {};
    if(routesTransplants.size() > COUNT_TRANSPLANTS) {
        allMoving.time = -1;
        return allMoving;
    }
    if((*routeA).id == (*routeB).id) {
        auto stops = routeA->stops;
        bool isReverse = (*stationB).idLocal < (*stationA).idLocal;
        if(isReverse) {
            std::reverse(stops.begin(), stops.end());
        }

        bool isStart = false;

        for(vector<Station>::iterator it = stops.begin(); it != stops.end(); it++) {
            if((*stationA).idLocal == it->idLocal) {
                isStart = true;
            }
            if(isStart) {
                allMoving.time += isReverse ? (it == stops.begin() ? 0 : (it - 1)->travelTime) : it->travelTime;
                allMoving.stations.push_back(*it);
            }
            if((*stationB).idLocal == it->idLocal) {
                isStart = false;
                break;
            }
        }
    } else {
        //** время и сами маршруты **//
        vector<containerResultMove> dataRoutes = {};
        routesTransplants.push_back((*routeA).id);

        auto allStationTransplants = getStationTransplants(routeA);
        if(!allStationTransplants.empty()) {
            for (auto &stationTrans: allStationTransplants) {
                containerResultMove dataMovingToTransSt = moving(stationA, &stationTrans);
                //** перебираем станции пересадочной ветки **/
                for (int idTransStation: stationTrans.transplants) {
                    auto stationByIdTrans = getStationByStationId(idTransStation);
                    Route *routeByTrans = getRouteByStation(stationByIdTrans);

                    // проверка, чтобы не возвращаться на маршрут на котором уже были!
                    if(std::find(routesTransplants.begin(), routesTransplants.end(), routeByTrans->id) != routesTransplants.end()) {
                        continue;
                    }

                    containerResultMove movingTimeData = dataMovingToTransSt;
                    movingTimeData.addContainer(moving(stationByIdTrans, stationB, routesTransplants));
                    if (movingTimeData.time == -1) {
                        continue;
                    }
                    dataRoutes.push_back(movingTimeData);
                }
            }
        }

        //вывести всевозможные маршруты ))))
        /*if(routesTransplants.size() == 1) {
            for (auto& route: dataRoutes) {
                printStations(route.stations);
                cout << "время в секундах: " << route.time << endl;
            }
        }*/

        if(!dataRoutes.empty()) {
            int minTime = 100000;
            containerResultMove minContainerMoving = {};
            for (auto& route: dataRoutes) {
                //printStations(route.stations);
                if (minTime > route.time) {
                    minTime = route.time;
                    minContainerMoving = route;
                }
            }
            allMoving.addContainer(minContainerMoving);
        }
    }
    return allMoving;
}

int main() {
    std::ifstream fileRoutes(DIR_CONFIG);
    if(fileRoutes.good()) {
        json fileRoutesJson = json::parse(fileRoutes);
        fileRoutes.close();

        for (auto& it: fileRoutesJson) {
            auto route = it.get<Route>();
            routes.push_back(route);
        }

        // проверка от дураков json
        if(checkValidateJson()) {
            return 0;
        }

        for(auto& route: routes) {
            cout << route.name << ": ";
            printStations(route.stops);
        }

        string inputA;
        cout << "Введите станцию точки А: ";
        getline(cin, inputA);
        Station* stationA = getStationByName(inputA);
        if(stationA == nullptr) {
            cout << "Станция пункта назначения А не найдена!" << endl;
            return 0;
        }

        string inputB;
        cout << "Введите станцию точки B: ";
        getline(cin, inputB);
        Station* stationB = getStationByName(inputB);
        if(stationB == nullptr) {
            cout << "Станция пункта назначения B не найдена!" << endl;
            return 0;
        }

        containerResultMove containerResultMove = moving(stationA, stationB);
        int secMinimumTime = stationA->stationId != stationB->stationId ? containerResultMove.time : 0;
        if(secMinimumTime != -1) {
            printStations(containerResultMove.stations);
            //cout << "Минимальное время " << secMinimumTime << " сек." << endl;
            cout << "Final minimum time: ";
            if(secMinimumTime > 3599) {
                int hour = secMinimumTime / 3600;
                int min = (secMinimumTime / 60) % 60;
                int sec = secMinimumTime % 60;
                cout << hour << " hours " << min << " min " << sec << " sec" << endl;
            } else if(secMinimumTime > 59) {
                cout << secMinimumTime / 60 << " min " << secMinimumTime % 60 << " sec" << endl;
            } else {
                cout << secMinimumTime << " sec" << endl;
            }

        } else {
            cout << "Unavailable route!" << endl;
        }
    } else {
        cout << "Ошибка открытия файла fileRoutes.json!" << endl;
    }
    return 0;
}