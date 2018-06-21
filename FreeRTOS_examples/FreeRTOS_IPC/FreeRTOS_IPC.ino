#include <Arduino_FreeRTOS.h>
#include <queue.h>

static void vSenderTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );

QueueHandle_t xQueue;

void setup() {  
  Serial.begin(9600);
  xQueue = xQueueCreate(5, sizeof(long));
  if( xQueue != NULL)
  {
    /* When FreeRTOS xTaskCreate API Function faces unreasonable memory allocating,  
     * it will stop unless there are enough space or assign property size of memory.
     * Try assign 1000 bytes for each task.
     */
    xTaskCreate(vSenderTask, "Sender1", 128, (void * ) 100, 1, NULL);
    xTaskCreate(vSenderTask, "Sender2", 128, (void * ) 200, 1, NULL);
    xTaskCreate(vReceiverTask, "Receiver", 128, NULL, 2, NULL);
    vTaskStartScheduler();
    
  }
  else
  {
    Serial.println("ERROR: QUEUE CREATING FAIL!!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}

static void vSenderTask( void *pvParameters )
{
  long lValueToSend;
  portBASE_TYPE xStatus;

  lValueToSend = (long) pvParameters;

  for( ;; )
  {
    xStatus = xQueueSendToBack(xQueue, &lValueToSend, 0);
    if( xStatus != pdPASS)
    {
      Serial.println("Could not sent to the queue.");
    }
    vTaskDelay( 1000 / portTICK_PERIOD_MS );
    taskYIELD();
  }
}

static void vReceiverTask( void *pvParameters )
{
  long lReceiveValue;
  portBASE_TYPE xStatus;
  const TickType_t xTicksToWait = 2000 / portTICK_PERIOD_MS;

  for( ;; )
  {
    if(uxQueueMessagesWaiting(xQueue) != 0)
    {
      Serial.println("Queue should have been empty.");
    }
    xStatus = xQueueReceive(xQueue, &lReceiveValue, xTicksToWait);
    if(xStatus == pdPASS)
    {
      Serial.print("Received = ");
      Serial.println(lReceiveValue);
    }
    else
    {
      Serial.println("Could not receive from the queue.");
    }
  }
}
