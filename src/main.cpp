#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>

/*функция обратного вызова для получения статуса отправки электронной почты */
void smtpCallback(SMTP_Status status);

namespace 
{
  /*объект сеанса SMTP, используемый для отправки электронной почты*/
  SMTPSession smtp;

  String default_ssid = "fh_3f41a0";
  String default_wifi_password = "wlanc0be5f";

  MB_String default_host = "smtp.gmail.com";
  MB_String default_author_email = "labalabesp8266@gmail.com";
  MB_String default_email_password = "xsea vhee vszt dwbd";
  uint16_t default_port = 465;
  MB_String default_recipient_email = "IDONTKNOWYOUREMAIL@gmail.com";
  MB_String default_message = "<div style=\"color:#2f4468;\"><h1>IoT will be on the next Saturday</h1><p>- sent from esp8266</p></div>";
}

void scanWiFi() 
{
  Serial.println("Scan Start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("Scan Done");
  if (n == 0) 
  {
      Serial.println("No Networks Found");
  } 
  else 
  {
      Serial.print(n);
      Serial.println(" Networks Found");
      for (int i = 0; i < n; i++) 
      {
          // Print SSID and RSSI for each network found
          Serial.print(i + 1);
          Serial.print(": ");
          Serial.print(WiFi.SSID(i));
          Serial.print(" (");
          Serial.print(WiFi.RSSI(i));
          Serial.print(")");
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
          delay(10);
      }
  }
  Serial.println("");
}

bool command_handler(const String& command) 
{
  if(command == "list") 
  {
    scanWiFi();
  }
  else if (command == "connect") 
  {
    Serial.print("SSID: ");
    while(Serial.available() == 0) {

    }
    String ssid = Serial.readString();
    ssid.replace("SSID: ", "");
    ssid.trim(); 
    Serial.print("PASSWORD: ");
    while(Serial.available() == 0) { 

    }
    String password = Serial.readString();
    password.replace("PASSWORD: ", "");
    password.trim(); 
    if(ssid.isEmpty() || password.isEmpty())
    {
      ssid = default_ssid;
      password = default_wifi_password;
    }
    Serial.print("Connecting to " + ssid + " with password " + password + " ");
    WiFi.begin(ssid.c_str(), password.c_str());

    while (WiFi.status() != WL_CONNECTED) 
    {
      Serial.print(".");
      delay(200);
    }
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    smtp.debug(1);
  }
  else if (command == "default_send") 
  {
    /* устанавливает функцию обратного вызова для получения результата отправки*/
    smtp.callback(smtpCallback);
  
    /*объявляет данные конфигурации сеанса*/
    ESP_Mail_Session session;
  
    /*устанавливает конфигурацию сеанса*/
    session.server.host_name = default_host;
    session.login.email = default_author_email;
    session.login.password = default_email_password;
    session.server.port = default_port;
    session.login.user_domain = "";
  
    /*объявляет класс сообщений*/
    SMTP_Message message;
  
    /*устанавливает отправителя в письме*/
    message.sender.name = "ESP8266";
    message.sender.email = default_author_email;
    message.subject = "ESP8266 Test Email";
    message.addRecipient("TupoaMudille", default_recipient_email);
  
    /*HTML настройка письма и текста*/
    message.html.content = default_message;
    message.text.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
    /*подключение к серверу с текущим конфигом*/
    if (!smtp.connect(&session))
    {
      return false;
    }
    /*начали отправку письма, не получилось и закрыли сессию*/
    if (!MailClient.sendMail(&smtp, &message))
    {
      Serial.println("error on send, " + smtp.errorReason());
    }
  }
  else
  {
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
}

void loop() 
{
  Serial.print("</ ");
  while (Serial.available() == 0) {
    // waiting for user input
  }
  String command = Serial.readString();
  command.replace("</ ", "");
  command.trim();
  command.toLowerCase();
  if(!command_handler(command)) 
  {
    Serial.println(command + " is invalid command");
  }
}

/*вызов функции для определения статуса отправки*/
void smtpCallback(SMTP_Status status)
{
  /*выписываем статус в Монитор порта*/
  Serial.println(status.info());

  /*выписываем результат*/
  if (status.success())
  {
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message has been sent: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message hasn't been sent: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /*получаем результат отправленного объекта*/
      SMTP_Result result = smtp.sendingResult.getItem(i);
      time_t ts = (time_t)result.timestamp;
      localtime_r(&ts, &dt);

      ESP_MAIL_PRINTF("Message Number: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date and Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients);
      ESP_MAIL_PRINTF("Object: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
