#pragma once

#ifndef _QQ_INFORMATION_H_
#define _QQ_INFORMATION_H_


typedef struct QQ_INFORMATION qq_information;
struct QQ_INFORMATION {
	char nickname[32];
	char email[48];
	int age;
	int level;
	char sign[16];
	char sex[8];
};

typedef struct QQ_FRIEND qq_friend;
struct QQ_FRIEND {
	unsigned int qq_id;
	qq_information infor;
	char remark[32];
};

typedef struct QQ_GROUP_MEMBER qq_group_member;
struct QQ_GROUP_MEMBER {
	qq_friend member;
	int group_id;
	char name[32];
	char member_name[32];
	char permission[16];
	char special_title[32];
	size_t join_timestamp;
	size_t last_speak_timestamp;
	size_t mute_time_remaining;
};

typedef struct QQ_GROUP qq_group;
struct QQ_GROUP {
	unsigned int group_id;
	char name[32];
	char permission[16];
	int iper_type;
	qq_group_member* member;
	int conunt;
};


#endif // !_WEB_SPIDER_H_