# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS
        main.cpp
        
        storage/Storage.cpp
        uploader/Uploader.cpp
        memoryW25Q/W25Q128.cpp
        dateTime/DateTimeSensor.cpp
    	eventMenager/EventMenager.cpp
    	msgCreater/MessageCreater.cpp
    	
    	sensor/ISensor.cpp
    	sensor/ds18b20/TermoSensor.cpp
    	sensor/dustSensor/DustSensor.cpp
    	sensor/sensorMenager/SensorMenager.cpp
    	
    	networkSystem/wifi/WIFI.cpp
    	networkSystem/NetworkMenager.cpp
    	networkSystem/webSocket/WebSocket.cpp
        networkSystem/multicastDNS/MulticastDNS.cpp
    	
    	msgFromServerProcessors/MsgFromServerProcessors.cpp
        msgFromServerProcessors/collectionProcessor/ICollectionProcessor.cpp
    	msgFromServerProcessors/collectionProcessor/PostSettingCollection.cpp
    	       
    INCLUDE_DIRS
    	
    	storage
    	uploader
    	dateTime
    	msgCreater
    	memoryW25Q
    	eventMenager
    	
    	msgFromServerProcessors
    	msgFromServerProcessors/collectionProcessor
    	
    	sensor
    	sensor/ds18b20
    	sensor/dustSensor	
    	sensor/sensorMenager
    	
    	networkSystem
    	networkSystem/wifi
    	networkSystem/webSocket
    	networkSystem/multicastDNS
    	
    	commonEnums
    	commonStruct
    	common_const
        commonFunctions
    	
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    	PRIV_REQUIRES       # optional, list the private requirements
)
