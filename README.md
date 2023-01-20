# C++ Server with Socket
This project implements a client with 4 servers in 2 layers.

Client --- Central Server(ServerM) --- Certificate Server(ServerC)

                                    |

                                    |

                                    -- Record Server(ServerCS, ServerEE)
                                
WorkFlow:
    1 client type in username and passport in cred.txt.
    2 message sent to serverM, then transferred to serverC to verify accout.
    3 if succeed, client type in course information to query.
    4 ctrl+c to kill servers.

## Components of project:
    serverM, serverC, serverCS, serverEE, client

## What are components for
    **serverM** is responsible for receiving requests from client and sending to corresponding backend server. **serverC** deals with authentication request. **serverCS** and **serverEE** deal with query request. **client** is used to send request based on user inputs through standard IO.

## How to start
    $make
    $./serverM
    $./serverC
    $./serverEE
    $./serverCS
    $./client

## Restrictions
    The length of username and password are no less that 5 and no more than 50.
    User name contains only lower characters.
    Passwords are case sensitive.

## Additional .h files
    The serverM will require serverM.h.

