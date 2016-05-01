#ifdef _WIN32
# include <Windows.h>
# define sleep( seconds) Sleep( seconds * 1000);
#else
# include <unistd.h>
#endif

#include "PosixTestClient.h"

/*******Appended Headed Files.*******/
#include <stdio.h>	
#include <fstream>	//object: fstream 'datafile'
#include <iostream>
#include <string.h>	//function: 'strtok'
#include <algorithm>	//function: 'sort'
#include <vector>
#include "Contract.h"
#include <sys/types.h>	//function: 'getpwuid()'
#include <pwd.h>	//struct: 'passwd'
#include <sys/stat.h>	//struct: 'stat' 	

using namespace std;

const unsigned MAX_ATTEMPTS = 50;
//const unsigned SLEEP_TIME = 10;
const unsigned SLEEP_TIME = 15;	//Keeping in view: 'IB' Historical Data Limitations.

//Enumerators.
//---------------------------------------------------------------------
enum Parameters{
	TRADE_ENDSESSIONDURATION,
	TRADE_PRICETOLERANCE,
	TRADE_SESSIONDURATION,
	TRADE_HOST,
	TRADE_CLIENTID,
	TRADE_PORTFOLIOCOUNT,
	TRADE_PORT,
	TRADE_PORTFOLIOSIZE,
	ALGO_DATAPOINTCOUNT,
	ALGO_PERCENTILE,
	ALGO_PERCENTILEWINDOWSIZE,
	ALGO_VOLWINDOWSIZE,
	ALGO_CONSTANTRHO,
	ALGO_PRICERANGEWINDOWSIZE,
	ALGO_MARKETVARIANCEDAYS,
	ALGO_RISKEXPOSURE,
	DB_UPDATETIME
};

struct TradeParameter
{
//	const char* fHost;
	char* fHost;
	unsigned int fPort;
	int fClientId;
	int fPortfolioSize;	//total no. of stocks in eack portfolio.
	int fPortfolioCount;	//total no. of portfolios.
	int fCapitalPerStock;
	int fSessionDuration;	//duration of a single trade session(in seconds), a user-defined parameter.(in seconds)
	int fEndSessionDuration;	//duration of a session in which Market-Orders are placed.(in seconds)
//	float fPriceTolerance; //price tolerance for 'Limit Orders'.
	double fPriceTolerance; //price tolerance for 'Limit Orders'.
	char * fDbUpdateTime;	//time of the day to update the Database File: 'highLowClosePortfolio.dat'
}tradeParam;

struct Algo{
	int fDataPointCount; //total no. of datapoints to compute the 'portfolioPosition'.
	int fPercentile;
	int fPercentileWindowSize;
	int fVolWindowSize;
	double fConstantRho;
	int fPriceRangeWindowSize;
	int fMktVarianceDays;
//	double fRiskExposure;
	long int fUniversalRiskExposure;
	long int * fRiskExposure;
//}algo;
}algoParam;

//void ParseCommandLine(int argc, char * * argv);
void ParseCommandLine(int argc, char * * argv, char * inputFileName);
//void MakePortfolio(int portfolioSize, Contract* portfolio);
//void MakePortfolio(int portfolioSize, Contract* portfolio, char * portfolioFile);
void MakePortfolio(int* contractLotSize, int portfolioSize, Contract* portfolio, char * portfolioFile);
void ExitWithHelp();
//void ReadConfigurationFile();
//void ReadConfigurationFile(char * portfolioFileName);
void ReadConfigurationFile(char * confFileName);

/*struct PortfolioPosition{
	int p0;
	int p1;
	int p2;
	int p3;
	int p4;
	int p5;
	int p6;
};*/

//void FillPortfolioPosition();
//void FillPortfolioPosition(int* position);
//void SortPortfolio(int* position, int portfolioSize, int portfolioCount);
void FillPortfolioPosition(int* position, int portfolioSize, int portfolioCount);

int main(int argc, char** argv)
{

//Find User's Home Directory.
//---------------------------------------------------------------------
	struct passwd * user;
	user = getpwuid(getuid());
	cout << "User's Home Direcory is: " << user->pw_dir << endl;
//x------------------------------------------------------------------x

//Default Portfolio File.
	char portfolioFile[1024];	//array containing the name of the 'Portfolio File'.
	strcpy(portfolioFile, user->pw_dir);
	strcat(portfolioFile, "/NeuralAlgo/portfolioL.dat");
	cout << "portfolioFile:" << portfolioFile << endl;


//	ReadConfigurationFile();
//	ReadConfigurationFile(portfolioFile);

	char * confFileName = new char[64];
	strcpy(confFileName, user->pw_dir);
	strcat(confFileName, "/NeuralAlgo/algo.conf");
	ReadConfigurationFile(confFileName);

////	strcpy(portfolioFile,"portfolioL.dat");
	ParseCommandLine(argc, argv, portfolioFile);

	char * mailFileName = new char[64];
	strcpy(mailFileName, "/var/mail/"); 
	strcat(mailFileName, user->pw_name);

//Check the modification time of the configuration file and the mail.
//-------------------------------------------------------------------->

	struct stat * mailFileStatus = new struct stat;
	stat(mailFileName, mailFileStatus);	
	cout << "Time of Last Modification of File: " << mailFileName << " is: " <<  mailFileStatus->st_mtime << endl;
	unsigned long int mailFileModTime = mailFileStatus->st_mtime;

	struct stat * confFileStatus = new struct stat;
	stat(confFileName, confFileStatus);	
	cout << "Time of Last Modification  of File: " <<  confFileName << " is: " << confFileStatus->st_mtime << endl;
	unsigned long int confFileModTime = confFileStatus->st_mtime;

	if((mailFileModTime > confFileModTime)){
		cout << "Condition '(mailFileModTime > confFileModTime)' evaluated as 'TRUE'" << endl;
		cout << "Value of 'tradeParam.fHost' is: " << tradeParam.fHost << endl;
		ReadConfigurationFile(mailFileName);
	}
//x-------------------------------------------------------------------x

	



//Assign the 'user-defined' parameters.
//------------------------------------------------------------------->

//Connection parameters.
//
//	const char* host = argc > 1 ? argv[1] : "";
	const char* host = tradeParam.fHost;
	cout << "Value of 'tradeParam.fHost' is: " << tradeParam.fHost << endl;
////	cout << "Value of 'host' is: " << host << endl;
	unsigned int port = tradeParam.fPort;
	int clientId = tradeParam.fClientId;

//Portfolio parameters.
//
	int portfolioSize = tradeParam.fPortfolioSize;
	int portfolioCount = tradeParam.fPortfolioCount;

//Trade-Session parameters.
//
	int sessionDuration = tradeParam.fSessionDuration;
	int endSessionDuration = tradeParam.fEndSessionDuration;
	float priceTolerance = tradeParam.fPriceTolerance;
//	int capitalPerStock = tradeParam.fCapitalPerStock;

//Algorithm parameters.
//
	int algoDataPointCount = algoParam.fDataPointCount;
	int algoPercentile = algoParam.fPercentile;
	int algoPercentileWindowSize = algoParam.fPercentileWindowSize;
	int algoVolWindowSize = algoParam.fVolWindowSize;
	double algoConstantRho = algoParam.fConstantRho;
	int algoPriceRangeWindowSize = algoParam.fPriceRangeWindowSize;
	int algoMktVarianceDays = algoParam.fMktVarianceDays;
//	double algoRiskExposure = algoParam.fRiskExposure;
	long int *  algoRiskExposure = algoParam.fRiskExposure;

//Database-Update-Time
//
	char *  dbUpdateTime = tradeParam.fDbUpdateTime;
/****				int tmHour = atoi(strtok(dbUpdateTime, ":"));
				cout << "tmHour:" << tmHour << endl;
				int tmMin  = atoi(strtok(NULL, ":"));
				cout << "tmMin:" << tmMin << endl;****/

//<------------------------------------------------------------------>

	

	int instrumentCount = portfolioSize*portfolioCount;

////	int* position = new int[instrumentCount];	//allocate the memory to hold the 'Desired exposure' for 'n' no. of Futures.
//	int* contractLotSize = new int[portfolioSize];	//an array containing 'lot-size' of each contract in the portfolio.
	int* contractLotSize = new int[instrumentCount];	//an array containing 'lot-size' of each contract in the portfolio.

	unsigned attempt = 0;

//	Contract portfolio[portfolioSize];
	Contract portfolio[instrumentCount];
//	MakePortfolio(contractLotSize, portfolioSize, portfolio, portfolioFile);
	MakePortfolio(contractLotSize, instrumentCount, portfolio, portfolioFile);

////	FillPortfolioPosition(position, portfolioSize, portfolioCount);	//Fill the 'portfolioPosition' Vector.	

	printf( "Start of POSIX Socket Client Test %u\n", attempt);

	for (;;) {
		++attempt;
		printf( "Attempt %u of %u\n", attempt, MAX_ATTEMPTS);

//		PosixTestClient client(clientId, portfolioSize, portfolioCount, portfolio, position, sessionDuration, contractLotSize, endSessionDuration, priceTolerance, attempt); 
//		PosixTestClient client(clientId, portfolioSize, portfolioCount, portfolio, sessionDuration, contractLotSize, endSessionDuration, priceTolerance, attempt, algoDataPointCount, algoPercentile, algoPercentileWindowSize, algoVolWindowSize, algoConstantRho, algoPriceRangeWindowSize, algoMktVarianceDays,algoRiskExposure); 
		PosixTestClient client(clientId, portfolioSize, portfolioCount, portfolio, sessionDuration, contractLotSize, endSessionDuration, priceTolerance, attempt, algoDataPointCount, algoPercentile, algoPercentileWindowSize, algoVolWindowSize, algoConstantRho, algoPriceRangeWindowSize, algoMktVarianceDays,algoRiskExposure, dbUpdateTime); 
		
////		cout << "Will connect now ..." << endl;
////		cout << "Value of 'host' is: " << host << endl;
		client.connect( host, port, clientId);

		while( client.isConnected()) {
			client.processMessages();
		}

		if( attempt >= MAX_ATTEMPTS) {
			break;
		}

		printf( "Sleeping %u seconds before next attempt\n", SLEEP_TIME);
		sleep( SLEEP_TIME);
	}

	printf ( "End of POSIX Socket Client Test\n");
	
	delete[] confFileName;
	delete[] mailFileName;
	delete confFileStatus;
	delete mailFileStatus;
	delete[] contractLotSize;
}

/*******Construct a portfolio from a given file.*******/
//void MakePortfolio(int portfolioSize, Contract* portfolio)
//void MakePortfolio(int portfolioSize, Contract* portfolio, char * portfolioFile)
void MakePortfolio(int* contractLotSize, int portfolioSize, Contract* portfolio, char * portfolioFile)
{
	cout << "In Method: 'MakePortfolio(int* contractLotSize, int portfolioSize, Contract* portfolio, char * portfolioFile)' ..." << endl;
	cout << "******************************************************************************************************************" << endl;
//------------------------------------------------------------------->
//
	Contract contract;
	char* stockSymbol;
	fstream stockFile;
//	stockFile.open("Portfolio.list", ios::in);
	stockFile.open(portfolioFile, ios::in);
	if(stockFile.is_open())
	{
		int id = 0;
		while((stockFile.good()) && (id < portfolioSize))
		{		
				cout << "Reading line no." << id << endl;
				stockSymbol = new char[256];
				stockFile.getline(stockSymbol,256, '\n');
				cout << stockSymbol << endl;
				char* token;
				token =	strtok(stockSymbol,"\t");
				int i = 0;
				while(token != NULL)
				{
					switch(i)
					{
						case 0:
							contract.symbol = token;
							token = strtok(NULL, "\t");
							break;
						case 1:
							contract.secType = token;
							token = strtok(NULL, "\t");
							break;
						case 2:
							contract.conId = atoi(token);
							token = strtok(NULL, "\t");
							break;
						case 3:
							contract.exchange = token;
							token = strtok(NULL, "\t");
							break;
						case 4:
							contract.currency = token;
							token = strtok(NULL, "\t");
							break;
						case 5:
							contract.localSymbol = token;
							token = strtok(NULL, "\t");
							break;
						case 6:
							contract.expiry = token;
							cout << "Value of 'contract.expiry' is: " << contract.expiry << endl;
							token = strtok(NULL,"\t");
							break;
						case 7:
							contract.multiplier = token;
							cout << "Value of 'contract.multiplier' is: " << contract.multiplier << endl;
							token = strtok(NULL, "\t");
							break;
						case 8:
							contractLotSize[id] = atoi(token);
							cout << "contractLotSize[" << id << "]:" << contractLotSize[id] << endl;
							token = strtok(NULL, "\t");
							break;
					}
					i++;
				}
			cout << "symbol =" << contract.symbol <<endl;
			cout << "secType =" << contract.secType << endl;
			cout << "conId =" << contract.conId << endl;
			cout << "exchange =" << contract.exchange << endl;
			cout << "currency =" << contract.currency << endl;
			cout << "localSymbol =" << contract.localSymbol << endl;
			cout << "expiry =" << contract.expiry << endl;
			cout << "multiplier =" << contract.multiplier << endl;
			cout << "lotSize = " << contractLotSize[id] << endl;

			portfolio[id] = contract;
			delete[] stockSymbol;
			id++;
			cout << "id:" << id << endl;
		}
		cout << "'while' loop ends." << endl;
//disconnect();
	}
	else
	{
		cout << "Stock File does not exist." << endl;
		exit(1);
	}
	cout << "Closing File: " << portfolioFile  << endl;
	stockFile.close();
	cout << "File: " << portfolioFile  << " closed." << endl;

//<------------------------------------------------------------------>
//
	cout << "Method: 'MakePortfolio(int* contractLotSize, int portfolioSize, Contract* portfolio, char * portfolioFile)' ends." << endl;
	cout << "x---------------------------------------------------------------------------------------------------------------x" << endl;
}

//void ParseCommandLine(int argc, char * * argv)
void ParseCommandLine(int argc, char * * argv, char * inputFileName)
{
	int i;
//	void (*print_func)(const char*) = NULL;	// default printing to stdout
//	bool printFunc = false;

/****Switched Off ---> Instead, initialize in Method: 'ReadConfigurationFile' 
//Default Values.
//--------------------------------------------------------------------

// Default 'connection' parameters.
	tradeParam.fHost = "";
	tradeParam.fPort = 7496;
	tradeParam.fClientId = 0;

// Default 'trade-session' parameters.
	tradeParam.fPortfolioSize = 3;
	tradeParam.fPortfolioCount = 1;
	tradeParam.fSessionDuration = 600;	//in seconds.
	tradeParam.fCapitalPerStock = 100000;
	tradeParam.fEndSessionDuration = 60;	//in seconds.
	tradeParam.fPriceTolerance = 0.05; 
	
// Default 'Algorithm' parameters.
//	tradeParam.fAlgoDataPointCount = 251;
	algoParam.fDataPointCount = 251;
	algoParam.fPercentile = 80;
	algoParam.fPercentileWindowSize = 21;
	algoParam.fVolWindowSize = 15;
	algoParam.fConstantRho = 3.0;
	algoParam.fPriceRangeWindowSize = 5;
	algoParam.fMktVarianceDays = 63;
	algoParam.fRiskExposure = 100000;****/
//Parse Options.
//---------------------------------------------------------------------
	for(i=1;i<argc;i++)
	{
		if(argv[i][0] != '-') break;
		if(++i>=argc)
			ExitWithHelp();
		switch(argv[i-1][1])
		{
			case 'c':
				tradeParam.fCapitalPerStock = atoi(argv[i]);
				break;
			case 'S':
				tradeParam.fSessionDuration = atoi(argv[i]);
				cout << "Value of 'tradeParam.fSessionDuration' is: " << tradeParam.fSessionDuration << endl;
				break;
			case 'h':
				tradeParam.fHost = argv[i];
				cout << "Value of 'tradeParam.fHost' is: " << tradeParam.fHost << endl;
				break;
			case 'i':
				tradeParam.fClientId = atoi(argv[i]);
				cout << "Value of tradeParam.fClientId is: " << tradeParam.fClientId << endl;
				break;
			case 'p':
				tradeParam.fPort = atoi(argv[i]);
				cout << "Value of tradeParam.fPort is: " << tradeParam.fPort << endl;
				break;
			case 's':
				tradeParam.fPortfolioSize = atoi(argv[i]);
				cout << "Value of tradeParam.fPortfolioSize is: " << tradeParam.fPortfolioSize << endl;
				break;
			case 'n':
				tradeParam.fPortfolioCount = atoi(argv[i]);
				cout << "Value of tradeParam.fPortfolioCount is: " << tradeParam.fPortfolioCount << endl;
				break;
			case 'E':
				tradeParam.fEndSessionDuration = atoi(argv[i]);
				cout << "Value of tradeParam.fEndSessionDuration is: " << tradeParam.fEndSessionDuration << endl;
				break;
			case 'T':
				tradeParam.fPriceTolerance = atof(argv[i]);
				cout << "Value of tradeParam.fPriceTolerance is:" << tradeParam.fPriceTolerance << endl;
				break;
			case 't':
				tradeParam.fDbUpdateTime = argv[i];
				cout << "Value of tradeParam.fDbUpdateTime is:" << tradeParam.fDbUpdateTime << endl;
				break;
			case 'u':
				ExitWithHelp();
				break;
			case '0':
				algoParam.fDataPointCount = atoi(argv[i]);
				cout << "Value of algoParam.fDataPointCount is: " << algoParam.fDataPointCount << endl;
				break;
			case '1':
				algoParam.fPercentile = atoi(argv[i]);
				cout << "algoParam.fPercentile:" << algoParam.fPercentile << endl;
				break;
			case '2':
				algoParam.fPercentileWindowSize = atoi(argv[i]);
				cout << "algoParam.fPercentileWindowSize:" << algoParam.fPercentileWindowSize << endl;
				break;
			case '3':
				algoParam.fVolWindowSize = atoi(argv[i]);
				cout << "algoParam.fVolWindowSize:" << algoParam.fVolWindowSize << endl;
				break;
			case '4':
				algoParam.fConstantRho = atof(argv[i]);
				cout << "algoParam.fConstantRho:" << algoParam.fConstantRho << endl;
				break;
			case '5':
				algoParam.fPriceRangeWindowSize = atoi(argv[i]);
				cout << "algoParam.fPriceRangeWindowSize:" << algoParam.fPriceRangeWindowSize << endl;
				break;
			case '6':
				algoParam.fMktVarianceDays = atoi(argv[i]);
				cout << "algoParam.fMktVarianceDays:" << algoParam.fMktVarianceDays << endl;
				break;
			case '7':
//				algoParam.fRiskExposure = atoi(argv[i]);
				algoParam.fUniversalRiskExposure = atoi(argv[i]);
//				cout << "algoParam.fRiskExposure:" << algoParam.fRiskExposure << endl;
				cout << "algoParam.fUniversalRiskExposure:" << algoParam.fUniversalRiskExposure << endl;
				for(int index = 0; index < tradeParam.fPortfolioCount; index++){
//					param.fRiskExposure[index] = 100000;
					algoParam.fRiskExposure[index] = algoParam.fUniversalRiskExposure;
					cout << "algoParam.fRiskExposure[" << index << "]:" << algoParam.fRiskExposure[index] << endl;
				}
				break;
			default:
				fprintf(stderr,"Unknown option: -%c\n", argv[i-1][1]);
				ExitWithHelp();
		}
	}
//	AliGlobalFunctions::SVMSetPrintStringFunction(printFunc);

	// determine filenames

/****	if(i>=argc)
		ExitWithHelp();****/
	if(i>=argc){
		return;
	}
	strcpy(inputFileName, argv[i]);

/*	if(i<argc-1)
		strcpy(modelFileName,argv[i+1]);
	else
	{
		char *p = strrchr(argv[i],'/');
		if(p==NULL)
			p = argv[i];
		else
			++p;
		sprintf(modelFileName,"%s.model",p);
	}*/
}

void ExitWithHelp()
{
	cout << "Usage: PosixSocketClientTest [options] portfolio_file\n";
	cout << "options:\n";
	cout << "-E end_session_duration : set the duration(in seconds) of the session in which market-orders are placed.(default 60)\n";
	cout << "-T price_tolerance : set the price tolerance for limit-orders.(default 0.05)\n";
	cout << "-S session_duration : set the duration(in seconds) of the single trade-session.(default 600)\n";
	cout << "-c capital_per_stock : set the amount of capital for each instrument in a portfolio.(default 135000)\n"; 
	cout << "-h host : set the address of the host to connect.\n";
	cout << "-i client_id : set the client-id.(default 0)\n";
	cout << "-n portfolio_count : set the no. of portfolios.(default 3) \n";
	cout << "-p port : set the port to connect.(default 7496)\n";
	cout << "-s portfolio_size : set the size of the portfolio.(default 7)\n";
	cout << "-t database_update_time : set the day-time(in 24-Hour Format) to update the database.(default 15:45)\n";
	cout << "-u help: to print 'Usage'.\n";
	cout << "-0 datapoint_count : set the no. of datapoints to predict the portfolio-exposure.(default 251)\n"; 
	cout << "-1 percentile:	set the n-percentile for algorithm.(default 80)\n";
	cout << "-2 percentile_window_size: set the window-size for percentile computation.(default 21).\n";
	cout << "-3 vol_window_size: set the window-size for volatility calculation.(default 15).\n";
	cout << "-4 constant_rho: set the rho.(default 3.0)\n";
	cout << "-5 price_range_window_size: set the rolling window-size to compute average price-range.(default 5).\n";
	cout << "-6 market_variance_days: set the no. of days to determine market-variation.(default 63)\n";
	cout << "-7 risk_exposure: set the risk-exposure.(default 100000).\n";
	exit(1);
}

void FillPortfolioPosition(int* position, int portfolioSize, int portfolioCount){

	fstream dataFile;
	dataFile.open("portfolioPositionExample.dat", ios::in);
	char* line = new char[256];

// Fill the array 'portfolioPosition' assuming 'portfolioPositions.dat'
// is a '7X3' Matrix, where, '7' is the count of futures in each port-
// folio.
// -------------------------------------------------------------------

	for(int portfolioId = 0; portfolioId < portfolioCount; portfolioId++)
	{
		cout << "Value of 'portfolioId' is: " << portfolioId << endl;
		dataFile.getline(line, 256);
		cout << "Value of 'line' is: " << line << endl;

		char* token = strtok(line, "\t");
		cout << "Value of 'token' is: " << token << endl;
			
		for(int stockId = portfolioSize*portfolioId; stockId < portfolioSize + portfolioSize*portfolioId; stockId++)
		{
					position[stockId] = atoi(token);
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  position[stockId] << endl;
					token = strtok(NULL, "\t");
		}
	}

/*
//Sort the portfolio in the increasing order.
//-------------------------------------------------------------------
	int instrumentCount = portfolioSize*portfolioCount;
	unsigned int* absPosition = new unsigned int[instrumentCount];
	for(int i = 0; i < instrumentCount; i++){//computes the absolute of 'position' vector.
		absPosition[i] = abs(position[i]);
	}

	
	vector<int> myvector (absPosition, absPosition+instrumentCount);               // 32 71 12 45 26 80 53 33
	vector<int>::iterator it;

// using default comparison (operator <):
	sort(myvector.begin(), myvector.begin()+instrumentCount);           //(12 32 45 71)26 80 53 33

// print out content:
	cout << "myvector contains:";
	for(it=myvector.begin(); it!=myvector.end(); ++it){
		cout << " " << *it;
	}

	cout << endl;

	
	cout << "Maximum Exposure is: " << myvector.back() << endl;*/
}
	
/*void FillPortfolioPosition(int* portfolioPosition){
//Fill in the portfolio structure.
//	PortfolioPosition* portfolio;
//	int* portfolioPosition;	//array containing the 'Desired Exposure'.

//	portfolio = new PortfolioPosition[3];
//	portfolioPosition = new int[21];	//allocate the memory to hold the 'Desired exposure' for 'n' no. of Futures.

	fstream dataFile;
	dataFile.open("portfolioPositions.dat", ios::in);
	char* line = new char[256];

// Fill the array 'portfolioPosition' assuming 'portfolioPositions.dat'
// is a '7X3' Matrix, where, '7' is the count of futures in each port-
// folio.
// -------------------------------------------------------------------

//	for(int portfolioId = 0; portfolioId < 3; portfolioId++)
	for(int portfolioId = 0; portfolioId < 1; portfolioId++)
	{
		cout << "Value of 'portfolioId' is: " << portfolioId << endl;
		dataFile.getline(line, 256);
		cout << "Value of 'line' is: " << line << endl;

		char* token = strtok(line, "\t");
		cout << "Value of 'token' is: " << token << endl;
			
//		int i = 0;

//		while(line != NULL)
		for(int stockId = 0; stockId < 7; stockId++)
		{
			switch(stockId){
				case 0:
//					portfolio.p0 = atoi(token);
//					portfolio[portfolioId].p0 = atoi(token);
					portfolioPosition[stockId] = atoi(token);
//					cout << "Value of 'portfolio[portfolioId].p0' is: " <<  portfolio[portfolioId].p0 << endl;
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  portfolioPosition[stockId] << endl;
					token = strtok(NULL, "\t");
					break;
				case 1:
//					portfolio.p1 = atoi(token);
//					portfolio[portfolioId].p1 = atoi(token);
					portfolioPosition[stockId] = atoi(token);
//					cout << "Value of 'portfolioPosition[portfolioId].p1' is: " <<  portfolioPosition[portfolioId].p1 << endl;
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  portfolioPosition[stockId] << endl;
					token = strtok(NULL, "\t");
					break;
				case 2:
//					portfolio.p2 = atoi(token);
//					portfolio[portfolioId].p2 = atoi(token);
					portfolioPosition[stockId] = atoi(token);
//					cout << "Value of 'portfolio[portfolioId].p2' is: " <<  portfolio[portfolioId].p2 << endl;
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  portfolioPosition[stockId] << endl;
					token = strtok(NULL, "\t");
					break;
				case 3:
//					portfolio.p3 = atoi(token);
//					portfolio[portfolioId].p3 = atoi(token);
					portfolioPosition[stockId] = atoi(token);
//					cout << "Value of 'portfolio[portfolioId].p3' is: " <<  portfolio[portfolioId].p3 << endl;
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  portfolioPosition[stockId] << endl;
					token = strtok(NULL, "\t");
					break;
				case 4:
//					portfolio.p4 = atoi(token);
//					portfolio[portfolioId].p4 = atoi(token);
					portfolioPosition[stockId] = atoi(token);
//					cout << "Value of 'portfolio[portfolioId].p4' is: " <<  portfolio[portfolioId].p4 << endl;
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  portfolioPosition[stockId] << endl;
					token = strtok(NULL, "\t" );
					break;
				case 5:
//					portfolio.p5 = atoi(token);
//					portfolio[portfolioId].p5 = atoi(token);
					portfolioPosition[stockId] = atoi(token);
//					cout << "Value of 'portfolio[portfolioId].p5' is: " <<  portfolio[portfolioId].p5 << endl;
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  portfolioPosition[stockId] << endl;
					token = strtok(NULL, "\t");
					break;
				case 6:
//					portfolio.p6 = atoi(token);
//					portfolio[portfolioId].p6 = atoi(token);
					portfolioPosition[stockId] = atoi(token);
//					cout << "Value of 'portfolio[portfolioId].p6' is: " <<  portfolio[portfolioId].p6 << endl;
					cout << "Value of 'portfolioPosition[stockId]' is: " <<  portfolioPosition[stockId] << endl;
					token = strtok(NULL, "\t");
					break;
			}
//			cout << "Hey!, I am out of 'switch' block." << endl;
		}

	}
}*/

//void ReadConfigurationFile(){
//void ReadConfigurationFile(char * portfolioFileName){
void ReadConfigurationFile(char * confFileName){
	cout << "In Method: 'ReadConfigurationFile(char * portfolioFileName, char * confFileName)'" << endl;
	cout << "*********************************************************************************" << endl;


	
//Default Values.
//--------------------------------------------------------------------
	if(tradeParam.fHost==NULL){
// Default 'connection' parameters.
		tradeParam.fHost = new char[16];
//	tradeParam.fHost = "";
//	sprintf(tradeParam.fHost,"");
		tradeParam.fPort = 7496;
		tradeParam.fClientId = 0;

// Default 'trade-session' parameters.
		tradeParam.fPortfolioSize = 7;
		tradeParam.fPortfolioCount = 3;
		tradeParam.fSessionDuration = 600;	//in seconds.
		tradeParam.fCapitalPerStock = 100000;
		tradeParam.fEndSessionDuration = 60;	//in seconds.
		tradeParam.fPriceTolerance = 0.05; 
		tradeParam.fDbUpdateTime = new char[8];
		strcpy(tradeParam.fDbUpdateTime, "15:45");	//Time Format: '24-hour'
		cout << "tradeParam.fDbUpdateTime:" << tradeParam.fDbUpdateTime << endl;
	
// Default 'Algorithm' parameters.
//		tradeParam.fAlgoDataPointCount = 251;
		algoParam.fDataPointCount = 251;
		algoParam.fPercentile = 80;
		algoParam.fPercentileWindowSize = 21;
		algoParam.fVolWindowSize = 15;
		algoParam.fConstantRho = 3.0;
		algoParam.fPriceRangeWindowSize = 5;
		algoParam.fMktVarianceDays = 63;
//		algoParam.fRiskExposure = 100000;
//		algoParam.fRiskExposure = new long int[portfolioCount];
		algoParam.fRiskExposure = new long int[tradeParam.fPortfolioCount];
//		for(int index = 0; index < portfolioCount; index++){
		for(int index = 0; index < tradeParam.fPortfolioCount; index++){
			algoParam.fRiskExposure[index] = 100000;
			cout << "algoParam.fRiskExposure[" << index << "]:" << algoParam.fRiskExposure[index] << endl;
		}
	}


//Read the 'Algo-Parameters' from File: 'algo.conf'.
//--------------------------------------------------------------------
////	int portfolioCount = 3;
//	int parameterCount = 7 + portfolioCount;
//	int parameterCount = 7 + 1;
//	int parameterCount = 16;
	int parameterCount = 17;

	int parameterLength = 64;

//Create the 2-Dimensional Array: 'parameter'
//---------------------------------------------------------------------
	
	char * * parameter = new char * [parameterCount];
	for(int i = 0; i < parameterCount; i++){
		parameter[i] = new char[parameterLength]; 
	}

//Fill in the Array: 'parameter'
	for(int index = 0; index < parameterCount; index++){
		switch(index){
			case TRADE_ENDSESSIONDURATION: 
				sprintf(parameter[index],"end_session_duration");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case TRADE_PRICETOLERANCE: 
				sprintf(parameter[index],"price_tolerance");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case TRADE_SESSIONDURATION: 
				sprintf(parameter[index],"session_duration");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case TRADE_HOST: 
				sprintf(parameter[index],"host");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case TRADE_CLIENTID: 
				sprintf(parameter[index],"client_id");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case TRADE_PORTFOLIOCOUNT: 
				sprintf(parameter[index],"portfolio_count");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case TRADE_PORT: 
				sprintf(parameter[index],"port");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case TRADE_PORTFOLIOSIZE: 
				sprintf(parameter[index],"portfolio_size");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case ALGO_DATAPOINTCOUNT: 
//				parameter[index]  = "datapoint_count";
				sprintf(parameter[index],"datapoint_count");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			
			case ALGO_PERCENTILE: 
//				parameter[index]  = "percentile";
				sprintf(parameter[index],"percentile");
				break;
			case ALGO_PERCENTILEWINDOWSIZE: 
//				parameter[index]  = "percentile_window_size";
				sprintf(parameter[index],"percentile_window_size");
				break;			
			case ALGO_VOLWINDOWSIZE: 
//				parameter[index]  = "vol_window_size";
				sprintf(parameter[index],"vol_window_size");
				break;			
			case ALGO_CONSTANTRHO: 
//				parameter[index]  = "constant_rho";
				sprintf(parameter[index],"constant_rho");
				break;
			case ALGO_PRICERANGEWINDOWSIZE: 
//				parameter[index]  = "price_range_window_size";
				sprintf(parameter[index],"price_range_window_size");
				break;
			case ALGO_MARKETVARIANCEDAYS: 
//				parameter[index]  = "market_variance_days";
				sprintf(parameter[index],"market_variance_days");
				break;
			case ALGO_RISKEXPOSURE: 
//				parameter[index] = "risk_exposure";
				sprintf(parameter[index],"risk_exposure");
				break;
			case DB_UPDATETIME: 
				sprintf(parameter[index],"database_update_time");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;			

		}
	}

//x-------------------------------------------------------------------x

//Read the Configuration File: 'algo.conf'
//---------------------------------------------------------------------

	fstream algoFile;

//	algoFile.open("algo.conf", ios::in);
/****	char * confFileName = new char[64];
	strcpy(confFileName, user->pw_dir);
	strcat(confFileName, "/NeuralAlgo/algo.conf");****/

	cout << "confFileName:" << confFileName << endl;

	algoFile.open(confFileName, ios::in);

	if(!algoFile.is_open()){
//		cout << "Failed to open File: 'algo.conf'." << endl;
		cout << "Failed to open File: " << confFileName << endl;
		exit(1);
	}
	
//	int lineLength = 512;
	int lineLength = 10240;
	char * line = new char[lineLength];

	int lineCount = 0;

//	Parameter param;
////	AlgoParameter algoParam;
////	TradeParameter tradeParam;
	
	if(algoFile.is_open()){

		while(algoFile.good()){
///		while(!(algoFile.eof()))
			lineCount++;
//		for(int lineCount = 0; lineCount < parameterCount; lineCount++)
////			cout << "Reading Line No.: " << lineCount << " ..." << endl;
////			cout << "tradeParam.fHost:" << tradeParam.fHost << endl;
			algoFile.getline(line, lineLength);
////			cout << "Value of 'line' is: " << line << endl;
				char * token = strtok(line, "=");
////			cout << "tradeParam.fHost:" << tradeParam.fHost << endl;
				if(token!=NULL){
////				cout << "Value of 'token' is: " << token << endl;
	
				for(int index = 0; index < parameterCount; index++){
  					if(!(strcmp(token, parameter[index]))){
//  					if(!(strncmp(parameter[index],token,20)))
//  					if(!(strcmp("datapoint_count",token)))
//  					if((parameter[index]==token))
						cout << "strcmp(token, parameter[index])=" << strcmp(token, parameter[index]) << endl;
//						cout << "strcmp(parameter[index],token)=" << strcmp(parameter[index],token) << endl;
//						cout << "strncmp(parameter[index],token,20)=" << strncmp(parameter[index],token,20) << endl;
//						cout << "strncmp(parameter[" << index << "],token,20)=" << strncmp(parameter[index],token,20) << endl;
//						cout << "Condition '(parameter[index]==token)' evaluated True" << endl;
						cout << "Matched Line Count is: " << lineCount << endl;
						switch(index){
							case TRADE_ENDSESSIONDURATION: 
								tradeParam.fEndSessionDuration = atoi(strtok(NULL, "="));
								cout << "tradeParam.fEndSessionDuration:" << tradeParam.fEndSessionDuration << endl;
								break;			
							case TRADE_PRICETOLERANCE: 
								tradeParam.fPriceTolerance = atof(strtok(NULL, "="));
								cout << "tradeParam.fPriceTolerance:" << tradeParam.fPriceTolerance  << endl;
								break;			
							case TRADE_SESSIONDURATION: 
								tradeParam.fSessionDuration = atoi(strtok(NULL, "="));
								cout << "tradeParam.fSessionDuration:" << tradeParam.fSessionDuration << endl;
								break;			
							case TRADE_HOST: 
//								tradeParam.fHost = strtok(NULL, "=");
////								tradeParam.fHost = new char[16];
								strcpy(tradeParam.fHost, strtok(NULL, "="));
//								sprintf(tradeParam.fHost, strtok(NULL, "="));
								cout << "tradeParam.fHost:" << tradeParam.fHost << endl;
								break;			
							case TRADE_CLIENTID: 
////								cout << "tradeParam.fHost:" << tradeParam.fHost << endl;
								tradeParam.fClientId = atoi(strtok(NULL, "="));
								cout << "tradeParam.fClientId:" << tradeParam.fClientId << endl;
////								cout << "tradeParam.fHost:" << tradeParam.fHost << endl;
								break;			
							case TRADE_PORTFOLIOCOUNT: 
								tradeParam.fPortfolioCount = atoi(strtok(NULL, "="));
								cout << "tradeParam.fPortfolioCount:" << tradeParam.fPortfolioCount << endl;
								break;			
							case TRADE_PORT: 
								tradeParam.fPort = atoi(strtok(NULL, "="));
								cout << "tradeParam.fPort:" << tradeParam.fPort << endl;
								break;			
							case TRADE_PORTFOLIOSIZE: 
								tradeParam.fPortfolioSize = atoi(strtok(NULL, "="));
								cout << "tradeParam.fPortfolioSize:" << tradeParam.fPortfolioSize << endl;
								break;			
							case ALGO_DATAPOINTCOUNT: 
								algoParam.fDataPointCount = atoi(strtok(NULL, "="));
								cout << "algoParam.fDataPointCount:" << algoParam.fDataPointCount << endl;
								break;			
							case ALGO_PERCENTILE: 
								algoParam.fPercentile = atoi(strtok(NULL, "="));
								cout << "algoParam.fPercentile:" << algoParam.fPercentile << endl;
								break;
							case ALGO_PERCENTILEWINDOWSIZE: 
								algoParam.fPercentileWindowSize = atoi(strtok(NULL, "="));
								cout << "algoParam.fPercentileWindowSize:" << algoParam.fPercentileWindowSize << endl;
								break;			
							case ALGO_VOLWINDOWSIZE: 
								algoParam.fVolWindowSize = atoi(strtok(NULL, "="));
								cout << "algoParam.fVolWindowSize:" << algoParam.fVolWindowSize << endl;
								break;			
							case ALGO_CONSTANTRHO: 
								algoParam.fConstantRho = atof(strtok(NULL, "="));
								cout << "algoParam.fConstantRho:" << algoParam.fConstantRho << endl;
								break;
							case ALGO_PRICERANGEWINDOWSIZE: 
								algoParam.fPriceRangeWindowSize = atoi(strtok(NULL, "="));
								cout << "algoParam.fPriceRangeWindowSize:" << algoParam.fPriceRangeWindowSize << endl;
								break;
							case ALGO_MARKETVARIANCEDAYS: 
								algoParam.fMktVarianceDays = atoi(strtok(NULL, "="));
								cout << "algoParam.fMktVarianceDays:" << algoParam.fMktVarianceDays << endl;
								break;
							case ALGO_RISKEXPOSURE:
								{
									char * riskExposure = strtok(NULL, "=");
									cout << "Value of 'riskExposure' is: " << riskExposure << endl;
/****									int tokenCount = 0;
									if(strtok(riskExposure,":")!=0){ tokenCount++;cout << "tokenCount:" << tokenCount << endl;}
									while((strtok(NULL,":")!=0)){tokenCount++; cout << "tokenCount:" << tokenCount << endl;}
									algoParam.fRiskExposure = new long int[tokenCount];****/
									int charCount = 0;
									char * pch = strpbrk(riskExposure, ":");
									while(pch!=NULL){//count the occurences of the Character: ':'
										charCount++;
										cout << "charCount:" << charCount << endl;
										pch = strpbrk(pch+1,":");
									}
//									for(int index = 0; index < tokenCount; index++)
									for(int index = 0; index < charCount + 1; index++){
										if(index==0){
											algoParam.fRiskExposure[index] = atoi(strtok(riskExposure, ":"));
											cout << "Value of 'riskExposure' is: " << riskExposure << endl;
											cout << "algoParam.fRiskExposure[" << index << "]:" << algoParam.fRiskExposure[index] << endl;
										}
										else{
											algoParam.fRiskExposure[index] = atoi(strtok(NULL, ":"));
											cout << "algoParam.fRiskExposure[" << index << "]:" << algoParam.fRiskExposure[index] << endl;
										}
									}
								}
								break;
							case DB_UPDATETIME: 
								tradeParam.fDbUpdateTime = strtok(NULL, "=");
								cout << "tradeParam.fDbUpdateTime:" << tradeParam.fDbUpdateTime << endl;
								break;
						}
					}
			
				}
			}
		}
		algoFile.close();
	}

	else{}
		
//x-------------------------------------------------------------------x

	cout << "Method: 'ReadConfigurationFile(char * portfolioFileName, char * confFileName)' ends." << endl;
	cout << "x----------------------------------------------------------------------------------x" << endl;
}
