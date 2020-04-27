/*

Name: client
purpose: sends requests to server after reading Transactions.txt file by creating a new socket"

*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <ctime>
#include <netinet/in.h>
#include <netdb.h> 
#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;
/* Structure to store transaction details */
struct client_transactions
{
	float start_time;
	int account_number;
	char txn_type;
	double amount;
}txn[10000];

//function to print error message
void print_error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *args[])
{
    int no_of_txns;
	int count_txns=0;
	long socketfd, port_num, status;
	struct sockaddr_in server_address;
	struct hostent *server;
	//to display the total time after all transactions
	double total_time = 0;
    //variable for storing timestamp of each transaction and sending rthe equest to server at that timestamp
	float time_stamp=0;
	string record;

	
    // running client takes three arguments
	if(argc < 3) {
		fprintf(stderr,"please provide the required arguments, usage %s hostname port\n", args[0]);
		exit(0);
	}
	char buffer[256];
    server = gethostbyname(args[1]);
	port_num = atoi(args[2]);
	
	
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if(socketfd < 0)
		print_error("error while opening socket");	

		
	server_address.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
	server_address.sin_port = htons(port_num);
	if(connect(socketfd,(struct sockaddr *) &server_address,sizeof(server_address)) < 0)
		print_error("error while connecting");
    //reading file record wise
	//Opening the Transactions file
	ifstream txn_file("Transactions.txt");
	bzero((char *) &server_address, sizeof(server_address));
	no_of_txns = count_txns;

   while(getline(txn_file, record))
   {
         //fillling the buffer with zeros
		bzero(buffer,256);
		strcpy(buffer,record.c_str());			
		istringstream string_stream(record);
        //display
		string_stream >> txn[count_txns].start_time >> txn[count_txns].account_number >> txn[count_txns].txn_type >> txn[count_txns].amount;    
		if(count_txns==0){
			time_stamp = txn[count_txns].start_time;
			cout << "++Timestamp: " << time_stamp <<" > for transaction: " << txn[count_txns].account_number << " " << txn[count_txns].txn_type << " " << txn[count_txns].amount;
            //sleep until next timestamp
			usleep(time_stamp);	
		}
		else{
            //current timestamp with respect to timestamp of previous transaction
			time_stamp = (txn[count_txns].start_time - txn[count_txns-1].start_time);
			cout << "++Timestamp: " << time_stamp <<" > for transaction: " << txn[count_txns].account_number << " " << txn[count_txns].txn_type << " " << txn[count_txns].amount;
			usleep(time_stamp);
		}	
		count_txns++;
       //start time of each transaction
		float start_time=clock();
		status = write(socketfd,buffer,strlen(buffer));
		if (status < 0)
			print_error("error while writing to socket");
		bzero(buffer,256);
        //Read the result from server
		status = read(socketfd,buffer,255);	
		float stop_time=clock();	
		double transaction_time = (stop_time-start_time)*1000/double(CLOCKS_PER_SEC);
		cout << "Transaction Time: " << transaction_time << " ";
		if(status < 0)
			print_error("error while reading from socket");	
		cout << buffer << endl;		
		total_time = total_time + transaction_time;
		
	}

	bzero(buffer,256);
    //sending end value to server for terminating the connection
	strcpy(buffer,"end");
	status = write(socketfd,buffer,strlen(buffer));
	if (status < 0)
		print_error("eror while writing to socket");
	bzero(buffer,256);
	status = read(socketfd,buffer,255);	
	if(status < 0)
		print_error("eror while reading from socket");
	cout << "Time taken for All Transactions is:" << total_time << "secs" << endl;
	cout << buffer << endl;
	close(socketfd);
	return 0;
}

