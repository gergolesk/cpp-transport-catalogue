#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view stop_name, const geo::Coordinates coordinates) {
        stops_.push_back({ std::string(stop_name), coordinates, {} });
        index_stops_[stops_.back().name] = &stops_.back();
    }

    void TransportCatalogue::AddBus(std::string_view bus_number, const std::vector<const Stop*> stops, bool route_type) {
        buses_.push_back({ std::string(bus_number), stops, route_type });
        index_buses_[buses_.back().number] = &buses_.back();
        for (const auto& route_stop : stops) {
            for (auto& stop_ : stops_) {
                if (stop_.name == route_stop->name) stop_.buses_by_stop.insert(std::string(bus_number));
            }
        }
    }

    const Bus* TransportCatalogue::FindBus(std::string_view bus_number) const {
        return index_buses_.count(bus_number) ? index_buses_.at(bus_number) : nullptr;
    }

    const Stop* TransportCatalogue::FindStop(std::string_view stop_name) const {
        return index_stops_.count(stop_name) ? index_stops_.at(stop_name) : nullptr;
    }

    size_t TransportCatalogue::UniqueStopsCount(std::string_view bus_number) const {
        std::unordered_set<std::string_view> unique_stops;
        for (const auto& stop : index_buses_.at(bus_number)->stops) {
            unique_stops.insert(stop->name);
        }
        return unique_stops.size();
    }

    void TransportCatalogue::AddDistanceBetweenStops(const Stop* from, const Stop* to, const int distance) {
        index_distances_between_stops_[{from, to}] = distance;
    }

    int TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
        if (index_distances_between_stops_.count({ from, to })) return index_distances_between_stops_.at({ from, to });
        else if (index_distances_between_stops_.count({ to, from })) return index_distances_between_stops_.at({ to, from });
        else return 0;
    }

    const std::map<std::string_view, const Bus*> TransportCatalogue::GetSortedAllBuses() const {
        std::map<std::string_view, const Bus*> result;
        for (const auto& bus : index_buses_) {
            result.emplace(bus);
        }
        return result;
    }

} // namespace transport