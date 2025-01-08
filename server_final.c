#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> // size_t, ssize_t...
#include <sys/socket.h> // funkce pro socket programming
#include <netdb.h> // getaddrinfo, gethostbyname, struct addrinfo
#include <string.h> // strlen...
#include <arpa/inet.h> // prace s IP adresami => inet_pton, ine_pton
#include <netinet/in.h> // specifikace internetovych protokolu => sin_family/port/adrr
#include <unistd.h> // close()
#include <fcntl.h>

#define BACKLOG 5 // kolik pripojeni muze byt v queue

int main() {
    struct addrinfo hints, *result; // result (struktura, kterou budeme pouzivat) se naplni podle structu hints

    memset(&hints, 0, sizeof(hints)); // pro jistotu nastavujeme hodnotu na 0, aby byla struktura opravdu prazdna
    // nastavovani prikladne struktury pro *result
    hints.ai_family = AF_UNSPEC; // je nam jedno, jaka to bude IP adresa (IPv4/IPv6)
    hints.ai_socktype = SOCK_STREAM; // socket bude typu Stream socket
    // hints.ai_protocol = 0 // vybere automaticky na zaklade typu socketu, mohu vynechat, protoze jsem uz to jakoby vyplnil pomoci memset
    hints.ai_flags = AI_PASSIVE; // server bude naslouchat na kazde IP adrese meho PC (WiFi, Ethernet, loopback) => nekdo na WiFi, pokud zna moji privatni IP adresu, tak se muze podivat na muj server
    // podiva se jake IP adresy jsou k dispozici na jakemkoli rohrani (WiFi, Ethernet)

    int status;
    if ( (status = getaddrinfo(NULL, "8080", &hints, &result) ) != 0) {
        perror("getaddrinfo() selhala"); // : human-readable form errno variable
    }

    int socketfd;
    socketfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (socketfd == -1) {
        perror("socket() selhal");
        return 1;
    }
    
    // setsockopt nastaveni socketu, socket adresa muze byt z predesleho volani jeste "plna/spojena" => TIME_WAIT, setsockopt nastavi to aby se tato socket adresa mohla pouzit znovu
    int optval = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt() selhal");
        return 1;
    }


    status = bind(socketfd, result->ai_addr, result->ai_addrlen);
    if (status != 0) {
        perror("bind() selhal");
        return 1;
    }

    freeaddrinfo(result); // program, ktery se bude chovat jako server jsme nastavili (sitova nastaveni), uz ho nepotrebujeme

    status = listen(socketfd, 5);
    if (status != 0) {
        perror("listen() selhal");
        return 1;
    }

    char htmlCode[500];

    int fileDescriptor = open("Documents/index.html", O_RDONLY);
    if (fileDescriptor == -1)
    {
        perror("open() selhalo");
        return 1;
    }

    size_t bytesRead = read(fileDescriptor, htmlCode, sizeof(htmlCode));
    close(fileDescriptor);
    htmlCode[bytesRead] = '\0';

    if (bytesRead <= 0)
    {
        perror("read() selhalo");
        return 1;
    }

    char httpResponseHeader[130];

    snprintf(httpResponseHeader, sizeof(httpResponseHeader), 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Cache-Control: no-cache\r\n"
    "Content-Length: %zu\r\n\r\n", bytesRead);

    char httpResponse[sizeof(htmlCode) + sizeof(httpResponseHeader)];
    snprintf(httpResponse, sizeof(httpResponse), "%s%s", httpResponseHeader, htmlCode);

    // printf("HTML CODE:\n%s", htmlCode);
    // printf("BYTES READ: %zu", bytesRead);
    // fflush(stdout);

    // accept primarne vraci novy socket descriptor ke komunikaci, ale pokud chceme zjistit informace o klientovi, tak tam musime vlozit pointer na sockaddr_in/in6/sockadd_storage (IPv4/IPv6)
    int comSocketfd;
    struct sockaddr_storage client_addr; // IPv4/IPv6
    socklen_t sizeClientAddr = sizeof(client_addr); // velikost struktury client_addr pro funkci accept
    
    while(1) {
        comSocketfd = accept(socketfd, (struct sockaddr*)&client_addr, &sizeClientAddr);

        if (comSocketfd == -1) {
            perror("accept() selhal");
            return 1;
        }

        status = send(comSocketfd, httpResponse, strlen(httpResponse), 0);
        
        if (status == -1) {
            perror("send() selhal");
            return 1;
        }

        close(comSocketfd);
    }
    return 0;    
}

    
// if ( ( status = connect(socketfd, result->ai_addr, result->ai_addrlen)) != 0) {
//     perror("connect() selhal");
// }

// size_t lenHttpCode = strlen(httpCode);
//     // \r znamena vratit kurzor na zacatek radku
//     snprintf(httpRequest, sizeof(httpRequest), "HTTP/1.1 200 OK\n\rContent-Type: text/html; charset=UTF-8\n\rContent-Length: %zu\n\rConnection: Close\n\n\r%s", lenHttpCode, httpCode);

// char httpCode[] = "<!DOCTYPE html>                                                                      "
    //                   " <html lang='en'>                                                                    "
    //                   "  <head>                                                                             "
    //                   "      <meta charset='UTF-8' />                                                       "
    //                   "      <meta name='viewport' content='width=device-width, initial-scale=1.0' />       "
    //                   "      <title>Web server - C</title>                                                  "
    //                   "      <style>                                                                        "
    //                   "          .middle {                                                                  "
    //                   "              display: flex;                                                         "
    //                   "              justify-content: center;                                               "
    //                   "              align-items: center;                                                   "     
    //                   "          }                                                                          "
    //                   "      </style>                                                                       "
    //                   "  </head>                                                                            "
    //                   "  <body>                                                                             "
    //                   "      <div class='middle'>                                                           "
    //                   "          <h1>Web server coded in C</h1>                                             "
    //                   "      </div>                                                                         "
    //                   "      </body>                                                                        "
    //                   "  </html>"; 
