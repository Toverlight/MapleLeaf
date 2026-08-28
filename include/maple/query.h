#pragma once
#include <maple/basic_types.h>
#include <maple/comp_types.h>
#include <stddef.h>
typedef struct WaitCond {
	CompId key; // 组件id
	i32 value; // 到对应条件容器字段的偏移量
} WaitCond;

typedef struct IdOffsetEntry {
	CompId key;
	i32 value;
} IdOffsetEntry, *IdOffsetMap;

typedef struct CompIdEntry {
    CompId key;
} CompIdEntry, *CompIdSet;

typedef void* QueryTarget;

typedef struct QueryIter {
	// 待加入的条件
	const WaitCond* conds;
	// 条件
	CompId* select;
	CompId* option;
	CompId* with;
	CompId* without;
	// 去重集
	CompIdSet all;
	// 结果集
	IdOffsetMap offsets; // 每个组件id对应结果集中相对查询目标的偏移量
	QueryTarget* results; // 指向具体组件的指针数组
	usize i; // 结果集中下一个要返回的查询目标的开头的索引
	usize stride; // 步长
} QueryIter;

QueryIter query_create(const WaitCond* conds);

#define Q_SELECT(comp_name) \
	(WaitCond) {comp_id(comp_name), offsetof(QueryIter, select)}

#define Q_OPTION(comp_name) \
	(WaitCond) {comp_id(comp_name), offsetof(QueryIter, option)}

#define Q_WITH(comp_name) \
	(WaitCond) {comp_id(comp_name), offsetof(QueryIter, with)}

#define Q_WITHOUT(comp_name) \
	(WaitCond) {comp_id(comp_name), offsetof(QueryIter, without)}

#define QUERY(...) \
	query_create((const WaitCond[]) {__VA_ARGS__, (WaitCond){COMP_ID_INVALID, 0}})

void query_init(QueryIter* q_iter);
#define QUERY_INIT(q_iter) query_init(q_iter)

bool query_next(QueryIter* q_iter, QueryTarget* target);
i32 maple_query_id_offset(QueryIter* q_iter, const char* comp_name); // 返回-1代表非法
void* query_fetch(QueryTarget target, i32 offset);

#define Q_NEXT(q_iter, target) query_next(q_iter, target)
#define Q_FETCH(q_iter, target, comp_name) \
({ \
    (void)sizeof(comp_name); \
	static i32 _offset = -2; \
	if (_offset == -2) _offset = maple_query_id_offset(q_iter, #comp_name); \
	query_fetch(target, _offset); \
})

void query_free(QueryIter* q_iter);
#define QUERY_FREE(q_iter) query_free(q_iter)

bool query_execute(QueryIter* q_iter);
#define Q_EXEC(q_iter) query_execute(q_iter)

QueryTarget* query_get(QueryIter* q_iter, Entity e);

#define Q_GET_BEGIN(q_iter, e, target) \
do { QueryTarget* target##_ptr = query_get(q_iter, e); \
	QueryTarget target = target##_ptr ? *target##_ptr : NULL;

#define Q_GET_END(target) arrfree(target##_ptr); \
} while(0);

#define Q_MOVE_TO_START(q_iter) \
do { \
	q_iter->i = 0; \
} while(0)
