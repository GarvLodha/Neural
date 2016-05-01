#include "PosixTestClient.h"

#include "EPosixClientSocket.h"
#include "EPosixClientSocketPlatform.h"

#include "Contract.h"
#include "Order.h"
#include <stdio.h>	//appended statement
#include <fstream>	//appended statement
#include <iostream>	//appended statement
#include <string.h>	//appended statement
#include <algorithm>	//function: 'sort'

#include "Execution.h"	//appended statement
#include "math.h"	//appended statement; functions: 'sqrt', 'fabs', 'pow'

#include <sys/types.h>	//function: 'getpwuid'
#include <pwd.h>	//struct: 'passwd'

using namespace std;

const int PING_DEADLINE = 10; // seconds
///const int PING_DEADLINE = 20; // seconds
const int SLEEP_BETWEEN_PINGS = 30; // seconds


///////////////////////////////////////////////////////////
// member funcs
PosixTestClient::PosixTestClient()
	: m_pClient(new EPosixClientSocket(this))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
{
}

PosixTestClient::PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int* portfolioPosition, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt)
	: m_pClient(new EPosixClientSocket(this))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
	, m_clientId(clientId)
	, m_portfolioCount(portfolioCount)
	, m_portfolio(portfolio)
	, m_portfolioSize(portfolioSize)
	, m_portfolioPosition(portfolioPosition)
	, m_methodCallCount(0)
	, m_sessionDuration(sessionDuration)
	, m_portfolioOrderPacketCount(0)
	, m_sleepBetweenPings(0)
	, m_contractLotSize(contractLotSize)
	, m_endSessionDuration(endSessionDuration)
	, m_priceTolerance(priceTolerance)
	, m_attempt(attempt)
	, m_algoFlag(true)
{
	m_portfolioPrice = new double[m_portfolioSize*m_portfolioCount];
	for(int index = 0; index < (m_portfolioSize*m_portfolioCount); index++){
		m_portfolioPrice[index] = 0;
		cout << "m_portfolioPrice[" << index << "]:" << m_portfolioPrice[index] << endl;
	} 
}

//PosixTestClient::PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt, int algoDataPointCount, int algoPercentile, int algoPercentileWindowSize, int algoVolWindowSize, double algoConstantRho, int algoPriceRangeWindowSize, int algoMktVarianceDays, double algoRiskExposure)
//PosixTestClient::PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt, int algoDataPointCount, int algoPercentile, int algoPercentileWindowSize, int algoVolWindowSize, double algoConstantRho, int algoPriceRangeWindowSize, int algoMktVarianceDays, long int * algoRiskExposure)
PosixTestClient::PosixTestClient(int clientId, int portfolioSize, int portfolioCount, Contract* portfolio, int sessionDuration, int* contractLotSize, int endSessionDuration, float priceTolerance, int attempt, int algoDataPointCount, int algoPercentile, int algoPercentileWindowSize, int algoVolWindowSize, double algoConstantRho, int algoPriceRangeWindowSize, int algoMktVarianceDays, long int * algoRiskExposure, char * dbUpdateTime)
	: m_pClient(new EPosixClientSocket(this))
	, m_state(ST_CONNECT)
	, m_sleepDeadline(0)
	, m_orderId(0)
	, m_clientId(clientId)
	, m_portfolioCount(portfolioCount)
	, m_portfolio(portfolio)
	, m_portfolioSize(portfolioSize)
	, m_methodCallCount(0)
	, m_sessionDuration(sessionDuration)
	, m_portfolioOrderPacketCount(0)
	, m_sleepBetweenPings(0)
	, m_contractLotSize(contractLotSize)
	, m_endSessionDuration(endSessionDuration)
	, m_priceTolerance(priceTolerance)
	, m_attempt(attempt)
	, m_algoFlag(true)
//	, m_algoFlag(false)
	, m_algoDataPointCount(algoDataPointCount)
	, m_algoPercentile(algoPercentile)
	, m_algoPercentileWindowSize(algoPercentileWindowSize)
	, m_algoVolWindowSize(algoVolWindowSize)
	, m_algoConstantRho(algoConstantRho)
	, m_algoPriceRangeWindowSize(algoPriceRangeWindowSize)
	, m_algoMktVarianceDays(algoMktVarianceDays)
	, m_algoRiskExposure(algoRiskExposure)
	, m_dbUpdateTime(dbUpdateTime)
{
	m_portfolioPrice = new double[m_portfolioSize*m_portfolioCount];
	m_portfolioPosition = new int[m_portfolioSize*m_portfolioCount];

//Assign Memory to 2-Dimensional Matrix 'm_algoHLCData'.
//--------------------------------------------------------------------
	m_algoHLCData = new NeuDatabase*[m_portfolioSize*m_portfolioCount];
	for(int i = 0; i < (m_portfolioSize*m_portfolioCount); i++){
		m_algoHLCData[i] = new NeuDatabase[m_algoDataPointCount];
	}

	m_weightedAvgPrice = new double[m_portfolioSize*m_portfolioCount];	
}


PosixTestClient::~PosixTestClient()
{
}

bool PosixTestClient::connect(const char *host, unsigned int port, int clientId)
{
	// trying to connect
	printf( "Connecting to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);

	bool bRes = m_pClient->eConnect( host, port, clientId);

	if (bRes) {
		printf( "Connected to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);
	}
	else
		printf( "Cannot connect to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);

	return bRes;
}

void PosixTestClient::disconnect() const
{
/*	cout << "In Method: 'PosixTestClient::disconnect()' ..." << endl;
	cout << "*********************************************" << endl;*/
//------------------------------------------------------------------>
//
	m_pClient->eDisconnect();

	printf ( "Disconnected\n");
//<------------------------------------------------------------------>
//
/*	cout << "Method: 'PosixTestClient::disconnect()' ends." << endl;
	cout << "x-------------------------------------------x" << endl;*/
}

bool PosixTestClient::isConnected() const
{
	return m_pClient->isConnected();
}

void PosixTestClient::processMessages()
{
	fd_set readSet, writeSet, errorSet;

	struct timeval tval;
	tval.tv_usec = 0;
	tval.tv_sec = 0;

	time_t now = time(NULL);

	switch (m_state) {
		case ST_REQHISTORICALDATA:
			cout << "In 'switch(ST_REQHISTORICALDATA)' ..." << endl;
			cout << "Calling Method: 'PosixTestClient:ReqHistoricalData()'." << endl;
			ReqHistoricalData();
			break;
		case ST_REQEXECUTIONS:
			cout << "In 'switch(ST_REQEXECUTIONS)' ..." << endl;
			cout << "Calling Method: 'PosixTestClient:ReqExecutions()'." << endl;
			ReqExecutions();
			break;
		case ST_REQMKTDATA:
			cout << "In 'switch(ST_REQMKTDATA)' ..." << endl;
			cout << "Calling Method: 'PosixTestClient:ReqMktData()'." << endl;
			ReqMktData();
			break;
		case ST_PLACEORDER:
			cout << "In 'switch(ST_PLACEORDER)' ..." << endl;
			cout << "Calling Method: 'PosixTestClient:placeOrder()'." << endl;
			placeOrder();
			break;
		case ST_REQCURRENTTIME:
			cout << "In 'switch(ST_REQCURRENTTIME)' ..." << endl;
			cout << "Calling Method: 'PosixTestClient:reqCurrentTime()'." << endl;
			reqCurrentTime();
			break;
		case ST_PLACEORDER_ACK:
			cout << "In 'switch(ST_PLACEORDER_ACK)' ..." << endl;
			{
				for(int reqId = 0; reqId < (m_portfolioSize*m_portfolioCount); reqId++){// Cancel 'Market-Data'.
					cout << "Cancelling the  Market Data for Contract: " << m_portfolio[reqId].localSymbol << endl;
					m_pClient -> cancelMktData(reqId);	//Cancel 'Market-Data' for the contract.
				}
				
				m_state = ST_REQCURRENTTIME;
			}
			break;
		case ST_CANCELORDER:
			cancelOrder();
			break;
		case ST_CANCELORDER_ACK:
			break;
		case ST_PING:
//			reqCurrentTime();
			cout << "In 'switch(ST_PING)' ..." << endl;
			if(m_algoFlag){
//				cout << "Calling Method: 'PosixTestClient:ReqMktData()'." << endl;
				cout << "Calling Method: 'PosixTestClient:placeOrder()'." << endl;
//				ReqMktData();
				placeOrder();
			}
			else{
				cout << "Calling Method: 'PosixTestClient::reqCurrentTime()'" << endl;
				reqCurrentTime();
			}
			break;
		case ST_PING_ACK:
////			cout << "In 'switch(ST_PING_ACK)' ..." << endl;
////			cout << "Value of 'm_sleepDeadline' is: " << m_sleepDeadline << endl;
			if(m_sleepDeadline < now) {
				cout << "Condition 'm_sleepDeadline < now' is evaluated as 'true'." << endl;
				if(m_algoFlag){
					m_remainingSessionDuration -= (PING_DEADLINE + 10);
////					cout << "Value of 'm_sleepDeadline' is: " << m_sleepDeadline << endl;
				}
				disconnect();
				return;
			}
			break;
		case ST_IDLE:
			{
//				cout << "In 'switch(ST_IDLE)' ..." << endl;
				time_t now = ::time(NULL);
				tm * nowPtr = localtime(&now);
				
				int tmHour = atoi(strtok(m_dbUpdateTime, ":"));
				cout << "tmHour:" << tmHour << endl;
				int tmMin  = atoi(strtok(NULL, ":"));
				cout << "tmMin:" << tmMin << endl;

//				if((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 45))
//				if((nowPtr->tm_hour == 9) && (nowPtr->tm_min == 45))
				if((nowPtr->tm_hour == tmHour) && (nowPtr->tm_min == tmMin))
				{
//					cout << "Condition: '((nowPtr->tm_hour == 15) && (nowPtr->tm_min == 45))' evaluated as 'TRUE'" << endl;
					cout << "Condition: '((nowPtr->tm_hour == " << tmHour <<" && (nowPtr->tm_min == " << tmMin << "))' evaluated as 'TRUE'" << endl;
					m_state = ST_REQHISTORICALDATA;
					cout << "m_state:ST_REQHISTORICALDATA" << endl;
//					cout << "Calling Method: 'PosixTestClient:ReqHistoricalData()'." << endl;
//					ReqHistoricalData();	//Call 'functions' rather than changing state variable('m_state') to prevent socket timeout resulting into API disconnections.
				}

				else{
					if(m_sleepDeadline < now){
						cout << "Condition 'm_sleepDeadline < now' is evaluated as 'true'." << endl;
						m_state = ST_PING;
						cout << "m_state:ST_PING" << endl;
						return;

					}
				}	

/****			if(m_sleepDeadline < now) {
				cout << "Condition 'm_sleepDeadline < now' is evaluated as 'true'." << endl;
				m_state = ST_PING;
				cout << "m_state:ST_PING" << endl;
				return;
			}****/
				break;
			}
	}

	if( m_sleepDeadline > 0) {
		// initialize timeout with m_sleepDeadline - now
		tval.tv_sec = m_sleepDeadline - now;
	}

	if( m_pClient->fd() >= 0 ) {

		FD_ZERO( &readSet);
		errorSet = writeSet = readSet;

		FD_SET( m_pClient->fd(), &readSet);

		if( !m_pClient->isOutBufferEmpty())
			FD_SET( m_pClient->fd(), &writeSet);

		FD_CLR( m_pClient->fd(), &errorSet);

		int ret = select( m_pClient->fd() + 1, &readSet, &writeSet, &errorSet, &tval);

		if( ret == 0) { // timeout
			return;
		}

		if( ret < 0) {	// error
			cout << "Condition: '(ret<0)' is evaluated as 'TRUE'" << endl;
			disconnect();
			return;
		}

		if( m_pClient->fd() < 0)
			return;

		if( FD_ISSET( m_pClient->fd(), &errorSet)) {
			// error on socket
			m_pClient->onError();
		}

		if( m_pClient->fd() < 0)
			return;

		if( FD_ISSET( m_pClient->fd(), &writeSet)) {
			// socket is ready for writing
			m_pClient->onSend();
		}

		if( m_pClient->fd() < 0)
			return;

		if( FD_ISSET( m_pClient->fd(), &readSet)) {
			// socket is ready for reading
			m_pClient->onReceive();
		}
	}
}

//////////////////////////////////////////////////////////////////
// methods
void PosixTestClient::reqCurrentTime()
{
	printf( "Requesting Current Time\n");

	// set ping deadline to "now + n seconds"
	m_sleepDeadline = time( NULL) + PING_DEADLINE;

	m_state = ST_PING_ACK;
	cout << "m_state:'ST_PING_ACK'" << endl;

	m_pClient->reqCurrentTime();
}

void PosixTestClient::placeOrder(){
	cout << "In Method: 'placeOrder(int tickerId)' ..." << endl;
	cout << "*****************************************" << endl;
//------------------------------------------------------------------->
//
	if(m_attempt != 1){//Value of 'm_sessionDuration' is decreased if there is disconnection.
		cout << "Condition 'm_attempt != 1' is evaluated as 'true'." << endl;
//		m_sessionDuration = m_remainingSessionDuration;
		m_sessionDuration -= (m_attempt*(PING_DEADLINE + 10));
		cout << "m_sessionDuration:" << m_sessionDuration << endl;
	}

	m_sessionDuration = m_sessionDuration - m_sleepBetweenPings;
	cout << "m_sessionDuration:" << m_sessionDuration << endl;

	m_remainingSessionDuration = m_sessionDuration;
	cout << "m_remainingSessionDuration:" << m_remainingSessionDuration << endl; 
	m_sleepBetweenPings = m_remainingSessionDuration/MaxExposure();
	cout << "m_sleepBetweenPings:" <<  m_sleepBetweenPings << endl;
	cout << "Next 'Ping' will be after: " << m_sleepBetweenPings << " seconds." << endl;

	int instrumentCount = m_portfolioSize*m_portfolioCount;
	bool startMarketOrders = false;

//Place Order-Packet.
//
	for(int stockId = 0; stockId < instrumentCount; stockId++){
		Contract contract = m_portfolio[stockId];
		Order order;

		cout << "m_remainingSessionDuration:" << m_remainingSessionDuration << endl;
		cout << "m_sleepBetweenPings:" << m_sleepBetweenPings << endl;

// Prevent placing limit-orders if:
// 1:'remainingSessionDuration' < 60 seconds OR 
// 2:'remainingSessionDuration' > 60 seconds but 
// 'remainingSessionDuration' = 'm_sleepBetweenPings',
// a scenario when 'MaxExposure()'= 1.
//--------------------------------------------------------------------

//		if(remainingSessionDuration > 60)
//		if((remainingSessionDuration > 60) && (remainingSessionDuration > m_sleepBetweenPings))
		if((m_remainingSessionDuration > m_endSessionDuration) && (m_remainingSessionDuration > m_sleepBetweenPings)){
//			order.totalQuantity = 1;
			order.totalQuantity = 1*m_contractLotSize[stockId];
			order.orderType = "LMT";

//			float epsilon = 0.05;	//price tolerance.
			float epsilon = m_priceTolerance;	//price tolerance.
			if(m_portfolioPosition[stockId]<0){
				order.action = "SELL";

//Calculate 'order.lmtPrice' in the multiples of '0.05'(least tick-value).
//--------------------------------------------------------------------
				double limitPrice = (m_portfolioPrice[stockId] + ((epsilon*m_portfolioPrice[stockId])/100));
				cout << "limitPrice:" << limitPrice << endl;

				int lmtPrice = (limitPrice*100);
				cout << "lmtPrice:" << lmtPrice << endl;

				cout << lmtPrice % 5 << endl;

//				order.lmtPrice = m_portfolioPrice[tickerId] + epsilon;
//				order.lmtPrice = m_portfolioPrice[tickerId] + ((epsilon*m_portfolioPrice[tickerId])/100);
				order.lmtPrice = (lmtPrice - (lmtPrice % 5))/100.0;
				cout << "order.lmtPrice:" << order.lmtPrice << endl;
			}

			else{
				order.action = "BUY";

//Calculate 'order.lmtPrice' in the multiples of '0.05'(least tick-value).
//--------------------------------------------------------------------
				double limitPrice = (m_portfolioPrice[stockId] - ((epsilon*m_portfolioPrice[stockId])/100));
				cout << "limitPrice:" << limitPrice << endl;

				int lmtPrice = (limitPrice*100);
				cout << "lmtPrice:" << lmtPrice << endl;

				cout << lmtPrice % 5 << endl;

//				order.lmtPrice = m_portfolioPrice[tickerId] - epsilon;
//				order.lmtPrice = m_portfolioPrice[tickerId] - ((epsilon*m_portfolioPrice[tickerId])/100);
				order.lmtPrice = (lmtPrice - (lmtPrice % 5))/100.0;
				cout << "order.lmtPrice:" << order.lmtPrice << endl;
			}
		}

		else{
			startMarketOrders = true;
//			order.totalQuantity = m_portfolioPosition[stockId]*m_contractLotSize[stockId];
			order.totalQuantity = (abs(m_portfolioPosition[stockId])*m_contractLotSize[stockId]);
			order.orderType = "MKT";

			if(m_portfolioPosition[stockId]<0){
				order.action = "SELL";
			}

			else{
				order.action = "BUY";
			}
		}
					
		printf("Placing Order %ld: %s %ld %s at %f\n", m_orderId, order.action.c_str(), order.totalQuantity, contract.symbol.c_str(), order.lmtPrice);
////		m_state = ST_PLACEORDER_ACK;
		if(order.totalQuantity > 0){
			m_pClient->placeOrder(m_orderId, contract, order);
		}

		m_orderId++;
		cout << "m_orderId:" << m_orderId << endl;

	}
		time_t now = ::time(NULL);

//		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
		m_sleepDeadline = now + m_sleepBetweenPings;
		cout << "m_sleepDeadline:" << m_sleepDeadline << endl;
		
		if(startMarketOrders){
// set ping deadline to "now + n seconds"
////			m_sleepDeadline = time( NULL) + PING_DEADLINE;
			m_algoFlag = false;
			cout << "m_algoFlag:" << m_algoFlag << endl;

			m_state = ST_PLACEORDER_ACK;
			cout << "m_state:ST_PLACEORDER_ACK" << endl;
		}
		
		else{
			m_state = ST_IDLE;
			cout << "m_state:ST_IDLE" << endl;
		}
}

void PosixTestClient::placeOrder(int tickerId){
	cout << "In Method: 'placeOrder(int tickerId)' ..." << endl;
	cout << "*****************************************" << endl;
//------------------------------------------------------------------->
//
	Contract contract = m_portfolio[tickerId];
	Order order;

//	int remainingSessionDuration = (m_sessionDuration - (m_portfolioOrderPacketCount*m_sleepBetweenPings));
	int remainingSessionDuration = m_sessionDuration;
	cout << "remainingSessionDuration:" << remainingSessionDuration << endl;

	cout << "m_sleepBetweenPings:" << m_sleepBetweenPings << endl;

// Prevent placing limit-orders if:
// 1:'remainingSessionDuration' < 60 seconds OR 
// 2:'remainingSessionDuration' > 60 seconds but 
// 'remainingSessionDuration' = 'm_sleepBetweenPings',
// a scenario when 'MaxExposure()'= 1.
//--------------------------------------------------------------------

//	if(remainingSessionDuration > 60){
//	if((remainingSessionDuration > 60) && (remainingSessionDuration > m_sleepBetweenPings)){
	if((remainingSessionDuration > m_endSessionDuration) && (remainingSessionDuration > m_sleepBetweenPings)){
//		order.totalQuantity = 1;
		order.totalQuantity = 1*m_contractLotSize[tickerId];
		order.orderType = "LMT";

//		float epsilon = 0.05;	//price tolerance.
		float epsilon = m_priceTolerance;	//price tolerance.
		if(m_portfolioPosition[tickerId]<0){
			order.action = "SELL";

//Calculate 'order.lmtPrice' in the multiples of '0.05'(least tick-value).
//--------------------------------------------------------------------
			double limitPrice = (m_portfolioPrice[tickerId] + ((epsilon*m_portfolioPrice[tickerId])/100));
			cout << "limitPrice:" << limitPrice << endl;

			int lmtPrice = (limitPrice*100);
			cout << "lmtPrice:" << lmtPrice << endl;

			cout << lmtPrice % 5 << endl;

//			order.lmtPrice = m_portfolioPrice[tickerId] + epsilon;
//			order.lmtPrice = m_portfolioPrice[tickerId] + ((epsilon*m_portfolioPrice[tickerId])/100);
			order.lmtPrice = (lmtPrice - (lmtPrice % 5))/100.0;
			cout << "order.lmtPrice:" << order.lmtPrice << endl;
		}

		else{
			order.action = "BUY";

//Calculate 'order.lmtPrice' in the multiples of '0.05'(least tick-value).
//--------------------------------------------------------------------
			double limitPrice = (m_portfolioPrice[tickerId] - ((epsilon*m_portfolioPrice[tickerId])/100));
			cout << "limitPrice:" << limitPrice << endl;

			int lmtPrice = (limitPrice*100);
			cout << "lmtPrice:" << lmtPrice << endl;

			cout << lmtPrice % 5 << endl;

//			order.lmtPrice = m_portfolioPrice[tickerId] - epsilon;
//			order.lmtPrice = m_portfolioPrice[tickerId] - ((epsilon*m_portfolioPrice[tickerId])/100);
			order.lmtPrice = (lmtPrice - (lmtPrice % 5))/100.0;
			cout << "order.lmtPrice:" << order.lmtPrice << endl;
		}
	}

	else{
		order.totalQuantity = m_portfolioPosition[tickerId]*m_contractLotSize[tickerId];
		order.orderType = "MKT";

		if(m_portfolioPosition[tickerId]<0){
			order.action = "SELL";
		}

		else{
			order.action = "BUY";
		}
	}
					
	printf("Placing Order %ld: %s %ld %s at %f\n", m_orderId, order.action.c_str(), order.totalQuantity, contract.symbol.c_str(), order.lmtPrice);
////	m_state = ST_PLACEORDER_ACK;
	m_pClient->placeOrder(m_orderId, contract, order);

	m_orderId++;
	cout << "m_orderId:" << m_orderId << endl;
//<------------------------------------------------------------------>
//
	cout << "Method: 'placeOrder(int tickerId)' ends." << endl;
	cout << "X---------------------------------------X" << endl;
}

void PosixTestClient::cancelOrder()
{
	printf( "Cancelling Order %ld\n", m_orderId);

	m_state = ST_CANCELORDER_ACK;

	m_pClient->cancelOrder( m_orderId);
}

///////////////////////////////////////////////////////////////////
// events
void PosixTestClient::orderStatus( OrderId orderId, const IBString &status, int filled,
	   int remaining, double avgFillPrice, int permId, int parentId,
	   double lastFillPrice, int clientId, const IBString& whyHeld)

{
	if( orderId == m_orderId) {
		if( m_state == ST_PLACEORDER_ACK && (status == "PreSubmitted" || status == "Submitted"))
			m_state = ST_CANCELORDER;

		if( m_state == ST_CANCELORDER_ACK && status == "Cancelled")
			m_state = ST_PING;

		printf( "Order: id=%ld, status=%s\n", orderId, status.c_str());
	}

		printf( "Order: id=%ld, status=%s\n", orderId, status.c_str());
	
}

void PosixTestClient::nextValidId( OrderId orderId)
{
	cout << "In nextValidId(). Value of 'm_state' is: " << m_state << endl; 

	m_orderId = orderId;
	cout << "Next valid Order Id is: " << orderId << endl;

//	m_state = ST_PLACEORDER;
//	m_state = ST_REQMKTDATA;
//	m_state = ST_REQEXECUTIONS;
	m_state = ST_REQHISTORICALDATA;
	cout << "Value of 'm_state' is: " << m_state << endl;
}

void PosixTestClient::currentTime( long time)
{
	cout << "In Method: 'PosixTestClient::currentTime(long time)' ..." << endl;
	cout << "********************************************************" << endl;
//------------------------------------------------------------------->

	if ( m_state == ST_PING_ACK) {
///	if ( m_state == ST_PLACEORDER_ACK) {
		time_t t = ( time_t)time;
		struct tm * timeinfo = localtime ( &t);
		printf( "The current date/time is: %s", asctime( timeinfo));

		time_t now = ::time(NULL);
		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;

		m_state = ST_IDLE;
		cout << "m_state: 'ST_IDLE'" << endl;
	}

	cout << "Method: 'PosixTestClient::currentTime(long time)' ends." << endl;
	cout << "x-----------------------------------------------------x" << endl;
}

void PosixTestClient::error(const int id, const int errorCode, const IBString errorString)
{
	cout << "In Method: 'PosixTestClient::error(const int id, const int errorCode, const IBString errorString)' ..." << endl;
	cout << "******************************************************************************************************" << endl;
//------------------------------------------------------------------->
//
	printf( "Error id=%d, errorCode=%d, msg=%s\n", id, errorCode, errorString.c_str());

	if( id == -1 && errorCode == 1100) // if "Connectivity between IB and TWS has been lost"
		disconnect();
//<------------------------------------------------------------------>
//
	cout << "Method: 'PosixTestClient::error(const int id, const int errorCode, const IBString errorString)' ends." << endl;
	cout << "x---------------------------------------------------------------------------------------------------x" << endl;
}

void PosixTestClient::tickPrice( TickerId tickerId, TickType field, double price, int canAutoExecute){
/****	cout << "In Method: 'PosixTestClient::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute)' ..." << endl;
	cout << "****************************************************************************************************************" << endl;****/

//------------------------------------------------------------------->
//
		if(field == 4){//'last' price.
/****			cout << "m_methodCallCount:" << m_methodCallCount << endl;

			cout << "tickerId:" << tickerId << "	field:" << field << "'last' price:" << price << "	canAutoExecute:" << canAutoExecute << endl;****/
			m_portfolioPrice[tickerId] = price;
			cout << "m_portfolioPrice[" << tickerId << "]:" << m_portfolioPrice[tickerId] << endl;
	
			if(m_state == ST_PING_ACK){
				bool startPlacingOrders = true;
				for(int stockId = 0; stockId < (m_portfolioSize*m_portfolioCount); stockId++){
////					cout << "m_portfolioPrice[" << stockId << "]:" << m_portfolioPrice[stockId] << endl;
////					cout << "Evaluating Condition: 'm_portfolioPrice[" << stockId <<  "] == 0'" << endl;
					if(m_portfolioPrice[stockId] == 0){ 
////						cout << "Condition: 'm_portfolioPrice[" << stockId <<  "] == 0' is evaluated true." << endl;
						startPlacingOrders = false;
					}
				}
				if(startPlacingOrders)
				{
					m_state = ST_PLACEORDER;
					cout << "m_state:ST_PLACEORDER" << endl;
				}
			}
		}
//<------------------------------------------------------------------>
//
/****	cout << "Method: 'PosixTestClient::tickPrice(TickerId tickerId, TickType field, double price, int canAutoExecute)' ends." << endl;
	cout << "x-------------------------------------------------------------------------------------------------------------x" << endl;****/
}

void PosixTestClient::tickSize( TickerId tickerId, TickType field, int size) {}
void PosixTestClient::tickOptionComputation( TickerId tickerId, TickType tickType, double impliedVol, double delta,
											 double optPrice, double pvDividend,
											 double gamma, double vega, double theta, double undPrice) {}
void PosixTestClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {}
void PosixTestClient::tickString(TickerId tickerId, TickType tickType, const IBString& value) {}
void PosixTestClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const IBString& formattedBasisPoints,
							   double totalDividends, int holdDays, const IBString& futureExpiry, double dividendImpact, double dividendsToExpiry) {}
void PosixTestClient::openOrder( OrderId orderId, const Contract&, const Order&, const OrderState& ostate) {}
void PosixTestClient::openOrderEnd() {}
void PosixTestClient::winError( const IBString &str, int lastError) {}
void PosixTestClient::connectionClosed() {}
void PosixTestClient::updateAccountValue(const IBString& key, const IBString& val,
										  const IBString& currency, const IBString& accountName) {}
void PosixTestClient::updatePortfolio(const Contract& contract, int position,
		double marketPrice, double marketValue, double averageCost,
		double unrealizedPNL, double realizedPNL, const IBString& accountName){}
void PosixTestClient::updateAccountTime(const IBString& timeStamp) {}
void PosixTestClient::accountDownloadEnd(const IBString& accountName) {}
void PosixTestClient::contractDetails( int reqId, const ContractDetails& contractDetails) {}
void PosixTestClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {}
void PosixTestClient::contractDetailsEnd( int reqId) {}

void PosixTestClient::execDetails( int reqId, const Contract& contract, const Execution& execution){
/****	cout << "In Method: 'PosixTestClient::execDetails(int req Id, const Contract& contract, const Execution& execution)' ..." << endl;
	cout << "***************************************************************************************************************" << endl;****/
//-------------------------------------------------------------------->
//

/****	cout << "reqId: " << reqId << " contract.symbol: " << contract.symbol << " execution.execId: " << execution.execId << " execution.time: " << execution.time << " execution.acctNumber: " << execution.acctNumber << " execution.exchange: " << execution.exchange << " execution.side: " << execution.side << " execution.shares: " << execution.shares << " execution.price: " << execution.price << " execution.permId: " << execution.permId << " execution.clientId: " << execution.clientId << " execution.orderId: " << execution.orderId << " execution.liquidation: " << execution.liquidation << " execution.cumQty: " << execution.cumQty << " execution.avgPrice: " << execution.avgPrice << endl;****/

//Update the 'Portfolio-Position Vector'.
//--------------------------------------------------------------------
	if(reqId != -1){
		for(int i = 0; i < (m_portfolioSize*m_portfolioCount); i++){
			cout << "m_portfolio[i].localSymbol:" << m_portfolio[i].localSymbol << endl;
			cout << "contract.localSymbol:" << contract.localSymbol << endl;
			cout << "Value of 'm_portfolio[i].localSymbol.compare(contract.localSymbol)' is:" << m_portfolio[i].localSymbol.compare(contract.localSymbol) << endl;
			if(m_portfolio[i].localSymbol == contract.localSymbol){
///			if(m_portfolio[i].localSymbol.compare(contract.localSymbol) == 0)
				if(m_algoFlag){
					if(execution.side == "BOT"){//decrement the position.
						m_portfolioPosition[i] -=  execution.shares/m_contractLotSize[i];
						cout << "m_portfolioPosition[" << i << "]:" << m_portfolioPosition[i] << endl;
					}
					else{
						if(execution.side == "SLD"){//increment the position.
							m_portfolioPosition[i] += execution.shares/m_contractLotSize[i];
							cout << "m_portfolioPosition[" << i << "]:" << m_portfolioPosition[i] <<  endl;
						}
					}
				}
				else{//Calculate the Weighted Average Price for all Executed-Trades.
					if(!m_algoFlag){
//						m_weightedAvgPrice[reqId] += (execution.shares*execution.Price);
//						double weightedAvgPrice = execution.avgPrice;
						m_weightedAvgPrice[reqId] = execution.avgPrice;
					}
				}
				
			}
		}
	}

	else{
		for(int i = 0; i < (m_portfolioSize*m_portfolioCount); i++){
			cout << "m_portfolio[i].localSymbol:" << m_portfolio[i].localSymbol << endl;
			cout << "contract.localSymbol:" << contract.localSymbol << endl;
			cout << "Value of 'm_portfolio[i].localSymbol.compare(contract.localSymbol)' is:" << m_portfolio[i].localSymbol.compare(contract.localSymbol) << endl;
			if(m_portfolio[i].localSymbol == contract.localSymbol){
///			if(m_portfolio[i].localSymbol.compare(contract.localSymbol) == 0){
				if(execution.side == "BOT"){//decrement the position.
					m_portfolioPosition[i] +=  -1;
					cout << "m_portfolioPosition[" << i << "]:" << m_portfolioPosition[i] << endl;
				}
				else{
					if(execution.side == "SLD"){//increment the position.
						m_portfolioPosition[i] += +1;
						cout << "m_portfolioPosition[" << i << "]:" << m_portfolioPosition[i] <<  endl;
					}
				}
			}
		}
/*		m_sleepBetweenPings = (m_sessionDuration - (m_portfolioOrderPacketCount*m_sleepBetweenPings))/MaxExposure();
		cout << "Value of 'm_sleepBetweenPings is: " <<  m_sleepBetweenPings << endl;
		cout << "Next 'Ping' will be after: " << m_sleepBetweenPings << " seconds." << endl;*/
	}

//<------------------------------------------------------------------>
//
/****	cout << "***Method: 'PosixTestClient::execDetails(int req Id, const Contract& contract, const Execution& execution)' ends.***" << endl;
	cout << "X----------------------------------------------------------------------------------------------------------------X" << endl;****/
}

void PosixTestClient::execDetailsEnd( int reqId){
/****	cout << "In Method: 'PosixTestClient::execDetailsEnd(int reqId)' ..." << endl;
	cout << "***********************************************************" << endl;****/
//------------------------------------------------------------------->

	if(m_algoFlag){
		m_methodCallCount += 1;
//		if(reqId == (m_portfolioSize*m_portfolioCount) - 1){
		if(m_methodCallCount == (m_portfolioSize*m_portfolioCount)){
			m_state = ST_REQMKTDATA;
			cout << "m_state:" << m_state << endl;
		}
	}
	else{
		if(!m_algoFlag){
//Find User's Home Directory.
//---------------------------------------------------------------------
		struct passwd * user;
		user = getpwuid(getuid());

		char * tradeFileName = new char[64];
		strcpy(tradeFileName, user->pw_dir);
		cout << "User's Home Direcory is: " << tradeFileName << endl;
		strcat(tradeFileName, "/NeuralAlgo/trades.log");

			fstream tradeFile;
//			tradeFile.open("trades.log", ios::out | ios::app);
			tradeFile.open(tradeFileName, ios::out | ios::app);
			if(!tradeFile.is_open()){
				cout << "File: "<< tradeFileName << "failed to open" << endl;
				exit(1);
			}
			if((reqId!=((m_portfolioSize*m_portfolioCount) - 1))){
				tradeFile << m_portfolio[reqId].localSymbol << ":" << m_weightedAvgPrice << "	";
			}
			else{
				tradeFile << m_portfolio[reqId].localSymbol << ":" << m_weightedAvgPrice << "\n";
			}
			tradeFile.close();
		}
		m_methodCallCount += 1;
//		if(reqId == (m_portfolioSize*m_portfolioCount) - 1){
		if(m_methodCallCount == (m_portfolioSize*m_portfolioCount)){
/****			m_state = ST_REQMKTDATA;
			cout << "m_state:" << m_state << endl;****/
			cout << "Trading Session for the day ends. Thank You." << endl;
			exit(0);
		}
	}

//<------------------------------------------------------------------>
//
/****	cout << "***Method: 'PosixTestClient::execDetailEnd(int reqId)' ends.***" << endl;
	cout << "X-------------------------------------------------------------X" << endl;****/
}


void PosixTestClient::updateMktDepth(TickerId id, int position, int operation, int side,
									  double price, int size) {}
void PosixTestClient::updateMktDepthL2(TickerId id, int position, IBString marketMaker, int operation,
										int side, double price, int size) {}
void PosixTestClient::updateNewsBulletin(int msgId, int msgType, const IBString& newsMessage, const IBString& originExch) {}
void PosixTestClient::managedAccounts( const IBString& accountsList) {}
void PosixTestClient::receiveFA(faDataType pFaDataType, const IBString& cxml) {}
void PosixTestClient::historicalData(TickerId reqId, const IBString& date, double open, double high,
									  double low, double close, int volume, int barCount, double WAP, int hasGaps){
	cout << "In Method: 'PosixTestClient::historicalData(10)' ..." << endl;
	cout << "****************************************************" << endl;
//------------------------------------------------------------------->
//
////	static int dataElement = 0;	//Switch On, when filling 'm_algoHLCData' from 'IB-Server'.
	if(open != -1){

		cout << "reqId:" << reqId << "	date:" << date << "	open:" << open << "	high:" << high << "	low:" << low << "	close:" << close << "	volume:" << volume << "	barCount:" << barCount << "	WAP:" << WAP << "	hasGaps:" << hasGaps <<  endl;

		m_algoHLCData[reqId][m_algoDataPointCount - 1].fHighPrice = high; 
		cout << "m_algoHLCData[" << reqId << "][" << m_algoDataPointCount - 1 << "].fHighPrice:" << m_algoHLCData[reqId][m_algoDataPointCount - 1].fHighPrice << endl;
		m_algoHLCData[reqId][m_algoDataPointCount - 1].fLowPrice = low;
		cout << "m_algoHLCData[" << reqId << "][" << m_algoDataPointCount - 1 << "].fLowPrice:" << m_algoHLCData[reqId][m_algoDataPointCount - 1].fLowPrice << endl;
		m_algoHLCData[reqId][m_algoDataPointCount - 1].fClosePrice = close;
		cout << "m_algoHLCData[" << reqId << "][" << m_algoDataPointCount - 1 << "].fClosePrice:" << m_algoHLCData[reqId][m_algoDataPointCount - 1].fClosePrice << endl;

//Fill the 'm_algoHLCData' from 'IB-Server'.
//--------------------------------------------------------------------
/****		cout << "Data Point: " << dataElement << endl; 
		m_algoHLCData[reqId][dataElement].fHighPrice = high; 
		cout << "m_algoHLCData[" << reqId << "][" << dataElement<< "].fHighPrice:" << m_algoHLCData[reqId][dataElement].fHighPrice << endl;
		m_algoHLCData[reqId][dataElement].fLowPrice = low;
		cout << "m_algoHLCData[" << reqId << "][" << dataElement<< "].fLowPrice:" << m_algoHLCData[reqId][dataElement].fLowPrice << endl;
		m_algoHLCData[reqId][dataElement].fClosePrice = close;
		cout << "m_algoHLCData[" << reqId << "][" << dataElement<< "].fClosePrice:" << m_algoHLCData[reqId][dataElement].fClosePrice << endl;
		
		dataElement += 1;
		cout << "dataElement:" << dataElement << endl;****/
	}

	else{
		cout << "All above data was retreived for instrument: " << m_portfolio[reqId].localSymbol << "by process: " << getpid() << endl;

		m_methodCallCount += 1;
		cout << "m_methodCallCount:" << m_methodCallCount << endl;

//Find User's Home Directory.
//---------------------------------------------------------------------
		struct passwd * user;
		user = getpwuid(getuid());

		char * dataFileName = new char[64];
		strcpy(dataFileName, user->pw_dir);
		cout << "User's Home Direcory is: " << dataFileName << endl;
		strcat(dataFileName, "/NeuralAlgo/highLowClosePortfolio.dat");


		if(((m_state == ST_PING_ACK) && (m_methodCallCount == (m_portfolioSize*m_portfolioCount)))){
			if(m_algoFlag){
//Fill the Matrix.
//--------------------------------------------------------------------

				fstream hLCDataFile;
			//	hLCDataFile.open("highLowCloseNTPC.csv", ios::in);
//				hLCDataFile.open("highLowClosePortfolio.dat", ios::in);
//				hLCDataFile.open("highLowClosePortfolio.dat", ios::in);
				hLCDataFile.open(dataFileName, ios::in);
			//	hLCDataFile.open("highLowClosePortfolio.format", ios::in);
			//	hLCDataFile.open("highLowClosePortfolio.format.dat", ios::in);
			
				int lineLength = 512;
			//	char* line = new char[256];
				char* line = new char[lineLength];
			
				if(hLCDataFile.is_open())
				{
			////		for(int stockId = 0; stockId < m_portfolioSize; stockId++)
			//		for(int dataPoint = 0; dataPoint < m_algoDataPointCount; dataPoint++)
					for(int dataPoint = 0; dataPoint < m_algoDataPointCount - 1; dataPoint++){
						cout << "Value of 'dataPoint' is:" << dataPoint << endl;
			
			//			hLCDataFile.getline(line, 256);
						hLCDataFile.getline(line, lineLength);
						cout << "Value of 'line' is: " << line << endl;
			//			char* token = strtok(line, ",");
						char* token = strtok(line, ",\t");
						cout << "Value of 'token' is: " << token << endl;
			
			//			for(int stockId = 0; stockId < m_portfolioSize; stockId++){
						for(int stockId = 0; stockId < (m_portfolioSize*m_portfolioCount); stockId++){
							for(int fieldId = 0; fieldId < 3; fieldId++){
								switch(fieldId){
									case 0:
			//							hLCData[dataPoint].fHighPrice = atof(token);
										m_algoHLCData[stockId][dataPoint].fHighPrice = atof(token);
			///							m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fHighPrice = atof(token);
			//							cout << "Value of 'm_algoHLCData.fHighPrice' is: " << m_algoHLCData[dataPoint].fHighPrice << endl;
			//							cout << "Value of 'm_algoHLCData.fHighPrice' is: " << m_algoHLCData[stockId][dataPoint].fHighPrice << endl;
										cout << "m_algoHLCData[" << stockId << "][" << dataPoint << "].fHighPrice' is: " << m_algoHLCData[stockId][dataPoint].fHighPrice << endl;
			///							cout << "m_algoHLCData[" << stockId + (algoCallCount*m_portfolioSize)<< "][" << dataPoint << "].fHighPrice' is: " << m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fHighPrice << endl;
			//							token = strtok(NULL, ",");
										token = strtok(NULL, ",\t");
										cout << "Value of 'token' is: " << token << endl;
										break;
									case 1:
			//							m_algoHLCData[dataPoint].fLowPrice = atof(token);
										m_algoHLCData[stockId][dataPoint].fLowPrice = atof(token);
			///							m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fLowPrice = atof(token);
			//							cout << "Value of 'm_algoHLCData.fLowPrice' is: " << m_algoHLCData[dataPoint].fLowPrice << endl;
			//							cout << "Value of 'm_algoHLCData.fLowPrice' is: " << m_algoHLCData[stockId][dataPoint].fLowPrice << endl;
										cout << "m_algoHLCData[" << stockId << "][" << dataPoint << "].fLowPrice' is: " << m_algoHLCData[stockId][dataPoint].fLowPrice << endl;
			///							cout << "m_algoHLCData[" << stockId + (algoCallCount*m_portfolioSize) << "][" << dataPoint << "].fLowPrice' is: " << m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fLowPrice << endl;
			//							token = strtok(NULL, ",");
										token = strtok(NULL, ",\t");
										cout << "Value of 'token' is: " << token << endl;
										break;
									case 2:
			//							m_algoHLCData[dataPoint].fClosePrice = atof(token);
										m_algoHLCData[stockId][dataPoint].fClosePrice = atof(token);
			///							m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fClosePrice = atof(token);
			//							cout << "Value of 'm_algoHLCData.fClosePrice' is: " << m_algoHLCData[dataPoint].fClosePrice << endl;
			//							cout << "Value of 'm_algoHLCData.fClosePrice' is: " << m_algoHLCData[stockId][dataPoint].fClosePrice << endl;
										cout << "m_algoHLCData[" << stockId << "][" << dataPoint << "].fClosePrice' is: " << m_algoHLCData[stockId][dataPoint].fClosePrice << endl;
			///							cout << "m_algoHLCData[" << stockId + (algoCallCount*m_portfolioSize) << "][" << dataPoint << "].fClosePrice' is: " << m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fClosePrice << endl;
			//							token = strtok(NULL, ",");
										token = strtok(NULL, ",\t");
			//							cout << "Value of 'token' is: " << token << endl;
										break;
								}
							}
						}
					}
					////}
				}
			//x------------------------------------------------------------------x
			
						for(int algoCallCount = 0; algoCallCount < m_portfolioCount; algoCallCount++){
						cout <<"Calling 'Method': PosixTestClient::Algorithm(" << algoCallCount << ")" << endl;
						PosixTestClient::Algorithm(algoCallCount);	//Generate the 'portfolioPosition'.
						}
/****							m_algoFlag = false;
							cout << "m_algoFlag:" << m_algoFlag << endl;****/
						m_state = ST_REQEXECUTIONS;
						cout << "m_state:ST_REQEXECUTIONS" << endl;
/****						cout << "Testing Ends. Disconnecting ..." << endl;
						disconnect();****/
			}
			else{//Update the File: 'highLowClosePortfolio.dat'
				cout << "Updating the Database File: 'highLowClosePortfolio.dat' ..." << endl;
//Delete first line from the File: 'highLowClosePortfolio.dat' and update the File.
//---------------------------------------------------------------------
//				int dataPointCount = 250;
				int dataPointCount = m_algoDataPointCount-1;	//no. of lines in the File.
			
				ifstream inFile;	//Input File Handle
//				inFile.open("highLowClosePortfolio.dat");
				inFile.open(dataFileName);
				
				if(!inFile.is_open()){
					cout << "Input File failed to open." << endl;
					exit(1);
				}
			
				int lineLength = 512;
				char * line = new char[lineLength];

				char * tmpFileName = new char[64];
				strcpy(tmpFileName, user->pw_dir);
				strcat(tmpFileName, "/NeuralAlgo/hLCTmp.dat");
			
				for(int lineCount = 0; lineCount < dataPointCount; lineCount++){
					inFile.getline(line,lineLength);
					cout << "Value of 'line' is: " << line << endl;
			
					ofstream outFile;	//Output File Handle, declare in the loop to save the stream-buffer from over-fill, in case of large file.
//					outFile.open("hLCTmp.dat", ios::out | ios::app);
					outFile.open(tmpFileName, ios::out | ios::app);
			
			
					if(lineCount != 0){// lines to write, here all except first are written.
						outFile << line << endl;
					}
			
					outFile.close();
				}
		
//				remove("highLowClosePortfolio.dat");	//remove the original File
				remove(dataFileName);	//remove the original File
//				rename("hLCTmp.dat","highLowClosePortfolio.dat");	//rename the new File to Old File.
				rename(tmpFileName,dataFileName);	//rename the new File to Old File.

//Update the File: 'highLowClosePortfolio.dat'
//---------------------------------------------------------------------
				ofstream dataFile;
//				dataFile.open("highLowClosePortfolio.dat", ios::app);
				dataFile.open(dataFileName, ios::app);

				if(!dataFile.is_open()){
					cout << "Data File: 'highLowClosePortfolio.dat' failed to open for update." << endl;
					exit(1);
				}

				for(int reqId = 0; reqId < (m_portfolioSize*m_portfolioCount); reqId++){// Write to File: 'highLowClosePortfolio.dat'.
					cout << "Writing High, Low, Close Values for the Contract: " << m_portfolio[reqId].localSymbol << endl; 
					dataFile << m_algoHLCData[reqId][m_algoDataPointCount - 1].fHighPrice << ",";
					dataFile << m_algoHLCData[reqId][m_algoDataPointCount - 1].fLowPrice << ",";
					if((reqId!=((m_portfolioSize*m_portfolioCount)-1))){
						dataFile << m_algoHLCData[reqId][m_algoDataPointCount - 1].fClosePrice << ",";
					}
					else{
						dataFile << m_algoHLCData[reqId][m_algoDataPointCount - 1].fClosePrice << "\n";
					}
				}
				
				dataFile.close();

						m_state = ST_REQEXECUTIONS;
						cout << "m_state:ST_REQEXECUTIONS" << endl;
/****						cout << "Testing Ends. Disconnecting ..." << endl;
						disconnect();****/
/****						m_state = ST_PLACEORDER_ACK;
						cout << "m_state:ST_PLACEORDER_ACK" << endl;****/
/****						cout << "Database File: 'highLowClosePortfolio.dat' Updated." << endl;
						disconnect();
						exit(0);****/


			}	
		}
	}
//<------------------------------------------------------------------>
//
	cout << "Method: 'PosixTestClient::historicalData(10)' ends." << endl;
	cout << "x-------------------------------------------------x" << endl;
}

void PosixTestClient::scannerParameters(const IBString &xml) {}
void PosixTestClient::scannerData(int reqId, int rank, const ContractDetails &contractDetails,
	   const IBString &distance, const IBString &benchmark, const IBString &projection,
	   const IBString &legsStr) {}
void PosixTestClient::scannerDataEnd(int reqId) {}
void PosixTestClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
								   long volume, double wap, int count) {}
void PosixTestClient::fundamentalData(TickerId reqId, const IBString& data) {}
void PosixTestClient::deltaNeutralValidation(int reqId, const UnderComp& underComp) {}

void PosixTestClient::tickSnapshotEnd(int reqId){

/****	cout << "In Method: 'tickSnapshotEnd(int reqId)'..." << endl;
	cout << "******************************************" << endl;****/
//------------------------------------------------------------------>
//
	cout << "reqId:" << reqId << endl;
	/*if(m_algoFlag == true){//Initialize the 'portfolioPosition' vector .
		m_methodCallCount += 1;
		cout << "m_methodCallCount:" << m_methodCallCount << endl;
		if((m_state = ST_PING_ACK) && (m_methodCallCount==(m_portfolioSize*m_portfolioCount))){
		
			for(int algoCallCount = 0; algoCallCount < m_portfolioCount; algoCallCount++){
				PosixTestClient::Algorithm(algoCallCount);	//Generate the 'portfolioPosition'.
			}
			m_algoFlag = false;
			cout << "m_algoFlag:" << m_algoFlag << endl;
			
////			m_state = ST_PLACEORDER;
////			cout << "m_state:ST_PLACEORDER" << endl;
			disconnect();
		}
	}*/

/****	m_methodCallCount++;
	cout << "m_methodCallCount:" << m_methodCallCount << endl;
	
	if((m_state = ST_PING_ACK) && (m_methodCallCount==(m_portfolioSize*m_portfolioCount))){
		time_t now = ::time(NULL);

//		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
		m_sleepDeadline = now + m_sleepBetweenPings;
		cout << "Value of 'm_sleepDeadline' is: " << m_sleepDeadline << endl;
		
		m_state = ST_IDLE;
		cout << "Value of 'm_state' is: " << m_state << endl;
			
		m_portfolioOrderPacketCount += 1;	//increment the value after sending every 'Portfolio-Order'.
//		cout << "Value of 'm_portfolioOrderPacketCount' is: " << m_portfolioOrderPacketCount << endl;
		cout << "m_portfolioOrderPacketCount:" << m_portfolioOrderPacketCount << endl;
	}****/

/**	if((reqId == ((m_portfolioSize*m_portfolioCount) - 1)) && (m_state == ST_PING_ACK)){
		time_t now = ::time(NULL);

//		m_sleepDeadline = now + SLEEP_BETWEEN_PINGS;
		m_sleepDeadline = now + m_sleepBetweenPings;
		cout << "Value of 'm_sleepDeadline' is: " << m_sleepDeadline << endl;

		m_state = ST_IDLE;
		cout << "Value of 'm_state' is: " << m_state << endl;
	}**/
//<------------------------------------------------------------------>
//
/****	cout << "Method: 'tickSnapshotEnd(int reqId)' ends." << endl;
	cout << "X----------------------------------------X" << endl;****/
}


/*******Definition of 'Appended Methods'.*******/

void PosixTestClient::ReqHistoricalData(){
	cout << "In Method: 'PosixTestClient::ReqHistoricalData()' ..." << endl;
	cout << "*****************************************************" << endl;
//------------------------------------------------------------------->
//
	char buffer[80];	//Currnet time in format: "%Y%m%d %H:%M:%S".	
	time_t now = time(NULL);

//Code appended to make requests at tm_min = integral multiple of 3 
//and tm_sec = 0.
//--------------------------------------------------------------------
/*	int factor = 3;
	time_t ptime = time(NULL);	//localtime
	tm * pPtr;
	pPtr = localtime(&ptime);
	while(((pPtr->tm_sec !=0)))
	{//loose seconds first
		ptime = time(NULL);
		pPtr = localtime(&ptime);
//		cout << "Current time is:" << asctime(pPtr) << endl;
	}

	while(((pPtr->tm_min) % factor) != 0)
	{//loose minutes then.
		ptime = time(NULL);
		pPtr = localtime(&ptime);
//		cout << "Current time is:" << asctime(pPtr) << endl;
	}	*/

// Set the 'm_sleepDeadline' to wait for 'Market-Data' or 'IB 
// Server-Response' for the 'Data-Request' before making an attempt 
// to re-connect.
//--------------------------------------------------------------------
	
//	strftime(buffer, 80, "%Y%m%d %H:%M:%S", pPtr);
	strftime(buffer, 80, "%Y%m%d %H:%M:%S", localtime(&now));
	m_sleepDeadline = time( NULL) + PING_DEADLINE;
	m_state = ST_PING_ACK;
	cout << "m_state:ST_PING_ACK" << endl; 


//Request 'Historical-Data' for the portfolio.
//--------------------------------------------------------------------
//	for(int reqId = 0; reqId < m_portfolioSize; reqId++)
	for(int reqId = 0; reqId < (m_portfolioSize*m_portfolioCount); reqId++){
//		cout << "Requesting 3 min bars over last 2 hours and 3 mins for Contract: " << m_portfolio[reqId].localSymbol << " at time: " << buffer << endl;
		cout << "Requesting 1 day bar over last 1 Day for Contract: " << m_portfolio[reqId].localSymbol << " at time: " << buffer << endl;
//		m_pClient -> reqHistoricalData(reqId, m_portfolio[reqId], buffer, "7380 S", "3 mins", "Trades", 1, 1);	//request data over last 2 hours and 3 mins
		m_pClient -> reqHistoricalData(reqId, m_portfolio[reqId], buffer, "1 D", "1 day", "Trades", 1, 1);	//request data over last 2 hours and 3 mins
	}
//<------------------------------------------------------------------>
//
	cout << "Method: 'PosixTestClient::ReqHistoricalData()' ends." << endl;
	cout << "x--------------------------------------------------x" << endl;
}


void PosixTestClient::ReqMktData(){
/****	cout << "In Method: 'PosixTestClient::ReqMktData()' ..." << endl;
	cout << "*****************************************************" << endl;****/
//------------------------------------------------------------------->
//

	char buffer[80];	//Currnet time in format: "%Y%m%d %H:%M:%S".	
	time_t now = time(NULL);

// Set the 'm_sleepDeadline' to wait for 'Market-Data' or 'IB 
// Server-Response' for the 'Data-Request' before making an attempt 
// to re-connect.
//--------------------------------------------------------------------
	
	strftime(buffer, 80, "%Y%m%d %H:%M:%S", localtime(&now));
	m_sleepDeadline = time( NULL) + PING_DEADLINE;
	cout << "Time now is: " << time(NULL) << endl;
	cout << "m_sleepDeadline:" << m_sleepDeadline << endl;
	m_state = ST_PING_ACK;
	cout << "m_state:ST_PING_ACK" << endl; 

//Request 'Market-Snapshot' for the portfolio.
//--------------------------------------------------------------------
//	for(int reqId = 0; reqId < m_portfolioSize; reqId++)
	for(int reqId = 0; reqId < (m_portfolioSize*m_portfolioCount); reqId++){

		cout << "Requesting Market Snapshot for Contract: " << m_portfolio[reqId].localSymbol << " at time: " << buffer << endl;
//		m_pClient -> reqMktData(reqId, m_portfolio[reqId],"", 1);	//request 'Market-Snapshot' for a contract.
		m_pClient -> reqMktData(reqId, m_portfolio[reqId],"", 0);	//request 'Market-Data' for a contract.
	}
//<------------------------------------------------------------------>
//
/****	cout << "Method: 'PosixTestClient::ReqHistoricalData()' ends." << endl;
	cout << "x--------------------------------------------------x" << endl;****/
}

int PosixTestClient::MaxExposure(){//computes the absolute maximum of the position vector.

//Write the current portfolio vector
//------------------------------------------------------------------

//Sort the portfolio in the increasing order.
//-------------------------------------------------------------------
	int instrumentCount = m_portfolioSize*m_portfolioCount;
	unsigned int* absPosition = new unsigned int[instrumentCount];
	for(int i = 0; i < instrumentCount; i++){//computes the absolute of 'position' vector.
		absPosition[i] = abs(m_portfolioPosition[i]);
	}

	
	vector<int> myVector (absPosition, absPosition+instrumentCount);               // 32 71 12 45 26 80 53 33
	vector<int>::iterator it;

// using default comparison (operator <):
	sort(myVector.begin(), myVector.begin()+instrumentCount);           //(12 32 45 71)26 80 53 33

// print out content:
	cout << "myVector contains:";
	for(it=myVector.begin(); it!=myVector.end(); ++it){
		cout << " " << *it;
	}

	cout << endl;

	int maxExposure = myVector.back();	
	cout << "Maximum Exposure is: " << maxExposure << endl;

	return maxExposure;
}

void PosixTestClient::ReqExecutions(){

	m_methodCallCount = 0;	//Reset the 'm_methodCallCount'.
	ExecutionFilter filter;
	filter.m_clientId = m_clientId;
//	filter.m_symbol = "SBIN";
	filter.m_secType = "FUT";
	filter.m_exchange = "NSE";
	
	m_sleepDeadline = time(NULL) + PING_DEADLINE;
	m_state = ST_PING_ACK;
	cout << "Value of 'm_state' is: " << m_state << endl;
	for(int reqId = 0; reqId < (m_portfolioSize*m_portfolioCount); reqId++){
		filter.m_symbol = m_portfolio[reqId].symbol;
		cout << "Execution Details with filters: " << endl;
		cout << "filter.m_clientId:" << filter.m_clientId << endl;
		cout << "filter.m_secType:" <<  filter.m_secType << endl;
		cout << "filter.m_exchange:" << filter.m_exchange << endl;
		cout << "filter.m_time:" << filter.m_time << endl;
		cout << "filter.m_symbol:" << filter.m_symbol << endl;
		cout << "will be requested now." << endl;
		m_pClient->reqExecutions(reqId, filter);
	}
}

void PosixTestClient::Algorithm(int algoCallCount){


/****
//Fill the Matrix.
//--------------------------------------------------------------------

	fstream hLCDataFile;
//	hLCDataFile.open("highLowCloseNTPC.csv", ios::in);
	hLCDataFile.open("highLowClosePortfolio.dat", ios::in);
//	hLCDataFile.open("highLowClosePortfolio.format", ios::in);
//	hLCDataFile.open("highLowClosePortfolio.format.dat", ios::in);

	int lineLength = 512;
//	char* line = new char[256];
	char* line = new char[lineLength];

	if(hLCDataFile.is_open())
	{
////		for(int stockId = 0; stockId < m_portfolioSize; stockId++)
//		for(int dataPoint = 0; dataPoint < m_algoDataPointCount; dataPoint++)
		for(int dataPoint = 0; dataPoint < m_algoDataPointCount - 1; dataPoint++){
			cout << "Value of 'dataPoint' is:" << dataPoint << endl;

//			hLCDataFile.getline(line, 256);
			hLCDataFile.getline(line, lineLength);
			cout << "Value of 'line' is: " << line << endl;
//			char* token = strtok(line, ",");
			char* token = strtok(line, ",\t");
			cout << "Value of 'token' is: " << token << endl;

			for(int stockId = 0; stockId < m_portfolioSize; stockId++){
				for(int fieldId = 0; fieldId < 3; fieldId++){
					switch(fieldId){
						case 0:
//							hLCData[dataPoint].fHighPrice = atof(token);
//							m_algoHLCData[stockId][dataPoint].fHighPrice = atof(token);
							m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fHighPrice = atof(token);
//							cout << "Value of 'm_algoHLCData.fHighPrice' is: " << m_algoHLCData[dataPoint].fHighPrice << endl;
//							cout << "Value of 'm_algoHLCData.fHighPrice' is: " << m_algoHLCData[stockId][dataPoint].fHighPrice << endl;
//							cout << "m_algoHLCData[" << stockId << "][" << dataPoint << "].fHighPrice' is: " << m_algoHLCData[stockId][dataPoint].fHighPrice << endl;
							cout << "m_algoHLCData[" << stockId + (algoCallCount*m_portfolioSize)<< "][" << dataPoint << "].fHighPrice' is: " << m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fHighPrice << endl;
//							token = strtok(NULL, ",");
							token = strtok(NULL, ",\t");
							cout << "Value of 'token' is: " << token << endl;
							break;
						case 1:
//							m_algoHLCData[dataPoint].fLowPrice = atof(token);
//							m_algoHLCData[stockId][dataPoint].fLowPrice = atof(token);
							m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fLowPrice = atof(token);
//							cout << "Value of 'm_algoHLCData.fLowPrice' is: " << m_algoHLCData[dataPoint].fLowPrice << endl;
//							cout << "Value of 'm_algoHLCData.fLowPrice' is: " << m_algoHLCData[stockId][dataPoint].fLowPrice << endl;
//							cout << "m_algoHLCData[" << stockId << "][" << dataPoint << "].fLowPrice' is: " << m_algoHLCData[stockId][dataPoint].fLowPrice << endl;
							cout << "m_algoHLCData[" << stockId + (algoCallCount*m_portfolioSize) << "][" << dataPoint << "].fLowPrice' is: " << m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fLowPrice << endl;
//							token = strtok(NULL, ",");
							token = strtok(NULL, ",\t");
							cout << "Value of 'token' is: " << token << endl;
							break;
						case 2:
//							m_algoHLCData[dataPoint].fClosePrice = atof(token);
//							m_algoHLCData[stockId][dataPoint].fClosePrice = atof(token);
							m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fClosePrice = atof(token);
//							cout << "Value of 'm_algoHLCData.fClosePrice' is: " << m_algoHLCData[dataPoint].fClosePrice << endl;
//							cout << "Value of 'm_algoHLCData.fClosePrice' is: " << m_algoHLCData[stockId][dataPoint].fClosePrice << endl;
//							cout << "m_algoHLCData[" << stockId << "][" << dataPoint << "].fClosePrice' is: " << m_algoHLCData[stockId][dataPoint].fClosePrice << endl;
							cout << "m_algoHLCData[" << stockId + (algoCallCount*m_portfolioSize) << "][" << dataPoint << "].fClosePrice' is: " << m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][dataPoint].fClosePrice << endl;
//							token = strtok(NULL, ",");
							token = strtok(NULL, ",\t");
//							cout << "Value of 'token' is: " << token << endl;
							break;
					}
				}
			}
		}
		////}
	}****/
	
//Read the Database.
//	for(int dataId = 0; dataId < m_algoDataPointCount; dataId++){
	


//-------------------------------------------------------------------

//1:Compute the Returns.
//-------------------------------------------------------------------
	
	int returnCount = m_algoDataPointCount - 1;

//Create 2-Dimensional Matrix
	double* *closePriceReturn = new double*[m_portfolioSize];	//closing price return.
	for(int i = 0; i < m_portfolioSize; i++){
//		closePriceReturn[i] = new double[m_algoDataPointCount - 1];
		closePriceReturn[i] = new double[returnCount];
	}

//	double* closePricePerAbsReturn = new double[m_algoDataPointCount - 1];
	double* *closePriceAbsReturn = new double*[m_portfolioSize];	//absolute of closing price return.
	for(int i = 0; i < m_portfolioSize; i++){
//		closePriceAbsReturn[i] = new double[m_algoDataPointCount - 1];
		closePriceAbsReturn[i] = new double[returnCount];
	}

	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
//		for(int i = 0; i < m_algoDataPointCount - 1; i++){
		for(int i = 0; i < returnCount; i++){
//			closePriceReturn[stockId][i] = ((m_algoHLCData[stockId][i+1].fClosePrice - m_algoHLCData[stockId][i].fClosePrice)*100.0)/(m_algoHLCData[stockId][i].fClosePrice);
			closePriceReturn[stockId][i] = ((m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][i+1].fClosePrice - m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][i].fClosePrice)*100.0)/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][i].fClosePrice);
//			closePriceReturn[stockId][i] = ((m_algoHLCData[stockId][i+1].fClosePrice - m_algoHLCData[stockId][i].fClosePrice))/(m_algoHLCData[stockId][i].fClosePrice);
			cout << "Value of 'closePriceReturn[" << stockId << "][" << i << "] is: " << closePriceReturn[stockId][i] << endl;

	//		closePricePerAbsReturn[i] = (m_algoHLCData[i+1].fClosePrice - m_algoHLCData[i].fClosePrice)*100.0/(m_algoHLCData[stockId][i].fClosePrice);
	//		closePricePerAbsReturn[i] = (m_algoHLCData[i+1].fClosePrice - m_algoHLCData[i].fClosePrice)*100.0/(m_algoHLCData[stockId][i].fClosePrice);
			closePriceAbsReturn[stockId][i] = (fabs(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][i+1].fClosePrice - m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][i].fClosePrice)*100.0)/(m_algoHLCData[stockId+ (algoCallCount*m_portfolioSize)][i].fClosePrice);
	//		cout << "Value of 'closePricePerAbsReturn[" << i << "] is: " << closePricePerAbsReturn[i] << endl;
			cout << "Value of 'closePriceAbsReturn[" << stockId << "][" << i << "] is: " << closePriceAbsReturn[stockId][i] << endl;
		}
	}

//2:Compute the 50-percentile.
//-------------------------------------------------------------------
//	int percentile = 50;
//	int percentile = 80;
	int percentile = m_algoPercentile;
//	int percentileWindowSize = 50;
//	int percentileWindowSize = 21;
	int percentileWindowSize = m_algoPercentileWindowSize;
//	int percentileReturnCount = (m_algoDataPointCount - 1 - percentileWindowSize) + 1;
	int percentileReturnCount = (returnCount - percentileWindowSize) + 1;

//Create a 2-Dimensional Matrix.
//	double* percentileReturn = new double[percentileReturnCount];
	double* * percentileReturn = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		percentileReturn[i] = new double[percentileReturnCount];
	}

			
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int i = 0; i < percentileReturnCount; i++){
			double* tmpPerAbsReturn = new double[percentile];
			for(int j = 0; j < percentile; j++){// Fill the array:'tmpPerAbsReturn' with 50 elements.
//				tmpPerAbsReturn[j] = closePricePerAbsReturn[j+i];
				tmpPerAbsReturn[j] = closePriceAbsReturn[stockId][j+i];
////				cout << "Value of 'tmpPerAbsReturn[" << j << "] is: " << tmpPerAbsReturn[j] << endl;
			}
			sort(tmpPerAbsReturn, tmpPerAbsReturn + percentileWindowSize);

// print out the contents of sorted vector:
			for(int k = 0; k < percentileWindowSize; k++){
////			cout << "Value of 'tmpPerAbsReturn[" << k << "] is: " << tmpPerAbsReturn[k] << endl;
			}

//calculate the percentile:	

			double rank = (percentile*(percentileWindowSize - 1))/100.0 + 1.0;
			cout << "Value of 'rank' is: " << rank << endl;
			if(rank == 1){
				percentileReturn[stockId][i] = tmpPerAbsReturn[0];
				cout << "percentileReturn[" << stockId << "][" << i << "]:" << percentileReturn[stockId][i] <<  endl; 
			}
			
			else{
				if(rank == percentile){
					percentileReturn[stockId][i] = tmpPerAbsReturn[percentileWindowSize - 1];
					cout << "percentileReturn[" << stockId << "][" << i << "]:" << percentileReturn[stockId][i] <<  endl; 
				}
				
				else{
					if((rank > 1) && (rank < percentile)){
						int rankIntComponent = (int)rank;
						cout << "Value of 'rankIntComponent' is: " << rankIntComponent << endl;
						double rankDecComponent = rank - rankIntComponent;
						cout << "Value of 'rankDecComponent' is: " << rankDecComponent << endl;
						percentileReturn[stockId][i] = tmpPerAbsReturn[rankIntComponent -1] + rankDecComponent*(tmpPerAbsReturn[rankIntComponent] - tmpPerAbsReturn[rankIntComponent -1]);
						cout << "percentileReturn[" << stockId << "][" << i << "]:" << percentileReturn[stockId][i] <<  endl; 
					}
				}
			} 
			
		}
	}

//3:Compute the 'Normalized Returns'.
//--------------------------------------------------------------------
//Create 2-Dimensional Matrix
	double* *closePriceNormalizedReturn = new double*[m_portfolioSize];	//closing price return.
	for(int i = 0; i < m_portfolioSize; i++){
		closePriceNormalizedReturn[i] = new double[percentileReturnCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int i = 0; i < percentileReturnCount; i++){
//			closePriceNormalizedReturn[stockId][i] = (closePriceReturn[stockId][percentileWindowSize-1+i]/percentileReturn[stockId][i])*100.0;
			closePriceNormalizedReturn[stockId][i] = (closePriceReturn[stockId][(percentileWindowSize + i) - 1]/percentileReturn[stockId][i]);
			cout << "closePriceNormalizedReturn[" << stockId << "][" << i << "]:" <<  closePriceNormalizedReturn[stockId][i] << endl;
		}
	}

//4:Compute the 'Volatility'.
//--------------------------------------------------------------------

//	int volWindowSize = 15;	//a user-defined parameter.
	int volWindowSize = m_algoVolWindowSize;	//a user-defined parameter.
	int factor = 3;
	int volSlotSize = volWindowSize/factor;

	int volCount = percentileReturnCount - volWindowSize + 1; 
//Create 2-Dimensional Matrix
	double* *closePriceVol = new double*[m_portfolioSize];	//closing price return.
	for(int i = 0; i < m_portfolioSize; i++){
		closePriceVol[i] = new double[volCount];
	}

	

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int volIndex = 0; volIndex < volCount; volIndex++){//Fill the 'volatility' array for each instrument.
			double closePriceVolComp0 = 0.0;
			double closePriceVolComp1 = 0.0;
			double closePriceVolComp2 = 0.0;
			cout << "Value of volIndex is: " << volIndex << endl; 
			for(int iter = 0; iter < factor; iter++){//Calculate the 'volatility'.
				cout << "Value of iter is: " << iter << endl; 
				for(int i = 0; i < ((iter+1)*volSlotSize); i++){
					cout << "Value of i is: " << i << endl; 

//					closePriceVol[stockId][volIndex] += closePriceNormalizedReturn[stockId][volIndex + volWindowSize-i-1];
//					cout << "closePriceVol[" << stockId << "][" << volIndex << "]:" <<  closePriceVol[stockId][volIndex] << endl;

					switch(iter){
						case 0:
							closePriceVolComp0 += closePriceNormalizedReturn[stockId][volIndex + volWindowSize-i-1];
							cout << "closePriceVolComp0:" << closePriceVolComp0 << endl;
							break;
						case 1:
							closePriceVolComp1 += closePriceNormalizedReturn[stockId][volIndex + volWindowSize-i-1];
							cout << "closePriceVolComp1:" << closePriceVolComp1 << endl;
							break;
						case 2:
							closePriceVolComp2 += closePriceNormalizedReturn[stockId][volIndex + volWindowSize-i-1];
							cout << "closePriceVolComp2:" << closePriceVolComp2 << endl;
							break;
						}
				}

//				closePriceVol[stockId][volIndex] = closePriceVol[stockId][volIndex]/(sqrt((iter+1)*volSlotSize));
//				cout << "closePriceVol[" << stockId << "][" << volIndex << "]:" <<  closePriceVol[stockId][volIndex] << endl;
				switch(iter){
					case 0:
						closePriceVolComp0 = closePriceVolComp0/(sqrt((iter+1)*volSlotSize));
						cout << "closePriceVolComp0:" << closePriceVolComp0 << endl;
						break;
					case 1:
						closePriceVolComp1 = closePriceVolComp1/(sqrt((iter+1)*volSlotSize));	
						cout << "closePriceVolComp1:" << closePriceVolComp1 << endl;
						break;
					case 2:
						closePriceVolComp2 = closePriceVolComp2/(sqrt((iter+1)*volSlotSize));
						cout << "closePriceVolComp2:" << closePriceVolComp2 << endl;
						break;
				}
			}
		
//			closePriceVol[stockId][volIndex] = closePriceVol[stockId][volIndex]/factor;
			closePriceVol[stockId][volIndex] = (closePriceVolComp0+closePriceVolComp1+closePriceVolComp2)/factor;	//1st element of array: 'closePriceVol' corresponds to the 'percentileWindowSize + volWindowSize - 1'(= 64th, in this case) data element of the stock.
			cout << "closePriceVol[" << stockId << "][" << volIndex << "]:" <<  closePriceVol[stockId][volIndex] << endl;
		}
	}

//5:Compute Price-Range.
//--------------------------------------------------------------------
	
//Create 2-Dimensional Matrix
	double* *priceRange = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		priceRange[i] = new double[m_algoDataPointCount];
	}

//Fill the array
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < m_algoDataPointCount; index++){
//			priceRange[stockId][index] = ((m_algoHLCData[stockId][index].fClosePrice - m_algoHLCData[stockId][index].fLowPrice)/(m_algoHLCData[stockId][index].fHighPrice - m_algoHLCData[stockId][index].fLowPrice)  - 0.5)*(2.0);
			priceRange[stockId][index] = ((m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][index].fClosePrice - m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][index].fLowPrice)/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][index].fHighPrice - m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][index].fLowPrice)  - 0.5)*(2.0);
			cout << "priceRange[" << stockId << "][" << index << "]:" <<  priceRange[stockId][index] << endl;
		}
	}

//6a:Compute 'Average Normalized Return' across the portfolio.
//--------------------------------------------------------------------

//Create 1-Dimensional Matrix
	double *averageNormalizedReturn = new double[percentileReturnCount];

//Fill the array.
	for(int index = 0; index < percentileReturnCount; index++){
		for(int stockId = 0; stockId < m_portfolioSize; stockId++){
			averageNormalizedReturn[index] += closePriceNormalizedReturn[stockId][index];
		}
		averageNormalizedReturn[index] = averageNormalizedReturn[index]/m_portfolioSize;
		cout << "averageNormalizedReturn[" << index << "]:" << averageNormalizedReturn[index] << endl;
	}
	

//6b:Compute 'Normalized-Return Deviation' for each instrument in the portfolio.
//--------------------------------------------------------------------

//Create 2-Dimensional Matrix
	double* *normalizedReturnDeviation = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
//		normalizedReturnDeviation[i] = new double[percentileReturnCount];
		normalizedReturnDeviation[i] = new double[volCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
//		for(int index = 0; index < percentileReturnCount; index++){
		for(int index = 0; index < volCount; index++){
//			normalizedReturnDeviation[stockId][index] = closePriceNormalizedReturn[stockId][index] - averageNormalizedReturn[index];
			normalizedReturnDeviation[stockId][index] = closePriceNormalizedReturn[stockId][volWindowSize + index - 1] - averageNormalizedReturn[volWindowSize + index - 1];
			cout << "normalizedReturnDeviation[" << stockId << "][" << index << "]:" << normalizedReturnDeviation[stockId][index] << endl;
		}
	}

//7a:Compute 'Average Volatility' across the portfolio.
//--------------------------------------------------------------------

//Create 1-Dimensional Matrix
	double *averageVol = new double[volCount];

//Fill the array.
	for(int index = 0; index < volCount; index++){
		for(int stockId = 0; stockId < m_portfolioSize; stockId++){
			averageVol[index] += closePriceVol[stockId][index];
		}
		averageVol[index] = averageVol[index]/m_portfolioSize;
		cout << "averageVol[" << index << "]:" << averageVol[index] << endl;
	}

//7b:Compute 'Volatility-Deviation' for each instrument in the portfolio.
//--------------------------------------------------------------------

//Create 2-Dimensional Matrix
	double* *volDeviation = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		volDeviation[i] = new double[volCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < volCount; index++){
			volDeviation[stockId][index] = closePriceVol[stockId][index] - averageVol[index];
			cout << "volDeviation[" << stockId << "][" << index << "]:" << volDeviation[stockId][index] << endl;
		}
	}

//8:Compute 'Rho' value.
//--------------------------------------------------------------------
//	double constantRho = 3.0;
	double constantRho = m_algoConstantRho;
//Create 2-Dimensional array.
	double* * rho = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		rho[i] = new double[volCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < volCount; index++){
//			double difference = normalizedReturnDeviation[stockId][(volWindowSize + index) - 1] - volDeviation[stockId][index];
			double difference = normalizedReturnDeviation[stockId][index] - volDeviation[stockId][index];
			cout << "difference:" << difference << endl;
			if(fabs(difference) < constantRho){
				rho[stockId][index] = difference;
				cout << "rho[" << stockId << "][" << index << "]:" << rho[stockId][index] << endl;
			}
	
			else{
				rho[stockId][index] = 0;
				cout << "rho[" << stockId << "][" << index << "]:" << rho[stockId][index] << endl;
			}
		}
	}

//9:Compute the 'five-Day Rolling-Average Price-Range' for each stock. 
//--------------------------------------------------------------------

//	int priceRangeWindowSize = 5;
	int priceRangeWindowSize = m_algoPriceRangeWindowSize;
	int avgPriceRangeCount = m_algoDataPointCount - priceRangeWindowSize + 1;
	
//Create 2-Dimensional array.
	double* * fiveDayRollAvgPriceRange = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		fiveDayRollAvgPriceRange[i] = new double[avgPriceRangeCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < avgPriceRangeCount; index++){
			for(int i = 0; i < priceRangeWindowSize; i++){
				fiveDayRollAvgPriceRange[stockId][index] += priceRange[stockId][i + (index)]; 
			}
			fiveDayRollAvgPriceRange[stockId][index] = fiveDayRollAvgPriceRange[stockId][index]/priceRangeWindowSize;
			cout << "fiveDayRollAvgPriceRange[" << stockId << "][" << index << "]:" << fiveDayRollAvgPriceRange[stockId][index] << endl;
			
		}
	}

//10:Compute the 'averagePriceRange' across the portfolio.
//--------------------------------------------------------------------

//Create 1-Dimensional Matrix.

	double* avgFiveDayRollAvgPriceRange = new double[avgPriceRangeCount];

//Fill the array.
	
	for(int index = 0; index < avgPriceRangeCount; index++){//array: 'avgPriceRange' index.
		for(int stockId = 0; stockId < m_portfolioSize; stockId++){//average across the portfolio.
			avgFiveDayRollAvgPriceRange[index] += fiveDayRollAvgPriceRange[stockId][index];
		}
		
		avgFiveDayRollAvgPriceRange[index] = avgFiveDayRollAvgPriceRange[index]/m_portfolioSize;
		cout << "avgFiveDayRollAvgPriceRange[" << index << "]:" << avgFiveDayRollAvgPriceRange[index] << endl;
	}

//11:Compute the 'priceRangeDeviation'.
//--------------------------------------------------------------------

//Create 2-Dimensional Matrix.
	double* * priceRangeDeviation = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		priceRangeDeviation[i] = new double[avgPriceRangeCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < avgPriceRangeCount; index++){
			priceRangeDeviation[stockId][index] = fiveDayRollAvgPriceRange[stockId][index] - avgFiveDayRollAvgPriceRange[index];
			cout << "priceRangeDeviation[" << stockId << "][" << index << "]:" << priceRangeDeviation[stockId][index] << endl;
		}
	}

//12:Compute Beta = (Rho-priceRangeDeviation)/(n-percentile)
//--------------------------------------------------------------------

//Create 2-Dimensional Matrix.
	double* * beta = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		beta[i] = new double[volCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < volCount; index++){
//			beta[stockId][index] = (rho[stockId][index] - priceRangeDeviation[stockId][(-priceRangeWindowSize + percentileWindowSize + volWindowSize - 1  + index) -1])/percentileReturn[stockId][volWindowSize + index - 1];
			cout << "rho[" << stockId << "][" << index << "]:" << rho[stockId][index] << endl;
			cout << "priceRangeDeviation[" << stockId << "][" << (-priceRangeWindowSize + percentileWindowSize + volWindowSize + 1 + index) -1 << "]:" << priceRangeDeviation[stockId][(-priceRangeWindowSize + percentileWindowSize + volWindowSize + 1 + index) -1] << endl;
			cout << "percentileReturn[" << stockId << "][" << volWindowSize + index - 1 << "]:" <<  percentileReturn[stockId][volWindowSize + index - 1] << endl;
			beta[stockId][index] = (rho[stockId][index] - priceRangeDeviation[stockId][(-priceRangeWindowSize + percentileWindowSize + volWindowSize + 1 + index) -1])/percentileReturn[stockId][volWindowSize + index - 1];
			cout << "beta[" << stockId << "][" << index << "]:" << beta[stockId][index] << endl;
		}
	}

//13:Compute the 'avgBeta' across the portfolio.
//--------------------------------------------------------------------

//Create the 1-Dimensional Matrix.
	double *avgBeta = new double[volCount];

//Fill the array.
	for(int index = 0; index < volCount; index++){
		for(int stockId = 0; stockId < m_portfolioSize; stockId++){
			avgBeta[index] += beta[stockId][index];
		}
		
		avgBeta[index] = avgBeta[index]/m_portfolioSize;
		cout << "avgBeta[" << index << "]:" << avgBeta[index] << endl;
	}
	
//14:Compute the 'avgBetaDeviation'.
//--------------------------------------------------------------------

//Create the 2-Dimensional Matrix.
	double* * avgBetaDeviation = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		avgBetaDeviation[i] = new double[volCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < volCount; index++){
			avgBetaDeviation[stockId][index] = beta[stockId][index] - avgBeta[index];
			cout << "avgBetaDeviation[" << stockId << "][" << index << "]:" << avgBetaDeviation[stockId][index] << endl;
		}
	}

//15:Compute Gamma = Beta(t-1)*closePriceReturn(t)
//--------------------------------------------------------------------

//Create the 2-Dimensional Matrix.
	double* * gamma = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		gamma[i] = new double[volCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < volCount; index++){
//			gamma[stockId][index] = beta[stockId][index]*closePriceReturn[stockId][(percentileWindowSize + volWindowSize - 1) - 1];
			if(index==0){
				cout << "closePriceReturn[" << stockId << "][" << (percentileWindowSize + volWindowSize - 1 + index) - 1 << "]:" << closePriceReturn[stockId][(percentileWindowSize + volWindowSize - 1 + index) - 1] << endl;
				gamma[stockId][index] = (0*closePriceReturn[stockId][(percentileWindowSize + volWindowSize - 1 + index) - 1])/100;
				cout << "gamma[" << stockId << "][" << index << "]:" << gamma[stockId][index] << endl;
			}
			
			else{
				cout << "closePriceReturn[" << stockId << "][" << (percentileWindowSize + volWindowSize - 1 + index) - 1 << "]:" << closePriceReturn[stockId][(percentileWindowSize + volWindowSize - 1 + index) - 1] << endl;
				cout << "avgBetaDeviation[" << stockId << "][" << index - 1 << "]:" << avgBetaDeviation[stockId][index - 1] << endl;
				gamma[stockId][index] = (avgBetaDeviation[stockId][index - 1]*closePriceReturn[stockId][(percentileWindowSize + volWindowSize - 1 + index) - 1])/100;
				cout << "gamma[" << stockId << "][" << index << "]:" << gamma[stockId][index] << endl;

			}
		}
	}

//16:Compute 'GammaSummation' across the portfolio.
//--------------------------------------------------------------------

//Create the 1-Dimensional Matrix.
	double* gammaSummation = new double[volCount];

//Fill the array.
	for(int index = 0; index < volCount; index++){
		for(int stockId = 0; stockId < m_portfolioSize; stockId++){
			gammaSummation[index] += gamma[stockId][index];
		}
		cout << "gammaSummation[" << index << "]:" << gammaSummation[index] << endl;
	}

//17:Compute 'CumulativeGammaSummation'.
//---------------------------------------------------------------------

//Create the 1-Dimensional Matrix.
	double* cumulativeGammaSummation = new double[volCount];

//Fille the array.
	for(int index = 0; index < volCount; index++){
		if(index == 0){
			cumulativeGammaSummation[index] =  gammaSummation[index];
			cout << "cumulativeGammaSummation[" << index << "]:" << cumulativeGammaSummation[index] << endl;
		}
		else{
			cumulativeGammaSummation[index] = cumulativeGammaSummation[index - 1] + gammaSummation[index];
			cout << "cumulativeGammaSummation[" << index << "]:" << cumulativeGammaSummation[index] << endl;
		}
	}

//18:Compute 'n' Days rolling average of 'gammaSummation'.
//--------------------------------------------------------------------

//	int mktVarianceDays = 63;	//no. of days to calculate variation.
	int mktVarianceDays = m_algoMktVarianceDays;	//no. of days to calculate variation.
	int stdDevGammaSummationCount = volCount - mktVarianceDays + 1;

//Create the 1-Dimensional Matrix.
	double* rollAvgGammaSummation = new double[stdDevGammaSummationCount];

//Fill the array.
	for(int index = 0; index < stdDevGammaSummationCount; index++){
		for(int i = 0; i < mktVarianceDays; i++){
			rollAvgGammaSummation[index] +=  gammaSummation[index + i];
		}
		rollAvgGammaSummation[index] = rollAvgGammaSummation[index]/mktVarianceDays;
		cout << "rollAvgGammaSummation[" << index << "]:" << rollAvgGammaSummation[index] << endl;
	}

//19:Compute 'StandardDeviationGammaSummation'.
//--------------------------------------------------------------------

//Create the 1-Dimensional Matrix.
	double* stdDevGammaSummation = new double[stdDevGammaSummationCount];

//Fill the array.
	for(int index = 0; index < stdDevGammaSummationCount; index++){
		for(int i = 0; i < mktVarianceDays; i++){
			stdDevGammaSummation[index] += pow((gammaSummation[index + i] - rollAvgGammaSummation[index]),2);
		}
		stdDevGammaSummation[index] = sqrt(stdDevGammaSummation[index]/(mktVarianceDays - 1));
		cout << "stdDevGammaSummation[" << index << "]:" << stdDevGammaSummation[index] << endl;
	}

//20:Compute 'portfolioPosition' = (avgBetaDeviation*riskExposure)/stdDevGammaSummation.
//--------------------------------------------------------------------	

//	int riskExposure = 60000;
//	double riskExposure = m_algoRiskExposure;
	long int riskExposure = m_algoRiskExposure[algoCallCount];
///	int riskExposure = 100000;

//Create the 2-Dimensional Matrix.
	double* * portfolioPosition = new double*[m_portfolioSize];
	for(int i = 0; i < m_portfolioSize; i++){
		portfolioPosition[i] = new double[stdDevGammaSummationCount];
	}

//Fill the array.
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
		for(int index = 0; index < stdDevGammaSummationCount; index++){
			portfolioPosition[stockId][index] = (avgBetaDeviation[stockId][mktVarianceDays + index - 1]*riskExposure)/stdDevGammaSummation[index];
			cout << "portfolioPosition[" << stockId << "][" << index << "]:" << portfolioPosition[stockId][index] << endl;
			printf("portfolioPosition[%u][%u]:%.9f\n", stockId, index, portfolioPosition[stockId][index]);
		}
	}
//21:Validation: sum the 'portfolioPosition' across the portfolio.
//--------------------------------------------------------------------

//Create 1-Dimensional Matrix.
	double * sumPortfolioPosition = new double[stdDevGammaSummationCount];

//Fill the array.
	for(int index = 0; index < stdDevGammaSummationCount; index++){
		for(int stockId = 0; stockId < m_portfolioSize; stockId++){
			sumPortfolioPosition[index] += portfolioPosition[stockId][index];
		}
		cout << "sumPortfolioPosition[" << index << "]:" << sumPortfolioPosition[index] << endl;
	}


//22:Write the 'output' in the file 'portfolioPosition.dat'.
//-------------------------------------------------------------------- 

	fstream positionFile;
	
	char * positionFileName = new char[64];
	strcpy(positionFileName, getenv("HOME"));
	strcat(positionFileName, "/NeuralAlgo/portfolioPositions.dat");
//	positionFile.open("portfolioPosition.dat", ios::out|ios::app);
	positionFile.open(positionFileName, ios::out|ios::app);
//	positionFile.open("portfolioPosition.dat", ios::out);

	if(!positionFile.is_open()){
		cout << "File: " << positionFileName << " failed to open." << endl;
	}


//	for(int stockId = algoCallCount*m_portfolioSize; stockId < m_portfolioSize + (algoCallCount*m_portfolioSize) ; stockId++)
	for(int stockId = 0; stockId < m_portfolioSize; stockId++){
//		if((stockId != (m_portfolioSize + (algoCallCount*m_portfolioSize))))
/****		if((stockId != (m_portfolioSize)))

//Write the 2-Dimensional matrix: 'portfolioPosition' in the File: 'portfolioPosition.dat'.
//--------------------------------------------------------------------
			positionFile << portfolioPosition[stockId][stdDevGammaSummationCount - 1] << "|";
			positionFile << (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]) << "|";
			positionFile << (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][250].fClosePrice*m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]) << "	";
//x------------------------------------------------------------------x****/

//			m_portfolioPosition[stockId] = (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_contractLotSize[stockId]);
			
//Round-off to the nearest-integer.
//--------------------------------------------------------------------
			int position = ((portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][250].fClosePrice*m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]))*10;

			int remainder = position%10;
			if(abs(remainder)<5){ 
//				m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] = (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][250].fClosePrice*m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]);
				m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] = position/10;
			cout << "m_portfolioPosition[" << stockId + (algoCallCount*m_portfolioSize) << "]:" <<  m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] << endl;
			}
			else{
//				m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] = position/10 + 1;
				m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] = position/10 + (remainder/abs(remainder));
				cout << "m_portfolioPosition[" << stockId + (algoCallCount*m_portfolioSize) << "]:" <<  m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] << endl;
			}

//Write the 2-Dimensional matrix: 'portfolioPosition' in the File: 'portfolioPosition.dat'.
//--------------------------------------------------------------------

		if((stockId != (m_portfolioSize))){

			positionFile << portfolioPosition[stockId][stdDevGammaSummationCount - 1] << "|";
			positionFile << (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]) << "|";
			positionFile << (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][250].fClosePrice*m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]) << "	";
		}

		else{	
//Write the 2-Dimensional matrix: 'portfolioPosition' in the File: 'portfolioPosition.dat'.
//--------------------------------------------------------------------

			positionFile << portfolioPosition[stockId][stdDevGammaSummationCount - 1] << "|";
			positionFile << (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]) << "|";
			positionFile << (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][250].fClosePrice*m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]) << endl << endl;
//x------------------------------------------------------------------x
/****			m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] = (portfolioPosition[stockId][stdDevGammaSummationCount - 1])/(m_algoHLCData[stockId + (algoCallCount*m_portfolioSize)][250].fClosePrice*m_contractLotSize[stockId + (algoCallCount*m_portfolioSize)]);
			cout << "m_portfolioPosition[" << stockId + (algoCallCount*m_portfolioSize) << "]:" <<  m_portfolioPosition[stockId + (algoCallCount*m_portfolioSize)] << endl;****/
		}
//x------------------------------------------------------------------x
	}
	positionFile.close();
}
