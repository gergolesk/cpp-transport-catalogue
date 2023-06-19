#include "json_reader.h"
#include "json_builder.h"

const json::Node& JsonReader::GetBaseRequests() const {
    if (!input_.GetRoot().AsDict().count("base_requests")) return dummy_;
    return input_.GetRoot().AsDict().at("base_requests");
}

const json::Node& JsonReader::GetStatRequests() const {
    if (!input_.GetRoot().AsDict().count("stat_requests")) return dummy_;
    return input_.GetRoot().AsDict().at("stat_requests");
}

const json::Node& JsonReader::GetRenderSettings() const {
    if (!input_.GetRoot().AsDict().count("render_settings")) return dummy_;
    return input_.GetRoot().AsDict().at("render_settings");
}

void JsonReader::ProcessRequests(const json::Node& stat_requests, RequestHandler& rh) const {
    json::Array result;
    for (auto& request : stat_requests.AsArray()) {
        const auto& request_map = request.AsDict();
        const auto& type = request_map.at("type").AsString();
        if (type == "Stop") result.push_back(PrintStop(request_map, rh).AsDict());
        if (type == "Bus") result.push_back(PrintRoute(request_map, rh).AsDict());
        if (type == "Map") result.push_back(PrintMap(request_map, rh).AsDict());
    }

    json::Print(json::Document{ result }, std::cout);
}

void JsonReader::FillCatalogue(transport_catalogue::TransportCatalogue& catalogue) {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsDict();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            catalogue.AddStop(stop_name, coordinates);
        }
    }
    FillStopDistances(catalogue);

    for (auto& request_bus : arr) {
        const auto& request_bus_map = request_bus.AsDict();
        const auto& type = request_bus_map.at("type").AsString();
        if (type == "Bus") {
            auto [bus_number, stops, circular_route] = FillRoute(request_bus_map, catalogue);
            catalogue.AddBus(bus_number, stops, circular_route);
        }
    }
}

std::tuple<std::string_view, geo::Coordinates, std::map<std::string_view, int>> JsonReader::FillStop(const json::Dict& request_map) const {
    std::string_view stop_name = request_map.at("name").AsString();
    geo::Coordinates coordinates = { request_map.at("latitude").AsDouble(), request_map.at("longitude").AsDouble() };
    std::map<std::string_view, int> stop_distances;
    auto& distances = request_map.at("road_distances").AsDict();
    for (auto& [stop_name, dist] : distances) {
        stop_distances.emplace(stop_name, dist.AsInt());
    }
    return std::make_tuple(stop_name, coordinates, stop_distances);
}

void JsonReader::FillStopDistances(transport_catalogue::TransportCatalogue& catalogue) const {
    const json::Array& arr = GetBaseRequests().AsArray();
    for (auto& request_stops : arr) {
        const auto& request_stops_map = request_stops.AsDict();
        const auto& type = request_stops_map.at("type").AsString();
        if (type == "Stop") {
            auto [stop_name, coordinates, stop_distances] = FillStop(request_stops_map);
            for (auto& [to_name, dist] : stop_distances) {
                auto from = catalogue.FindStop(stop_name);
                auto to = catalogue.FindStop(to_name);
                catalogue.AddDistanceBetweenStops(from, to, dist);
            }
        }
    }
}

std::tuple<std::string_view, std::vector<const transport_catalogue::Stop*>, bool> JsonReader::FillRoute(const json::Dict& request_map, transport_catalogue::TransportCatalogue& catalogue) const {
    std::string_view bus_number = request_map.at("name").AsString();
    std::vector<const transport_catalogue::Stop*> stops;
    for (auto& stop : request_map.at("stops").AsArray()) {
        stops.push_back(catalogue.FindStop(stop.AsString()));
    }
    bool circular_route = request_map.at("is_roundtrip").AsBool();

    return std::make_tuple(bus_number, stops, circular_route);
}

svg::Color GetColorByNode(const json::Node& color_node) {
    svg::Color color;
    if (color_node.IsString()) {
        color = color_node.AsString();
    }
    else if (color_node.AsArray().size() == 3) {
        color = svg::Rgb(color_node.AsArray()[0].AsDouble(),
            color_node.AsArray()[1].AsDouble(),
            color_node.AsArray()[2].AsDouble());
    }
    else if (color_node.AsArray().size() == 4) {
        color = svg::Rgba(color_node.AsArray()[0].AsDouble(),
            color_node.AsArray()[1].AsDouble(),
            color_node.AsArray()[2].AsDouble(),
            color_node.AsArray()[3].AsDouble());
    }
    return color;
}

renderer::MapRenderer JsonReader::FillRenderSettings(const json::Dict& request_map) const {

    std::vector<svg::Color> color_palette;
    for (const auto& color_node : request_map.at("color_palette").AsArray()) {
        color_palette.push_back(GetColorByNode(color_node));
    }

    renderer::RenderSettings render_settings;
    render_settings.width = request_map.at("width").AsDouble();
    render_settings.height = request_map.at("height").AsDouble();
    render_settings.padding = request_map.at("padding").AsDouble();
    render_settings.stop_radius = request_map.at("stop_radius").AsDouble();
    render_settings.line_width = request_map.at("line_width").AsDouble();
    render_settings.bus_label_font_size = request_map.at("bus_label_font_size").AsInt();
    render_settings.bus_label_offset = { request_map.at("bus_label_offset").AsArray()[0].AsDouble(), request_map.at("bus_label_offset").AsArray()[1].AsDouble() };
    render_settings.stop_label_font_size = request_map.at("stop_label_font_size").AsInt();
    render_settings.stop_label_offset = { request_map.at("stop_label_offset").AsArray()[0].AsDouble(), request_map.at("stop_label_offset").AsArray()[1].AsDouble() };
    render_settings.underlayer_color = GetColorByNode(request_map.at("underlayer_color"));
    render_settings.underlayer_width = request_map.at("underlayer_width").AsDouble();
    render_settings.color_palette = color_palette;

    return render_settings;
}

const json::Node JsonReader::PrintRoute(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const std::string& route_number = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();

    if (!rh.IsBusNumber(route_number)) {
        result = json::Builder{}
            .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found")
            .EndDict()
            .Build();
    }
    else {
        const auto& route_info = rh.GetBusStat(route_number);
        result = json::Builder{}
            .StartDict()
            .Key("request_id").Value(id)
            .Key("curvature").Value(route_info->curvature)
            .Key("route_length").Value(route_info->route_length)
            .Key("stop_count").Value(static_cast<int>(route_info->route_stops_count))
            .Key("unique_stop_count").Value(static_cast<int>(route_info->unique_stops_count))
            .EndDict()
            .Build();
    }
    return result;
}

const json::Node JsonReader::PrintStop(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const std::string& stop_name = request_map.at("name").AsString();
    const int id = request_map.at("id").AsInt();

    if (!rh.IsStopName(stop_name)) {
        result = json::Builder{}
            .StartDict()
            .Key("request_id").Value(id)
            .Key("error_message").Value("not found")
            .EndDict()
            .Build();
    }
    else {
        json::Array buses;
        for (const auto& bus : rh.GetBusesByStop(stop_name)) {
            buses.push_back(bus);
        }
        result = json::Builder{}
            .StartDict()
            .Key("request_id").Value(id)
            .Key("buses").Value(buses)
            .EndDict()
            .Build();
    }
    return result;
}

const json::Node JsonReader::PrintMap(const json::Dict& request_map, RequestHandler& rh) const {
    json::Node result;
    const int id = request_map.at("id").AsInt();
    std::ostringstream strm;
    svg::Document map = rh.RenderMap();
    map.Render(strm);

    result = json::Builder{}
        .StartDict()
        .Key("request_id").Value(id)
        .Key("map").Value(strm.str())
        .EndDict()
        .Build();

    return result;
}