#include <iostream>  
#include <string> //for argument parsing
#include <sys/socket.h> // for creating socket descriptor
#include <arpa/inet.h> // for htons() function
#include <netinet/in.h> // for inet_aton() function
#include <unistd.h>
#include <thread> // for creating threads
#include <strings.h> // for bzero() function
#include <vector>
#include <cstring> // for strcmp
#include <unordered_map>
using namespace std;

unordered_map<string, // userid 
              unordered_map<string, vector<string>> // other data 
              > user_profiles;
unordered_map<string, string> active_users;
unordered_map<string, vector<string>> groups;

vector<string> parsecommand(string command) {
  // Split the space/tab-seperated command 
 
  int idx = 0;
  vector<string> output;
  // parse input
  string input;

  while(idx < command.size()) {
    // remove forward whitespaces if any 
    while(idx < command.size() && (command[idx] == ' ' || command[idx] == '\t')) {
      ++idx;
    }

    // push the characters into input 
    while(idx < command.size() && command[idx] != ' ') {
      input.push_back(command[idx]);
      ++idx;
    }

    // add the parsed input into output vector 
    if(!input.empty()) {
      output.push_back(input);
    }

    // clear input string for next iteration 
    input.clear();
  }

  return output;
}

int add_active(string user_id, string ip_port) {
  // add user_id, ip_port pair to the active_users map
  // return -1 if an entry for user_id already exists
  if(active_users.find(user_id) != active_users.end()) {
    // entry already exists 
    return -1; 
  }
  else {
    // add the new entry to user_profiles 
    active_users[user_id] = ip_port;
    return 0;
  }
}
void remove_active(string user_id, string ip_port) {
  // remove user_id, ip_port pair from the active_users map
}


int store_cred(string user_id, string pass) {
  //check if userid already exists in user_profiles 
  if(user_profiles.find(user_id) != user_profiles.end()) {
    // entry already exists 
    return -1; 
  }
  else {
    // add the new entry to user_profiles 
    user_profiles[user_id]["pass"].push_back(pass);  
    return 0;
  }
}

bool check_cred(string user_id, string pass) {
  if(user_profiles.find(user_id) != user_profiles.end()) {
    // entry exists so we check if password matches 
    if(strcmp(user_profiles[user_id]["pass"][0].c_str(), pass.c_str()) == 0) {
      // password matches 
      return true;
      // TODO: Get the IP:PORT of the user and store it in active_users 
    }
    // password doesn't match
    return false;
  }
  else {
    // no entry for user id 
    return false;
  }
}

int create_group(string group_id, string user_id) {
  //check if userid already exists in user_profiles 
  if(groups.find(group_id) != groups.end()) {
    // entry already exists 
    return -1; 
  }
  else {
    // add the new entry to group
    // TODO: mark the user_id as owner in the user_profiles
    groups[group_id].push_back(user_id);

    return 0;
  }
}


string process_query(string query) {
  // we will process the query and do the required functionality

  // parse the query string
  vector<string> query_args = parsecommand(query);

  string command = query_args[0];

  string response;

  if(strcmp(command.c_str(), "create_user") == 0) {
    cout << "Storing User Credentials " << endl;

    if(query_args.size() < 4) {
      response = "Missing argument for create_user command";
      return response;
    }

    string user_id = query_args[1];
    string pass = query_args[2];
    string ip_port = query_args[3]; // this will be implicitly sent by the client 

    if(user_id.empty() || pass.empty()) {
      response = "User ID or password cannot be empty";
      return response;
    }

    // store (userid, pass) in DB
    // logic for storing user details
    int status = store_cred(user_id, pass);
    if(status == -1) {
      response = "User already exists";
    }
    else {
      // store the ip_port value in the active_users map
      add_active(user_id, ip_port);
      response = "User created successfully";
    }

    return response;
  }
  else if(strcmp(command.c_str(), "login") == 0) {

    if(query_args.size() < 3) {
      response = "Missing argument for login command";
      return response;
    }

    string user_id = query_args[1];
    string pass = query_args[2];
    string ip_port = query_args[3]; // this will be implicitly sent by the client 

    cout << "Checking User Credentials " << endl;
    
    //check if (userid, pass) exists in DB
    if(check_cred(user_id, pass) == true) {
      // mark the peer as alive in active_users map 
      int status = add_active(user_id, ip_port);
      if(status == -1) {
        response = "User already logged in";
      }
      else {
        response = "Logged in successfully";
      }
    }
    else {
      response = "Incorrect credentials";
    }
  
    return response;
  }
  else if(strcmp(command.c_str(), "create_group") == 0) {

    string group_id = query_args[1];
    string user_id = query_args[2]; // this will be implicitly sent by the client
    
    // add (group id, owner) data to DB 
    int status = create_group(user_id, group_id);
    if(status == -1) {
      response = "Group already exists";
    }
    else {
      response = "Group created successfully";
    }
  }
  else if(strcmp(command.c_str(), "join_group") == 0) {

    // add the request of group join to the request list
    // of group owner 
    // add_join_request();

    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "leave_group") == 0) {

    // remove the user from the group 
    // also assign a new owner if old owner leaves group
    // delete group if no more users are left 
    // leave_group();

    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "list_requests") == 0) {
    
    // list the requests for joining a given group
    // list_requests();
    
    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "accept_request") == 0) {

    // accept a requst from the list of requests
    // return something in case there is no such request 
    // accept_request();

    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "list_groups") == 0) {

    // list the group names in the DB 
    // list_groups();

    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "list_files") == 0) {

    // list all the files that the user can access
    // list_files();

    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "upload_file") == 0) {

    // upload the file metadata and related information
    // in the tracker
    // upload_file();

    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "download_file") == 0) {
    cout << "Checking User Credentials " << endl;
    response = "127.0.0.1:8080";
    return response;
  }
  else if(strcmp(command.c_str(), "logout") == 0) {
    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "show_downloads") == 0) {
    cout << "Checking User Credentials " << endl;
  }
  else if(strcmp(command.c_str(), "stop_share") == 0) {
    cout << "Checking User Credentials " << endl;
  }

  response = "Incorrect query";
  return response;
}

void handleClientQuery(int new_server_socket) {
  cout << "Client Query Handler Created" << endl;

  int e; //for checking errors 
  
  string input_msg;
  char buffer[1024] = {0};

  while(true) {
    // read query from client
    bzero(buffer, 1024);
    e = read(new_server_socket, &buffer, 1024);
    if(e < 0) {
      cout << "Error reading message from buffer" << endl;
      break;
    }
    else if(e == 0) {
      cout << "Connection terminated" << endl;
      break;
    }
    else {
      cout << buffer << endl; 
    }
    
    // cin >> input_msg;
    // process query
    string response = process_query(buffer);
    // send response to the client
    e = send(new_server_socket, response.c_str(), response.size(), 0);
    if(e == -1) {
      cout << "Failed to send message" << endl;
    }
    else {
      cout << "Message sent from server" << endl; 
    }
  }

  //close socket connection
  e = close(new_server_socket);
  if(e == -1) {
    cout << "Error in closing socket file descriptor" << endl;
  }
}

void connectToServer(int server_PORT, string server_IP) {
  cout << "Creating a connection to server [" << server_IP << " : " << server_PORT << "]" << endl;

  int e; //for checking errors 
 
  int client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // AF_INET for IPV4 and SOCK_STREAM for 2-way connection-based byte stream
  // Generally only a single protocol for each family, which is specified with 0
  if(client_sockfd < 0) {
    cout << "Error creating client socket" << endl;
    return;
  }

  sockaddr_in serv_addr; 
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(server_PORT); 


  e = inet_pton(AF_INET, server_IP.c_str(), &serv_addr.sin_addr);
  if(e <= 0) {
    cout << "Error in IP address conversion" << endl;
    return;
  }
  
  // We will now connect the client_sockfd with the address specified by serv_addr which contains 
  // the server address (IP & PORT info). This does not create a new socket like accept 
  e = connect(client_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
  if(e < 0) {
    cout << "Error in creating connection" << endl;
    return;
  } 
  else {
    cout << "Connection to server created" << endl; 
  }

  string input_msg;
  char buffer[1024] = {0};

  while(true) {
    cin >> input_msg;
    // send input message to the server
    e = send(client_sockfd, input_msg.c_str(), input_msg.size(), 0);
    if(e <= 0) {
      cout << "Failed to send message" << endl;
      break;
    }
    else {
      cout << "Message sent from client" << endl; 
    }

    bzero(buffer, 1024);
    e = read(client_sockfd, &buffer, 1024);
    if(e == -1) {
      cout << "Error reading message from buffer" << endl;
    }
    else if(e == 0) {
      cout << "Empty response from server" << endl;
      break;
    }
    else {
      cout << "Output: " << buffer << endl; 
    }
  }

  // closing the connected socket
  close(client_sockfd);
}


int main(int argc, char *argv[]) {

  if(argc != 1 + 2) { // 1 arg always is the file argument
    cout << "Insufficient or excess arguments given" << endl;
    return -1;
  }

  int e; // for storing status to check if any error has occured 

  string TRACKER_FILENAME = argv[1];
  try {
    int TRACKER_NO = stoi(argv[2]);
  } 
  catch (...) {
    cout << "Tracker number is NaN" << endl;
    return -1;
  }

  // extract IP address and PORT number from arguments
  // by reading the IP:PORT associated with the input tracker number 
  // TODO: Write code to parse the tracker_info.txt file

  string IP = "127.0.0.1"; //hardcoded  

  int PORT = stoi("8000"); //hardcoded 
 
  cout << "IP: " << IP << endl;
  cout << "PORT: " << PORT << endl;

  // We want to run the main thread for server side, i.e., for listening to peers  
  // and other threads for client side, i.e., requesting connections from other trackers
  // For this, we will create threads after setting up listen, for connecting 
  // to server side of other trackers.

  // --- SETUP SERVER SIDE OF PEER ---   
  

  // Before marking this peer in tracker list as active, we will take authentication
  // This is because and unauthenticated peer is practically non-existent in our system.
  // Thus tracker will not keep track of it 

  // --OPTIONAL UI TEXT DESIGN--

  // AUTH CHECKING CODE
  
  // VARIOUS UI COMMANDS
  
  // create a socket file descriptor 
  int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // AF_INET for IPV4 and SOCK_STREAM for 2-way connection-based byte stream
  // Generally only a single protocol for each family, which is specified with 0

  if(server_sockfd < 0) {
    cout << "Error creating server socket" << endl;
    return -1;
  }

  // To forcefully get the required port, we will manipulate the options for the socket
  // before the bind() call using setsockopt()
  int opt = 1;
  // Forcefully get the port PORT
  e = setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
  if(e < 0) {
    cout << "Error in mainpulating socket options " << endl;
    return -1;
  }

  // binding the socket by allocating a port number to it. The socket itself is 
  // unable to transfer any data. It requires a IP and port number 
  // to get the complete address (socket address)
  
  // We will bind the socket file descriptor to the an address containing both 
  // the IP address and port number

  // For that, we will give sockaddr pointer as argument
  // sockaddr addr;
  // addr.sa_family = AF_INET;
  // addr.sa_data = htons(PORT);

  // Internet socket address 
  // In memory, size of struct sockaddr_in and struct sockaddr is the same, so 
  // you can freely cast the pointer of one type to the other.  
  // The benefit of using sockaddr_in is that it has in_addr data structure 
  // in it, which holds the s_addr   
 
  sockaddr_in s_addr; 
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons(PORT);

  // We have converted the PORT number into 
  // now we need to convert the IP address into binary form
  // and store it in the in_addr data structure within sockaddr_in data structure   
  
  // addr.sin_addr.s_addr is of the type in_addr_t , so we need to convert it to in_addr
  // in order to pass it to the function inet_aton() 
  // Reference: http://wongchiachen.blogspot.com/2012/02/inaddr-vs-inaddrt.html

  // inet_lnaof(struct in_addr in)

  //inet_network() converts IP string into host byte order

  // in_addr_t t = in_addr_t();  
  // t = inet_lnaof(t);
  // inet_lnaof(addr.sin_addr.s_addr);
  // inet_aton("63.161.169.137", ); 
  // the problem in the above line is that it inet_aton takes in_addr* as arg 
  // but we are giving in_addr_t  

  //inet_pton converts IPv4 from text to binary
  e = inet_pton(AF_INET, IP.c_str(), &s_addr.sin_addr);
  if(e <= 0) {
    cout << "Error in IP address conversion" << endl;
    return -1;
  }

  // We now have the IP in binary and have converting
  // the PORT into network byte order  

  cout << "IP in binary: " << s_addr.sin_addr.s_addr << endl;
  cout << "PORT in netork byte order: " << s_addr.sin_port << endl;

  // The reason we don't use sockaddr directly is because it is a 
  // generic descriptor for any socket operation, whereas socketaddr_in
  // is specific to IP (socketaddr_un is for Unix domain sockets)
  // (struct sockaddr *) &s tricks in a way the compiler by telling it
  // that it points to a sockaddr type
  // Reference: https://stackoverflow.com/questions/21099041/why-do-we-cast-sockaddr-in-to-sockaddr-when-calling-bind


  // as bind function requires const sockaddr pointer, we will
  // cast sockaddr_in into it (the reasoning explained above with reference)
  int fd = bind(server_sockfd, (struct sockaddr*)&s_addr, sizeof(s_addr)); 
  // After binding, it is ready to receive/send information

  if(fd < 0) {
    cout << "Error in binding " << endl;
  }
  else {
    cout << "Binding complete " << endl;
  }

  // we decide a max queue size for connection requests
  int backlog = 3;

  // listen for connection requests
  e = listen(server_sockfd, backlog);
  if(e < 0) {
    cout << "Error in listening" << endl;
  }
  else {
    cout << "Listening on " << PORT << endl;
  }

  // Before accepting connection, we need to connectToServer so that we 
  // can share our info. Only then can we expect to get any connection request 
  // as only the tracker will tell others about us 
  
  // --- SETUP CLIENT SIDE OF PEER ---
  
  // The client only creates a socket and calls connect() onto the server

  // The client now needs a IP:PORT of the server to send a request on. This will 
  // be provided by the tracker, when peer-to-peer communication is needed.
  // To contact with the tracker itself, we will be given IP:PORT of the tracker 
  // from the tracker_info.txt file from the arguments.   

  // TODO: extract tracker server port from the file and store in server_PORT 
  int server_PORT = stoi(TRACKER_FILENAME); //this port is that of the server that we will connect into
  cout << "server PORT: " << server_PORT << endl;
  string server_IP = "127.0.0.1"; //hardcoded [will need to extract from tracker_info.txt file] 
 
  // call the connectToServer function as a thread to connect to server
  thread t(connectToServer, server_PORT, server_IP);
  t.detach();

  // accept a connection request on a socket. The newly created socket
  // is not in the listening state and the original socket file descriptor 
  // send in the argument is unaffected by this call. 
  // What we get is a new socket file descriptor that is for sending info 
  // (writing) at the client 
  // This new socket that is created is the one that we will transmit data from
  // server to client
  // 3rd argument is the size of the data structure addr, just casted to socklen_t  
  int addrlen = sizeof(s_addr); 
  
  while(true) {
    int new_server_socket  = accept(server_sockfd, (struct sockaddr*)&s_addr, (socklen_t *)&addrlen);

    if(new_server_socket < 0) {
      cout << "Error in accepting connection" << endl;
    }
    else {
      cout << "Connection accepted"<< endl;
    }
    
    // call the handleClientQuery function as a thread so that
    // we can deal with multiple clients simulatenously
    thread t(handleClientQuery , new_server_socket);
    t.detach();
  } 




  
  //inet_pton converts IPv4 from text to binary



  // send(sock, "", strlen(hello.c_str()), 0);
  // printf("Hello message sent\n");
  // valread = read(sock, buffer, 1024);
  // printf("%s\n", buffer);

  // closing the connected socket
  // close(client_fd);

  return 0;
}
