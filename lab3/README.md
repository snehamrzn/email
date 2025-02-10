# User manual document

The application enables users to send emails via an SMTP server, including support for attachments, CC, and BCC recipients.

# Prerequisites

1. OpenSSL Libraries: The application uses OpenSSL for secure email communication.
2. Gmail SMTP Server Access: This application connects to Gmail’s SMTP server (smtp.gmail.com)
   - App Passwords: create an App Password in your Gmail settings and use it instead of your regular password.

# Clone or Download the SMTP Client Source Code

- OpenSSL Libraries: These are required for secure SSL/TLS communication with the SMTP server.
- Base64 Encoding/Decoding: The application uses Base64 encoding to handle attachments.

# Sending an Email

The application follows these steps:

1. Establishes a Connection with the SMTP Server Over SSL.

- The program connects to the SMTP server using port 465, which is designated for SSL/TLS encryption. This ensures a secure transmission of data.

```
    const char *smtpServer = "smtp.gmail.com";
    const int smtpPort = 465; // SMTP over SSL
```

2. Enter the Credentials and Update the sender's email and password:

```
    const std::string emailSender = "youremail@gmail.com";
    const std::string emailPassword = "*** *** **** ****"; //App Password

```

3. Authenticates Using Base64-Encoded Credentials

- The email sender must authenticate with the SMTP server using Base64-encoded credentials. The username and password (App Password for Gmail) are converted to Base64 and sent securely.

```
   sendSMTPCommand(ssl, "AUTH LOGIN\r\n", "334");
   std::string base64Email = base64*encode(reinterpret_cast<const unsigned char *>(emailSender.c*str()), emailSender.size());
   std::string base64Password = base64_encode(reinterpret_cast<const unsigned char *>(emailPassword.c_str()), emailPassword.size());

```

4. Sends an Email to the Recipient

- Once authenticated, the email is constructed and sent using the SMTP protocol commands

```
        sendSMTPCommand(ssl, "MAIL FROM:<" + "emailSender@gmail.com" + ">\r\n", "250");
        sendSMTPCommand(ssl, "RCPT TO:<" + "emailReceiver@gail.com" + ">\r\n", "250");
```

5. Including CC and Bcc

- CC and BCC recipients are added using additional "RCPT TO" commands.

```

for (const auto &cc : ccRecipients)
    sendSMTPCommand(ssl, "RCPT TO:<" + cc + ">\r\n", "250");

for (const auto &bcc : bccRecipients)
    sendSMTPCommand(ssl, "RCPT TO:<" + bcc + ">\r\n", "250");

```

Specify CC and BCC recipients using std::vector:

```
  const std::vector<std::string> ccRecipients = {"recipient@gmail.com"};
  const std::vector<std::string> bccRecipients = {"recipient@gmail.com"};
```

6. Attaches a File

- The program reads the file (under the `mimetype` function), encodes it in Base64, and attaches it to the email.
  Ensure that the file you want to attach is located at the specified path.

```
    std::string file = "./file/lab3.txt"; // Path to the file you want to attach

    std::string fileContent = readFile(file); // Read the File
    std::string encodedFile = base64_encode(reinterpret_cast<const unsigned char *>(fileContent.c_str()),
                                                fileContent.length());    //Encode it in Base64

```

7. Terminates the Session

- the program signals the end of the email, confirms transmission, and closes the connection.

```
     // Quit
     sendSMTPCommand(ssl, "QUIT\r\n", "221");

     // Cleanup
     SSL_shutdown(ssl);
     SSL_free(ssl);
     SSL_CTX_free(ctx);
     close(sock);

```

# Compile and Build the Application

- To run the application, you can use the following command:

```
  g++ -std=c++17 base64.cpp smtpClient.cpp -I/opt/homebrew/opt/openssl@3/include -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto -o smtpClient

```

# Run the Application

- run the application using the following command:

```
   ./smtp_client
```

# Conclusion

- This SMTP client allows you to send emails securely with attachments, CC, and BCC recipients using Gmail’s SMTP server.
