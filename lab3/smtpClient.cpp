// Command to run : g++ -std=c++17 base64.cpp smtpClient.cpp -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -o smtpClient

// libraries for socket communication and SSL/TLS support
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SOCKET_ERROR_CODE errno
#define CLOSE_SOCKET(s) close(s)
#define INIT_SOCKETS()
#define CLEANUP_SOCKETS()

// Include OpenSSL libraries for SSL/TLS encryption and decryption
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>
#include <string>
#include "base64.h"
#include <fstream>
#include <sstream>

#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

// send SMTP command and check server's response
void sendSMTPCommand(SSL *ssl, const std::string &command, const std::string &expectedResponse)
{
    SSL_write(ssl, command.c_str(), command.size());
    char response[1024] = {0};
    SSL_read(ssl, response, sizeof(response) - 1);
    std::cout << "Server Response: " << response;
    if (expectedResponse != "" && std::string(response).find(expectedResponse) == std::string::npos)
    {
        throw std::runtime_error("Unexpected SMTP response.");
    }
}
#include <string>
#include <algorithm>

// determine the MIME type based on file extension
std::string mimeType(const std::string &filename)
{
    // Get file extension
    size_t dot_pos = filename.find_last_of(".");
    if (dot_pos == std::string::npos)
        return "application/octet-stream"; // No extension found

    std::string ext = filename.substr(dot_pos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower); // Normalize to lowercase

    // Check file extension
    if (ext == ".txt")
        return "text/plain";
    else if (ext == ".pdf")
        return "application/pdf";
    else if (ext == ".jpg" || ext == ".jpeg")
        return "image/jpeg";
    else if (ext == ".png")
        return "image/png";
    else if (ext == ".gif")
        return "image/gif";
    else if (ext == ".docx")
        return "application/msword";
    else if (ext == ".xls" || ext == ".xlsx")
        return "application/vnd.ms-excel";
    else if (ext == ".zip")
        return "application/zip";
    else if (ext == ".mp3")
        return "audio/mpeg";
    else if (ext == ".mp4")
        return "video/mp4";

    // Default case for unknown extensions
    return "application/octet-stream";
}

// read the content of a file into a string
std::string readFile(const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Cannot open file: " + filepath);
    }
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

int main()
{

    // SMTP server details
    const char *smtpServer = "smtp.gmail.com";
    const int smtpPort = 465; // SMTP over SSL

    // Sender credentials
    const std::string emailSender = "snehamrzzn@gmail.com";
    const std::string emailPassword = "youraddpassword";

    // Recipient details
    const std::string emailReceiver = "jooohn.eth@gmail.com";                  // Recipient email address
    const std::vector<std::string> ccRecipients = {"diwanshimagar@gmail.com"}; // CC recipients list
    const std::vector<std::string> bccRecipients = {"jooohnng@gmail.com"};     // BCC recipients list

    // Email subject and body content
    const std::string subject = "Test Email";
    const std::string body = "This is a test email sent using  during Lab 4 BTN";

    try
    {
        // Create socket
        SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET)
        {
            throw std::runtime_error("Socket creation failed.");
        }

        // Resolve SMTP server address
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(smtpPort);
        struct hostent *host = gethostbyname(smtpServer);
        memcpy(&serverAddr.sin_addr, host->h_addr_list[0], host->h_length);

        // Connect to the SMTP server
        if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            close(sock);
            throw std::runtime_error("Connection to SMTP server failed.");
        }

        // Initialize OpenSSL
        SSL_library_init();
        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();
        SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
        if (!ctx)
        {
            close(sock);
            throw std::runtime_error("Failed to create SSL context.");
        }

        // Create SSL structure and connect it to the socket
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sock);
        if (SSL_connect(ssl) <= 0)
        {
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);
            throw std::runtime_error("SSL connection failed.");
        }

        // SMTP Communication
        char buffer[1024];
        SSL_read(ssl, buffer, sizeof(buffer) - 1); // Read initial response

        sendSMTPCommand(ssl, "EHLO localhost\r\n", "250");
        sendSMTPCommand(ssl, "AUTH LOGIN\r\n", "334");

        // Encode credentials in Base64
        std::string base64Email = base64_encode(reinterpret_cast<const unsigned char *>(emailSender.c_str()), emailSender.size());
        std::string base64Password = base64_encode(reinterpret_cast<const unsigned char *>(emailPassword.c_str()), emailPassword.size());

        sendSMTPCommand(ssl, base64Email + "\r\n", "334");
        sendSMTPCommand(ssl, base64Password + "\r\n", "235");

        // Send email
        sendSMTPCommand(ssl, "MAIL FROM:<" + emailSender + ">\r\n", "250");
        sendSMTPCommand(ssl, "RCPT TO:<" + emailReceiver + ">\r\n", "250");

        // Send CC and BCC recipients
        for (const auto &cc : ccRecipients)
            sendSMTPCommand(ssl, "RCPT TO:<" + cc + ">\r\n", "250");

        for (const auto &bcc : bccRecipients)
            sendSMTPCommand(ssl, "RCPT TO:<" + bcc + ">\r\n", "250");

        std::string ccHeader;
        if (!ccRecipients.empty())
        {
            ccHeader = "Cc: ";
            for (size_t i = 0; i < ccRecipients.size(); ++i)
            {
                ccHeader += ccRecipients[i];
                if (i < ccRecipients.size() - 1)
                {
                    ccHeader += ", ";
                }
            }
            ccHeader += "\r\n";
        }

        // File to attach (relative path)
        std::string file = "./file/hello.docx"; // Path to the file you want to attach

        // Read the File and Encode it in Base64
        std::string fileContent = readFile(file);
        std::string encodedFile = base64_encode(reinterpret_cast<const unsigned char *>(fileContent.c_str()),
                                                fileContent.length());

        // Construct the Email with an file Attachment
        std::string emailContent = "From: " + emailSender + "\r\n"
                                                            "To: " +
                                   emailReceiver + "\r\n" +
                                   ccHeader + // CC recipients visible in header
                                   "Subject: " + subject + "\r\n"
                                                           "MIME-Version: 1.0\r\n"
                                                           "Content-Type: multipart/mixed; boundary=\"boundary\"\r\n\r\n"
                                                           "--boundary\r\n"
                                                           "Content-Type: text/plain; charset=utf-8\r\n\r\n" +
                                   body + "\r\n\r\n"
                                          "--boundary\r\n"
                                          "Content-Type:" +
                                   mimeType(file) + "; name =\"" +
                                   file + "\"\r\n"
                                          "Content-Transfer-Encoding: base64\r\n"
                                          "Content-Disposition: attachment; filename=\"" +
                                   file + "\"\r\n\r\n" +
                                   encodedFile + "\r\n\r\n"
                                                 "--boundary--\r\n.\r\n";
        // Send the Email Data
        sendSMTPCommand(ssl, "DATA\r\n", "354");
        sendSMTPCommand(ssl, emailContent, "250");

        // Quit
        sendSMTPCommand(ssl, "QUIT\r\n", "221");

        // Cleanup
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock);

        std::cout << "Email sent successfully.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}