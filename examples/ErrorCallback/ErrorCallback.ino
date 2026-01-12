#include <Seeed_Arduino_FS.h>

// Error callback function example
void errorCallback(int32_t code, int32_t detail) {
    Serial.print("SeeedFS Error: code ");
    Serial.print(code);
    Serial.print(" (detail: ");
    Serial.print(detail);
    Serial.println(")");
    // See Seeed_Error.h for error code descriptions
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    // Register error callback function
    seeedfs_setErrorCallback(errorCallback);

    // Initialize filesystem
    if (!SD.begin()) {
        Serial.println("SD initialization failed!");
        return;
    }

    Serial.println("SD initialized successfully.");

    // Try to open a nonexistent file to trigger error
    File file = SD.open("/nonexistent.txt", FILE_READ);
    if (!file) {
        Serial.println("File open failed as expected.");
    }

    // Check last error
    int32_t lastError = seeedfs_lastError();
    if (lastError != SEEED_FS_OK) {
        Serial.print("Last error: code ");
        Serial.println(lastError);
        Serial.print("Detail: ");
        Serial.println(seeedfs_lastErrorDetail());
        // See Seeed_Error.h for error code descriptions
    }
}

void loop() {
    // Do nothing
}