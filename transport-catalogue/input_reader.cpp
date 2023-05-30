#include "input_reader.h"
#include "geo.h"
#include <unordered_set>

namespace transport_catalogue {

    namespace input_queries_utils {

        namespace detail {

            std::string GetToken(std::string& line, const std::string& delim) {
                size_t pos = line.find(delim);
                std::string token = std::move(line.substr(0, pos));

                if (pos != std::string::npos) {
                    line.erase(0, pos + delim.length());
                }
                else {
                    line.erase(0, pos);
                }
                return token;
            }

        }

        using namespace std::string_literals;

        void ParseStopQueries(TransportCatalogue& catalogue, const std::vector<std::pair<std::string, std::string>>& stop_queries) {
            for (auto stop_query : stop_queries) {
                while (stop_query.second.find("m to "s) != std::string::npos) {
                    size_t distance = std::stoul(detail::GetToken(stop_query.second, "m to "s));
                    std::string to_stop = detail::GetToken(stop_query.second, ", "s);
                    catalogue.AddDistanceBetweenStops(stop_query.first, distance, to_stop);
                }
            }
        }

        void ParseBusQueries(TransportCatalogue& catalogue, const std::vector<std::string>& bus_queries) {
            for (std::string bus_query : bus_queries) {
                Bus bus;

                bus.number = detail::GetToken(bus_query, ": "s);

                std::string delim;

                if (bus_query.find(" > "s) == std::string::npos) {
                    bus.route_type = RouteType::Radial;
                    delim = " - "s;
                }
                else {
                    bus.route_type = RouteType::Circular;
                    delim = " > "s;
                }

                std::string first_stop;
                std::string prev_stop;
                std::string curr_stop;
                std::unordered_set<const Stop*> unique_stops;
                bus.route_stops_count = 0;
                bus.route_length = 0;
                double calc_route_length = 0.0;

                while (bus_query.find(delim) != std::string::npos) {
                    prev_stop = std::move(curr_stop);
                    curr_stop = detail::GetToken(bus_query, delim);
                    unique_stops.emplace(catalogue.FindStop(curr_stop));
                    if (bus.route_stops_count) {
                        calc_route_length += ComputeDistance(
                            catalogue.FindStop(prev_stop)->point,
                            catalogue.FindStop(curr_stop)->point);
                        bus.route_length += catalogue.GetDistanceBetweenStops(prev_stop, curr_stop);
                        if (bus.route_type == RouteType::Radial) {
                            bus.route_length += catalogue.GetDistanceBetweenStops(curr_stop, prev_stop);
                        }
                    }
                    else {
                        first_stop = curr_stop;
                    }
                    ++bus.route_stops_count;
                }

                if (bus.route_type == RouteType::Circular) {
                    ++bus.route_stops_count;
                    calc_route_length += ComputeDistance(
                        catalogue.FindStop(curr_stop)->point,
                        catalogue.FindStop(first_stop)->point);
                    bus.route_length += catalogue.GetDistanceBetweenStops(curr_stop, first_stop);
                }
                else {
                    bus.route_stops_count = bus.route_stops_count * 2 + 1;
                    prev_stop = std::move(curr_stop);
                    curr_stop = detail::GetToken(bus_query, delim);
                    unique_stops.emplace(catalogue.FindStop(curr_stop));
                    calc_route_length += ComputeDistance(
                        catalogue.FindStop(prev_stop)->point,
                        catalogue.FindStop(curr_stop)->point);
                    calc_route_length *= 2;
                    bus.route_length += catalogue.GetDistanceBetweenStops(prev_stop, curr_stop)
                        + catalogue.GetDistanceBetweenStops(curr_stop, prev_stop);
                }

                bus.unique_stops_count = unique_stops.size();

                bus.curvature = bus.route_length / calc_route_length;

                catalogue.AddBus(std::move(bus));

                for (const auto& stop : unique_stops) {
                    catalogue.AddBusThroughStop(stop, bus.number);
                }
            }
        }

        void UpdateTransportCatalogue(TransportCatalogue& catalogue, std::istream& is) {

            std::vector<std::string> bus_queries;
            std::vector<std::pair<std::string, std::string>> stop_queries;

            std::string line;
            std::getline(is, line);
            size_t queries_count = std::stoul(line);

            bus_queries.reserve(queries_count);
            stop_queries.reserve(queries_count);

            for (int i = 0; i < queries_count; ++i) {

                std::getline(is, line);
                std::string query_type = detail::GetToken(line, " "s);

                if (query_type == "Stop"s) {
                    Stop stop;

                    stop.name = detail::GetToken(line, ": "s);
                    stop.point.lat = std::stod(detail::GetToken(line, ", "s));
                    stop.point.lng = std::stod(detail::GetToken(line, ", "s));

                    stop_queries.emplace_back(stop.name, std::move(line));

                    catalogue.AddStop(std::move(stop));
                }
                else if (query_type == "Bus"s) {
                    bus_queries.emplace_back(std::move(line));
                }
            }

            ParseStopQueries(catalogue, stop_queries);

            ParseBusQueries(catalogue, bus_queries);
        }

    }

}
