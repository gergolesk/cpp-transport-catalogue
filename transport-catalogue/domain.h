/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>
#include <unordered_map>

namespace transport_catalogue {

    struct Stop {
        std::string name;
        geo::Coordinates point;
        std::set<std::string> buses_by_stop;
    };

    struct Bus {
        std::string number;
        std::vector<const Stop*> stops;
        bool is_circle;
    };

    struct RouteInfo {
        size_t route_stops_count;
        size_t unique_stops_count;
        double route_length;
        double curvature;
    };

} // namespace transport