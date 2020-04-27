/*
 Centralized Multi-User Concurrent
 Bank Account Manager.
 Name: server
 Features: Multi-Threaded
           Concurrent
           Scalable

*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

using namespace std;


//structure which stores transactions of each client.
struct client_transactions
{
	float start_time;
	long account_number;
	char txn_type;
	double amount;
}txn;


//structure which stores bank records of all bank members.
struct bank_account
{
	int account_number;
	string name;
	double balance;
    //mutex to appply locks
	pthread_mutex_t lock;
}account[10000];


//variable which will store number of accounts in the sever
int no_of_accounts;
//temp variable
float temp_var;

//function to print error messages
void print_error(const char *msg)
{
	perror(msg);
	exit(1);
}

char buffer[256];
int n;

//funtion to handle threads for every client request

void *InitializeThread(void *newsocketfd)
{
	
	client_transactions tmp;


	while(1) {
		bzero(buffer,256);
		read((long)newsocketfd,buffer,255);
		int status = strcmp(buffer,"end");
        //If the client sends "end" message, server will stop.
		if(status==0){
			break;
		}
		string records(buffer);
        istringstream stringstream(records);
        stringstream >> tmp.start_time >> tmp.account_number >> tmp.txn_type >> tmp.amount;

            int flag = 0;
            temp_var = 0;
            istringstream txn_line(records);
            txn_line >> txn.start_time >> txn.account_number >> txn.txn_type>> txn.amount;
            for(int i=0;i<no_of_accounts;i++)
            {
                if(account[i].account_number==txn.account_number)
                {
                     //Account number found
                    flag = 1;
                    
                    //check condition: whther deposit request or withdraw request
                    if(txn.txn_type=='d')
                    {
                         //lock the criticial section while performing the transaction
                        pthread_mutex_lock(&account[i].lock);
                        account[i].balance = account[i].balance + txn.amount;
                        temp_var = account[i].balance;
                        //unlocking the critical section
                        pthread_mutex_unlock(&account[i].lock);
                        
                        cout << "++Data received from client " << (long)newsocketfd << " : " << tmp.account_number << " " << tmp.txn_type << " " << tmp.amount << " Transaction Successful | Current Balance: " << temp_var << endl;
                            n = write((long)newsocketfd,"++Transaction completed",23);
                            if (n < 0) print_error("ERROR while writing to socket");
                    }
                    else    //withdrawal request
                    {
                        if(account[i].balance < txn.amount)    //insufficient balance
                        {
                            cout << "++Data received from client " << (long)newsocketfd << " : " << tmp.account_number << " " << tmp.txn_type << " " << tmp.amount << " [Failure : Insufficient Funds]" << endl;
                            n = write((long)newsocketfd,"++Transaction failed : Cannot withdraw due to Insufficient Funds",64);
                            if (n < 0) print_error("ERROR while writing to socket");
                        }
                        else
                        {    
                          //lock the criticial section while performing the transaction
                            pthread_mutex_lock(&account[i].lock);
                            account[i].balance = account[i].balance - txn.amount;
                            temp_var = account[i].balance;
                            //unlocking the critical section
                            pthread_mutex_unlock(&account[i].lock);
                           
                            cout << "++Data received from client " << (long)newsocketfd << " : " << tmp.account_number << " " << tmp.txn_type << " " << tmp.amount << " Transaction Successful | Current Balance: " << temp_var << endl;
                                n = write((long)newsocketfd,"++Transaction completed",23);
                                if (n < 0) print_error("ERROR while writing to socket");
                           
                        }
                    }
                }
                else
                {
                    continue;
                }
            }
        
        //Account number not found in records
        if(flag==0)
        {
            cout << "++Data received from client " << (long)newsocketfd << " : " << tmp.account_number << " " << tmp.txn_type << " " << tmp.amount << " [Failure : Invalid Account Number, Please re-check]" << endl;
        n = write((long)newsocketfd,"++Transaction failed : Account number does not exist",55);
        if (n < 0) print_error("ERROR while writing to socket");
    }
        

	}

	n = write((long)newsocketfd,"++All transactions completed",28);
	if (n < 0) print_error("ERROR while writing to socket");
	close((long)newsocketfd);
}

//function to add interest periodically

void *PeriodicInterest(void * args)
{

	float interest_rate = 1.1;
	while(1)
	{ //for every 10 secs
		sleep(10);
		for(int i=0; i<no_of_accounts; i++)
		  {
		    float temp_var = 0 ;
			if(account[i].balance >= 1000)
			{
			pthread_mutex_lock(&account[i].lock);
			account[i].balance = account[i].balance * interest_rate;
			temp_var = account[i].balance;
			pthread_mutex_unlock(&account[i].lock);
            cout << "++interest deposited in Bank- Amount:10pc,Acc.No: " << account[i].account_number << ". Current Balance: " << temp_var << endl;
	}
			else
				continue;
		}
	}
}

// main: execution starts from here
int main(int argc, char *args[])
{
    int port_tmp = 1;
	struct sockaddr_in server_address, client_address;
	long socketfd, newsocketfd[200], portno;
	socklen_t clientlength;
	// for counting number of transactions in server
	int count=0;	

	//Opening the Records.txt file
	ifstream records_file("Records.txt");
	
	string record;
 
     //Reading file line by line
    
	while(getline(records_file, record))
	{
		istringstream txn_line(record);
		txn_line >> account[count].account_number >> account[count].name >> account[count].balance;
		count++;
	}
    
    //update every time new record was found
	
	no_of_accounts = count;

	cout << "Initial Records:" << endl;
	for(int i=0;i<no_of_accounts;i++)
	{
		cout << account[i].account_number << "	" << account[i].name << "	" << account[i].balance << endl;
        
		//Initializing all mutex objects for each account
		if(pthread_mutex_init(&account[i].lock, NULL) != 0)
		{
			cout << "Initializing mutex has failed" << endl;
		}

	}	

//thread for periodically adding interest to all acounts
	pthread_t interest;
//threads for handling client requests
	pthread_t threads;


	if (argc < 2) {
		fprintf(stderr,"Please give a valid port\n");
		exit(1);
	}

	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	//returns -1 if error
	if(socketfd < 0)
		print_error("ERROR while opening socket");

	bzero((char *) &server_address, sizeof(server_address));
	portno = atoi(args[1]);
	
	//running on local host
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	//binding to the port received from arguments
	server_address.sin_port = htons(portno);
	
 //forcing the server to reuse the port incase if it's already using the same port
    
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&port_tmp, sizeof(port_tmp)) < 0)
        print_error("setsockopt(SO_REUSEADDR) failed");
    
#ifdef SO_REUSEPORT
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&port_tmp, sizeof(port_tmp)) < 0)
        print_error("setsockopt(SO_REUSEPORT) failed");
#endif
    

	if(bind(socketfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
		print_error("ERROR while binding");
     	
	listen(socketfd,100);
	clientlength = sizeof(client_address);
	
	int interest_status = pthread_create(&interest, NULL, PeriodicInterest, NULL);


	cout << "Transaction Logs:" << endl;
	
	
	int socket_index = 0;
	
	int n = 200;

    //Server listening to client requests continuously
	while(1)
	{	
	//accept call: which accepts connections from clients
		newsocketfd[socket_index] = accept(socketfd,(struct sockaddr *) &client_address,&clientlength);

		pthread_create(&threads, NULL, InitializeThread, (void *)newsocketfd[socket_index]);
		
		socket_index=(socket_index+1)%n;
	}

}

