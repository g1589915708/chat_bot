#pragma once

#ifndef _QQ_MESSAGE_H_
#define _QQ_MESSAGE_H_

enum QQ_INFO_TYPE
{
	QSOURCE,
	QQUOTE,
	QAT,
	QATALL,
	QFACE,
	QPLAIN,
	QIMAGE,
	QFLASHIMAGE,
	QVOICE,
	QXML,
	QJSON,
	QAPP,
	QPOKE,//Poke,ShowLove,Like,Heartbroken,SixSixSix,FangDaZhao
	QDICE,
	QMUSICSHARE,
	QFORWARDMESSAGE,
	QFILE
};
typedef struct QQ_INFO_SOURCE qq_info_source;
struct QQ_INFO_SOURCE {
	int id;
	int time;
};
/* origin:on */
typedef struct QQ_INFO_QUOTE qq_info_quote;
struct QQ_INFO_QUOTE {
	int id;
	int group_id;
	int sender_id;
	int target_id;
	int count;
};
typedef struct QQ_INFO_AT qq_info_at;
struct QQ_INFO_AT {
	int target;
	char display[64];
};
typedef struct QQ_INFO_FACE qq_info_face;
struct QQ_INFO_FACE {
	int id;
	char name[8];
};
/* base64:on */
typedef struct QQ_INFO_IMAGE qq_info_image;
struct QQ_INFO_IMAGE {
	char image_id[128];
	char voice_id[128];
	char url[128];
	char path[128];
	char* base64;
	int length;
};
typedef struct QQ_INFO_MUSICSHARE qq_info_musicshare;
struct QQ_INFO_MUSICSHARE {
	char kind[128];
	char title[128];
	char summary[128];
	char jump_url[128];
	char picture_url[128];
	char music_url[128];
	char brief[128];
};
/* message:on */
typedef struct QQ_INFO_FORWARD qq_info_forward;
struct QQ_INFO_FORWARD {
	int sender_id;
	int time;
	char sender_name[64];
	int count;
	int source_id;		/* ������QQ�� */
};
typedef struct QQ_INFO_FILE qq_info_file;
struct QQ_INFO_FILE {
	char id[256];
	char name[256];
	int size;
	char* context;
};
typedef struct QQ_INFO_CURRENCY qq_info_currency;
struct QQ_INFO_CURRENCY {
	int id;
	int type;
	char * text;
	int length;
	int size;
	int time;
};
typedef union QQ_MESSAGE_TYPE qq_message_type;
union QQ_MESSAGE_TYPE {
	qq_info_source source;
	qq_info_face face;
	qq_info_image image;
	qq_info_quote quote;
	qq_info_currency curr;		//ͨ��,text��Ҫ����
	qq_info_at at;
	qq_info_musicshare music;
	qq_info_forward forward;
	qq_info_file file;
};
typedef struct QQ_MESSAGE qq_message;
struct QQ_MESSAGE {
	struct QQ_MESSAGE* prev;/* ��һ�� */
	struct QQ_MESSAGE* next;/* ��һ�� */
	struct QQ_MESSAGE* child;/* �Ӽ� */
	int type;
	char stype[32];
	qq_message_type message;
	qq_friend fend;
	qq_group_member member;
	int index;
	int size;
};


#endif // !_QQ_MESSAGE_H_