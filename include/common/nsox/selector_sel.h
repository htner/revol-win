#ifndef nsox_basic_selector_h
#define nsox_basic_selector_h

#include <map>
#include <list>
#include "selector.h"
#include "nuautoptr.h"
#include "abstract_selector.h"

nsox_namespace_begin

class selector_select : public abstract_selector
{
public:
		virtual void run(nu_time_t timeout);
};


nsox_namespace_end

#include "selector_sel.hpp"

#endif //nsox_basic_selector_h

