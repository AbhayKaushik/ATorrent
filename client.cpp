#include <iostream>  
#include <string> //for argument parsing
#include <sys/socket.h> // for creating socket descriptor
#include <arpa/inet.h> // for htons() function
#include <netinet/in.h> // for inet_aton() function
#include <unistd.h>
#include <thread> // for creating threads
#include <strings.h> // for bzero() function
#include <unordered_map>
#include <vector>
#include <deque>
#include <cstring>
#include <fcntl.h> // for open() function
#include <openssl/sha.h>
using namespace std;

// store the ip_port for connecting to the client
string ip_port = "";
// store the user_id the client is authenticated as
string UID = "";

string homepath = getenv("HOME");
string path = getenv("PWD");

unordered_map<string, vector<int>> chunk_list; // store(filename+group_id, chunk_bitmap)
unordered_map<string, string> chunk_path; // store(filename+group_id, filepath)

// int copy_file(string filename, string destination) {
//   // copy a file from filename to dest
//   
//   // get the absolute source path for the filename 
//   // The filename here can be only name or path + name 
//
//   filename = resolve_path(filename);
//
//   char* sourcepath = NULL;
//   sourcepath = realpath(filename.c_str(), sourcepath);
//
//   if(sourcepath == NULL) {
//     errormessage("Incorrect sourcefile name/path");
//     return -1;
//   }
//
//   // open() returns a file descriptor upon successful execution
//   // we want to copy content from sourcepath, so we will 
//   // open the source file in read mode 
//   int sfd = open(sourcepath, O_RDONLY);
//   
//   if(sfd == -1) {
//     errormessage("Invalid source file");
//     return -1;
//   }
//
//   //obtain permissions for source file 
//   struct stat buffer;
//   int status = stat(sourcepath, &buffer);
//   if(status == -1) {
//     errormessage("Unable to get file status");
//     return -1;
//   }
//
//   // add filename to destination to get destpath 
//   // extract filename 
//   size_t idx = filename.find_last_of('/');
//   if(idx != string::npos) {
//     // some path in the filename input
//     //filename = filename.substr(0, idx);
//     filename = filename.substr(idx+1, filename.size());
//   }
//   string destpath = destination;
//   destpath.push_back('/');
//   destpath.append(filename);
//   errormessage(destpath);
//   
//   // creat() returns a file descriptor upon successful execution
//   // and will create the file if it doesn't exist and overwrite the
//   // file contents if it does 
//   int dfd = creat(destpath.c_str(),  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
//
//   if(dfd == -1) {
//     //errormessage("Invalid destination");
//     return -1;
//   }
//   
//   // set filepermissions for the newly created file 
//   status = chmod(destpath.c_str(), buffer.st_mode);
//   if(status == -1){
//     errormessage("Unable to set file permissions");
//     return -1; 
//   }
//
//   // set owner for the newly created file 
//   status = chown(destpath.c_str(), buffer.st_uid, buffer.st_gid);
//   if(status == -1){
//     errormessage("Unable to set owner for file");
//     return -1; 
//   } 
//
//
//   char buf[8192]; 
//
//   int rstatus = read(sfd, &buf, 8192);  
//   int wstatus;
//   while(rstatus != 0 && rstatus != -1) {
//     //read until the entire file has been read 
//     
//     // we write rstatus bytes from the buffer as
//     // rstatus denotes the number of bytes read to buffer 
//     wstatus = write(dfd, &buf, rstatus);
//     if(wstatus == -1) {
//       errormessage("Unable to write into file");
//       return -1;
//     } 
//     rstatus = read(sfd, &buf, 8192);  
//   }
//
//   if(rstatus == -1) {
//     errormessage("Unable to read into file");
//     return -1;
//   }
//
//   // errormessage(sourcepath);
//   errormessage("File copied successfully");
//
//   return 0;
// }

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

string resolve_path (string dest) {
  // Resolve the input path 

  // Can later be made a global variable
  // string homepath = getenv("HOME");
  
  if(dest[0] == '/') {
    //already absolute, so no need to do anything
    return dest;
  }
  else if(dest[0] == '~') {
      dest.replace(0, 1, homepath);
  }
  else if(dest == "." || dest == "./") { //dest[0] == '.') {
    // replace dest with current path
    dest = path;
  }
  else if(dest[0] == '.' && dest[1] == '.') {
    // dest starts with ".." so we give reduced current path
    string reduced_path = path;
    while(reduced_path.size() > 1 && reduced_path[reduced_path.size() - 1] != '/') {
      // cout << new_path[new_path.size() - 1]<< endl; 
      reduced_path.pop_back();
    }
    // pop the '/' found too 
    if(reduced_path.size() > 1) { // to handle case when path is '/' only
      reduced_path.pop_back();
    }
    
    dest.replace(0, 2, reduced_path);
  }
  else if(dest[0] == '.' && dest[1] == '/') {
    //dest starts with "./" so we replace it with current path
    dest.replace(0, 1, path);
    // cout << ">>" << dest << "<<" << endl;
  }
  else{
    // the path begins with the filename directly 
    // so we need to add the current path before 

    string new_path = path;

    // add the directory name to the absolute path 
    new_path.push_back('/');
    new_path.append(dest);

    dest = new_path;
  }

  return dest;
}

void encrypt(string input, unsigned char* hash) {
  //store the SHA1 of the input in the hash 
  SHA1((unsigned char *)input.c_str(), input.size() - 1, hash);
  return;
}

deque<string> get_SHA1(string filepath, int &err) {
  cout << "Getting Chunkwise File SHA" << endl;

  deque<string> output;

  // get the actual path from the filepath
  filepath = resolve_path(filepath);

  char* sourcepath = NULL;
  sourcepath = realpath(filepath.c_str(), sourcepath);
  if(sourcepath == NULL) {
    cout << "Incorrect filepath: " + filepath << endl;
    err = 1;
    return output;
  }
  cout << sourcepath << endl;
 
  string filename = sourcepath;
  cout << filename.find_last_of('/') << endl;
  filename = filename.substr(filename.find_last_of('/') + 1);
  cout << filename << endl;

  // read the data from file in chunks
  int sfd = open(string(sourcepath).c_str(), O_RDONLY);
  
  if(sfd == -1) {
    cout << "Invalid source file" << endl;
    err = 1;
    return output;
  }

  int chunk_no = 1; //every file will have atleast 1 chunk

  const int chunk_size = 524288; // 1024*512B = 524288B = 512KB 
  char buf[chunk_size]; 
  int offset = (chunk_no - 1) * 524288;
  
  // int rstatus = read(sfd, &buf, 8192);    
  int rstatus = pread(sfd, &buf, chunk_size, offset);    

  if(rstatus == 0) {
    cout << "No data at this chunk number" << endl;
    err = 1;
    return output;
  }
  if(rstatus == -1) {
    cout << "Unable to read into file" << endl;
    err = 1;
    return output;
  }
  cout << "Bytes read: " << rstatus << endl;

  output.push_back(filename);

  // find the SHA1 hash of the chunk
  unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
  encrypt(string(buf), hash);

  // append hash in output 
  string str(hash, hash + SHA_DIGEST_LENGTH);
  cout << str.size() << endl;
  output.push_back(str);

  while(rstatus == chunk_size) {
    //update chunk_no and offset  
    ++chunk_no;
    offset = (chunk_no - 1) * 524288;

    //read the next chunk
    memset(buf, '\0', chunk_size);
    rstatus = pread(sfd, &buf, chunk_size, offset);
    cout << "Bytes read: " << rstatus << endl;
    if(rstatus == 0) {
      // no chunk left to read 
      --chunk_no;
      break;
    }

    // find the SHA1 hash of the chunk
    unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
    encrypt(string(buf), hash);

    // append hash in output 
    string str(hash, hash + SHA_DIGEST_LENGTH);
    cout << str.size() << endl;
    output.push_back(str);
  }

  //add the chunk_no at the front of the output vector 
  output.push_front(to_string(chunk_no));
  
  return output;
}

int read_file(string sourcepath, int chunk_no, string &response) {
  // read a file based on given offset for upto chunk_size bytes
  // chunk_no is 1-indexed

  // open() returns a file descriptor upon successful execution
  // we want to copy content from sourcepath, so we will 
  // open the source file in read mode 
  int sfd = open(sourcepath.c_str(), O_RDONLY);
  
  if(sfd == -1) {
    cout << "Invalid source file" << endl;
    return -1;
  }

  const int chunk_size = 524288; // 1024*512B = 524288B = 512KB 
  char buf[chunk_size] = {0}; 
  int offset = (chunk_no - 1) * 524288;

  // int rstatus = read(sfd, &buf, 8192);  
  memset(buf, '\0', chunk_size); // this is necessary else the old request data will also be sent [not neccessay now as the above declaration has = {0} now]
  int rstatus = pread(sfd, &buf, chunk_size, offset);    
  if(rstatus == 0) {
    cout << "No data at this chunk number" << endl;
    return -1;
  }
  if(rstatus == -1) {
    cout << "Unable to read into file" << endl;
    return -1;
  }

  // The file has been read upto the chunk size, so 
  // we can now send it to the client 
  response = buf;

  return 0;
}

string process_query(string query) {
  // we will process the query and do the required functionality
  // As a peer, we will only get requests for file chunk transfers 
  // Thus, our argument will be of the form [filename, chunk_no, filepath] 
  // which we will send in the network

  // parse the query string
  vector<string> query_args = parsecommand(query);

  string command = query_args[0];
  
  if(strcmp(command.c_str(), "chunk_list") == 0) {
  
    // return the chunk_bitmap for files you have
    string group_id = query_args[1];
    string filename = query_args[2];

    string response;
    //iterate the chunk_bitmap 
    vector<int> chunk_bitmap = chunk_list[filename+"_"+group_id]; 
    for(int i = 0; i < chunk_bitmap.size(); ++i) {
      response += chunk_bitmap[i];
    }

    return response;
  }
  else if(strcmp(command.c_str(), "get_chunk") == 0) {
  
    // return the chunk_bitmap for files you have
    string group_id = query_args[1];
    string filename = query_args[2];
    int chunk_no = stoi(query_args[3]);
    
    string response;
    
    // check if you have chunk 
    int check = chunk_list[filename+"_"+group_id][chunk_no - 1]; 
    if(check == 0) {
      response = "Request chunk not available";
      return response;
    }

    // return chunk data as response
    // // hardcoded directory path
    // string DIR_PATH = "/home/abhayk/Documents/IIITH/AOS_CourseWork/Assignments/A3/";
    // 
    // string sourcepath = DIR_PATH + '/' + filename;
    
    // get the filepath from chunk_path
    string sourcepath = chunk_path[filename + "_" + group_id];

    // call the read_file to read a chunk of the file and store the data in 
    // the response string
    int status = read_file(sourcepath, chunk_no, response);
    
    if(status == -1) {
      response = "Read operation failed";
    }

    return response;
  }


  if(query_args.size() != 2) {
    string response = "Insufficient or excess arguments";
    return response;
  }

  string filename = query_args[0];

  int chunk_no = -1;
  try {
    chunk_no = stoi(query_args[1]);
  } 
  catch (...) {
    string response = "Tracker number is NaN. Unable to process request";
    return response;
  }

  // hardcoded directory path
  string DIR_PATH = "/home/abhayk/Documents/IIITH/AOS_CourseWork/Assignments/A3/";
  
  string sourcepath = DIR_PATH + '/' + filename;

  // call the read_file to read a chunk of the file and store the data in 
  // the response string
  string response;
  int status = read_file(sourcepath, chunk_no, response);
  
  if(status == -1) {
    response = "Read operation failed";
  }

  return response;
}

// handleClientQuery is same as server_read_file
void handleClientQuery(int new_server_socket) {
  cout << "Client Query Handler Created" << endl;

  int e; //for checking errors 
  
  string input_msg;
  char buffer[1024] = {0};

  while(true) {
    // read query from client
    // bzero(buffer, 1024);
    memset(buffer, '\0', 1024);
    
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
      cout << "Client query: " << buffer << endl; 
    }
    
    // cin >> input_msg;
    // process query
    string response = process_query(buffer);
    cout << "Size: " << response.size() << endl;
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

void connectToTrackerServer(int server_PORT, string server_IP) {
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
  const int chunk_size = 524288; // 1024*512B = 524288B = 512KB 
  char buffer[chunk_size]; 

  // We want to store the chunks that we get into a file, so we will
  // create a new file with the filename that we want and write the 
  // buffer into it

  while(true) {
    // cin >> input_msg;
    cout << "> "; 
    getline(cin, input_msg);

    string fpath, gid;

    vector<string> input_args = parsecommand(input_msg);
    cout << "Arg Count: " << input_args.size() << endl;

    if(strcmp(input_args[0].c_str(), "login") != 0 && 
       strcmp(input_args[0].c_str(), "create_user") != 0) {
      // check if UID is set => auth is done or not 
      if(UID.empty()) {
        cout << "Please Login/Create User before using any other command" << endl;
        continue;
      }
    }
    if(strcmp(input_args[0].c_str(), "upload_file") == 0) {
      // we need to also append the number of chunks 
      // followed by the chunkwise SHA1 of the file
      if(input_args.size() < 3) {
        cout << "Missing arguments for upload_file command";
      }

      int err = 0;
      deque<string> hashes = get_SHA1(input_args[1], err);

      if(err == 1) {
        // some error has occured, so we will not proceed further
        // and redirect back to user input 
        continue;
      }

      // we will now append this deque in the message that we send to server
      for(int i = 0; i < hashes.size(); ++i) {
        input_msg += " " + hashes[i];
      }

      // store filepath and group_id for later storage if command is valid
      fpath = input_args[1];
      gid = input_args[2];
    }
    else {
      cout << "First arg: >" << input_args[0] << "<" << endl;
    }

    // append user_id implicitly 
    // is user_id is NULL, append the ip_port implicitly 
    if(UID.empty()) {
      cout << "Adding IP:PORT" << endl;
      input_msg += " " + ip_port;
    }
    else {
      cout << "Adding UID" << endl;
      input_msg += " " + UID;
    }

    cout << "Message to send: " << input_msg << endl;
      

    // send input message to the server
    e = send(client_sockfd, input_msg.c_str(), input_msg.size(), 0);
    if(e < 0) {
      cout << "Failed to send message" << endl;
      break;
    }
    else if(e == 0) {
      cout << "Empty message sent to server" << endl;
    }
    else {
      cout << "Message sent from client" << endl; 
    }

    // read message from server
    // bzero(buffer, chunk_size); // DEPRECATED
    memset(buffer, '\0', chunk_size);
    cout << "||" << buffer << "||" << endl;
    e = recv(client_sockfd, &buffer, chunk_size, 0);
    if(e == -1) {
      cout << "Error reading message from buffer" << endl;
    }
    else if(e == 0) {
      cout << "Connection closed from Server [Empty response from server]" << endl;
      break;
    }
    else {
      cout << "Output: >" << buffer << "<" << endl; 
      // we will process the response baseed on the input we sent

      if(strcmp(input_args[0].c_str(), "login") == 0) {
        if(strcmp(buffer, "Logged in successfully") == 0) {
          // logged in successfully so the user_id given as 
          // input_args[1] was marked active so we will set our UID as the same 
          cout << buffer << endl;
          cout << "Updating UID" << endl;
          UID = input_args[1];
        }
      }

      else if(strcmp(input_args[0].c_str(), "create_user") == 0) {
        // we check if we have received a valid response from server
        if(strcmp(buffer, "User created successfully. Logged in") == 0) {
          // logged in successfully so the user_id given as 
          // input_args[1] was marked active so we will set our UID as the same 
          cout << buffer << endl;
          cout << "Updating UID" << endl;
          UID = input_args[1];
        }
      }
      
      else if(strcmp(input_args[0].c_str(), "upload_file") == 0) {
        // we check if we have received a valid response from server
        if(buffer[0] != '~') {
          // command was valid so we can save fpath and gid 
          cout << buffer << endl;
          string fname = resolve_path(path);
          chunk_path[fname+"_"+gid] = fpath;
        }
      }
      
      else if(strcmp(input_args[0].c_str(), "download_file") == 0) {
        // we check if we have received a valid response from server 
        if(buffer[0] != '~') {
          // we have received valid response, so we will call client_read_file 
          // to get the chunks of the data and write into the file
        
          vector<string> buffer_args = parsecommand(buffer); 
          int idx = 0;
          // extract the peers we can contact 
          vector<string> peer_list;
          int peer_count = stoi(buffer_args[idx]);
          for(int i = 0; i < peer_count; ++i) {
            ++idx;
            peer_list.push_back(buffer_args[idx]);
          }
          ++idx;
          // extract the chunkwise hash 
          vector<string> hash_list;
          int chunk_count = stoi(buffer_args[idx]);
          for(int i = 0; i < chunk_count; ++i) {
            ++idx;
            hash_list.push_back(buffer_args[idx]);
          }

          // call each peer to get the chunks they have and 
          // decide on the order by the piece selection algo
 
          // call thread to fetch a chunk of data
        }
      }

      else if(strcmp(input_args[0].c_str(), "list_groups") == 0 || 
          strcmp(input_args[0].c_str(), "list_files") == 0 || 
          strcmp(input_args[0].c_str(), "list_requests") == 0 || 
          strcmp(input_args[0].c_str(), "show_downloads") == 0 || 
          strcmp(input_args[0].c_str(), "create_group") == 0) {
        // we don't need to process it further as we will output
        // the response in both the cases 
        cout << buffer << endl;
      }
      else {
        cout << buffer << endl;
      }


    }

  }

  // closing the connected socket
  close(client_sockfd);
}

// connectToServer is same as client_read_file
void connectToServer(int server_PORT, string server_IP) {
// void connectToServer(int server_PORT, string server_IP, string filename, int chunk_no) {

  // connect to the peer server side on IP:PORT and ask for chunk for a file
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
  const int chunk_size = 524288; // 1024*512B = 524288B = 512KB 
  char buffer[chunk_size]; 

  // We want to store the chunks that we get into a file, so we will
  // create a new file with the filename that we want and write the 
  // buffer into it

  string filename = "AOS_Assignment3.pdf";
  int chunk_no = 1; 
  // input_msg = filename + " " + chunk_no; 
  input_msg = filename + " " + to_string(chunk_no); 

  // getline(cin, input_msg);
  cout << "Message to send: " << input_msg << endl;
  // send input message to the server
  e = send(client_sockfd, input_msg.c_str(), input_msg.size(), 0);
  if(e < 0) {
    cout << "Failed to send message" << endl;
    return;
  }
  else if(e == 0) {
    cout << "Empty message sent to server" << endl;
  }
  else {
    cout << "Message sent from client" << endl; 
  }

  // bzero(buffer, chunk_size); // DEPRECATED
  memset(buffer, '0', 524288);
  e = read(client_sockfd, &buffer, chunk_size);
  if(e == -1) {
    cout << "Error reading message from buffer" << endl;
  }
  else if(e == 0) {
    cout << "Empty response from server" << endl;
    return;
  }
  else {
    cout << "Output: " << buffer << endl; 
  }

  // while(true) {
  //   // cin >> input_msg;
  // }


  // closing the connected socket
  close(client_sockfd);
}


int main(int argc, char *argv[]) {

  if(argc != 1 + 2) { // 1 arg always is the file argument
    cout << "Insufficient or excess arguments given" << endl;
    return -1;
  }

  int e; // for storing status to check if any error has occured 


  //extract IP address and PORT number from argument
  string input = argv[1];
  int idx = input.find(":");
  if(idx == -1) {
    cout << "Incorrect <IP>:<PORT> input" << endl;
    return -1;
  }
  cout << idx << endl;

  //update ip_port string
  ip_port = input;

  string IP = input.substr(0,idx); // we get : at idx so we need length as idx 

  //get the client PORT number from the remaining string 
  int PORT = stoi(input.substr(idx + 1));

  string TRACKER_FILENAME = argv[2];
   
  cout << "IP: " << IP << endl;
  cout << "PORT: " << PORT << endl;

  // We want to run the main thread for server side, i.e., for listening 
  // and other threads for client side, i.e., requesting connections from others.
  // For this, we will create threads after setting up listen, for connecting 
  // to server side of others

  // --- SETUP SERVER SIDE OF PEER ---   
  
  // The port number and IP are that of the tracker. This is because
  // the peer will contact the tracker to gain knowledge about the
  // environment : the files, the peers it has and the groups it is part of

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
 
  // call the connectToTrackerServer function as a thread to connect to a tracker server
  thread t(connectToTrackerServer, server_PORT, server_IP);
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
