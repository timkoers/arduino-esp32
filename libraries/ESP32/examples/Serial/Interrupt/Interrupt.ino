#define BUFFER_SIZE 8

// This semaphore is here to handle the interruption of the loop when the interrupt is running.
SemaphoreHandle_t bufferSemaphore;

static volatile char inputBuffer[BUFFER_SIZE];
static volatile size_t inputBufferLength = 0;

// Please keep in mind, since the ESP32 is dual core, 
// the interrupt will be running on the same core as the setRxInterrupt function was called on.
static void IRAM_ATTR onSerialRX(uint8_t character, void* user_arg){
	
	// Cast the user_arg back to a array
	char* buffer = (char*)user_arg;
	
	BaseType_t xHighPriorityTaskWoken;
	
	if(xSemaphoreTakeFromISR(bufferSemaphore, &xHighPriorityTaskWoken) == pdTRUE){	
		if(inputBufferLength < BUFFER_SIZE){
			buffer[inputBufferLength++] = (char)character;
		}
		xSemaphoreGiveFromISR(bufferSemaphore, &xHighPriorityTaskWoken);
	}
}

void setup()
{
  bufferSemaphore = xSemaphoreCreateBinary();
	
  Serial.begin(115200);
  Serial2.begin(115200);
  
  // The user_arg is expected to be a void pointer (void*), 
  // so take the reference of the variable, and it to a void*
  Serial2.setRxInterrupt(onSerialRX, (void*)&inputBuffer);
}

void loop()
{ 
  if(xSemaphoreTake(bufferSemaphore, portMAX_DELAY) == pdTRUE && inputBufferLength == (BUFFER_SIZE - 1)){
	for(size_t i = 0; i < inputBufferLength; i++){
	  Serial.write(inputBuffer[i]);
	  
	  // Clear the buffer
	  inputBuffer[i] = '\0';
	}
	// Clear the bufferLength
	inputBufferLength = 0;
	
	xSemaphoreGive(bufferSemaphore);
  }
  
  delay(1000); // Wait for one second
}
