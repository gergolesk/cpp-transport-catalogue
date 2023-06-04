#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <vector>
#include <algorithm>

namespace transport_catalogue {

	namespace input_queries_utils {

		void ParseStopQueries(TransportCatalogue& catalogue, const std::vector<std::pair<std::string, std::string>>& stop_queries);
		void ParseBusQueries(TransportCatalogue& catalogue, const std::vector<std::string>& bus_queries);
		void UpdateTransportCatalogue(TransportCatalogue& catalogue, std::istream& is);
	}
}