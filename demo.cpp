#include <OMS.hpp>

std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> res;
    std::string token;
    std::stringstream ss(line);

    while (getline(ss, token, ',')) {
        res.push_back(token);
    }
    return res;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./gateway symbols.csv events.txt\n";
        return 1;
    }

    OMS* oms = new OMS(argv[1]);

    std::ifstream fin(argv[2]);
    std::string line;
    int line_no = 0;

    while (getline(fin, line)) {
        line_no++;

        auto row = split(line);

        if (row.empty()) {
            std::cout << "PARSE_REJECT," << line_no << ",EMPTY\n";
            continue;
        }

        char type = row[0][0];

        if (type == 'N' && row.size() == 8){
            Side buy_sell_indicator = oms->parse_side(row[5]);
            if(buy_sell_indicator == Side::INVALID){
                std::cout << "REJECT," << row[1] << "," << row[2] << "," << row[3] << ",INVALID_SIDE" << std::endl;
                continue;
            }
            
            RiskErrorCodes ret =  oms->PlaceNewOrder(std::stol(row[1]), row[2], row[3], row[4], buy_sell_indicator,std::stoi(row[6]), std::stod(row[7]) * 100 );
        }
        else if (type == 'C' && row.size() == 4){
            RiskErrorCodes ret = oms->PlaceCancelOrder(std::stol(row[1]), row[2], row[3]);
        }
        else if (type == 'F' && row.size() == 5){
            oms->onTradeConfirmation(std::stol(row[1]), row[2], row[3], std::stoi(row[4]));
        }
        else{
            std::cout << "PARSE_REJECT," << line_no << ",MALFORMED_INPUT\n";
        }
    }

    return 0;
}