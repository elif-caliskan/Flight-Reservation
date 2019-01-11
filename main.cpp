#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <string>

using namespace std;
//this used in reservation queue
//it has integer seat and integer client fields, also it points to its next resevation
struct Reservation 
{ 
  int seat;
  int client; 
  struct Reservation *next; 
}; 
//write_mutex is used when adding to reserveQueue
//a server cannot pop from the queue when a new reservation is appending and vice versa
pthread_mutex_t write_mutex= PTHREAD_MUTEX_INITIALIZER;
//when a client wants an available seat but there is also another client who wants the same seat,
//the lock is taken for that specific seat
pthread_mutex_t seat_lock[100];
int client_num;
//reseveQueue is a linkedlist
struct Reservation* reserveQueue=NULL;
//this function is run when a server thread is created its parameter is the id of its client
void *printServer(void *threadid){
    long id;
    id = (long)threadid;
    //it always checks if the first reservation in the queue is its client's reservation
    //when it is found, queue cannot be changed by any other threads, the first reservation is popped and output is written
    //after pop operation the server thread is terminated
    while(true){
	if(reserveQueue!=NULL && reserveQueue->client==id){
	    pthread_mutex_lock(&write_mutex);
	    cout<<"Client"<<(id+1)<<" reserves Seat"<<(reserveQueue->seat+1)<<endl;
	    struct Reservation *temp=reserveQueue;
	    reserveQueue=reserveQueue->next;
	    delete temp;
	    pthread_mutex_unlock(&write_mutex);
	    pthread_exit(0);
	}
    }
}
//this function is run when a client thread is created its parameter is its id
void *client(void *threadid) {
	long id;
   	id = (long)threadid;
	pthread_t servertid;
	//its serverthread is created
	pthread_create (&servertid, NULL, printServer, (void *)id);
	//after the creation of server thread, the client sleeps for 50-200 miliseconds
	int sltime = rand() % 150+50;
	//this takes microseconds as parameter so we multiply by 1000
	usleep(sltime*1000);
	//a random seat is taken whether it is empty or taken
	int chosenSeat;
	//while loop goes on until an available seat is chosen
	while(true){
        //this loop checks if the seat is empty
        //the loop ends when an empty seat is found
        chosenSeat=rand() % client_num;
        //if the lock is taken, it means the seat is assigned and ready to be added to reservation queue
            if( pthread_mutex_trylock(&seat_lock[chosenSeat])==0 ){
                pthread_mutex_lock(&write_mutex);
	        break;
            }
        }
	//a new reservation is created and appended to queue
	//now there cannot be a writing to or popping from the queue
	struct Reservation *newRes=new Reservation;
	newRes->client=id;
	newRes->seat=chosenSeat;
	newRes->next=NULL;
	if(reserveQueue==NULL){
	    reserveQueue=newRes;
	}
	else{
	    struct Reservation *current=reserveQueue;
	    while(current->next!=NULL)
		current=current->next;
	    current->next=newRes;
	}
	pthread_mutex_unlock(&write_mutex);
	//client waits for the server to finish
	pthread_join(servertid, NULL);
	pthread_exit(0);
}

int main (int argc, const char * argv[]) {
    srandom((unsigned)time(NULL));
    //client_num is the first argument
    client_num=atoi(argv[1]);
    //output is written to output.txt
    string outputFile="output.txt";
    pthread_t threads[client_num];
    int i;
    //mutex for every seat is initialised
    for(i=0;i<100;i++){
	seat_lock[i]=PTHREAD_MUTEX_INITIALIZER;
    }
    freopen(outputFile.c_str(), "w",stdout);
    cout<<"Number of total seats: "<<client_num<<endl;
    //clients are created
    for( i = 0; i < client_num; i++ ) {
      	pthread_create(&threads[i], NULL, client, (void *)i);
    }
    //clients are waited to terminate
    for (i = 0; i < client_num; i++){
	pthread_join(threads[i], NULL);
    }
    cout<<"All seats are reserved."<<endl;
    fclose(stdout);
    pthread_exit(NULL);
}

