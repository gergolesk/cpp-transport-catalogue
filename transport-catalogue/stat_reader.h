#pragma once

#include "transport_catalogue.h"
#include <iostream>
#include <vector>

namespace transport_catalogue {

	namespace stat_queries_utils {

		void PrintQueriesData(const TransportCatalogue& catalogue, const std::vector<std::string>& queries, std::ostream& os);
		void ReadTransportCatalogue(const TransportCatalogue& catalogue, std::istream& is, std::ostream& os);

	}
}