#ifdef	nsox_protocol_h
#define nsox_protocol_h

namespace nsox{

struct Protocol
{
		virtual			void onRequest(Request& request);
};

} // end of namespace nsox
#endif //nsox_thread_conn_handler_h