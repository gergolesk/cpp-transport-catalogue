#include "request_handler.h"

std::optional<transport_catalogue::RouteInfo> RequestHandler::GetBusStat(const std::string_view bus_number) const {
    transport_catalogue::RouteInfo bus_stat{};
    const transport_catalogue::Bus* bus = catalogue_.FindBus(bus_number);

    if (!bus) throw std::invalid_argument("bus not found");
    if (bus->is_circle) bus_stat.route_stops_count = bus->stops.size();
    else bus_stat.route_stops_count = bus->stops.size() * 2 - 1;

    int route_length = 0;
    double geographic_length = 0.0;

    for (size_t i = 0; i < bus->stops.size() - 1; ++i) {
        auto from = bus->stops[i];
        auto to = bus->stops[i + 1];
        if (bus->is_circle) {
            route_length += catalogue_.GetDistanceBetweenStops(from, to);
            geographic_length += geo::ComputeDistance(from->point,
                to->point);
        }
        else {
            route_length += catalogue_.GetDistanceBetweenStops(from, to) + catalogue_.GetDistanceBetweenStops(to, from);
            geographic_length += geo::ComputeDistance(from->point,
                to->point) * 2;
        }
    }

    bus_stat.unique_stops_count = catalogue_.UniqueStopsCount(bus_number);
    bus_stat.route_length = route_length;
    bus_stat.curvature = route_length / geographic_length;

    return bus_stat;
}

const std::set<std::string> RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name)->buses_by_stop;
}

bool RequestHandler::IsBusNumber(const std::string_view bus_number) const {
    return catalogue_.FindBus(bus_number);
}

bool RequestHandler::IsStopName(const std::string_view stop_name) const {
    return catalogue_.FindStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(catalogue_.GetSortedAllBuses());
}