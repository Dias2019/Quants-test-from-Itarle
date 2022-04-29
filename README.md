# Assessment-by-Itarle

We were provided by a dataset (*"scandi.csv"*) consisting from 100 Scandinavian blue chip stocks and their trades over 4 days. The task is to conduct data analysis on the given data and answer the questions.

Please refer to "*Source.cpp*" file for more details of used functions. Also, the complete answers to the questions are presented in the ```answers.xlsx``` file on stock-by-stock basis.

---
## Pre-processing of dataset
Before analysing and answering questions, we should make sure that our dataset is consistent and without any misalignments.

After reading a csv file via ```readCSV()``` function, we should check the stock codes with ```checkStockCodes()``` function. This supposed to return around 100 unique stock codes as it was specified in a task.

After running ```checkStockCodes()```, we confirmed that there are 100 unique stocks.
![image](./img/checkStocks.JPG)


However, there are a number of specification that we were instructed of.
1) Auction trades were filtered out by ```isnotAuction()``` function
    - however, we are including "XT" or empty condition codes.  
2) We should consider no trading periods as it might introduce large time gaps.
    - IQD was used to detect outliers. For both time between trades and time between tick changes, we specified a threshold value (calculated via Median and Interquartile Deviation Method), and any value above this threshold were removed from analysis. Because it was no trading periods.

3) I believe that it might be better to sort out the given data in terms of date because this will make sure that two consecutive trades are calculated.
    - Even though most of the data were presented in ascending time order, I decided that it would necessary to sort these values to make sure that these are consecutive trades.
    - In a trade-off between speed and accuracy, I chose an accuracy as this type of post-analysis might consider accuracy as more important factor.


---

## Question 1: mean time between trades

Mean time between trades is calculated by ```meanTimeBetweenTrades()``` function. 
1) Outliers were removed by IQD method
2) Every time we make sure that a trade was executed by checking trade volume (which has to be more than 0)


---
## Question 2: median time between trades
Median is considered to be a middle element of sorted time differences. Therefore, time differences, which were calculated in similar way as in Question 1, were stored in a vector data structure. Then, a median was found via sorting the derived vector and outputting an element in the **middle** of the vector.

This was calculated by ```medianTimeBetweenTrades()``` function and outputted the following times:

---
## Question 3: mean time between tick changes
Tick is a measure of minimum upward or downward movement in the price of a security. In detail, we define two types of tick as a movement indicator - uptick and downtick.
- Uptick indicates a trade where the transaction has occurred at a price higher than the previous transaction.
- Downtick indicates a transaction that has occurred at a lower price.

In other words, we can calculate a slope between two consecutive trading points. If the slope is positive, then it is uptick. In opposite case, it is a downtick.

Therefore, we calculate a mean time when tick changes from uptick to downtick or vice versa. For details, please refer to ```meanTimeBetweenTick()``` function.

## #Image with results

---

## Question 4: median time between tick changes
Procedure would be similar to the previous question. However, we would be saving time differences between tick changes and outputing a middle value after sorting the time differences.