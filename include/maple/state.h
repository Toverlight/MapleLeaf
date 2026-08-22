#pragma once

#define DECLARE_STATE_SET_BEGIN(set_name) typedef enum set_name {
#define DECLARE_STATE_SET_END(set_name) } set_name; \
void state_set_default_##set_name##_fn(set_name state); \
void reg_state_on_enter_##set_name##_fn(void(*on_enter)(set_name)); \
void reg_state_on_exit_##set_name##_fn(void(*on_exit)(set_name)); \
void state_change_##set_name##_fn(set_name state); \
void state_get_current_##set_name##_fn(void);

#define IMPL_STATE_SET(set_name) \
static set_name current_state_##set_name; \
static void(*on_enter_##set_name)(set_name) = nullptr; \
static void(*on_exit_##set_name)(set_name) = nullptr; \
void state_set_default_##set_name##_fn(set_name state) { \
	current_state_##set_name = state; \
} \
void reg_state_on_enter_##set_name##_fn(void(*on_enter)(set_name)) { \
	if (!on_enter_##set_name) on_enter_##set_name = on_enter; \
} \
void reg_state_on_exit_##set_name##_fn(void(*on_exit)(set_name)) { \
	if (!on_exit_##set_name) on_exit_##set_name = on_exit; \
} \
void state_change_##set_name##_fn(set_name state) { \
	if (current_state_##set_name == state) return; \
	if (on_exit_##set_name) on_exit_##set_name(current_state_##set_name); \
	if (on_enter_##set_name) on_enter_##set_name(state); \
	current_state_##set_name = state; \
} \
void state_get_current_##set_name##_fn(void) { \
	return current_state_##set_name; \
}

#define state_set_default(set_name, state) set_state_default_##set_name##_fn(state)
#define reg_state_on_enter(set_name, on_enter) reg_state_on_enter_##set_name##_fn(on_enter)
#define reg_state_on_exit(set_name, on_exit) reg_state_on_exit_##set_name##_fn(on_exit)
#define state_change(set_name, state) change_state_##set_name##_fn(state)
#define state_get_current(set_name) get_current_state_##set_name##_fn()
