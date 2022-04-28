#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>


using namespace std;


struct DataRows {
	string BloombergCode;
	double bidPrice;
	double askPrice;
	double tradePrice;
	double tradeVol;
	double date;
	double secsPastMidnight;
	string conditionCodes;

	double time;
};


// helper functions
bool conditionCodeChecker(const std::string code) {
	
	// return True if 
	if (code.find("XT") != std::string::npos || code.compare("") != 0) {
		return false;
	}
	return true;
}


bool compareTime(const DataRows& a, const DataRows& b) {
	if (a.date != b.date)
		return a.secsPastMidnight < b.secsPastMidnight;
	return a.secsPastMidnight < b.secsPastMidnight;
}



class ScandiAnalysis {
public:
	void readCSV(const std::string& csvFilePath, std::map<std::string, std::vector<DataRows>>& dataframe);
	bool isnotAuction(const DataRows& data);
	void checkStockCodes(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void longestTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe);
	vector<DataRows> getDataframe();

private:
	vector<DataRows> dataframe;
};

void ScandiAnalysis::readCSV(const std::string& csvFilePath, std::map<std::string, std::vector<DataRows>>& dataframe) {

	fstream fin;

	fin.open(csvFilePath, ios::in);

	vector<string> row;
	string line, word, temp;

	int count = 0;
	while (fin >> temp && count < 1000) {

		count++;

		row.clear();

		getline(fin, line);

		stringstream s(temp + line);

		while (getline(s, word, ',')) {
			row.push_back(word);
		}

		DataRows temp;
		temp.BloombergCode = row[0]; temp.bidPrice = stod(row[2]); temp.askPrice = stod(row[3]);
		temp.tradePrice = stod(row[4]); temp.date = stod(row[10]); temp.secsPastMidnight = stod(row[11]);
		temp.conditionCodes = row[14]; temp.tradeVol = stod(row[7]);

		dataframe[row[0]].push_back(temp);

		// as we should filter out auction trades
		if (isnotAuction(temp)) {
			dataframe[row[0]].push_back(temp);
		}
	}

	// need to sort each vector afterwards
	for (auto& stock : dataframe) {
		sort(stock.second.begin(), stock.second.end(), compareTime);
	}
}


bool ScandiAnalysis::isnotAuction(const DataRows& data) {

	if ((data.bidPrice > data.askPrice && (data.conditionCodes != "XT" || data.conditionCodes != "")) || data.bidPrice == 0 || data.askPrice == 0)
		return false;

	return true;
}


void ScandiAnalysis::checkStockCodes(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "The number of unique stock codes in the given dataset is " << dataframe.size() << endl;

	cout << "Let's manually look through stock codes:" << endl;
	for (const auto& stock : dataframe) {
		cout << stock.first << endl;
	}
}


void ScandiAnalysis::meanTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	// exclude no trading periods 
	// (time between first and last trade - no trading time) / number of trades



	cout << "---Mean time between trades---" << endl;


	for (const auto& stock : dataframe) {

		double sum = 0;
		int n = stock.second.size(), count = 0;

		for (int i = 0; i < n; i++) {

			// we verify if any trade has happened at that time by checking the volume of trades
			if (i + 1 < n && stock.second[i].tradeVol > 0 && stock.second[i].date == stock.second[i + 1].date) {
				sum += (stock.second[i + 1].secsPastMidnight - stock.second[i].secsPastMidnight);
				count += 1;
			}

		}

		cout << stock.first << ": " << (sum / count) << endl;

	}

}


void ScandiAnalysis::medianTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Median time between trades---" << endl;

	for (const auto& stock : dataframe) {

		vector<double> timeDiff;

		int n = stock.second.size();
		for (int i = 0; i < n; i++) {

			if (i + 1 < n && stock.second[i].tradeVol > 0 && stock.second[i].date == stock.second[i+1].date) {
				timeDiff.push_back(stock.second[i + 1].secsPastMidnight - stock.second[i].secsPastMidnight);
			}

		}

		sort(timeDiff.begin(), timeDiff.end());
		if (n % 2 == 1)
			cout << stock.first << ": " << (timeDiff[n/2] + timeDiff[n / 2 + 1]) / 2 << endl;
		else
			cout << stock.first << ": " << timeDiff[n / 2] << endl;
	}

}


void ScandiAnalysis::longestTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Longest time between trades---" << endl;

	for (const auto& stock : dataframe) {

		vector<double> timeDiff;

		int n = stock.second.size();
		for (int i = 0; i < n; i++) {

			if (i + 1 < n && stock.second[i].tradeVol > 0 && stock.second[i].date == stock.second[i + 1].date) {
				timeDiff.push_back(stock.second[i + 1].secsPastMidnight - stock.second[i].secsPastMidnight);
			}

		}

		double longest = *max_element(timeDiff.begin(), timeDiff.end());
		cout << stock.first << ": " << longest << endl;
	}

}


void ScandiAnalysis::meanBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Mean bid ask spread---" << endl;

	for (const auto& stock : dataframe) {

		double sum = 0;
		int n = stock.second.size(), count = 0;

		for (int i = 0; i < n; i++) {

			// we verify if any trade has happened at that time by checking the volume of trades
			if (stock.second[i].askPrice >= stock.second[i].bidPrice) {
				sum += (stock.second[i].askPrice - stock.second[i].bidPrice);
				count += 1;
			}

		}

		cout << stock.first << ": " << (sum / count) << endl;

	}

}


void ScandiAnalysis::medianBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Median bid ask spread---" << endl;

	for (const auto& stock : dataframe) {

		vector<double> bidaskSpread;

		int n = stock.second.size();
		for (int i = 0; i < n; i++) {

			if (stock.second[i].askPrice >= stock.second[i].bidPrice) {
				bidaskSpread.push_back(stock.second[i].askPrice - stock.second[i].bidPrice);
			}

		}

		sort(bidaskSpread.begin(), bidaskSpread.end());
		if (n % 2 == 1)
			cout << stock.first << ": " << (bidaskSpread[n / 2] + bidaskSpread[n / 2 + 1]) / 2 << endl;
		else
			cout << stock.first << ": " << bidaskSpread[n / 2] << endl;
	}

}


void ScandiAnalysis::meanTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	for (const auto& stock : dataframe) {

		double first_tick = (stock.second[0].tradePrice > stock.second[1].tradePrice) ? 0 : 1;
		double latestTickChange[2] = { first_tick, stock.second[0].time };

		int count = 1;
		double sum = stock.second[1].time - stock.second[0].time;

		int n = stock.second.size();
		for (int i = 1; i < n; i++) {

			if (i + 1 < n) {

				double tick = (stock.second[i].tradePrice > stock.second[i + 1].tradePrice) ? 0 : 1;
				if (latestTickChange[0] != tick) {

					// tick change happens
					sum += (stock.second[i].time - latestTickChange[1]);
					count += 1;
					latestTickChange[0] = tick; latestTickChange[1] = stock.second[i].time;

				}

			}

		}

		cout << stock.first << ": " << (sum / count) << endl;

	}

}



void medianTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	for (const auto& stock : dataframe) {

		double first_tick = (stock.second[0].tradePrice > stock.second[1].tradePrice) ? 0 : 1;
		double latestTickChange[2] = { first_tick, stock.second[0].time };

		std::vector<double> timeDiff;

		int n = stock.second.size();
		for (int i = 1; i < n; i++) {

			if (i + 1 < n) {

				double tick = (stock.second[i].tradePrice > stock.second[i + 1].tradePrice) ? 0 : 1;
				if (latestTickChange[0] != tick) {

					// tick change happens
					timeDiff.push_back(stock.second[i].time - latestTickChange[1]);
					latestTickChange[0] = tick; latestTickChange[1] = stock.second[i].time;

				}

			}

		}

		sort(timeDiff.begin(), timeDiff.end());
		if (n % 2 == 1)
			cout << stock.first << ": " << (timeDiff[n / 2] + timeDiff[n / 2 + 1]) / 2 << endl;
		else
			cout << stock.first << ": " << timeDiff[n / 2] << endl;

	}

}


int main() {

	map<string, vector<DataRows>> dataframe;
	string csvFilePath = "C:\\Users\\diaszhandar\\Downloads\\scandi.csv";
	
	ScandiAnalysis dataloader;
	dataloader.readCSV(csvFilePath, dataframe);

	for (const auto& stock : dataframe) {

		for (const auto& row : stock.second) {

			cout << "BLBG code: " << stock.first << endl;
			cout << "blm code: " << row.BloombergCode << endl;
			cout << "bid price: " << row.bidPrice << endl;
			cout << "ask price: " << row.askPrice << endl;
			cout << endl;
		}

	}

	return 0;
}