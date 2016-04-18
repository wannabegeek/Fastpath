# FastPath Messaging
[![Build Status](https://travis-ci.org/wannabegeek/DCM.svg?branch=master)](https://travis-ci.org/wannabegeek/DCM)
## Introduction

This documentation described the C++ API for using the FastPath messaging bus.

FastPath is a very low latency messaging framework, for communicating
between process either running locally or on different host within the network.

## Getting Started

The idea of FastPath is to be able to quickly develop applications communicating with one-another
via a fast, resilient and reliable messaging bus.

The first call your application should make is `fp::Session::initialise()` to initialise the session.
This will start up the internal working and threads required by the framework.
The application should call `fp::Session::destroy()` to shut the session
down again at the end of the applications life.

### Create Message Publisher

Below is an example of how to create an application which starts up, published a message
and shuts down again.

To be able to do any message broadcasting between processes, we need to have a `fprouter` running.
To start this up in the we can run;
```bash
$ fprouter --service 6363
```

Hopefully the code along with the comments is pretty self explanatory.
```cpp
#include <fastpath.h>

int main(int argc, char *argv[]) {
    // Initialise the FastPath session
    fp::Session::initialise();

    // Create a transport connection to the `fprouter` running locally on port 6363
    fp::TCPTransport transport("tcp://localhost:6363", "");

    // Check the transport is valid
    if (transport.valid()) {

        // Create a message object
        fp::Message msg;

        // Set the subject and some fields on the message
        msg.setSubject("MY.TEST.SUBJECT");
        msg.addDataField("name", "Joe Bloggs");
        msg.addScalarField("age", 21);

        // Send the message on the transport and check for errors
        if (transport.sendMessage(msg) == fp::OK) {
            std::cout << "Message send successfully" << std::endl;
        } else {
            std::cerr << "Failed to send message" << std::endl;;
        }
    } else {
        std::cerr << "Failed to send message - transport not connected" << std::endl;
    }

    // Shutdown our session again
    fp::Session::destroy();
}
```

