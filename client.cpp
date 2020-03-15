#include <iostream> 
#include <sys/socket.h> 
#include <netdb.h>
#include <fstream>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <cstring>

using namespace std; 

#define size 256

// function to communicate with server
void startComm(int port, const char* server_IP) {
    int client_SD, n = 0;

    struct sockaddr_in server_address;
     
    struct hostent* host = gethostbyname(server_IP);

    char buffer[size];

    client_SD = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char*)&server_address, sizeof(server_address));

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    server_address.sin_port = htons(port);

    if (client_SD < 0) {
        perror("Socket Failed");
        exit(EXIT_FAILURE); // terminate with error
    }      
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    int validIP = inet_pton(AF_INET, server_IP, &server_address.sin_addr);
    if(validIP <= 0) { 
        perror("Invalid IP"); 
        exit(EXIT_FAILURE); // terminate with error
    } 

    int status = connect(client_SD, (struct sockaddr *)&server_address, sizeof(server_address));
    if (status < 0) { 
        perror("Connection Failed"); 
        exit(EXIT_FAILURE); // terminate with error
    }

    string city;
    cin.ignore(); // flush the input stream for getline function
    cout << "Enter a city name: ";
    std::getline(cin, city); // input city name with white space

    char city1[size]; // to convert it to char * type from string
    bzero(city1, size);

    for (int i = 0; i<city.length();i++) { // copy the input city name
        city1[i] = city[i];
    }

    n = write(client_SD, city1, city.length()); // send the city name to server

    if (n < 0) {
        perror("ERROR writing to socket");
    }
    
    bzero(buffer,size);
    
    n = read(client_SD,buffer,size - 1); // read the information from server
    
    if (n < 0) {
        perror("ERROR reading from socket");
    }
    
    cout << buffer; // display the information of city
    
    close(client_SD); // close the connection
}

int main(int argc, char const *argv[]) {
    char* server_IP;
    int port = 0;

    // input the IP of Server and Port
    cout << "Enter the server host name: ";
    cin >> server_IP;
    cout << "Enter the server port number: ";
    cin >> port;

    // loose check of localhost to convert it to valid IP address
    if ((strlen(server_IP) == 9) && server_IP[0] == 'l' && server_IP[8] == 't') {
        startComm(port, "127.0.0.1");
    } else {
        startComm(port, server_IP);
    }

    exit(EXIT_SUCCESS);
    
    return 0; 
} 
