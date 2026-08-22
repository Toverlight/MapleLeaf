#include <maple/query.h>
#include <maple/comp_types.h>
#include <stb_ds.h>
#include <maple/tools.h>

QueryIter query_create(const WaitCond* conds) {
	return (QueryIter) {
		conds, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0, 0
	};
}

void query_init(QueryIter* q_iter) {
	if (!q_iter->stride) return; // 防止误重复初始化
	for (i32 i = 0; q_iter->conds[i].key != COMP_ID_INVALID; i++) {
		// 查all，没有则插；有则跳过
		isize j = hmgeti(q_iter->all, q_iter->conds[i].key);
		if (j >= 0) continue;
		hmputs(q_iter->all, (CompIdEntry){ .key = q_iter->conds[i].key });
		CompId* target_container = (void*)q_iter + q_iter->conds[i].value;
		arrput(target_container, q_iter->conds[i].key);
		if (q_iter->conds[i].value == offsetof(QueryIter, select) ||
			q_iter->conds[i].value == offsetof(QueryIter, option)) {
				q_iter->stride++;
		}
	}
}

QueryTarget query_next(QueryIter* q_iter) {
	if (!q_iter->stride || q_iter->i >= arrlen(q_iter->results)) return nullptr;
	QueryTarget res = q_iter->results[q_iter->i];
	q_iter->i += q_iter->stride;
	return res;
}
i32 maple_query_id_offset(QueryIter* q_iter, const char* comp_name) {
	CompId id = comp_id_fn(comp_name);
	isize i = hmgeti(q_iter->offsets, id);
	if (i >= 0) {
		return q_iter->offsets[i].value;
	}
	return -1;
}
void* query_fetch(QueryTarget target, i32 offset) {
	if (!target || offset < 0) return nullptr;
	return target + offset;
}

void query_free(QueryIter* q_iter) {
	if (q_iter) {
		arrfree(q_iter->select);
		arrfree(q_iter->option);
		arrfree(q_iter->with);
		arrfree(q_iter->without);
		hmfree(q_iter->all);
		hmfree(q_iter->offsets);
		arrfree(q_iter->results);
		q_iter = nullptr;
	}
}

static const CompType* find_shortest_comp_arr(QueryIter* q_iter, usize* i_out, bool* is_select_out) {
	CompTypeEntry* type = nullptr;
	bool is_select = true;
	CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg);
	for (isize i = 0; i < arrlen(q_iter->select); i++) {
		if (!type) type = hmgetp_null(comp_type_reg, q_iter->select[i]);
		if (type) {
			CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->select[i]);
			if (cur != nullptr && type->value.dense_len > cur->value.dense_len) {
				type = cur;
				*i_out = i;
			}
		} // TODO else error log
	}
	for (isize i = 0; i < arrlen(q_iter->with); i++) {
		if (!type) type = hmgetp_null(comp_type_reg, q_iter->with[i]);
		if (type) {
			CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->with[i]);
			if (cur != nullptr && type->value.dense_len > cur->value.dense_len) {
				type = cur;
				is_select = false;
				*i_out = i;
			}
		} // TODO else error log
	}
	*is_select_out = is_select;
	return &type->value;
}

bool query_execute(QueryIter* q_iter) {
	bool is_outer_select;
	usize outer_i;
	const CompType* type = find_shortest_comp_arr(q_iter, &outer_i, &is_outer_select);
	if (!type) return false; // 可能因为QueryIter没被初始化
	hmfree(q_iter->offsets); // 清除可能的脏数据
	arrfree(q_iter->results);
	QueryTarget* results = nullptr;
	CompId* ids = nullptr;
	arrsetcap(results, 1024);
	arrsetcap(ids, 10);
	CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg);
	for (isize i = 0; i < type->dense_len; i++) { // 短循环
		Entity e = *((Entity*)(type->dense_set + i * type->comp_size));

		size_t counter = 0;
		if (is_outer_select) {
			arrput(results, type->dense_set + type->sparse_set[e] * type->comp_size);
			counter++;
		}
		// Select
		for (i32 j = 0; j < arrlen(q_iter->select); j++) {
			if (is_outer_select && outer_i == j) continue;
			CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->select[j]);
			#ifdef DEBUG
			if (!cur) continue; // TODO 加条警告，未注册的组件id
			#endif
			if (maple_entity_has(cur->value.sparse_set, e)) {
				arrput(ids, q_iter->select[j]);
				arrput(results, cur->value.dense_set + cur->value.sparse_set[e] * cur->value.comp_size);
				counter++;
			}
		}
		// Option
		for (i32 j = 0; j < arrlen(q_iter->option); j++) {
			CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->option[j]);
			#ifdef DEBUG
			if (!cur) continue; // TODO 加条警告，未注册的组件id
			#endif
			arrput(ids, q_iter->option[j]);
			if (maple_entity_has(cur->value.sparse_set, e)) {
				arrput(results, cur->value.dense_set + cur->value.sparse_set[e] * cur->value.comp_size);
			} else {
				arrput(results, nullptr);
			}
			counter++;
		}
		// With
		bool filter_pass = true;
		for (i32 j = 0; j < arrlen(q_iter->with); j++) {
			if (!is_outer_select && outer_i == j) continue;
			CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->with[j]);
			#ifdef DEBUG
			if (!cur) continue; // TODO 加条警告，未注册的组件id
			#endif
			if (!maple_entity_has(cur->value.sparse_set, e)) {
				filter_pass = false;
				break;
			}
		}
		// Without
		if (filter_pass) {
			for (i32 j = 0; j < arrlen(q_iter->without); j++) {
				CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->without[j]);
				#ifdef DEBUG
				if (!cur) continue; // TODO 加条警告，未注册的组件id
				#endif
				if (!maple_entity_has(cur->value.sparse_set, e)) {
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

	if (arrlen(ids) >= q_iter->stride) {
		for (i32 i = 0; i < q_iter->stride; i++) {
			hmput(q_iter->offsets, ids[i], i); // (id, offset)对
		}
		q_iter->results = results; // 转移所有权
	} else {
		arrfree(results);
	}
	arrfree(ids);
	return true;
}

QueryTarget* query_get(QueryIter* q_iter, Entity e) {
	hmfree(q_iter->offsets); // 清除可能的脏数据
	void** results = nullptr;
	CompId* ids = nullptr;
	arrsetcap(results, 10);
	arrsetcap(ids, 10);
	CompTypeReg comp_type_reg = HACKER_COPIED(comp_type_reg);
	// Select
	for (i32 j = 0; j < arrlen(q_iter->select); j++) {
		CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->select[j]);
		#ifdef DEBUG
		if (!cur) continue; // TODO 加条警告，未注册的组件id
		#endif
		if (maple_entity_has(cur->value.sparse_set, e)) {
			arrput(ids, q_iter->select[j]);
			arrput(results, cur->value.dense_set + cur->value.sparse_set[e] * cur->value.comp_size);
		} else {
			goto End;
		}
	}
	// Option
	for (i32 j = 0; j < arrlen(q_iter->option); j++) {
		CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->option[j]);
		#ifdef DEBUG
		if (!cur) continue; // TODO 加条警告，未注册的组件id
		#endif
		arrput(ids, q_iter->option[j]);
		if (maple_entity_has(cur->value.sparse_set, e)) {
			arrput(results, cur->value.dense_set + cur->value.sparse_set[e] * cur->value.comp_size);
		} else {
			arrput(results, nullptr);
		}
	}
	// With
	for (i32 j = 0; j < arrlen(q_iter->with); j++) {
		CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->with[j]);
		#ifdef DEBUG
		if (!cur) continue; // TODO 加条警告，未注册的组件id
		#endif
		if (!maple_entity_has(cur->value.sparse_set, e)) {
			goto End;
		}
	}
	// Without
	for (i32 j = 0; j < arrlen(q_iter->without); j++) {
		CompTypeEntry* cur = hmgetp_null(comp_type_reg, q_iter->without[j]);
		#ifdef DEBUG
		if (!cur) continue; // TODO 加条警告，未注册的组件id
		#endif
		if (!maple_entity_has(cur->value.sparse_set, e)) {
			goto End;
		}
	}

	bool failed = false;
	if (arrlen(ids) >= q_iter->stride) {
		for (i32 i = 0; i < q_iter->stride; i++) {
			hmput(q_iter->offsets, ids[i], i); // (id, offset)对
		}
	} else {
	End:
		failed = true;
	}
	arrfree(results);
	arrfree(ids);
	if (failed) return nullptr;
	return results;
}
