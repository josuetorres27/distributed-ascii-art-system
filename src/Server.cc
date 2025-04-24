/**
 * Distributed ASCII Art System
 * Module: Server.cc
 *
 * Multi-threaded HTTP-like server that listens for incoming client connections
 * and serves ASCII art figures from a simulated block file system.
 *
 * Author: Josue Torres Sibaja
 * Copyright (c) 2025 Josue Torres Sibaja
 * Licensed under the MIT License.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <filesystem>
#include <algorithm>  // std::remove
#include <cctype>     // std::isspace
#include <cstdint>
#include <vector>
#include "FileSystem.h"
#include "VSocket.h"
#include "Socket.h"

#define PORT 1234
#define BUFSIZE 4096

/**
 * Handles a client request in a separate thread.
 * Parses HTTP request and routes to raw LIST or figure content.
 *
 * @param client The client socket
 * @param fs Reference to the FileSystem instance
 */
void task(VSocket *client, FileSystem& fs) {
  char buffer[BUFSIZE] = {};
  client->Read(buffer, BUFSIZE);
  std::string request(buffer);

  std::string response;

  if (request.substr(0, 4) == "GET ") {
    size_t pos1 = request.find("GET ");
    size_t pos2 = request.find(" HTTP");

    if (pos1 != std::string::npos && pos2 != std::string::npos) {
      std::string url = request.substr(pos1 + 4, pos2 - (pos1 + 4));

      url.erase(std::remove(url.begin(), url.end(), '\r'), url.end());
      url.erase(std::remove(url.begin(), url.end(), '\n'), url.end());

      if (url == "/LIST") {
        std::string list = fs.getFigureList();
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\n" + list;
      } else {
        if (url.find("/GET/") == 0) {
          url = url.substr(5);
        } else if (url.find("/") == 0) {
          url = url.substr(1);
        }

        std::string figureName = url;
        std::string figure = fs.getFigureContent(figureName);

        if (!figure.empty() && figure != "Figure not found.\n") {
          response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\n" + figure;
        } else {
          response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\nFigure not found";
        }
      }
    }
  } else {
    response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nConnection: close\r\n\r\nInvalid command";
  }

  client->Write(response.c_str(), response.size());
  client->Close();
}

int main() {
  FileSystem fs("disk.bin");

  int figureCount = fs.countFigures("ascii_figures");
  int dirBlocks = (figureCount + DIR_ENTRIES - 1) / DIR_ENTRIES;

  if (dirBlocks == 0) {
    dirBlocks = 1;
  }

  if (!std::filesystem::exists("disk.bin")) {
    fs.initDisk(dirBlocks);
  }

  fs.loadFiguresFromFolder("ascii_figures");

  VSocket *s1, *client;
  s1 = new Socket('s', true);

  s1->Bind(PORT);
  s1->MarkPassive(5);

  std::cout << "====================================================\n";
  std::cout << "   DISTRIBUTED ASCII ART SYSTEM - HTTP SERVER       \n";
  std::cout << "====================================================\n";
  std::cout << " [✓] Server listening on port: " << PORT << "\n";
  std::cout << " [✓] Total ASCII figures indexed: " << figureCount << "\n";
  std::cout << "====================================================\n\n";

  for (;;) {
    client = s1->AcceptConnection();
    std::thread(task, client, std::ref(fs)).detach();
  }

  return 0;
}
