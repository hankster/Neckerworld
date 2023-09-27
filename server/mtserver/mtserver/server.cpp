/*
   main.cpp

   Multithreaded work queue based example server in C++.
  
   ------------------------------------------

   Copyright (c) 2013 Vic Hargrave

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "thread.h"
#include "wqueue.h"
#include "tcpacceptor.h"

string cube_decode_message(char* msg, int msglen);

class WorkItem
{
    TCPStream* m_stream;
 
  public:
    WorkItem(TCPStream* stream) : m_stream(stream) {}
    ~WorkItem() { delete m_stream; }
 
    TCPStream* getStream() { return m_stream; }
};

class ConnectionHandler : public Thread
{
    wqueue<WorkItem*>& m_queue;
 
  public:
    ConnectionHandler(wqueue<WorkItem*>& queue) : m_queue(queue) {}
 
    void* run() {
        // Remove 1 item at a time and process it. Blocks if no items are 
        // available to process.
        for (int i = 0;; i++) {
	  // printf("server.cpp: handler thread %lu, loop %d - waiting for item...\n", (long unsigned int)self(), i);
	  WorkItem* item = m_queue.remove();
	  printf("server.cpp: handler thread %lu, loop %d - got one item\n", (long unsigned int)self(), i);
	  TCPStream* stream = item->getStream();
	  printf("server.cpp: handler - new incoming stream\n");

	  char message[1500];
	  int message_length;
	  string response;
	  bool disconnect = false;
	  string goodbye = "{\"mesage_type\": \"GoodBye\"}";
	  
	  while ((message_length = stream->receive(message, sizeof(message))) > 0 ){
	    response = cube_decode_message(message, message_length);
	    if (response == goodbye) disconnect = true;
	    int response_length = response.size();
	    if (response_length > 0) stream->send(&response[0], response_length);
	  }

	  delete item; 
        }

        // Should never get here
        return NULL;
    }
};

wqueue<WorkItem*>  queue;
TCPAcceptor* connectionAcceptor;
std::vector<ConnectionHandler*>ch;

int server_main(int workers, int port, string ip)
{
    // Create the queue and consumer (worker) threads
    for (int i = 0; i < workers; i++) {
        ConnectionHandler* handler = new ConnectionHandler(queue);
        if (!handler) {
            printf("server.cpp: Could not create ConnectionHandler %d\n", i);
            exit(1);
        } 
        ch.push_back(handler);

	handler->start();
    }
 
    // Create an acceptor then start listening for connections
    if (ip.length() > 0) {
        connectionAcceptor = new TCPAcceptor(port, (char*)ip.c_str());
    }
    else {
        connectionAcceptor = new TCPAcceptor(port);        
    }                                        
    if (!connectionAcceptor || connectionAcceptor->start() != 0) {
        printf("server.cpp: Could not create an connection acceptor\n");
        exit(1);
    }

    // SetSocketBlockingEnabled(int fd, false);
      
    printf("server.cpp: Multithread server initialized with %d workers using port %d and ip %s\n", workers, port, &ip[0]);
    return 0;
}

// Add a work item to the queue for each connection
void server_loop()
{
  // printf("server.cpp: server_loop() start\n");

  WorkItem* item;
  TCPStream* connection = connectionAcceptor->accept(); 
  if (!connection) {
    // Non-blocking socket comes back immediately.
    // printf("server.cpp: Could not accept a connection\n");
    return;
  }
  item = new WorkItem(connection);
  if (!item) {
    printf("server.cpp: Could not create work item a connection\n");
    return;
  }
  queue.add(item);

  // printf("server.cpp: server_loop() end\n");

  return;
}

// Shut down the server
void server_stop()
{
  for (int i=0; i<(int)ch.size(); ++i){
    // printf("server.cpp: deleting handler thread %lu\n", (long unsigned int)ch.at(i)->self());
    delete ch.at(i);
  }
  delete connectionAcceptor;
  printf("server.cpp: server shutdown\n");
}
