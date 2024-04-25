// Cargar la biblioteca de Wi-Fi
#include <WiFi.h>

// Credenciales de la red
const char* ssid = "-";
const char* password = "-";

// Establecer el número de puerto del servidor web en 80
WiFiServer server(80);

// Variable para almacenar la solicitud HTTP
String header;

// Variables auxiliares para almacenar el estado actual de salida
String output26State = "off";
String output27State = "off";

// Asignar variables de salida a los pines GPIO
const int output26 = 26;
const int output27 = 25;
const int output33 = 33;

// Tiempo actual
unsigned long currentTime = millis();
// Tiempo anterior
unsigned long previousTime = 0;
// Definir tiempo de espera en milisegundos (ejemplo: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Inicializar las variables de salida como salidas
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  pinMode(output33, OUTPUT);
  // Establecer las salidas a BAJO
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // Conectar a la red Wi-Fi con SSID y contraseña
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Imprimir la dirección IP local e iniciar el servidor web
  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();   

  if (client) {                             // Si se conecta un nuevo cliente,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("Nuevo Cliente.");       // imprimir un mensaje en el puerto serie
    String currentLine = "";                // crear una String para mantener los datos entrantes del cliente
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // bucle mientras el cliente esté conectado
      currentTime = millis();
      if (client.available()) {             // si hay bytes para leer del cliente,
        char c = client.read();             // leer un byte, luego
        Serial.write(c);                    // imprimirlo en el monitor serie
        header += c;
        if (c == '\n') {                    // si el byte es un carácter de nueva línea
          // si la línea actual está en blanco, tienes dos caracteres de nueva línea seguidos.
          // ese es el fin de la solicitud HTTP del cliente, así que envía una respuesta:
          if (currentLine.length() == 0) {
            // Los encabezados HTTP siempre comienzan con un código de respuesta
            // y un tipo de contenido para que el cliente sepa qué va a recibir, luego una línea en blanco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // alternar los GPIOs entre encendido y apagado
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 encendido");
              output26State = "on";
              digitalWrite(output26, HIGH);
              tone(output33, 1000); // Emitir sonido a 1000 Hz
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 apagado");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 encendido");
              output27State = "on";
              digitalWrite(output27, HIGH);
              digitalWrite(output26, LOW);
              noTone(output33);     // Detener el sonido
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 apagado");
              output27State = "off";
              digitalWrite(output27, LOW);
            }

            // Mostrar la página web HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta charset='UTF-8'><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS para estilizar los botones de encendido/apagado
            // Los atributos de color de fondo y tamaño de fuente
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            // Encabezado de la página web
            client.println("<body><h1>Botones de Emergencia</h1>");

            // Mostrar el estado actual, y botones de ENCENDIDO/APAGADO para el GPIO 26
            client.println("<p>Enviar Señal:</p>");
            // Si el estado de output26 es apagado, muestra el botón de ENCENDIDO
            if (output26State == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">Activar</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">Desactivar</button></a></p>");
            }

            // Mostrar el estado actual, y botones de ENCENDIDO/APAGADO para el GPIO 27
            client.println("<p>Recibir Señal:</p>");
            // Si el estado de output27 es apagado, muestra el botón de ENCENDIDO
            if (output27State == "off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">Activar</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">Desactivar</button></a></p>");
            }
            client.println("</body></html>");

            // La respuesta HTTP termina con otra línea en blanco
            client.println();
            // Salir del bucle
            break;
          } else { // si recibes una nueva línea, entonces limpia currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // si recibes cualquier cosa excepto un carácter de retorno de carro,
          currentLine += c;      // añádelo al final de currentLine
        }
      }
    }
    // Limpiar la variable header
    header = "";
    // Cerrar la conexión
    client.stop();
    Serial.println("Cliente desconectado.");
    Serial.println("");
  }
}
