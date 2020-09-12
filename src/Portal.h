class Portal{
	private:
		ESP8266WebServer* server;
    	WiFiUDP UDP;
		IPAddress myIP; 
		
		String apSsid= "NewDevice2" ;
		String apPassword ="12345678";
		
    char ssid[32]={};
    char password[32]={};
    char dName[32]={};
    int localUdpPort = 65001;
    
    bool credentialsFound;
    bool connectedToLocal;
    bool triedConnectedToLocal;
    
		String GetHtmlTemplate(String content);
		
		//pages
		void showIndexPage();
		void showConfigurePage();
		void showSavePage();
		void showInfoPage();
    	void showServerInfoPage();
		void showNotFoundPage();

    //credentials
    void loadCredentials();
    void saveCredentials();

    //connections
    bool connectToLocal();
	void sendBroadcastPacket();

	public:
		Portal();
		void initialize();
		void handleClient();
		

};
