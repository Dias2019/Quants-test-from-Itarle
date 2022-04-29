#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <numeric>


using namespace std;


struct Time
{
	double date;
	double secondsPastMidnight;

	Time(double date = 0, double secondsPastMidnight = 0)
		: date(date), secondsPastMidnight(secondsPastMidnight)
	{
	}

	Time operator-(const Time& a) const {
		return Time(date - a.date, secondsPastMidnight - a.secondsPastMidnight);
	}

	bool operator<(const Time& a) const {
		if (a.date != date)
			return a.date < date;
		return a.secondsPastMidnight < secondsPastMidnight;
	}

};


struct DataRows {
	string BloombergCode;
	double bidPrice;
	double askPrice;
	double tradePrice;
	double tradeVol;
	double date;
	double secsPastMidnight;
	Time time;
	string conditionCodes;
};


// helper functions
bool conditionCodeChecker(const std::string code) { 
	if (code.find("XT") != std::string::npos || code.compare("") != 0) {
		return false;
	}
	return true;
}

bool compareTime(const DataRows& a, const DataRows& b) {
	return b.time < a.time;
}

bool isnotAuction(const DataRows& data) {
	if (data.bidPrice > data.askPrice && (data.conditionCodes != "XT" || data.conditionCodes != ""))
		return false;
	return true;
}

void outlierDetection(std::vector<double>& time_between) {

	int n = time_between.size();
	int Nmod4 = n % 4;
	int M, ML, MU;
	double q2, q1, q3;

	if (n == 0 || n == 1) return;

	if (Nmod4 == 0) {
		M = n / 2; ML = M / 2; MU = M + ML;
		q1 = (time_between[ML] + time_between[ML - 1]) / 2;
		q2 = (time_between[M] + time_between[M - 1]) / 2;
		q3 = (time_between[MU] + time_between[MU - 1]) / 2;
	}
	else if (Nmod4 == 1) {
		M = n / 2; ML = M / 2; MU = M + ML + 1;
		q1 = (time_between[ML] + time_between[ML - 1]) / 2;
		q2 = time_between[M];
		q3 = (time_between[MU] + time_between[MU - 1]) / 2;
	}
	else if (Nmod4 == 2) {
		M = n / 2; ML = M / 2; MU = M + ML;
		q1 = time_between[ML];
		q2 = (time_between[M] + time_between[M - 1]) / 2;
		q3 = time_between[MU];
	}
	else {
		M = n / 2; ML = M / 2; MU = M + ML + 1;
		q1 = time_between[ML];
		q2 = time_between[M];
		q3 = time_between[MU];
	}
	
	double IQR = q3 - q1;
	double upperThreshold = 1.5 * IQR + q3;    // a threshold for defining suspected outlier

	// any value greater than threshold is removed as outlier
	while (time_between.size() > 0 && upperThreshold < time_between[time_between.size() - 1])
		time_between.pop_back();

}



class ScandiAnalysis {
public:
	void readCSV(const std::string& csvFilePath, std::map<std::string, std::vector<DataRows>>& dataframe);
	void checkStockCodes(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void longestTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe);

};

void ScandiAnalysis::readCSV(const std::string& csvFilePath, std::map<std::string, std::vector<DataRows>>& dataframe) {

	fstream fin;
	fin.open(csvFilePath, ios::in);

	vector<string> row;
	string line, word, temp;

	int count = 0;
	while (fin >> temp && count < 50000) {

		count++;

		row.clear();

		getline(fin, line);

		stringstream s(temp + line);

		while (getline(s, word, ',')) {
			row.push_back(word);
		}

		DataRows temp;
		Time tmp(stod(row[10]), stod(row[11]));
		temp.BloombergCode = row[0]; temp.bidPrice = stod(row[2]); temp.askPrice = stod(row[3]); temp.tradePrice = stod(row[4]);
		temp.tradeVol = stod(row[7]); temp.time = tmp;
		temp.conditionCodes = row[14];

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



void ScandiAnalysis::checkStockCodes(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "The number of unique stock codes in the given dataset is " << dataframe.size() << endl;

	cout << "Let's manually look through stock codes:" << endl;
	for (const auto& stock : dataframe) {
		cout << stock.first << endl;
	}
}


void ScandiAnalysis::meanTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Mean time between trades---" << endl;

	for (const auto& stock : dataframe) {
		
		vector<double> timeBetweenTrades;
		int n = stock.second.size();

		for (int i = 0; i < n; i++) {

			// we verify if any trade has happened at that time by checking the volume of trades
			if (i + 1 < n && stock.second[i].tradeVol > 0) {
				Time timeDiff = stock.second[i + 1].time - stock.second[i].time;
				timeBetweenTrades.push_back(timeDiff.date * 86400 + timeDiff.secondsPastMidnight);
			}

		}

		sort(timeBetweenTrades.begin(), timeBetweenTrades.end());
		outlierDetection(timeBetweenTrades);

		double meanTimeBetweenTrades = accumulate(timeBetweenTrades.begin(), timeBetweenTrades.end(), 0) / timeBetweenTrades.size();
		cout << stock.first << ": " << meanTimeBetweenTrades << endl;

	}

}


void ScandiAnalysis::medianTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Median time between trades---" << endl;

	for (const auto& stock : dataframe) {

		vector<double> timeBetweenTrades;
		int n = stock.second.size();

		for (int i = 0; i < n; i++) {

			if (i + 1 < n && stock.second[i].tradeVol > 0) {
				Time timeDiff = stock.second[i + 1].time - stock.second[i].time;
				timeBetweenTrades.push_back(timeDiff.date * 86400.0 + timeDiff.secondsPastMidnight);
			}

		}

		sort(timeBetweenTrades.begin(), timeBetweenTrades.end());
		outlierDetection(timeBetweenTrades);

		int updated_size = timeBetweenTrades.size();
		double median;
		if (updated_size % 2 == 1)
			median = (timeBetweenTrades[updated_size / 2.0] + timeBetweenTrades[updated_size / 2.0 - 1]) / 2.0;
		else
			median = timeBetweenTrades[updated_size / 2.0];

		cout << stock.first << ": " << median << endl;
	}

}


void ScandiAnalysis::longestTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Longest time between trades---" << endl;

	for (const auto& stock : dataframe) {

		vector<double> timeBetweenTrades;
		int n = stock.second.size();

		for (int i = 0; i < n; i++) {

			if (i + 1 < n && stock.second[i].tradeVol > 0) {
				Time timeDiff = stock.second[i + 1].time - stock.second[i].time;
				timeBetweenTrades.push_back(timeDiff.date * 86400.0 + timeDiff.secondsPastMidnight);
			}

		}

		sort(timeBetweenTrades.begin(), timeBetweenTrades.end());
		if (stock.first == "WRT1V FH Equity") {
			for (auto x : timeBetweenTrades) {
				cout << x << endl;
			}
		}

		sort(timeBetweenTrades.begin(), timeBetweenTrades.end());
		outlierDetection(timeBetweenTrades);

		double longestTime = timeBetweenTrades[timeBetweenTrades.size() - 1];
		cout << stock.first << ": " << longestTime << endl;
	}

}

/*
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

}*/



int main() {

	map<string, vector<DataRows>> dataframe;
	string csvFilePath = "C:\\Users\\diaszhandar\\Downloads\\scandi.csv";

	ScandiAnalysis analysis;
	analysis.readCSV(csvFilePath, dataframe);
	analysis.medianTimeBetweenTrades(dataframe);
	analysis.medianTimeBetweenTrades(dataframe);
	analysis.longestTimeBetweenTrades(dataframe);


	/*for (const auto& stock : dataframe) {

		for (const auto& row : stock.second) {

			cout << "BLBG code: " << stock.first << endl;
			cout << "date: " << row.time.date << endl;
			cout << "SAM: " << row.time.secondsPastMidnight << endl;
			cout << endl;
		}

	}*/

	return 0;
}

