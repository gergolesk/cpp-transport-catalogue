#pragma once

#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <iostream>

#include "geo.h"

namespace transport_catalogue {

    enum class RouteType {
        Radial,
        Circular
    };

    struct Bus {
        std::string number;
        RouteType route_type;
        std::vector<std::string> stops;
    };

    struct RouteInfo {
        size_t route_stops_count;
        size_t unique_stops_count;
        size_t route_length;
        double curvature;
    };

    struct Stop {
        std::string name;
        geo::Coordinates point;

        size_t Hash() const {
            return std::hash<std::string>{}(name)
                +37 * std::hash<double>{}(point.lng)
                + 37 * 37 * std::hash<double>{}(point.lat);
        }
    };

    namespace detail {

        struct CompareBuses {
            bool operator() (const Bus* lhs, const Bus* rhs) const {
                return lhs->number < rhs->number;
            }
        };

        struct PairHash {
            size_t operator() (const std::pair<const Stop*, const Stop*>& pair) const {
                return pair.first->Hash() + 37 * pair.second->Hash();
            }
        };

    }

    class TransportCatalogue {
    public:

        TransportCatalogue() = default;
        void AddBus(const Bus& bus);
        void AddStop(const Stop& stop);

        const Bus* FindBus(const std::string_view name) const;
        const Stop* FindStop(const std::string_view name) const;
        
        const RouteInfo GetRouteInfo(const std::string_view route_number) const;
        void AddBusThroughStop(const Stop* stop, const std::string_view bus_number);
        const std::set<const Bus*, detail::CompareBuses>* GetBusesThroughStop(const Stop* stop) const;
        void AddDistanceBetweenStops(const std::string_view from_stop, const size_t distance, const std::string_view to_stop);
        size_t GetDistanceBetweenStops(const std::string_view from_stop, const std::string_view to_stop) const;
        size_t GetDistanceBetweenStops(const Stop* from, const Stop* to) const;

    private:

        std::deque<Bus> buses_;
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Bus*> index_buses_;
        std::unordered_map<std::string_view, const Stop*> index_stops_;
        std::unordered_map<const Stop*, std::set<const Bus*, detail::CompareBuses>> index_buses_through_stop_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, detail::PairHash> index_distances_between_stops_;
    };

}
