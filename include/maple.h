#pragma once

// TODO 门面：使用引擎核心的demo实例有且仅有这一个头文件依赖，而非具体到某一个引擎头文件

#include <maple/application.h>
#include <maple/basic_types.h>
#include <maple/command.h>
#include <maple/comp_types.h>
#include <maple/default_plugin.h>
#include <maple/event.h>
#include <maple/query.h>
#include <maple/resource.h>
#include <maple/state.h>
#include <maple/timer.h>
#include <maple/tools.h>

#include <maple/builtin/components.h>
#include <maple/builtin/resources.h>
#include <maple/traits/copy.h>
#include <maple/traits/default.h>
#include <maple/traits/serde.h>
