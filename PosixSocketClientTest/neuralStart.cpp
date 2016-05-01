#include <iostream>
#include <fstream>
#include <stdio.h>	//functions: 'remove', 'rename' 
#include <stdlib.h>	//function: 'exit', 'getenv'
#include <sys/types.h>	//function: 'getpwuid'
#include <pwd.h>	//struct: 'passwd'
#include <string.h>	//functions: 'strcpy', 'strcat'

using namespace std;

enum Parameters{
	NEURAL_STARTTIME
};
void ReadConfigurationFile(char * startTime);

int main(){

	char * startTime = new char[5];
////	cout << "Calling Method: 'ReadConfigurationFile' ..." << endl;
	ReadConfigurationFile(startTime);
//	cout << "startTime:" << startTime << endl;
	cout << "Start Time of Algo is:" << startTime << endl;

	int replaceLineCount = 23;	// Count of the line to replace.
	cout << "Replacement Line No.: " << replaceLineCount << endl;

	
	char * fileName = new char[64];
	strcpy(fileName, "/etc/crontab");
////	cout << "fileName:" << fileName << endl;


//Replace first five charactes of Line No.: 'replaceLineCount' from the File: 'crontab'.
//---------------------------------------------------------------------
	fstream file;	//File Handle
	file.open(fileName, ios::in|ios::out|ios::binary);
	
	if(!file.is_open()){
//		cout << "File: " << fileName << " failed to open." << endl;
		cout << "File failed to open." << endl;
		cout << "Usage: sudo neuralStart" << endl;
		exit(1);
	}

	int lineLength = 512;
	char * line = new char[lineLength];
	
	int lineCount = 0;

//	char * readBlock = new char[1024];

//	char * writeBlock = new char[1024];
//	char * writeBlock = new char[66];
	char * writeBlock = new char[5];
//	strcpy(writeBlock, "15 20	* * 1-5	garv	cd /home/garv/NeuralAlgo && neural > output.log\n");	// line length = 66 characters
	strcpy(writeBlock, "09 32");	// line length = 5 characters

	while(file.good()){
		int position = file.tellg();
////		cout << "position:" << position << endl;

		file.getline(line,lineLength);
////		cout << "Value of 'line' is: " << line << endl;

		lineCount++;

		if((lineCount == (replaceLineCount))){//Replace the line.
			cout << "Replacing the first 5 characters of the Line:\n" << line << endl;
////			file.read(readBlock, 1024);
////			cout << "readBlock:" << readBlock << endl; 
//			file << "15 20	* * 1-5	garv	cd /home/garv/NeuralAlgo && neural > output.log" << endl;
			file.seekp(position,ios::beg);
//			file.write(writeBlock,64);
//			file.write(writeBlock,5);
//			file.write(startTime,5);
			file.write(&startTime[3],2);
			file.seekp(1,ios::cur);
			file.write(&startTime[0],2);

			file.seekg(position,ios::beg);
			file.getline(line,lineLength);
			cout << "Edited Line is:\n" << line << endl;
			file.close();
			break;
		}
	}
//<------------------------------------------------------------------>
//
	return 0;
}

void ReadConfigurationFile(char * startTime){//read configuration File: 'algo.conf'
	
	cout << "In Method: 'ReadConfigurationFile'" << endl;
	cout << "-------------------------------->" << endl;

	int parameterCount = 1;
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
			case NEURAL_STARTTIME: 
//				parameter[index]  = "datapoint_count";
				sprintf(parameter[index],"neural_start_time");
				cout << "parameter[" << index << "]:" << parameter[index] << endl;
				break;
		}
	}

//x-------------------------------------------------------------------x

//Read the Configuration File: 'algo.conf'
//---------------------------------------------------------------------

	fstream algoFile;

//Find the Value of Environment Variable: $Home.
//---------------------------------------------------------------------
/****	struct passwd *user;	//man getpwuid
	user = getpwuid(getuid());	//Get the uid of the running process and use it to get a record from /etc/passwd
	printf("The Home Directory is: %s\n", user->pw_dir);****/


	char * configFileName = new char[64];
//	strcpy(configFileName, user->pw_dir);
	strcpy(configFileName, getenv("HOME"));
	strcat(configFileName, "/NeuralAlgo/algo.conf");


//	algoFile.open("algo.conf", ios::in);
	algoFile.open(configFileName, ios::in);

	if(!algoFile.is_open()){
		cout << "File: " << configFileName << " failed to open." << endl;
		exit(1);
	}
	
	int lineLength = 512;
	char * line = new char[lineLength];

	int lineCount = 0;

	if(algoFile.is_open()){

		while(algoFile.good()){
			lineCount++;
////			cout << "Reading Line No.: " << lineCount << " ..." << endl;
			algoFile.getline(line, lineLength);
////			cout << "Value of 'line' is: " << line << endl;
			char * token = strtok(line, "=");
			if(token!=NULL){
////				cout << "Value of 'token' is: " << token << endl;
		
				for(int index = 0; index < parameterCount; index++){
  					if(!(strcmp(token, parameter[index]))){
////						cout << "strcmp(token, parameter[index])=" << strcmp(token, parameter[index]) << endl;
						switch(index){
							case NEURAL_STARTTIME: 
//								startTime = strtok(NULL, "=");
								strcpy(startTime, strtok(NULL, "="));
								cout << "startTime:" << startTime << endl;
								break;			
						}
					}
				}
			}
		}
	}

	cout << "Method: 'ReadConfigurationFile' ends." << endl;
	cout << "x-----------------------------------x" << endl;
}
