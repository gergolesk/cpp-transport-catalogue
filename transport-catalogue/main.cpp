#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main() {

    using namespace transport_catalogue;

    TransportCatalogue catalogue;
    input_queries_utils::UpdateTransportCatalogue(catalogue, cin);
    stat_queries_utils::ReadTransportCatalogue(catalogue, cin, cout);

    return 0;
}
