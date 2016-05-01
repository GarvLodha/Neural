#ifndef posixtestclient_h__INCLUDED
#define posixtestclient_h__INCLUDED

#include "EWrapper.h"

#include <memory>

class EPosixClientSocket;

enum State {
	ST_CONNECT,
	ST_REQHISTORICALDATA,
	ST_REQEXECUTIONS,
	ST_REQMKTDATA,
	ST_PLACEORDER,
	ST_REQCURRENTTIME,
	ST_PLACEORDER_ACK,
	ST_CANCELORDER,
	ST_CANCELORDER_ACK,
	ST_PING,
	ST_PING_ACK,
	ST_IDLE
};


//Appended Structures.
//--------------------------------------------------------------------
struct NeuDatabase{
	double fHighPrice;
	double fLowPrice;
	double fClosePrice;
};

class PosixTestClient : public EWrapper
{
public:

	PosixTestClient();
	PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int* portfolioPosition, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt);	//appended statement.
//	PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt, int algoDataPointCount, int algoPercentile, int algoPercentileWindowSize, int algoVolWindowSize, double algoConstantRho, int algoPriceRangeWindowSize, int algoMktVarianceDays, double algoRiskExposure);	//appended statement.
//	PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt, int algoDataPointCount, int algoPercentile, int algoPercentileWindowSize, int algoVolWindowSize, double algoConstantRho, int algoPriceRangeWindowSize, int algoMktVarianceDays, long int * algoRiskExposure);	//appended statement.
	PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt, int algoDataPointCount, int algoPercentile, int algoPercentileWindowSize, int algoVolWindowSize, double algoConstantRho, int algoPriceRangeWindowSize, int algoMktVarianceDays, long int * algoRiskExposure, char * dbUpdateTime);	//appended statement.
	~PosixTestClient();

	void processMessages();

public:

	bool connect(const char * host, unsigned int port, int clientId = 0);
	void disconnect() const;
	bool isConnected() const;

private:

	void reqCurrentTime();
	void placeOrder();
	void placeOrder(int tickerId);
	void cancelOrder();

/*******Appended Methods.*******/
	void ReqHistoricalData();
	void Algorithm(int algoCallCount);
	void ReqExecutions();
	void ReqMktData();
	int MaxExposure();

public:
	// events
	void tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute);
	void tickSize(TickerId tickerId, TickType field, int size);
	void tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
		double optPrice, double pvDividend, double gamma, double vega, double theta, double undPrice);
	void tickGeneric(TickerId tickerId, TickType tickType, double value);
	void tickString(TickerId tickerId, TickType tickType, const IBString& value);
	void tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
		double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry);
	void orderStatus(OrderId orderId, const IBString &status, int filled,
		int remaining, double avgFillPrice, int permId, int parentId,
		double lastFillPrice, int clientId, const IBString& whyHeld);
	void openOrder(OrderId orderId, const Contract&, const Order&, const OrderState&);
	void openOrderEnd();
	void winError(const IBString &str, int lastError);
	void connectionClosed();
	void updateAccountValue(const IBString& key, const IBString& val,
		const IBString& currency, const IBString& accountName);
	void updatePortfolio(const Contract& contract, int position,
		double marketPrice, double marketValue, double averageCost,
		double unrealizedPNL, double realizedPNL, const IBString& accountName);
	void updateAccountTime(const IBString& timeStamp);
	void accountDownloadEnd(const IBString& accountName);
	void nextValidId(OrderId orderId);
	void contractDetails(int reqId, const ContractDetails& contractDetails);
	void bondContractDetails(int reqId, const ContractDetails& contractDetails);
	void contractDetailsEnd(int reqId);
	void execDetails(int reqId, const Contract& contract, const Execution& execution);
	void execDetailsEnd(int reqId);
	void error(const int id, const int errorCode, const IBString errorString);
	void updateMktDepth(TickerId id, int position, int operation, int side,
		double price, int size);
	void updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
		int side, double price, int size);
	void updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch);
	void managedAccounts(const IBString& accountsList);
	void receiveFA(faDataType pFaDataType, const IBString& cxml);
	void historicalData(TickerId reqId, const IBString& date, double open, double high,
		double low, double close, int volume, int barCount, double WAP, int hasGaps);
	void scannerParameters(const IBString &xml);
	void scannerData(int reqId, int rank, const ContractDetails &contractDetails,
		const IBString &distance, const IBString &benchmark, const IBString &projection,
		const IBString &legsStr);
	void scannerDataEnd(int reqId);
	void realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
		long volume, double wap, int count);
	void currentTime(long time);
	void fundamentalData(TickerId reqId, const IBString& data);
	void deltaNeutralValidation(int reqId, const UnderComp& underComp);
	void tickSnapshotEnd(int reqId);

private:

	std::auto_ptr<EPosixClientSocket> m_pClient;
	State m_state;
	time_t m_sleepDeadline;

	OrderId m_orderId;
	
/*******Appended Data Members.*******/
	int m_clientId;
	int m_portfolioCount;	//'total' no. of portfolios, a user-defined parameter.
	Contract* m_portfolio;	//an array of 'contracts' in a portfolio, a user-input.
	int m_portfolioSize;	//'size' of each portfolio, a user-defined parameter.
	double* m_portfolioPrice;	//an array of 'spot price' of each contract in a portfolio.
	int* m_portfolioPosition;	//an array of 'desired positions'for each contract in a portfolio.

	int m_capitalPerStock;
	
	int m_methodCallCount;	//keeps count of a call to the method.

	int m_sessionDuration;	//duration of a single trade-session(in seconds), a user-defined parameter.
	int m_portfolioOrderPacketCount;	//no. of times a set of 'BUY'/'SELL' Orders are placed for the portfolio in a single trade-session.
	int m_sleepBetweenPings;	//time-interval between subsequent pings.
	int* m_contractLotSize;	//an array containing lot-size for each contract in the portfolio.
	int m_endSessionDuration;	//duration of a session in which Market-Orders are placed.
	float m_priceTolerance;	//price tolerance for Limit-Orders.
	int m_remainingSessionDuration;	//total time remaining for a single trade-session to close. 
	int m_attempt; //no. of attempts made to connect to 'IB-Server'.

//Algorithm-Parameters
//--------------------------------------------------------------------
	bool m_algoFlag;	// flag to predict the 'portfolioPosition' vector..	
	int m_algoDataPointCount;	// 'Data-Sample Size' to predict the 'portfolioPosition' vector..  
	NeuDatabase* *  m_algoHLCData;	// Two-Dimensional Matrix of size: '(m_portfolioSize)*(m_portfolioCount)' X 'm_algoDataPointCount'.
	int m_algoPercentile;	// n-percentile.
	int m_algoPercentileWindowSize;	// window-size for 'percentile' computation.
	int m_algoVolWindowSize; // window-size for 'volatility' computation.
	double m_algoConstantRho;	// constant 'rho' value in 'IF' condition.
	int m_algoPriceRangeWindowSize;	// window-size for 'rollAvgPriceRange' computation.
	int m_algoMktVarianceDays;	// no. of days to determine 'Market-Variation'.
////	double m_algoRiskExposure;	// 'risk-exposure' for 'portfolioPosition' computation.
	long int * m_algoRiskExposure;	// 'risk-exposure' vector for 'portfolioPosition' computation.
	double * m_weightedAvgPrice;	//  'Weighted-Average-Price' vector of the 'Executed-Trades' of size=(portfolioSize*portfolioCount). 
////	bool m_proTradeFlag;	//	flag is set to 'TRUE' when a trade session is finished.
	char * m_dbUpdateTime;	// time of the day to update Database File: 'highLowClosePortfolio.dat'	
};

#endif
