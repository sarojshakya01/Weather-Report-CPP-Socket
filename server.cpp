#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sys/wait.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

#define size 256

int cityCount = 0;

// function to extract comma separated value
string entry(string src, int idx) {
    int commaCount = 0;
    string value, temp;
    for (int i = 0; i < src.length(); i++) {
        if (src[i] == ',') {
            commaCount++;
        }
    }
    if (idx > commaCount + 1) {
        return "";
    }
    commaCount = 0;

    for (int i = 0; i < src.length(); i++) {
        if (idx == commaCount) {
            break;
        }
        if (src[i] == ',') {
            
            commaCount++;
            value = temp;
            temp = "";
        } else {
            temp = temp + src[i];
        }
    }
    
    if (idx > commaCount) {
        value = temp;
    }
    
    return value;
}

// class to store information of the city weather
class cityWeather {
    string name;
    int maxTemp;
    string skyCond;
public:
    cityWeather(){
        name = "";
        maxTemp = 0;
        skyCond = "";
    }
    void setCityWeather(string name, int maxTemp, string skyCond) {
        this -> name = name;
        this -> maxTemp = maxTemp;
        this -> skyCond = skyCond;
    }
    void displayCityWeather() {
        cout << "Weather report for " << name << endl;
        cout << "Tomorrow’s maximum temperature is " << maxTemp << " F" << endl;
        cout << "Tomorrow’s sky condition is " << skyCond << endl;
    }
    string weatherInfo() {
        string info;
        info = "Tomorrow’s maximum temperature is " + to_string(maxTemp) + " F" + "\n";
        info = info + "Tomorrow’s sky condition is " + skyCond + "\n";
        return info;
    }
    string getCity() {
        return name;
    }
    ~cityWeather() {
    }
};

// global object
cityWeather *cw;

string searchRecord(string city) {
    for (int i = 0; i < cityCount; ++i) {
        if (cw[i].getCity() == city) {
            cw[i].displayCityWeather();
            return cw[i].weatherInfo();
        }
    }
    cout << "Weather report for " + city + "\nNo data\n"; 
    return "No data\n";
}

// function thread to coonect to the client
void startComm(int port) {
    
    int server_SD, n = 0, opt = 1;

    char buffer[size];

    struct sockaddr_in server_address;
    
    bzero((char*)&server_address, sizeof(server_address));
    
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    int addrlen = sizeof(server_address);
    
    // Creating socket file descriptor
    server_SD = socket(AF_INET, SOCK_STREAM, 0);
    if (server_SD <= 0) {
        perror("Socket Failed");
        exit(EXIT_FAILURE);
    }
    
    // Forcefully attaching socket to the port
    int set_socket_status = setsockopt(server_SD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    
    if (set_socket_status < 0) {
        perror("Set Socket Failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the server_address
    int bind_status = bind(server_SD, (struct sockaddr *)&server_address, sizeof(server_address));
    
    if (bind_status < 0) {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }

    int listen_status = listen(server_SD, 3);

    if (listen_status < 0) {
        perror("Listen Failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in new_sock_addr;
    socklen_t new_sock_addr_size = sizeof(new_sock_addr);
    
    //accept, create a new socket descriptor to handle the new connection with client
    int new_socket = accept(server_SD, (struct sockaddr *)&server_address, &new_sock_addr_size);
    if (new_socket < 0) {
        perror("Accept Failed");
        exit(EXIT_FAILURE);
    }

    bzero(buffer,size);

    // read the city name from the client
    n = read(new_socket, buffer, size - 1);

    if (n < 0) {
        perror("ERROR reading from socket");
    }

    // search from city and store information in info
    string info = searchRecord(buffer);
    
    bzero(buffer,size);
    // copy the info to buffer
    for (int i = 0; i < info.length(); i++) {
        buffer[i] = info[i];
    }

    // write into the buffer to send back to client
    n = write(new_socket, buffer, strlen(buffer));

    if (n < 0) {
        perror("ERROR writing to socket");
    }

    // destroy the object
    delete [] cw;
    // close the connections
    close(new_socket);
    close(server_SD);
            
}

int main(int argc, char const *argv[]) {

    // read file to find out total city and to create object accordingly
    ifstream infile ("weather20.txt");
    string str;
    while(getline(infile, str)) {
        cityCount++;
    }
    infile.close();

    cw = new cityWeather[cityCount];

    // again read file to extract the information from it
    infile.open("weather20.txt");
    
    cityCount = 0;
    while(getline(infile, str)) {
        // set the value to object
        cw[cityCount].setCityWeather(entry(str,1),stoi(entry(str,2)),entry(str,3));
        cityCount++;
    }

    infile.close();

    // input to take the port number
    int port;
    cout << "Enter server port number: ";
    cin >> port;

    startComm(port);
    
    exit(EXIT_SUCCESS);

    return 0;
}
