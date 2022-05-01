#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <numeric>
#include <limits>
using namespace std;


// customized data structures
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
int lastDigit(const string& input);
void normalize(double* input, int size);
bool conditionCodeChecker(const std::string code);
bool compareTime(const DataRows& a, const DataRows& b);
bool isnotAuction(const DataRows& data);
void outlierDetection(std::vector<double>& time_between);



// main class
class ScandiAnalysis {
public:
	void readCSV(const std::string& csvFilePath, std::map<std::string, std::vector<DataRows>>& dataframe);
	void checkStockCodes(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void longestTimeBetweenTrades(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void longestTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void meanBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void medianBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe);
	void RoundNumberEffect(const string& csvFilePath);

};

void ScandiAnalysis::readCSV(const std::string& csvFilePath, std::map<std::string, std::vector<DataRows>>& dataframe) {

	fstream fin;
	fin.open(csvFilePath, ios::in);

	vector<string> row;
	string line, word, temp;

	int count = 0;
	while (fin >> temp && count < 5000000) {

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

		double meanTimeBetweenTrades = (float)accumulate(timeBetweenTrades.begin(), timeBetweenTrades.end(), 0) / (float)timeBetweenTrades.size();
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

		int updated_size = timeBetweenTrades.size();
		double median;
		if (updated_size % 2 == 0)
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
		outlierDetection(timeBetweenTrades);

		double longestTime = timeBetweenTrades[timeBetweenTrades.size() - 1];
		cout << stock.first << ": " << longestTime << endl;
	}

}


void ScandiAnalysis::meanTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Mean time between tick changes---" << endl;

	for (const auto& stock : dataframe) {

		int latestTick = (stock.second[1].tradePrice > stock.second[0].tradePrice) ? 1 : 0;
		Time latestTickTime = stock.second[1].time;
		vector<double> timeBetween;

		int n = stock.second.size();
		for (int i = 2; i < n; i++) {


			int tick = (stock.second[i].tradePrice > stock.second[i - 1].tradePrice) ? 1 : 0;
			if (latestTick != tick) {

				// tick change happened
				Time timeDiff = stock.second[i].time - latestTickTime;
				timeBetween.push_back(timeDiff.date * 86400.0 + timeDiff.secondsPastMidnight);
				latestTick = tick; latestTickTime = stock.second[i].time;
			}


		}
		sort(timeBetween.begin(), timeBetween.end());
		outlierDetection(timeBetween);

		double meanTimeBetweenTick = (timeBetween.size() > 0) ? accumulate(timeBetween.begin(), timeBetween.end(), 0) / timeBetween.size() : numeric_limits<double>::infinity();
		cout << stock.first << ": " << meanTimeBetweenTick << endl;

	}

}


void ScandiAnalysis::medianTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Median time between tick changes---" << endl;

	for (const auto& stock : dataframe) {

		int latestTick = (stock.second[1].tradePrice > stock.second[0].tradePrice) ? 1 : 0;
		Time latestTickTime = stock.second[1].time;
		vector<double> timeBetween;

		int n = stock.second.size();
		for (int i = 1; i < n; i++) {

			int tick = (stock.second[i].tradePrice > stock.second[i - 1].tradePrice) ? 1 : 0;
			if (latestTick != tick) {

				// tick change happened
				Time timeDiff = stock.second[i].time - latestTickTime;
				timeBetween.push_back(timeDiff.date * 86400.0 + timeDiff.secondsPastMidnight);
				latestTick = tick; latestTickTime = stock.second[i].time;
			}

		}

		sort(timeBetween.begin(), timeBetween.end());

		double median;
		int updated_size = timeBetween.size();
		if (updated_size == 0)
			median = numeric_limits<double>::infinity();
		else if (updated_size % 2 == 0)
			median = (timeBetween[updated_size / 2.0] + timeBetween[updated_size / 2.0 - 1]) / 2.0;
		else
			median = timeBetween[updated_size / 2.0];

		cout << stock.first << ": " << median << endl;
	}

}


void ScandiAnalysis::longestTimeBetweenTick(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Longest time between tick changes---" << endl;

	for (const auto& stock : dataframe) {

		int latestTick = (stock.second[1].tradePrice > stock.second[0].tradePrice) ? 1 : 0;
		Time latestTickTime = stock.second[1].time;
		vector<double> timeBetween;

		int n = stock.second.size();
		for (int i = 1; i < n; i++) {

			int tick = (stock.second[i].tradePrice > stock.second[i - 1].tradePrice) ? 1 : 0;
			if (latestTick != tick) {

				// tick change happened
				Time timeDiff = stock.second[i].time - latestTickTime;
				timeBetween.push_back(timeDiff.date * 86400.0 + timeDiff.secondsPastMidnight);
				latestTick = tick; latestTickTime = stock.second[i].time;
			}

		}

		sort(timeBetween.begin(), timeBetween.end());
		outlierDetection(timeBetween);

		double longestTime = (timeBetween.size() > 0) ? timeBetween[timeBetween.size() - 1] : numeric_limits<double>::infinity();
		cout << stock.first << ": " << longestTime << endl;
	}
}


void ScandiAnalysis::meanBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Mean bid ask spread---" << endl;

	for (const auto& stock : dataframe) {

		double sum = 0, count = 0;
		int n = stock.second.size();

		for (int i = 0; i < n; i++) {

			if (stock.second[i].askPrice > 0 && stock.second[i].bidPrice > 0) {
				sum += (stock.second[i].askPrice - stock.second[i].bidPrice);
				count += 1;
			}

		}

		double mean = sum / count;
		cout << stock.first << ": " << (sum / count) << endl;
	}

}


void ScandiAnalysis::medianBidAskSpread(const std::map<std::string, std::vector<DataRows>>& dataframe) {

	cout << "---Median bid ask spread---" << endl;

	for (const auto& stock : dataframe) {

		vector<double> bidaskSpread;

		int n = stock.second.size();
		for (int i = 0; i < n; i++) {

			if (stock.second[i].askPrice > 0 && stock.second[i].bidPrice > 0) {
				bidaskSpread.push_back(stock.second[i].askPrice - stock.second[i].bidPrice);
			}

		}

		sort(bidaskSpread.begin(), bidaskSpread.end());

		int spreadSize = bidaskSpread.size();
		if (spreadSize % 2 == 1)
			cout << stock.first << ": " << (bidaskSpread[spreadSize / 2] + bidaskSpread[spreadSize / 2 + 1]) / 2 << endl;
		else
			cout << stock.first << ": " << bidaskSpread[spreadSize / 2] << endl;
	}

}


void ScandiAnalysis::RoundNumberEffect(const string& csvFilePath) {

	cout << "---Round Number Effect---" << endl;

	fstream fin;
	fin.open(csvFilePath, ios::in);
	vector<string> row;
	string line, word, temp;
	
	double occurances[10]{};

	while (fin >> temp) {

		row.clear();

		getline(fin, line);

		stringstream s(temp + line);

		while (getline(s, word, ',')) {
			row.push_back(word);
		}

		if (stod(row[7]) > 0) {
			string tradePrice = row[4];
			string tradeVol = row[7];

			occurances[lastDigit(tradePrice)]++;
			occurances[lastDigit(tradeVol)]++;
		}
	}

	for (int i = 0; i < 10; i++) {
		cout << i << ": " << occurances[i] << endl;
	}

	normalize(occurances, 10);

	for (int i = 0; i < 10; i++) {
		cout << i << ": " << occurances[i] << endl;
	}
}




int lastDigit(const string& input) {
	
	int lastDigit;

	int l = input.length();
	if (input[l - 1] == 0) {

		if (input[l - 2] == '.')
			lastDigit = (int)input[l - 3] - 48;
		else
			lastDigit = 0;

	}
	else
		lastDigit = (int)input[l - 1] - 48;

	return lastDigit;
}

void normalize(double* input, int size) {

	double sum = accumulate(input, input + size, 0);
	for (int i = 0; i < size; i++) {
		input[i] = input[i] / sum * 100;
	}
}

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
	double upperThreshold = 1.5 * IQR + q3;    // a threshold for defining suspected outliers

	// any value greater than threshold is removed as outlier
	while (time_between.size() > 0 && upperThreshold < time_between[time_between.size() - 1])
		time_between.pop_back();

}


