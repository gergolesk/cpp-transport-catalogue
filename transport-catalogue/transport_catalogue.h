#pragma once

#include "geo.h"
#include "domain.h"

#include <iostream>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <optional>
#include <unordered_set>
#include <set>
#include <map>

namespace transport_catalogue {

    class TransportCatalogue {
    public:
        struct PairHash {
            size_t operator()(const std::pair<const Stop*, const Stop*>& points) const {
                size_t hash_first = std::hash<const void*>{}(points.first);
                size_t hash_second = std::hash<const void*>{}(points.second);
                return hash_first + hash_second * 37;
            }
        };

        void AddStop(std::string_view name, const geo::Coordinates coordinates);
        void AddBus(std::string_view name, const std::vector<const Stop*> stops, bool route_type);
        const Bus* FindBus(std::string_view name) const;
        const Stop* FindStop(std::string_view name) const;
        size_t UniqueStopsCount(std::string_view bus_number) const;
        void AddDistanceBetweenStops(const Stop* from, const Stop* to, const int distance);
        int GetDistanceBetweenStops(const Stop* from, const Stop* to) const;
        const std::map<std::string_view, const Bus*> GetSortedAllBuses() const;

    private:
        std::deque<Bus> buses_;
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Bus*> index_buses_;
        std::unordered_map<std::string_view, const Stop*> index_stops_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, int, PairHash> index_distances_between_stops_;
    };

} // namespace transport