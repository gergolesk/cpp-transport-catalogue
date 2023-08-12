#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <memory>

const double eps = 1e-6;

namespace transport_catalogue {

	struct RoutingSettings {
		int bus_wait_time = 0;
		double bus_velocity = 0.0;
	};

	struct EdgeInfo {
		const Stop* stop;
		const Bus* bus;
		int count;
	};

	struct CompletedRoute {
		struct Line {
			const Stop* stop;
			const Bus* bus;
			double wait_time;
			double run_time;
			int count_stops;
		};
		double total_time;
		std::vector<Line> route;
	};

	class Router {
	public:
		Router() = default;

		Router(const int bus_wait_time, const double bus_velocity) {
			routing_settings_.bus_wait_time = bus_wait_time;
			routing_settings_.bus_velocity = bus_velocity;
		}

		Router(const Router& settings, const TransportCatalogue& catalogue) {
			//bus_wait_time_ = settings.bus_wait_time_;
			//bus_velocity_ = settings.bus_velocity_;
			routing_settings_ = settings.routing_settings_;

			BuildGraph(catalogue);
		}

		const graph::DirectedWeightedGraph<double>& BuildGraph(const TransportCatalogue& catalogue);
		//const std::optional<graph::Router<double>::RouteInfo> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
		const std::optional<graph::Router<double>::RouteInfo> GetRoute(const std::string_view stop_from, const std::string_view stop_to) const;
		std::optional<CompletedRoute> FindRoute(const std::string_view stop_from, const std::string_view stop_to);
		const graph::DirectedWeightedGraph<double>& GetGraph() const;

	private:
		//int bus_wait_time_ = 0;
		//double bus_velocity_ = 0.0;
		RoutingSettings routing_settings_;
		graph::DirectedWeightedGraph<double> graph_;
		std::map<std::string, graph::VertexId> stop_ids_;
		std::unordered_map<graph::EdgeId, EdgeInfo> edges_;
		std::unique_ptr<graph::Router<double>> router_;
	};

}	