#pragma once
#include "basic_types.h"
#include "stb_ds.h"

typedef u32 MsgId;
#define MSG_ID_INVALID 0

typedef struct Message {
	Entity source;
	MsgId msg_id;
	void* data;
} Message;

typedef struct MsgNameEntry {
    const char* key;
    MsgId value;
} MsgNameEntry, *MsgNameReg;

void maple_unreg_msg_all(void);

#define DEFINE_MESSAGE(name) struct name{};

MsgId msg_id_fn(const char* name);
MsgId reg_msg_fn(const char* name);
#define msg_id(msg_name) ((void)sizeof(msg_name), msg_id_fn(#msg_name))
#define reg_msg(msg_name) ((void)sizeof(msg_name), reg_msg_fn(#msg_name))

#define message_send(source, message_name, data) \
do { \
	(void)sizeof(struct message_name); \
	arrput(app_get()->next_messages, (Message) { (source), MSG_ID(#message_name), (data)}); \
} while(0)

struct Application;
Message message_receive_fn(struct Application* app, const char* message_name);
#define message_receive(app, message_name) ((void)sizeof(struct message_name), message_receive_fn(app_get(), #message_name))


typedef u32 SigId;
// TODO （示例信号，未实装）
enum : SigId {
	SIG_INVALID = 0,

	Sig_Button_Clicked,
	Sig_Button_Hovered,
	Sig_Button_Entered,
	Sig_Button_Exited,

	// ...

	Sig_Custom
};

typedef struct Signal {
	SigId sig_id;
	void* data;
} Signal;

typedef void(*G_ObserverFn)(struct Application* app, Signal signal);
typedef void(*E_ObserverFn)(Entity source, Signal signal);

typedef struct G_ObserverEntry {
	SigId key;
	G_ObserverFn* value;
} G_ObserverEntry, *G_ObserverReg;

void signal_add_global_observer_fn(struct Application* app, SigId sig_id, G_ObserverFn g_fn);
#define signal_add_global_observer(sig_id, g_fn) signal_add_global_observer_fn(app_get(), (sig_id), (g_fn))

typedef struct E_ObserverEntry {
	Entity key;
	E_ObserverFn* value;
} E_ObserverEntry;
typedef struct E_SigIdEntry {
	SigId key;
	E_ObserverEntry* value;
} E_SigIdEntry, *E_ObserverReg;

// TODO 实体销毁时解除对应绑定

void maple_unreg_e_observer_all(void);

void signal_add_entity_observer_fn(Entity entity, SigId sig_id, E_ObserverFn e_fn);
#define signal_add_entity_observer(entity, sig_id, e_observer_fn) signal_add_entity_observer_fn((entity), (sig_id), (e_observer_fn))

void signal_global_trigger_fn(struct Application* app, Signal signal);
#define signal_global_trigger(signal) signal_global_trigger_fn(app_get(), (signal))

void signal_entity_trigger_fn(Entity entity, Signal signal);
#define signal_entity_trigger(entity, signal) signal_entity_trigger_fn((entity), (signal))
