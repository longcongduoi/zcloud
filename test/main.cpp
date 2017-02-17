/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: tamvh
 *
 * Created on January 10, 2016, 2:53 PM
 */

#include "ZApiHTTPClient.h"

using namespace std;

/*
 * 
 */
int main(int argc, char** argv) {
    ZApiHTTPClient *client = new ZApiHTTPClient();
    client->setHost("127.0.0.1");
    client->setPort(9980);
    client->setMethod("POST");
    client->setApi("api/user/register");
    client->setParam("email", "asd@gmail.com");
    client->setParam("password", "123123");
    client->setReqBody();
    client->sendRequest();
    std::string data = client->responseData();
}

