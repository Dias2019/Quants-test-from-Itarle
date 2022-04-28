# Assessment-by-Itarle

We were provided by a dataset (*"scandi.csv"*) consisting from 100 Scandinavian blue chip stocks and their trades over 4 days. The task is to conduct data analysis on the given data and answer the given questions.

Please refer to "*Source.cpp*" file for more details of used functions.

---
## Pre-processing of dataset
Before digging into a questions we should make sure that our dataset is consistent and withougt any misalignments.

After reading a csv file and loading it into dataframe via ```readCSV()``` function, we can check the unique stock codes with ```checkStockCodes()``` function. This supposed to return around 100 unique stock codes as it was specified in a task.

After running ```checkStockCodes()```, we confirmed that there are 100 unique stocks as it was supposed.
![image](./img/checkStocks.JPG)

*(an output of checkStockCodes)*

However, there are a number of specification that we were instructed of.
1) We should exclude auction trades which are usually conducted twice a day
    - however, we are including "XT" or empty condition codes.
    - In detail implementation of this filter could be seen from ```isnotAuction()``` function.

2) We should consider no trading periods as it might introduce large time gaps.

3) I believe that it might be better to sort out the given data in terms of date because this will make sure that two consecutive trades are calculated.
    - In a trade-off between speed and accuracy, I chose an accuracy as this type of post-analysis might consider accuracy as more important factor.


---

## Question 1: mean time between trades

Mean time between trades is calculated by ```meanTimeBetweenTrades()``` function. There are two important features to point out in my calculation of mean time between trades.
 - Every data row has been sorted by data and time to make sure that there are consecutive trades.
 - Consecutive trades should be executed in the same date. Otherwise, it is a gap of no trade period.

 Consequently, we recevied the following results:


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