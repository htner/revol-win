#pragma	   once
#include "project_def.h"

#ifdef REX_KOREA
	#include "korea.h"
#else
	#ifdef REX_ENUS
		#include "enus.h"
	#else
		#ifdef REX_TW
			#include "taiwan.h"
		#else
			#ifdef REX_VN
				#include "vietnam.h"
			#else
				#ifdef REX_RU
					#include "ru.h"
				#else
					//others
				#endif //REX_RU
			#endif //REX_VN
		#endif //REX_TW
	#endif //REX_ENUS
#endif //REX_KOREA


