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