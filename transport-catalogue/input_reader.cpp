#include "input_reader.h"
#include "geo.h"
#include <unordered_set>

namespace transport_catalogue {

    namespace input_queries_utils {

        namespace token {

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
                    size_t distance = std::stoul(token::GetToken(stop_query.second, "m to "s));
                    std::string to_stop = token::GetToken(stop_query.second, ", "s);
                    catalogue.AddDistanceBetweenStops(stop_query.first, distance, to_stop);
                }
            }
        }

        void ParseBusQueries(TransportCatalogue& catalogue, const std::vector<std::string>& bus_queries) {
            for (std::string bus_query : bus_queries) {
                Bus bus;

                bus.number = token::GetToken(bus_query, ": "s);

                std::string delim;

                if (bus_query.find(" > "s) == std::string::npos) {
                    bus.route_type = RouteType::Radial;
                    delim = " - "s;
                }
                else {
                    bus.route_type = RouteType::Circular;
                    delim = " > "s;
                }               

                std::string curr_stop;
                std::unordered_set<const Stop*> unique_stops;

                while (bus_query.find(delim) != std::string::npos) {
                    curr_stop = token::GetToken(bus_query, delim);
                    bus.stops.push_back(curr_stop);
                    unique_stops.emplace(catalogue.FindStop(curr_stop));
                                        
                }

                curr_stop = token::GetToken(bus_query, delim);
                bus.stops.push_back(curr_stop);
                unique_stops.emplace(catalogue.FindStop(curr_stop));

               
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
                std::string query_type = token::GetToken(line, " "s);

                if (query_type == "Stop"s) {
                    Stop stop;

                    stop.name = token::GetToken(line, ": "s);
                    stop.point.lat = std::stod(token::GetToken(line, ", "s));
                    stop.point.lng = std::stod(token::GetToken(line, ", "s));

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