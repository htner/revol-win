class AppServer
{
public:
        void run(); // ÔËÐÐ        
        void setSessionManager(SessionManager* manager);
protected:
        SessionManager* sessions;            
        WokerManager*   workers;
}    
