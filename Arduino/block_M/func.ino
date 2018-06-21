void connect() {
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "0cce1b17", "43365f8d45ed2a89")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nconnectedMQTT!");

}
