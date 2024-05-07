//DataStruct for LoRa payload 
struct dataStruct{
  uint8_t nodeId=0; //Node-Id of this Node
  double gpsLatitude=0; //GPS-Location Latitude
  double gpsLongitude=0; //GPS-Location Longitude
  uint8_t gpsHour=0; //GPS-Time Hour
  uint8_t gpsMinute=0; //GPS-Time Minute
  uint8_t gpsSecond=0; //GPS-Time Second
  float temp1 = 0; 
  float temp2 = 0;
  float pressure = 0; 
  float distance = 0; 
}LoRaData;

void dummy_sensor_function()
{
  // Do Sensor stuff, fill the Datastruct
  LoRaData.nodeId = 1; //example
  LoRaData.gpsLatitude = 53.503215;
  LoRaData.gpsLongitude = 8.074883;
  LoRaData.gpsHour = 12;
  LoRaData.gpsMinute = 56;
  LoRaData.gpsSecond = 08;
  LoRaData.temp1 = 20.25;
  LoRaData.temp2 = 18.15;
  LoRaData.pressure = 100001;
  LoRaData.distance = 210.20;
}

void send_LoRa() //Function sendLoRa() sends the LoRa Message via the Mesh-Messaging mechanism of the RadioHead Library.
{
  if (process_done_sendLoRa) //if boolean "process_done_sendLoRa" is true. See comments in function "start_processing()" for further explanation
  {  

  byte tx_buf[sizeof(LoRaData)]={0}; //Buffer for LoRa Payload 

  memcpy(tx_buf, &LoRaData, sizeof(LoRaData)); //copying the LoRa payload dataset into a Buffer for transmission 

  // sendtoWait() sends a message to the destination node. Initialises the RHRouter message header (the SOURCE address is set to the address of this node, HOPS to 0)
  // and calls route() which looks up in the routing table the next hop to deliver to.
  // If no route is known, initiates route discovery and waits for a reply.
  // Then sends the message to the next hop Then waits for an acknowledgement from the next hop (but not from the destination node (if that is different). 

  if (manager->sendtoWait(tx_buf, sizeof(tx_buf), NODEADRESS1) == RH_ROUTER_ERROR_NONE)
  {
    uint8_t len = sizeof(buf); //size of a Buffer needed for function "recvfromAckTimeout()"
    uint8_t from; // Stores the source ID when a message is acknowledged in "recvfromAckTimeout()"

  // recvfromAckTimeout() starts the receiver, processes and possibly routes any received messages addressed to other nodes and delivers any messages addressed to this node.
  // If there is a valid application layer message available for this node (or RH_BROADCAST_ADDRESS),
  // send an acknowledgement to the last hop address (blocking until this is complete),then copy the application message payload data to buf and return true else return false.
  // If a message is copied, *len is set to the length. If from is not NULL, the originator SOURCE address is placed in *source.
  // If to is not NULL, the DEST address is placed in *dest. This might be this nodes address or RH_BROADCAST_ADDRESS. 
  // This is the preferred function for getting messages addressed to this node. If the message is not a broadcast, acknowledge to the sender before returning. 
        if (manager->recvfromAckTimeout(buf, &len,2000, &from)) //Buffer, size of the Buffer, Timeout length, Source of received Ack
    {
      Serial.print("got OK : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);  
    }
        else
    {
      Serial.println("No reply, any Nodes running?");
    }
  }
    else
     Serial.println("sendtoWait failed");
  }
}