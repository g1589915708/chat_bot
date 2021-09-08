#pragma once

#ifndef _QQ_EVENT_H_
#define _QQ_EVENT_H_

#include "qq_information.h"

typedef struct QQ_CURRENCY_EVENT qq_currency_event;
struct QQ_CURRENCY_EVENT {
	unsigned int qq_id;
	char text[128];
	size_t event_id;
	size_t from_id;
	size_t group_id;
	char group_name[32];
	char nick[32];
	char message[64];
};

typedef struct QQ_FRIENT_EVENT qq_frient_event;
struct QQ_FRIENT_EVENT {
	unsigned int qq_id;
	qq_friend qfriend;			
	int inputting;				//是否输入
	char from[32];				//原网名
	char to[32];				//新网名
	size_t author_id;			//原消息发送者的QQ号
	size_t message_id;			//原消息messageId
	size_t time;				//原消息发送时间
	unsigned int qoperator;		//好友QQ号或BotQQ号
};
typedef struct QQ_GROUP_EVENT qq_group_event;
struct QQ_GROUP_EVENT {
	qq_group qg;
	qq_group_member qmember;
	qq_group_member operat;
	char origin[1024];
	char current[1024];
	int iorigin;
	int icurrent;
	int duration_seconds;
	size_t author_id;
	size_t message_id;
	size_t time;
	int isBot;
	char action[16];
	char honor[16];
};
typedef union QQ_EVENT_TYPE qq_event_type;
union QQ_EVENT_TYPE {
	qq_currency_event cevent;
	qq_frient_event fevent;
	qq_group_event gevent;
};

typedef struct QQ_EVENT qq_event;
struct QQ_EVENT {
	int type;
	char str_type[48];
	qq_event_type event;
};


#endif
