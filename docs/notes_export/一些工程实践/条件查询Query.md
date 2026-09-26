# 条件查询Query

前面的Has和Get操作只解决了给定实体的组件问题。而ECS中，系统常常需要对满足某个约束的多个组件进行操作。这涉及到多个可能的实体，而且这些实体id对系统来说是可选的。

常见的约束，可以归纳成四种：**必选、可选、筛选、排除**。这些约束之间不是互斥的，它们常常也组合存在，用以表达各种复杂的关系。

## 必选Select

**必选**：表达式中的组件，必须存在，测试成功时作为目标组件。

最简表达：AB...

必选是最常用，也是最能”代表“查询Query的操作。

## 可选Option

**可选**：表达式（包装）中的组件，如果存在于符合其他条件的实体上，则目标为这个组件；否则，目标为空值。（该关系不影响实体是否满足条件。实质不是约束而是一种包装）

最简表达：Option<A,B,...>或[AB...]

## 筛选With

**筛选**：表达式中的组件，在条件测试上起到正选的作用，但是测试成功时不作为目标组件。

最简表达：With<A,B,...>或(AB...)

## 排除Without

**排除**：表达式中的组件，在条件测试上起到反选的作用。

最简表达：Without<A,B,...>或~AB...~

## 实现

### 定义与初始化

首先是如何定义Query的形式。Query查询之后应该返回一个能够遍历、不断产生下一个可用结果的”容器“。这里采用迭代器的方式：

```c
// .h
typedef struct {
	uint_32_t id; // 组件id
	int offset; // 到对应条件容器字段的偏移量
} WaitCond;

typedef struct {
	uint_32_t id;
	int offset;
} IdOffsetEntry, *IdOffsetMap;

typedef struct {
	// 待加入的条件
	const WaitCond* conds;
	// 条件
	uint32_t* select;
	uint32_t* option;
	uint32_t* with;
	uint32_t* without;
	// 去重集
	uint32_t* all;
	// 结果集
	IdOffsetMap offsets; // 每个组件id对应结果集中相对查询目标的偏移量
	void** results; // 指向具体组件的指针数组
	size_t i; // 结果集中下一个要返回的查询目标的开头的索引
	size_t stride; // 步长
} QueryIter;

QueryIter create_query_iter(const WaitCond[] conds);

// .c
QueryIter create_query_iter(const WaitCond[] conds) {
	return (QueryIter) {
		conds, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0
	};
}
```

其中步长`stride`​是每次迭代会前进的长度，等于select和option的长度和。查询方法会根据`stride`的值是否非零来判断该Query是否可以进行查询。

去重集`all`​用集合的数据结构实现，用来**去重**，确保四种条件约束中**同一组件仅出现零或一次**。

~~集合数据结构的实现细节这里不深究，假设集合提供了~~​~~​`setput()`​~~ ​~~这样的接口，参数是集合指针和元素，若已存在则直接返回false，否则插入并返回true。还有~~​~~​`setclr()`​~~ ​~~用于清空容器。~~

> “**集合**”可以用**==只有key的哈希表==**等价。同样是stb_ds.h。

待加入的条件`WaitCond`在Query开始第一次获取时“消费”掉。

`i`​指向的“**查询目标**”是指一个实体的select和option组件集。

引擎提供这样的接口：

```c
#define __TO_ALL(comp_name) setput(__q_iter.all, COMP_ID(#comp_name))
#define __Q_ITER_SIZE(field) sizeof(((QueryIter*)0)->field)
#define __Q_SELECT_OFFSET __Q_ITER_SIZE(conds)
#define __Q_OPTION_OFFSET __Q_SELECT_OFFSET + __Q_ITER_SIZE(select)
#define __Q_WITH_OFFSET __Q_OPTION_OFFSET + __Q_ITER_SIZE(option)
#define __Q_WITHOUT_OFFSET __Q_WITH_OFFSET + __Q_ITER_SIZE(with)

#define Q_SELECT(comp_name) \
	(WaitCond) {COMP_ID(#comp_name), __Q_SELECT_OFFSET}

#define Q_OPTION(comp_name) \
	(WaitCond) {COMP_ID(#comp_name), __Q_OPTION_OFFSET}

#define Q_WITH(comp_name) \
	(WaitCond) {COMP_ID(#comp_name), __Q_WITH_OFFSET}

#define Q_WITHOUT(comp_name) \
	(WaitCond) {COMP_ID(#comp_name), __Q_WITHOUT_OFFSET}

#define QUERY(...) \
	create_query_iter((const WaitCond[]) {__VA_ARGS__, (WaitCond){UINT_MAX, 0}})
```

> 上面使用末尾加上`offset`字段为0的元素作为数组结尾标志的方式。这样就不用传长度值进去。
>
> 另外，考虑使用[offsetof](siyuan://blocks/20260726013037-3916btu)宏简化这里一系列的OFFSET定义。

根据新建查询的`conds`，更新条件数组：

```c
// .h
void init_query(QueryIter* q_iter);

#define QUERY_INIT(q_iter) init_query(q_iter)

// .c
void init_query(QueryIter* q_iter) {
	if (!q_iter->stride) return; // 防止误重复初始化
	for (int i = 0; q_iter->conds[i].offset != 0; i++) {
		if (q_iter->conds[i].id == UINT_MAX || !setput(q_iter->all, q_iter->conds[i].id)) continue;
		arrput((uint32_t*)(q_iter + q_iter->conds[i].offset), id);
		if (q_iter->conds[i].offset == __Q_SELECT_OFFSET ||
			q_iter->conds[i].offset == __Q_OPTION_OFFSET) {
				q_iter->stride++;
		}
	}
}
```

初始化Query的遍历过程中，**id为UINT_MAX的项代表未注册的组件或者错误的标识符**。可以写个日志宏在这里弹出个一次性警告或错误。因为这会影响到后续结果获取（**只有合法值注册到**当前query iter的offsets表中、影响stride，用户在初始化Query时造成的非法id对应的组件，在后续可能也通过next操作请求获取，这些是获取不到的（返回结果为NULL））。

**使用**：

```c
QueryIter q_iter = QUERY(
	Q_SELECT(Position),
	Q_SELECT(Velocity),
	Q_OPTION(HealthPoint),
	Q_WITH(InGameObject),
	Q_WITHOUT(Player)
);
QUERY_INIT(&q_iter);
```

### 结果获取

首先是最常用的**迭代接口**：一次性返回一个查询目标，每次返回后索引`i`​步进`stride`​的**Q_NEXT**；由Iter查表得到所求组件相对查询目标的偏移量，进而拿到所求组件指针的**Q_FETCH**。

```c
// .h
void* query_next(QueryIter* q_iter);
int __id_offset(QueryIter* q_iter, const char* comp_name); // 返回-1代表非法
void* query_fetch(QueryTarget target, int offset);

typedef void* QueryTarget;

#define Q_NEXT(q_iter) query_next(q_iter)
#define Q_FETCH(q_iter, target, comp_name) \
({ \
	static int offset = -2; \
	if (offset == -2) offset = __id_offset(q_iter, #comp_name); \
	query_fetch(target, offset); \
})

// .c
void* query_next(QueryIter* q_iter) {
	if (!q_iter->stride || q_iter->i >= arrlen(q_iter->results)) return NULL;
	void* res = q_iter->results[q_iter->i];
	q_iter->i += q_iter->stride;
	return res;
}
int __id_offset(QueryIter* q_iter, const char* comp_name) {
	uint32_t id = COMP_ID(comp_name);
	ptrdiff_t i = hmgeti(q_iter->offsets, id);
	if (i >= 0) {
		return q_iter->offsets[i].offset;
	}
	return -1;
}
void* query_fetch(QueryTarget target, int offset) {
	if (!target || offset < 0) return NULL;
	return target + offset;
}
```

做此规定：**Query的Select条件至少要有一个**，否则视作无效或不予初始化。

关于规定，设想这种情况：不Select组件，却需要根据With筛选一个实体，对这个实体进行操作（删除实体、添加/删除组件等）。这种情况是有意义的。

但是前面定义的组件里都已经注入了entity字段，所以显然可以通过该字段快速反向定位实体。

不突破这种限制的前提下，最好的做法就是，将With中的任意一个组件提至Select中，即可。

> 之前的init函数可以加入相应语句进行判断。这里不展开。

> 事实上，有了Query操作后，之前[稀疏集](siyuan://blocks/20260804222226-c50dkrt)里定义的`ecs_comp()`函数不再需要。统一使用Query进行存取。

**使用**：

```c
// 某个系统中 ... 假设前面已有QueryIter q_iter
QueryTarget target;
while (target = Q_NEXT(&q_iter)) { // 产出下一个查询目标。直到全部产完
	// Select组件的抓取
	Position* position = (Position*)Q_FETCH(&q_iter, target, Position);
	Velocity* velocity = (Velocity*)Q_FETCH(&q_iter, target, Velocity);
	// Option组件的获取
	HealthPoint* hp = (HealthPoint*)Q_FETCH(&q_iter, target, HealthPoint);
	if (hp != NULL) { // Option注意判空
		// ... 处理
	}
}
```

使用到查询的系统最后记得释放Query申请的堆空间（因为用到了动态数组）。

```c
//.h
void query_free(QueryIter* q_iter) {
	if (q_iter) {
		arrfree(q_iter->select);
		arrfree(q_iter->option);
		arrfree(q_iter->with);
		arrfree(q_iter->without);
		setfree(q_iter->all);
		hmfree(q_iter->offsets);
		arrfree(q_iter->results);
		q_iter = NULL;
	}
}

#define QUERY_FREE(q_iter) query_free(q_iter);
```

### 短循环优化的查询过程

由于组件已经实现了[双链](siyuan://blocks/20260712205042-apd6pc4)，我们可以只简单遍历一遍其中一个**正选**的组件数组，花费*O(n)* 时间，对于每个符合该正选条件的组件只需*O(1)* 时间就能快速找到同实体的其他组件进行验证。得到结果集。所以总时间开销为*O(n)* 。

> 这里的“正选”是指前面条件的Select和With。

唯一的性能瓶颈就是遍历的组件数组的长度n。只要在正选条件中选择一个最短的组件数组作为遍历对象即可最优化这个过程。

```c
CompType* __find_shortest_comp_arr(QueryIter* q_iter, size_t* i_out, bool* is_select_out) {
	CompType* type = NULL;
	bool is_select = true;
	for (int i = 0; i < arrlen(q_iter->select); i++) {
		if (!type) type = hmgetp_null(__comp_type_reg, q_iter->select[i]);
		if (type) {
			CompType* cur = hmgetp_null(__comp_type_reg, q_iter->select[i]);
			if (cur != NULL && *(type->dense_len) > *(cur->dense_len)) {
				type = cur;
				*i_out = i;
			}
		} // else error log
	}
	for (int i = 0; i < arrlen(q_iter->with); i++) {
		if (!type) type = hmgetp_null(__comp_type_reg, q_iter->with[i]);
		if (type) {
			CompType* cur = hmgetp_null(__comp_type_reg, q_iter->with[i]);
			if (cur != NULL && *(type->dense_len) > *(cur->dense_len)) { 
				type = cur;
				is_select = false;
				*i_out = i;
			}
		} // else error log
	}
	*is_select_out = is_select;
	return type;
}
```

上面使用_null结尾的函数表示如果注册表中查不到就返回NULL。**理论上不应该有查不到的情况（理论不可达）** 。这里以防万一做了个非空判断。建议在注册表找不到组件信息的分支加上一次性错误日志方便排查。

找到要遍历的组件数组，就可以开始遍历。这里用一次执行就得到完整结果集的方式：

```c
bool query_execute(QueryIter* q_iter) {
	bool is_outer_select;
	size_t outer_i;
	CompType* type = __find_shortest_comp_arr(q_iter, &is_outer_select, &outer_i);
	if (!type) return false; // 可能因为QueryIter没被初始化
	hmfree(q_iter->offsets); // 清除可能的脏数据
	arrsetlen(q_iter->results, 0); // 清除可能的脏数据
	void** results = NULL;
	uint32_t* ids = NULL;
	arrsetcap(results, 1024);
	arrsetcap(ids, 10);
	for (int i = 0; i < type->dense_len; i++) {
		Entity e = type->dense_set[i].owner;
		
		size_t counter = 0;
		if (is_outer_select) { 
			arrput(results, type->dense_set[type->sparse_set[e] * type->comp_size]);
			counter++;
		}
		// Select
		for (int j = 0; j < arrlen(q_iter->select); j++) {
			if (is_outer_select && outer_i == j) continue;
			CompType* cur = hmgetp_null(__comp_type_reg, q_iter->select[i]);
			if (ecs_has(cur->sparse_set, e)) {
				arrput(ids, q_iter->select[i]);
				arrput(results, cur->dense_set[cur->sparse_set[e] * cur->comp_size]);
				counter++;
			}
		}
		// Option
		for (int j = 0; j < arrlen(q_iter->option); j++) {
			CompType* cur = hmgetp_null(__comp_type_reg, q_iter->option[i]);
			arrput(ids, q_iter->option[i]);
			if (ecs_has(cur->sparse_set, e)) {
				arrput(results, cur->dense_set[cur->sparse_set[e] * cur->comp_size]);
			} else {
				arrput(results, NULL);
			}
			counter++;
		}
		// With
		bool filter_pass = true;
		for (int j = 0; j < arrlen(q_iter->with); j++) {
			if (!is_outer_select && outer_i == j) continue;
			CompType* cur = hmgetp_null(__comp_type_reg, q_iter->with[i]);
			if (!ecs_has(cur->sparse_set, e)) {
				filter_pass = false;
				break;
			}
		}
		// Without
		if (filter_pass) {
			for (int j = 0; j < arrlen(q_iter->without); j++) {
				CompType* cur = hmgetp_null(__comp_type_reg, q_iter->without[i]);
				if (!ecs_has(cur->sparse_set, e)) {
					filter_pass = false;
					break;
				}
			}
		}
		
		if (!filter_pass || counter != q_iter->stride) { // 当前实体对应组件集不满足所有约束，回溯。
			arrsetlen(results, arrlen(results) - counter);
			arrsetlen(ids, arrlen(ids) - counter);
		}
	}

	if (arrlen(q_iter->ids) >= q_iter->stride) {
		for (int i = 0; i < q_iter->stride; i++) {
			hmput(q_iter->offsets, ids[i], i); // (id, offset)对
		}
		q_iter->results = results;
	} else {
		arrfree(results);
	}
	arrfree(ids);
	return true;
}
```

> TODO：静态分配优化。

提供统一风格的宏接口：

```c
#define Q_EXEC(q_iter) query_execute(q_iter)
```

### 指定实体的查询

考虑这样一种情况，用户只是想要通过一种约束来拿到给定实体的对应组件集。这种情况是不需要执行“全量”查询拿到完整的结果集的（*O(n)* ）。而只需要对这个给定的实体进行“验证”（*O(1)* ）：

```c
QueryTarget* query_get(QueryIter* q_iter, Entity e) {
	hmfree(q_iter->offsets); // 清除可能的脏数据
	void** results = NULL;
	uint32_t* ids = NULL;
	arrsetcap(results, 10);
	arrsetcap(ids, 10);
	// Select
	for (int j = 0; j < arrlen(q_iter->select); j++) {
		CompType* cur = hmgetp_null(__comp_type_reg, q_iter->select[i]);
		if (ecs_has(cur->sparse_set, e)) {
			arrput(ids, q_iter->select[i]);
			arrput(results, cur->dense_set[cur->sparse_set[e] * cur->comp_size]);
		} else {
			goto End;
		}
	}
	// Option
	for (int j = 0; j < arrlen(q_iter->option); j++) {
		CompType* cur = hmgetp_null(__comp_type_reg, q_iter->option[i]);
		arrput(ids, q_iter->option[i]);
		if (ecs_has(cur->sparse_set, e)) {
			arrput(results, cur->dense_set[cur->sparse_set[e] * cur->comp_size]);
		} else {
			arrput(results, NULL);
		}
	}
	// With
	for (int j = 0; j < arrlen(q_iter->with); j++) {
		CompType* cur = hmgetp_null(__comp_type_reg, q_iter->with[i]);
		if (!ecs_has(cur->sparse_set, e)) {
			goto End;
		}
	}
	// Without
	for (int j = 0; j < arrlen(q_iter->without); j++) {
		CompType* cur = hmgetp_null(__comp_type_reg, q_iter->without[i]);
		if (!ecs_has(cur->sparse_set, e)) {
			goto End;
		}
	}

	bool failed = false;
	if (arrlen(q_iter->ids) > 0) {
		for (int i = 0; i < q_iter->stride; i++) {
			hmput(q_iter->offsets, ids[i], i); // (id, offset)对
		}
	} else {
	End:
		failed = true;
	}
	arrfree(results);
	arrfree(ids);
	if (failed) return NULL;
	return results;
}
```

提供统一风格的接口：

```c
#define Q_GET_BEGIN(q_iter, e, target) \
do { QueryTarget* target##_ptr = query_get(q_iter, e); \
	QueryTarget target = target##_ptr ? *target##_ptr : NULL;
```

**注意**，通过Get方式获取的Target是独立存在的，它涉及到内存分配（所以这里宏名用了 **_BEGIN**后缀标明，方便与带释放功能的 **_END**函数配对），用户需要在Fetch完内容后对它进行释放。

配对的释放宏，用do while包裹可以有效避免用户忘记调用：

```c
#define Q_GET_END(target) arrfree(target##_ptr); \
} while(0);
```

> Next方式得到的Target指向Iter内部的结果集，统一在**QUERY_FREE**里释放，这部分就不需要用户手动释放。

**使用**：

```c
... // 假定已有实体e
Q_GET_BEGIN(&q_iter, e, target)
if (target) {
	// Select组件的抓取
	Position* position = (Position*)Q_FETCH(&q_iter, target, Position);
	Velocity* velocity = (Velocity*)Q_FETCH(&q_iter, target, Velocity);
	// Option组件的抓取
	HealthPoint* hp = (HealthPoint*)Q_FETCH(&q_iter, target, HealthPoint);
	if (hp != NULL) { // Option注意判空
		// ... 处理
	}
	...
}
Q_GET_END(target)
```

Get直接获取Target，这里和Next操作一样需要对Target判空，以正确应对查找失败的情况。

Get操作还有一点与Exec操作不同，Get只会更新Iter的offsets集，results则不被更新。

### 重置迭代器变量

有可能一个系统函数需要对结果集进行不止一次的遍历，每当需要下一次遍历前，都需要对结果集索引`i`（即迭代器变量）重置。本质就是设为0（回到开头）即可。

```c
#define Q_MOVE_TO_START(q_iter) \
do { \
	q_iter->i = 0; \
} while(0)
```
