#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddBus(const Bus& bus) {
        const auto pos = buses_.insert(buses_.begin(), bus);
        index_buses_.insert({ pos->number, &(*pos) });
    }

    void TransportCatalogue::AddStop(const Stop& stop) {
        const auto pos = stops_.insert(stops_.begin(), stop);
        index_stops_.insert({ pos->name, &(*pos) });
    }

    const Bus* TransportCatalogue::FindBus(const std::string_view name) const {
        auto it = index_buses_.find(name);
        return it == index_buses_.end() ? nullptr : it->second;
    }

    const Stop* TransportCatalogue::FindStop(const std::string_view name) const {
        auto it = index_stops_.find(name);
        return it == index_stops_.end() ? nullptr : it->second;
    }

   const RouteInfo TransportCatalogue::GetRouteInfo(const std::string_view route_number) const {
        RouteInfo info;
        auto find_ = index_buses_.find(route_number);

        auto bus = FindBus(route_number);
      

        std::unordered_set<std::string_view> unique_stops;
        for (const auto& stop : bus->stops) {
            unique_stops.insert(stop);
        }

        info.unique_stops_count = unique_stops.size();

        if (bus->route_type == RouteType::Circular) {
            info.route_stops_count = bus->stops.size();
        }
        else {
            info.route_stops_count = bus->stops.size() * 2 - 1;
        }

        double length = 0.0;
        double geo_length = 0.0;


        for (size_t i = 1; i < bus->stops.size(); ++i) {

            geo_length += ComputeDistance(index_stops_.at(find_->second->stops[i - 1])->point, index_stops_.at(find_->second->stops[i])->point);
            if (bus->route_type == RouteType::Radial) {
                length += GetDistanceBetweenStops(index_stops_.at(find_->second->stops[i - 1]), index_stops_.at(find_->second->stops[i])) +
                    GetDistanceBetweenStops(index_stops_.at(find_->second->stops[i]), index_stops_.at(find_->second->stops[i - 1]));
            }
            else {
                length += GetDistanceBetweenStops(index_stops_.at(find_->second->stops[i - 1]), index_stops_.at(find_->second->stops[i]));
            }

        }
        if (bus->route_type == RouteType::Radial) {
            geo_length *= 2.0;
        }

        
        info.route_length = length;
        info.curvature = length / geo_length;
        return info;

    }

    void TransportCatalogue::AddBusThroughStop(const Stop* stop, const std::string_view bus_number) {
        index_buses_through_stop_[stop].emplace(FindBus(bus_number));
    }

    const std::set<const Bus*, detail::CompareBuses>* TransportCatalogue::GetBusesThroughStop(const Stop* stop) const {
        auto it = index_buses_through_stop_.find(stop);
        return it == index_buses_through_stop_.end() ? nullptr : &(it->second);
    }

    void TransportCatalogue::AddDistanceBetweenStops(const std::string_view from_stop, const size_t distance, const std::string_view to_stop) {
        index_distances_between_stops_.emplace(std::pair(FindStop(from_stop), FindStop(to_stop)), distance);
    }

    size_t TransportCatalogue::GetDistanceBetweenStops(const std::string_view from_stop, const std::string_view to_stop) const {
        const Stop* find_from = FindStop(from_stop);
        const Stop* find_to = FindStop(to_stop);

        auto it = index_distances_between_stops_.find(std::pair(find_from, find_to));

        if (it != index_distances_between_stops_.end()) {
            return it->second;
        }
        else {
            it = index_distances_between_stops_.find(std::pair(find_to, find_from));
            return it->second;
        }
    }

    size_t TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
        if (index_distances_between_stops_.count({ from, to })) {
            return index_distances_between_stops_.at({ from, to });
        }

        else if (index_distances_between_stops_.count({ to, from })) {
            return index_distances_between_stops_.at({ to, from });
        }

        else {
            return 0;
        }
    }
}