#define _POSIX_C_SOURCE 200112L
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>  
#include <sys/poll.h>     

#include <netdb.h>  

#include <semaphore.h>
#include <pthread.h>
 
#include <ctype.h>
#include <limits.h> 
#include <sys/socket.h>
#include <netinet/in.h>      

#define MAX_LENGTH 1024


typedef enum {
    rremote = 0, local, dest } define;


typedef struct type_var {
    define type;
    char message[MAX_LENGTH];
} type_var;

static int counter_val; 
static int count_temp;


static List *list_print; 
static List *list_send; 




 
int check_space(char *point, int length) {
    char *ptr = point;
    int flag = 1;
    for(int i = 0; *ptr != '\0' && i < length; i++, ptr++) {
        if (!isspace(*ptr))
            flag = 0;
    }
    return flag;
}

//  function to convert  char to int 
int convert_to_integer(const char *arr) {
    errno = 0;
    char *temp_ptr;
    int flag;
    
    long val = strtol(arr, &temp_ptr, 0); 
   
    if (errno == ERANGE){
        flag = -1;
        perror("value is -ve");
    }
    if ( arr == temp_ptr){
        flag = -1;
        perror("value is -ve");
    }
    if(*temp_ptr != '\0' && *temp_ptr != '\n' && *temp_ptr != '\r'){
        flag = -1;
        perror("value is -ve");
    }
    if (val > INT_MAX || val < INT_MIN) {
        flag = -2;
        perror("value is -ve");
    }
    else {
        flag = (int) val;
    }
    return flag;
}

int check_local_port(int *port)
{
    if(*port < 1025 || *port > 65535)
        return 0;
    return 1;
}

int check_remote_port(int *remport)
{
    if(*remport < 1025 || *remport > 65535)
        return 0;
    return 1;
}

// Convert my port and dest,port from string to int 
int convert_string_to_integer(char *local_port, char *remote_port, int *local_val, int *remote_val) {
    int point1 = 1;
    int flag = 0; 
    if(!local_val || !remote_val)
       return 1;
    int val = convert_to_integer(local_port);
   
    if (val > 0) { 
        *local_val = val; 
    } else{
        flag = 1;
        point1 =0;
        count_temp++;
    }
       
    
    val=convert_to_integer(remote_port);
    
    if (val < 0) { 
        flag = 1;
        point1 =0;
        count_temp++;
    } else
        *remote_val = val;
   
    
    if (flag == 0) {
          
        if (*local_val < 1025 || *local_val > 65535 )  { 
            flag = 1;
        }
        if(*remote_val < 1025 || *remote_val > 65535){
            flag =1;
        }
    }
    return flag;
}

int check_free_state(int *arg1, int *arg2){
     int flag = 0;
     int *ptr1 = *arg1;
     int *ptr2 = *arg2;
     if(*ptr1 == *ptr2)
         flag = 1;
     return flag;
}
int read_message_from_buffer(char *arr, int size) {
    char *ptr;
    char *ptr2;
    int flag;

    memset(arr, 0, sizeof(ptr));  
    memset(arr, 0, sizeof(ptr2));
    
    if (fgets(arr, size, stdin)) { 
        flag =  0;   
    } else
        flag -1; 
    
    return flag;
}

typedef struct socket_info_create {
    int socket_id;   
    struct addrinfo *remote_address; 
} socket_info_create;

//makes list of adrressed
struct addrinfo *initializing_address_list(int port, char *address) {
     struct addrinfo address1, *address2;
    int ptr1;
    int ptr2;
    char port_arr[6];  
    sprintf(port_arr, "%d", port); 
    
    memset(&address1, 0, sizeof address1); 
    
    if (!address){
        address1.ai_flags = AI_PASSIVE; 
        puts("Welcome, Start chatting");
        ptr1 =0;
        ptr2 =0;
        count_temp++;
    }  
    
    address1.ai_family = AF_INET;      
    address1.ai_socktype = SOCK_DGRAM; 
    
    int val=getaddrinfo(address, port_arr, &address1, &address2); 
   
   if (val  != 0) { 
       ptr2 = 0;
       ptr1 = 1;
       count_temp++; 
        return NULL;
    }
    return address2;
}

//adds new job to the list 
void add_new_job(define type, char *arr, List *temp) {
    int poin1;
    int poin2;
    type_var*job = malloc(sizeof(type_var));
    type_var*job2 = malloc(sizeof(type_var));
    strcpy(job->message, arr);
    strcpy(job2->message, arr);
    job->type = type;
    List_prepend(temp, job);
}

//this function sets up the udp socket
int datagram_port_socket(int port) {
    struct addrinfo *addr;
    int socket_discpt = 0;
    int bind_tracker;
    int point1;
    int point2;
    
    addr = initializing_address_list(port, NULL);
    if (!addr) {
        point1 = 0;
        point2 = 1;
        return -1;  
    }
    struct addrinfo *ptr;
    for (ptr = addr; ptr != NULL; ptr = ptr->ai_next) {
        socket_discpt = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);
        if (socket_discpt == -1) {  
            perror("Error in client: socket_create");
            point1 =1;
            continue;
        }
        bind_tracker = bind(socket_discpt, ptr->ai_addr, ptr->ai_addrlen);
        if (bind_tracker == -1) { 
            pclose(socket_discpt);
            point2 =0;
            continue;
        }
        break;
    }
  
    freeaddrinfo(addr); 
    if (!ptr) {
        point1--;
        point2--;
        return -1;   
    } 
    else
        return socket_discpt;  
}

void check_remote_addr(int *port, struct addrinfo *addr){
     if (*port < 1002 || *port > 65535)
          return;
     addr->ai_flags = AI_PASSIVE;
     addr->ai_family = AF_INET;
     return;
}

// setup the address with the port  
struct addrinfo *setup_port_with_destAddr(int destination, char *address) {
    struct addrinfo *address1, *address2;
    struct addrinfo *temp_adr, *temp_adr2;
    
    address1 = initializing_address_list(destination, address);
    
    if(!address1) {
       temp_adr = address1;
       count_temp++;
       return NULL;
        }
   

    else{
        
        while (address2 = address1->ai_next) {
            free(address1); 
            address1 = address2;
            address2 = address2->ai_next;
            temp_adr2 = address2;
            count_temp--;
    }
}
return address1;
    
}

// The function to delete memory
void socket_info_delete(socket_info_create *mySockInfo) {
    freeaddrinfo(mySockInfo->remote_address);
    close(mySockInfo->socket_id);
}


// Make memory from package 
void make_mem_from_pack(type_var *aPckg) {
    free(aPckg);
}



static sem_t print_job;  
static sem_t send_job;  
static pthread_mutex_t print_sem; 
static pthread_mutex_t send_sem;    


void receive_job(char *message, List *arr){
    pthread_mutex_lock(&print_sem); 
    add_new_job(local, message, arr); 
    pthread_mutex_unlock(&print_sem);
}

// Thread for recieving UDP datagram 
void *receiving_data_from_datagram(void *temp_var) {
    int point1;
    int point2;
    int value1 = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    int value2 = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    if(value1 != 0)
        perror(" pthread_setcancelstate is not enabled");
    if(value2 != 0)
        perror(" pthread_setcancelstate is not asyncronous");
    int byte_count;
    struct sockaddr_storage source_addr;
    socklen_t length_of_remote_addr;
    char message[MAX_LENGTH];
    int received_socket_id = ((socket_info_create *) temp_var)->socket_id;   
    

    while (!counter_val) {
        length_of_remote_addr = sizeof(source_addr);   
       
        memset(message, 0, sizeof message); 
        
        if ((byte_count = recvfrom(received_socket_id, message, MAX_LENGTH - 1, 0, (struct sockaddr *) &source_addr, &length_of_remote_addr)) == -1) {
            perror("error in recvfrom, check the number");
            point1 = 0;
            count_temp--;
        } else {
            
            message[byte_count] = '\0';  
            if (message[0] == 33 &&check_space(message + 1,MAX_LENGTH-1)) {   
                counter_val = 1;
            } else {
               receive_job(&message, list_print);
            }
             sem_post(&print_job);
        }
    }
    sem_post(&send_job);
    pthread_exit(NULL); 
}


// thread to print data on screen 
void *printing_on_screen(void *var) {
    type_var *print;
    char *ptr =print->message;
    int flag =1;
    
    while (!counter_val) {
        /*for(int i = 0; *ptr != '\0' && i < length; i++, ptr++) {
        if (!isspace(*ptr))
            flag = 0;
    }*/
        sem_wait(&print_job);
        pthread_mutex_lock(&print_sem);
        print= List_trim(list_print);
        pthread_mutex_unlock(&print_sem);
                              
        if (print!= NULL) {
            if(check_space(print->message,MAX_LENGTH)){
             flag = 1;
             count_temp--;
         }
            
            else if (print->type == local)
                printf("New Message: %s", print->message);
            else
                printf("Send Message: %s", print->message);
            
            free(print);
        }
    }

    pthread_exit(NULL);
}


void record_job(char *message, List *arr){
    pthread_mutex_lock(&print_sem); 
    add_new_job(rremote, message, arr); 
    pthread_mutex_unlock(&print_sem);
}


// thread to record input 
void *record_input_from_keyboard(void *temp_var) {
     char message[MAX_LENGTH+1];
    int byte_count;
    int checker;
    int record_status;
    while (!counter_val) {
        struct pollfd fds;
        fds.fd = 0;       
        fds.events = POLLIN;
        if (!(poll(&fds, 1, 0) != 1) ) {
            if (!read_message_from_buffer(message, MAX_LENGTH)) {
                byte_count = (int) strlen(message);
                if (byte_count) { 
                    if (message[0] == 33 &&
                        check_space(message + 1, MAX_LENGTH - 1)) {    
                        counter_val = 1;
                    } else {
                        record_job(&message, list_print);
                    }
                    record_job(&message, list_send);

                    sem_post(&print_job);
                    sem_post(&send_job);
                }
            }
        }
    }
    pthread_exit(NULL);   
}
// thread to send message 
void *send_message_to_dest(void *var) {
    type_var *send;
    int point1;
    int point2;
    
    int soc_num = ((socket_info_create *) var)->socket_id;
    
    struct addrinfo *remote_address = ((socket_info_create *) var)->remote_address;
   
    int count;

    while (!counter_val ) {
    
        sem_wait(&send_job);

        pthread_mutex_lock(&send_sem);
        send = List_trim(list_send);
        pthread_mutex_unlock(&send_sem);

        if (send != NULL && !check_space(send->message,MAX_LENGTH)) {
             count = sendto(soc_num, send->message, strlen(send->message), 0,
                                   remote_address->ai_addr, remote_address->ai_addrlen);
            if (count == -1) { 
               point1 =0;
               count_temp--;
            }
            
            free(send); // free memory from sending job 
        }
    }
    pthread_exit(NULL); // done with sending 
}




static socket_info_create *thread_info;



// main program starts from here
int main(int argc, char *argv[]) { 
  
      
    list_print = List_create();
    list_send  = List_create();
    char local_port_arr[5];      
    char remport_port_arr[5];
    thread_info = malloc(sizeof(socket_info_create)); 

    char addr[INET6_ADDRSTRLEN];

    int *point1;
    int *point2;
    char *ptr;
    char *ptr2;   
    int local_port;      
    int remote_port;  
    int val;   
          
    int socket_id;
    int temp=2;
    counter_val = 0;
    count_temp =5;

    pthread_attr_t thread_attr;        
    pthread_t arr_thread[4]; 
    int temp2;  
  

    
    int flag = 0;
    if (argc > 2) {
        flag = convert_string_to_integer(argv[1], argv[3], &local_port, &remote_port);
        strcpy(addr,argv[2]);
    }

    if (argc < 3 ) {
        temp2 = 3;
        flag=1;
        while (temp2-- > 0 && flag) {
            printf(" Input the program parameters in the form of-  your +ve port number (1025-65525) , remote address , remote +ve port number\n");
       
           
            memset(addr, 0, sizeof addr);
            memset(local_port_arr, 0, sizeof local_port_arr);
            memset(remport_port_arr, 0, sizeof remport_port_arr);
           
            if (scanf(" ", local_port_arr, addr, remport_port_arr) == 3)  
                flag = convert_string_to_integer(local_port_arr, remport_port_arr, &local_port, &remote_port);
        }
    }
 if ( flag ==1) {
        temp2 = 3;
        flag=1;
        while (temp2-- > 0 && flag) {
            printf(" Input the program parameters in the form of-  your +ve port number (1025-65525) , remote address , remote +ve port number\n");
          
            memset(local_port_arr, 0, sizeof local_port_arr);
            memset(remport_port_arr, 0, sizeof remport_port_arr);
            memset(addr, 0, sizeof addr);
           
            if (scanf(" ", local_port_arr, addr, remport_port_arr) == 3) 
                flag = convert_string_to_integer(local_port_arr, remport_port_arr, &local_port, &remote_port);
        }
    }
    
    socket_id =datagram_port_socket(local_port);
    if (flag == 1 || socket_id == -1) {
        return 1;
    }

    bool tracker = 0 ;// no error
    // Setup the socket satrtup
    if(temp!=2){
        tracker =1;
        perror("error while creating socket");
    }
    else {
        thread_info->socket_id = socket_id;
    }

    // this checks the validation of remote address 
    if ((thread_info->remote_address = setup_port_with_destAddr(remote_port, addr)) == NULL) {
        perror("encountered an error in remote address");
        return 3;
    }
    
    if (sem_init(&print_job, 0, 0) < 0) {
        tracker = 1;
        puts("error in print_job");
        return 0;
     }
    if (sem_init(&send_job, 0, 0) < 0) {
        tracker =1;
        puts("error in sed_job");
        return 0;
    }
    
 
    if ((val = pthread_mutex_init(&print_sem, NULL)) != 0){}
    if ((val = pthread_mutex_init(&send_sem, NULL)) != 0){}

    if(pthread_attr_init(&thread_attr)!=0){}

    
    // creating the threads
    pthread_create(&arr_thread[0], &thread_attr, record_input_from_keyboard, NULL);
    pthread_create(&arr_thread[1], &thread_attr, printing_on_screen, NULL);
    pthread_create(&arr_thread[2], &thread_attr, send_message_to_dest, (void *) thread_info);
    pthread_create(&arr_thread[3], NULL, receiving_data_from_datagram, (void *) thread_info);

    for (temp2 = 0; temp2 < 3; temp2++)
        pthread_join(arr_thread[temp2], NULL);  

    
    if((val=pthread_cancel(arr_thread[3]))!=0 && val!=3) 
        puts("error in receive");

   
    socket_info_delete(thread_info);
    List_free(list_print, make_mem_from_pack);
    List_free(list_send, make_mem_from_pack); // memory from data is freed

   
    return 0;           
}
	/* used the instructor provided list.o file and not sure if the code works for all the test cases. some of these are testsed.
    thank you very much for your support! THe teaching teame is awesome.*/
	
	

