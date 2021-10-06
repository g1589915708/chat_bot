// QQ-spider.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define PCRE2_CODE_UNIT_WIDTH 8
#include <stdio.h>
#include <malloc.h>
#include <cJSON/cJSON.h>
#include <libiconv/iconv.h>
#include <errno.h>
#include <pcre/pcre2.h>
#include <string.h>

#include "qq_spider.h"

#ifdef _WIN32
    #pragma comment(lib,"../lib/web-spider.lib")
    #pragma comment(lib,"../lib/iconv.lib")
    #pragma comment(lib,"../lib/prce2-8.lib")
#endif

static char* transcoding(const char* tocode, const char* fromcode, const char* inbuf);
//static char* parsing_events_informations(CURL** curl, qq_spider* qq, const char* data);
static qq_friend* parse_qq_friend(cJSON* root);
static qq_group* parse_qq_group(cJSON* root);
static qq_group_member* parse_qq_group_member(cJSON* root);
static qq_message* parse_qmessage_item(cJSON * root);
static qq_message* parse_qmessage(cJSON * root);

qq_spider* _QQ_SPIDER_ init_qq_spider(CURL** curl,const char* ip, int port,const char* aKey, size_t qnumber)
{
    qq_spider* qq = NULL;
    char verify[128] = { 0 };
    char bind[128] = {0};
    char res[128] = {0};
    char param[128] = {0};
    cJSON* root = NULL,*code,*session;
    qq = malloc(sizeof(qq_spider));
    if (qq == NULL)return NULL;
    memset(qq, 0, sizeof(qq_spider));
    strncpy(qq->ip, ip,128);
    strncpy(qq->authKey, aKey,128);
    qq->port = port;
    snprintf(qq->surl, 128, "http://%s:%d", qq->ip, qq->port);
    snprintf(verify, 128, "%s/%s", qq->surl, "verify");
    snprintf(param, 128, "{\"verifyKey\":\"%s\"}", qq->authKey);
    send_post(curl, verify, param, res, NULL,NULL);
    root = cJSON_Parse(res);
    if (root == NULL) {
        free(qq);
        qq = NULL;
        return qq;
    }
    code = cJSON_GetObjectItem(root,"code");
    session = cJSON_GetObjectItem(root,"session");
    if (NULL != code && code->valueint == 0) {
        strncpy(qq->sessionKey,session->valuestring,128);
        qq->qnumber = qnumber;
        snprintf(bind, 128, "%s/%s", qq->surl, "bind");
        snprintf(param, 128, "{\"sessionKey\":\"%s\",\"qq\":\"%d\"}", qq->sessionKey,qq->qnumber);
        res[0] = '\0';
        send_post(curl, bind, param, res, NULL, NULL);
        cJSON_Delete(root);
        root = cJSON_Parse(res);
        code = cJSON_GetObjectItem(root, "code");
        session = cJSON_GetObjectItem(root, "msg");
        if (NULL != code && code->valueint == 0) {
            if (strcmp(session->valuestring, "success") == 0) {
                cJSON_Delete(root);
                return qq;
            }
        }
        else {
            free(qq);
            qq = NULL;
        }
    }
    else {
        free(qq);
        qq = NULL;
    }
    cJSON_Delete(root);
    return qq;
}

char* _QQ_SPIDER_ send_message(CURL** curl,qq_spider * bot, size_t qq, char* message)
{
    char sendFriendMessage[128];
    char param[8192] = { 0 };
    char res[128] = { 0 };
    cJSON* root = NULL, * code, * msg, * messageId;

    snprintf(sendFriendMessage, 128, "%s/%s", bot->surl, "sendFriendMessage");
    snprintf(param, 8192, "{\"sessionKey\":\"%s\",\"qq\":\"%d\",\"messageChain\":[{ \"type\":\"Plain\", \"text\":\"%s\" }]}", bot->sessionKey, qq,message);
    send_post(curl, sendFriendMessage, param, res, NULL, NULL);

    root = cJSON_Parse(res);
    if (root == NULL)return NULL;
    code = cJSON_GetObjectItem(root, "code");
    msg = cJSON_GetObjectItem(root, "msg");
    messageId = cJSON_GetObjectItem(root, "messageId");
    if (code->valueint == 0 && strcmp(msg->valuestring,"success"))  strcpy(res, messageId->valuestring);
    cJSON_Delete(root);

    return res;
}

char* _QQ_SPIDER_ send_basic_message(CURL** curl, qq_spider* bot, size_t qq, char* message)
{
    char sendFriendMessage[128];
    char param[16384] = { 0 };
    char res[128] = { 0 };
    cJSON* root = NULL, * code, * msg, * messageId;

    snprintf(sendFriendMessage, 128, "%s/%s", bot->surl, "sendFriendMessage");
    snprintf(param, 16384, "{\"sessionKey\":\"%s\",\"qq\":\"%d\",\"messageChain\":%s}", bot->sessionKey, qq, message);
    send_post(curl, sendFriendMessage, param, res, NULL, NULL);

    root = cJSON_Parse(res);
    if (root == NULL)return NULL;
    code = cJSON_GetObjectItem(root, "code");
    msg = cJSON_GetObjectItem(root, "msg");
    messageId = cJSON_GetObjectItem(root, "messageId");
    if (code->valueint == 0 && strcmp(msg->valuestring, "success"))  strcpy(res, messageId->valuestring);
    cJSON_Delete(root);

    return res;
}

char* _QQ_SPIDER_ send_group_message(CURL** curl, qq_spider* bot, size_t qq, char* message)
{

    char sendFriendMessage[128];
    char param[8192] = { 0 };
    char res[128] = { 0 };
    //char *p;
    cJSON* root = NULL, * code, * msg, * messageId;

    snprintf(sendFriendMessage, 128, "%s/%s", bot->surl, "sendGroupMessage");
    snprintf(param, 8192, "{\"sessionKey\":\"%s\",\"target\":%d,\"messageChain\":[{ \"type\":\"Plain\", \"text\":\"%s\" }]}", bot->sessionKey, qq, message);
    send_post(curl, sendFriendMessage, param, res, NULL, NULL);
    //p = transcoding("GBK", "utf-8", res);
    root = cJSON_Parse(res);
    if (root == NULL)return NULL;
    code = cJSON_GetObjectItem(root, "code");
    msg = cJSON_GetObjectItem(root, "msg");
    messageId = cJSON_GetObjectItem(root, "messageId");
    if (code->valueint == 0 && strcmp(msg->valuestring, "success"))  strcpy(res, messageId->valuestring);
    cJSON_Delete(root);

    return res;
}

char* _QQ_SPIDER_ send_basic_group_message(CURL** curl, qq_spider* bot, size_t qq, char* message)
{

    char sendFriendMessage[128];
    char param[16384] = { 0 };
    char res[128] = { 0 };
    //char *p;
    cJSON* root = NULL, * code, * msg, * messageId;

    snprintf(sendFriendMessage, 128, "%s/%s", bot->surl, "sendGroupMessage");
    snprintf(param, 16384, "{\"sessionKey\":\"%s\",\"target\":%d,\"messageChain\":%s}", bot->sessionKey, qq, message);
    send_post(curl, sendFriendMessage, param, res, NULL, NULL);
    //p = transcoding("GBK", "utf-8", res);
    root = cJSON_Parse(res);
    if (root == NULL)return NULL;
    code = cJSON_GetObjectItem(root, "code");
    msg = cJSON_GetObjectItem(root, "msg");
    messageId = cJSON_GetObjectItem(root, "messageId");
    if (code->valueint == 0 && strcmp(msg->valuestring, "success"))  strcpy(res, messageId->valuestring);
    cJSON_Delete(root);

    return res;
}

int _QQ_SPIDER_ unread_message_number(CURL** curl, qq_spider* bot)
{
    char countMessage[128] = { 0 };
    char res[128] = { 0 };
    int ret = 0;
    cJSON* root = NULL, * code, * data;

    snprintf(countMessage, 128, "%s/%s?sessionKey=%s", bot->surl, "countMessage", bot->sessionKey);
    send_get(curl, countMessage, res, NULL, NULL);

    root = cJSON_Parse(res);
    if (root == NULL)return -1;
    code = cJSON_GetObjectItem(root, "code");
    data = cJSON_GetObjectItem(root, "data");
    if (code->valueint == 0)  ret = data->valueint;
    cJSON_Delete(root);

    return ret;
}

char* _QQ_SPIDER_ get_old_unread_information(CURL** curl, qq_spider* bot, int length)
{
    char fetchMessage[256] = { 0 };
    char * res = NULL;
    char * ret = 0;
    int size = sizeof(char) * 2048 * length;
    cJSON* root = NULL, * code, * data;

    res = malloc(size);
    if (res == NULL) return NULL;
    memset(res, 0, size);
    snprintf(fetchMessage, 128, "%s/%s?sessionKey=%s&count=%d", bot->surl, "fetchMessage", bot->sessionKey, length);
    send_get(curl, fetchMessage, res, NULL, NULL);

    root = cJSON_Parse(res);
    if (root == NULL) { free(res); return NULL; }
    code = cJSON_GetObjectItem(root, "code");
    data = cJSON_GetObjectItem(root, "data");
    if (code->valueint == 0) {
        ret = cJSON_Print(data);
        strcpy(res, ret);
        free(ret);
#ifdef _WIN32
        ret = transcoding("gb2312", "utf-8", res);
        free(res);
        res = ret;
#endif
    }
    cJSON_Delete(root);

    return res;
}

char* _QQ_SPIDER_ get_new_unread_information(CURL** curl, qq_spider* bot, int length)
{
    char fetchLatestMessage[128] = { 0 };
    char* res = NULL;
    char* ret = 0;
    int size = sizeof(char) * 2048 * length;
    cJSON* root = NULL, * code, * data;

    res = malloc(size);
    if (res == NULL) return NULL;
    memset(res, 0, size);
    snprintf(fetchLatestMessage, 128, "%s/%s?sessionKey=%s&count=%d", bot->surl, "fetchLatestMessage", bot->sessionKey, length);
    send_get(curl, fetchLatestMessage, res, NULL, NULL);

    root = cJSON_Parse(res);
    if (root == NULL) { free(res); return NULL; }
    code = cJSON_GetObjectItem(root, "code");
    data = cJSON_GetObjectItem(root, "data");
    if (code->valueint == 0) {
        ret = cJSON_Print(data);
        strcpy(res, ret);
        free(ret);
#ifdef _WIN32
        ret = transcoding("gb2312", "utf-8", res);
        free(res);
        res = ret;
#endif
    }
    cJSON_Delete(root);

    return res;
}

int _QQ_SPIDER_ get_friend_list(CURL** curl, qq_spider* bot)
{
    char friendList[128] = { 0 };
    char* res = NULL;
    char* tmp = NULL;
    int ret = 0;
    int size = sizeof(char) * 128 * 5000;
    int i = 0;
    cJSON* root = NULL, * code, * data;

    res = malloc(size);
    if (res == NULL) return NULL;
    memset(res, 0, size);
    snprintf(friendList, 128, "%s/%s?sessionKey=%s", bot->surl, "friendList", bot->sessionKey);
    send_get(curl, friendList, res, NULL, NULL);
    
#ifdef _WIN32
    tmp = transcoding("GBK", "utf-8", res);
    if (tmp != NULL) {
        free(res);
        res = tmp;
    }
#endif

    root = cJSON_Parse(res);
    if (root == NULL) { free(res); return -1; }
    code = cJSON_GetObjectItem(root, "code");
    data = cJSON_GetObjectItem(root, "data");
    if (code->valueint == 0) {
        ret = cJSON_GetArraySize(data);
        bot->count = ret;
        bot->friend = malloc(sizeof(qq_friend) * ret);
        if (bot->friend == NULL) {
            cJSON_Delete(root);
            free(res);
            return -1;
        }
        memset(bot->friend, 0, sizeof(qq_friend) * ret);
        for (i = 0; i < ret; i++)
        {
            bot->friend[i].qq_id = cJSON_GetArrayItem(data, i)->child->valuedouble;
            strcpy(bot->friend[i].infor.nickname,cJSON_GetArrayItem(data, i)->child->next->valuestring);
            strcpy(bot->friend[i].remark , cJSON_GetArrayItem(data, i)->child->next->next->valuestring);
        }
    }
    free(res);
    cJSON_Delete(root);

    return ret;
}

int _QQ_SPIDER_ get_group_list(CURL** curl, qq_spider* bot)
{
    char groupList[128] = { 0 };
    char* res = NULL;
    char* tmp = NULL;
    int ret = 0;
    int size = sizeof(char) * 128 * 5000;
    int i = 0;
    cJSON* root = NULL, * code, * data;

    res = malloc(size);
    if (res == NULL) return NULL;
    memset(res, 0, size);
    snprintf(groupList, 128, "%s/%s?sessionKey=%s", bot->surl, "groupList", bot->sessionKey);
    send_get(curl, groupList, res, NULL, NULL);

#ifdef _WIN32
    tmp = transcoding("GBK", "utf-8", res);
    if (tmp != NULL) {
        free(res);
        res = tmp;
    }
    else {
        free(res);
        return -1;
    }
#endif

    root = cJSON_Parse(res);
    if (root == NULL) { free(res); return -1; }
    code = cJSON_GetObjectItem(root, "code");
    data = cJSON_GetObjectItem(root, "data");
    if (code->valueint == 0) {
        ret = cJSON_GetArraySize(data);
        bot->gcount = ret;
        bot->group = malloc(sizeof(qq_group) * ret);
        if (bot->friend == NULL) {
            cJSON_Delete(root);
            free(res);
            return -1;
        }
        memset(bot->group, 0, sizeof(qq_group) * ret);
        for (i = 0; i < ret; i++)
        {
            bot->group[i].group_id = cJSON_GetArrayItem(data, i)->child->valuedouble;
            strcpy(bot->group[i].name, cJSON_GetArrayItem(data, i)->child->next->valuestring);
            strcpy(bot->group[i].permission, cJSON_GetArrayItem(data, i)->child->next->next->valuestring);
        }
    }
    cJSON_Delete(root);
    free(res);

    return ret;
}

int _QQ_SPIDER_ get_member_list(CURL** curl, qq_spider* bot, size_t target)
{
    char memberList[128] = { 0 };
    char* res = NULL;
    char* tmp = NULL;
    int ret = 0;
    int size = sizeof(char) * 128 * 5000;
    int i = 0;
    int j = 0;
    cJSON* root = NULL, * code, * data, * member;

    res = malloc(size);
    if (res == NULL) return NULL;
    memset(res, 0, size);
    snprintf(memberList, 128, "%s/%s?sessionKey=%s&target=%d", bot->surl, "memberList", bot->sessionKey, target);
    send_get(curl, memberList, res, NULL, NULL);

#ifdef _WIN32
    tmp = transcoding("GBK", "UTF-8", res);
    if (tmp != NULL) {
        free(res);
        res = tmp;
    }
#endif

    root = cJSON_Parse(res);
    if (root == NULL) { free(res); return -1; }
    code = cJSON_GetObjectItem(root, "code");
    data = cJSON_GetObjectItem(root, "data");
    if (code->valueint == 0) {
        ret = cJSON_GetArraySize(data);
        for (i = 0; i < bot->gcount; i++)
        {
            if (bot->group[i].group_id == target) {
                bot->group[i].conunt = ret;
                bot->group[i].member = malloc(sizeof(qq_group_member) * ret);
                if (bot->group[i].member == NULL) {
                    cJSON_Delete(root);
                    free(res);
                    return -1;
                }
                memset(bot->group[i].member, 0, sizeof(qq_group_member) * ret);
                for (j = 0; j < ret; j++)
                {
                    member = cJSON_GetArrayItem(data, j);
                    bot->group[i].member[j].member.qq_id = cJSON_GetObjectItem(member,"id")->valuedouble;
                    bot->group[i].member[j].join_timestamp = cJSON_GetObjectItem(member, "joinTimestamp")->valuedouble;
                    bot->group[i].member[j].last_speak_timestamp = cJSON_GetObjectItem(member, "lastSpeakTimestamp")->valuedouble;
                    bot->group[i].member[j].mute_time_remaining = cJSON_GetObjectItem(member, "muteTimeRemaining")->valuedouble;
                    strcpy(bot->group[i].member[j].member_name, cJSON_GetObjectItem(member, "memberName")->valuestring);
                    strcpy(bot->group[i].member[j].permission, cJSON_GetObjectItem(member, "permission")->valuestring);
                    strcpy(bot->group[i].member[j].special_title, cJSON_GetObjectItem(member, "specialTitle")->valuestring);
                }
                break;
            }
        }
    }
    cJSON_Delete(root);
    free(res);

    return ret;
}

int _QQ_SPIDER_ get_friend_profile(CURL** curl, qq_spider* bot, size_t number)
{
    char friendProfile[128] = { 0 };
    char* res = NULL;
    char* tmp = NULL;
    int ret = 0;
    int size = sizeof(char) * 128 * 5000;
    int i = 0; 
    int j = 0;
    int f = 0;
    cJSON* root, * code, * data;

    res = malloc(size);
    if (res == NULL) return -1;
    memset(res, 0, size);
    snprintf(friendProfile, 128, "%s/%s?sessionKey=%s&target=%d", bot->surl, "friendProfile", bot->sessionKey,number);
    send_get(curl, friendProfile, res, NULL, NULL);

#ifdef _WIN32
    tmp = transcoding("GBK", "UTF-8", res);
    if (tmp != NULL) {
        free(res);
        res = tmp;
    }
    else {
        free(res);
        return -1;
    }
#endif

    root = cJSON_Parse(res);
    if (root == NULL) { free(res); return -1; }
    if (bot->count != 0 && cJSON_GetObjectItem(root, "nickname")->valuestring != NULL) {
        for (i = 0; i < bot->count; i++)
        {
            if (bot->friend[i].qq_id == number) {
                f = 1;
                bot->friend[i].infor.age = cJSON_GetObjectItem(root, "age")->valueint;
                bot->friend[i].infor.level = cJSON_GetObjectItem(root, "level")->valueint;
                strcpy(bot->friend[i].infor.email, cJSON_GetObjectItem(root, "email")->valuestring);
                strcpy(bot->friend[i].infor.sex, cJSON_GetObjectItem(root, "sex")->valuestring);
                strcpy(bot->friend[i].infor.sign, cJSON_GetObjectItem(root, "sign")->valuestring);
                strcpy(bot->friend[i].infor.nickname, cJSON_GetObjectItem(root, "nickname")->valuestring);
                break;
            }
        }
    }
    cJSON_Delete(root);
    free(res);
    return 0;
}

int _QQ_SPIDER_ get_member_profile(CURL** curl, qq_spider* bot, size_t group_id, size_t number)
{
    char memberProfile[128] = { 0 };
    char* res = NULL;
    char* tmp = NULL;
    int ret = 0;
    int size = sizeof(char) * 128 * 5000;
    int i = 0;
    int j = 0;
    int f = 0;
    cJSON* root, * code, * data;

    res = malloc(size);
    if (res == NULL) return -1;
    memset(res, 0, size);
    snprintf(memberProfile, 128, "%s/%s?sessionKey=%s&target=%d&memberId=%d", bot->surl, "memberProfile", bot->sessionKey, group_id, number);
    send_get(curl, memberProfile, res, NULL, NULL);

#ifdef _WIN32
    tmp = transcoding("GBK", "UTF-8", res);
    if (tmp != NULL) {
        free(res);
        res = tmp;
    }
    else {
        free(res);
        return -1;
    }
#endif

    root = cJSON_Parse(res);
    if (root == NULL) { free(res); return -1; }
    if (bot->count != 0 && cJSON_GetObjectItem(root, "nickname")->valuestring != NULL) {
        for (i = 0; i < bot->gcount; i++)
        {
            if (group_id == bot->group[i].group_id)
            {
                for (j = 0; j < bot->group[i].conunt; j++)
                {
                    if (bot->group[i].member[j].member.qq_id == number) {
                        f = 1;
                        bot->group[i].member[j].member.infor.age = cJSON_GetObjectItem(root, "age")->valueint;
                        bot->group[i].member[j].member.infor.level = cJSON_GetObjectItem(root, "level")->valueint;
                        strcpy(bot->group[i].member[j].member.infor.email, cJSON_GetObjectItem(root, "email")->valuestring);
                        strcpy(bot->group[i].member[j].member.infor.sex, cJSON_GetObjectItem(root, "sex")->valuestring);
                        strcpy(bot->group[i].member[j].member.infor.sign, cJSON_GetObjectItem(root, "sign")->valuestring);
                        strcpy(bot->group[i].member[j].member.infor.nickname, cJSON_GetObjectItem(root, "nickname")->valuestring);
                        break;
                    }
                }
                if (f) break;
            }
        }
    }
    cJSON_Delete(root);
    free(res);
    return 0;
}

int _QQ_SPIDER_ parsing_events_information(const char* data, qq_spider** pbot)
{
    cJSON* root = NULL, * object = NULL, * sender = NULL, * messageChain = NULL, * message = NULL, * group = NULL;
    cJSON* item = NULL;
    cJSON* ctmp = NULL;
    qq_spider* bot = *pbot;
    qq_friend* qf = NULL;
    qq_group* qg = NULL;
    qq_group_member* qm = NULL;
    int size = 0;
    int i = 0;
    int j = 0;
    char m[512] = { 0 };
    char* tmp;
    int ret = 0;
    printf("%s\n",data);
    root = cJSON_Parse(data);
    if (root == NULL) return NULL;
    /* 获得事件和信息的总和 */
    size = cJSON_GetArraySize(root);
    if (size == 0) return NULL;
    bot->lately_message == NULL ? 
	bot->lately_message = calloc(sizeof(qq_message*), size) : 
	(bot->lately_message = realloc(bot->lately_message,sizeof(qq_message*) * (size + bot->mcount)));//bug1
    if (bot->lately_message == NULL) return -1;
    bot->lately_event == NULL ? bot->lately_event = calloc(sizeof(qq_event), size) : (bot->lately_event = realloc(bot->lately_event,sizeof(qq_event) * (size + bot->ecount)));
    if (bot->lately_event == NULL) { free(bot->lately_message); return -1; }
    /* 解析事件和信息 */
    for (i = 0; i < size; i++)
    {
        object = cJSON_GetArrayItem(root, i);
        tmp = cJSON_GetObjectItem(object, "type")->valuestring;
        /* event deal with */
        if (strcmp(tmp, "BotOnlineEvent") == 0 || strcmp(tmp, "BotOfflineEventActive") == 0 || strcmp(tmp, "BotOfflineEventForce") == 0 || strcmp(tmp, "BotOfflineEventDropped") == 0 || strcmp(tmp, "BotReloginEvent") == 0) {//如果Bot登录成功
            //机器人的上下线状态暂不处理
        }
        else if (strcmp(tmp, "FriendInputStatusChangedEvent") == 0 || strcmp(tmp, "FriendNickChangedEvent") == 0) {//如果是好友输入状态改变 || 好友昵称改变
            strcpy(bot->lately_event[bot->ecount].str_type, tmp);
            qf = parse_qq_friend(cJSON_GetObjectItem(object, "friend"));
            memcpy(&bot->lately_event[bot->ecount].event.fevent.qfriend, qf, sizeof(qq_friend));
            item = cJSON_GetObjectItem(object, "inputting");
            if (item != NULL) {
                bot->lately_event[bot->ecount].event.fevent.inputting = item->valueint;
            }
            else {
                strcpy(bot->lately_event[bot->ecount].event.fevent.from, cJSON_GetObjectItem(object, "from"));
                strcpy(bot->lately_event[bot->ecount].event.fevent.to, cJSON_GetObjectItem(object, "to"));
            }
            free(qf);
            qf = NULL;
            bot->ecount++;
        }
        else if (strcmp(tmp, "BotGroupPermissionChangeEvent") == 0 || strcmp(tmp, "BotLeaveEventActive") == 0 || strcmp(tmp, "GroupAllowConfessTalkEvent") == 0) {//如果是Bot在群里的权限被改变
            /* OWNER、ADMINISTRATOR或MEMBER */
            strcpy(bot->lately_event[bot->ecount].str_type, tmp);
            qg = parse_qq_group(cJSON_GetObjectItem(object, "group"));
            memcpy(&bot->lately_event[bot->ecount].event.gevent.qg, qg, sizeof(qq_group));
            item = cJSON_GetObjectItem(object, "origin");
            if (item && item->type == 4) {
                strcpy(bot->lately_event[bot->ecount].event.gevent.origin, item->valuestring);
                strcpy(bot->lately_event[bot->ecount].event.gevent.current, cJSON_GetObjectItem(object, "current")->valuestring);
            }
            else if (item && item->type <= 1) {
                bot->lately_event[bot->ecount].event.gevent.iorigin = item->valueint;
                bot->lately_event[bot->ecount].event.gevent.icurrent = cJSON_GetObjectItem(object, "current")->valueint;
                item = cJSON_GetObjectItem(object, "isBot");
                if (item) {
                    bot->lately_event[bot->ecount].event.gevent.isBot = item->valueint;
                }
            }
            else if (item && item->type <= 1) {
                bot->lately_event[bot->ecount].event.gevent.author_id = cJSON_GetObjectItem(object, "authorId")->valuedouble;
                bot->lately_event[bot->ecount].event.gevent.message_id = cJSON_GetObjectItem(object, "messageId")->valuedouble;
                bot->lately_event[bot->ecount].event.gevent.time = cJSON_GetObjectItem(object, "time")->valuedouble;
            }
            free(qg);
            qg = NULL;
            bot->ecount++;
        }
	else if (strcmp(tmp, "BotJoinGroupEvent") == 0 || strcmp(tmp, "BotLeaveEventKick") == 0 ){
		strcpy(bot->lately_event[bot->ecount].str_type, tmp);
		qg = parse_qq_group(cJSON_GetObjectItem(object, "group"));
		memcpy(&bot->lately_event[bot->ecount].event.gevent.qg, qg, sizeof(qq_group));
		if(cJSON_GetObjectItem(object, "operator") != NULL){
	printf("oh no!!\n");
			qm = parse_qq_group_member(cJSON_GetObjectItem(object, "operator"));
		}else if(cJSON_GetObjectItem(object, "invitor") != NULL){
	printf("oh yes!!\n");
			qm = parse_qq_group_member(cJSON_GetObjectItem(object, "invitor"));
		}
		if(qm){
			memcpy(&bot->lately_event[bot->ecount].event.gevent.operat, qm, sizeof(qq_group_member));
            		free(qm);
		}
		free(qg);
	} 
        else if (strcmp(tmp, "BotMuteEvent") == 0 || strcmp(tmp, "BotUnmuteEvent") == 0 || strcmp(tmp, "MemberJoinEvent") == 0 || strcmp(tmp, "MemberLeaveEventQuit") == 0 || strcmp(tmp, "MemberCardChangeEvent") == 0 || strcmp(tmp, "MemberSpecialTitleChangeEvent") == 0 || strcmp(tmp, "MemberPermissionChangeEvent") == 0 || strcmp(tmp, "MemberHonorChangeEvent") == 0) {
            strcpy(bot->lately_event[bot->ecount].str_type, tmp);
	    ctmp = cJSON_GetObjectItem(object, "operator");
	    if(ctmp == NULL) ctmp = cJSON_GetObjectItem(object, "member");
	    if(ctmp == NULL) { printf("program error!!  funtion:%s,line:%d\n",__FUNCTION__,__LINE__); }
            qm = parse_qq_group_member(ctmp);
	    if(qm != NULL){
		memcpy(&bot->lately_event[bot->ecount].event.gevent.operat, qm, sizeof(qq_group_member));
		free(qm);
		qm = NULL;
	    }
            item = cJSON_GetObjectItem(object, "durationSeconds");
            if (item != NULL) { bot->lately_event[bot->ecount].event.gevent.duration_seconds = item->valuedouble; }
            else { 
                item = cJSON_GetObjectItem(object, "origin");
                if(strcmp(tmp, "BotUnmuteEvent") == 0)bot->lately_event[bot->ecount].event.gevent.duration_seconds = -1;
                else if (item) {
                    strcpy(bot->lately_event[bot->ecount].event.gevent.origin, item->valuestring);
                    strcpy(bot->lately_event[bot->ecount].event.gevent.current, cJSON_GetObjectItem(object, "current")->valuestring);
                }
                else if(cJSON_GetObjectItem(object, "action") != NULL){
                    strcpy(bot->lately_event[bot->ecount].event.gevent.action, cJSON_GetObjectItem(object, "action")->valuestring);
                    strcpy(bot->lately_event[bot->ecount].event.gevent.honor, cJSON_GetObjectItem(object, "honor")->valuestring);
                }
            }
            bot->ecount++;
        }
        else if (strcmp(tmp, "MemberLeaveEventKick") == 0 || strcmp(tmp, "MemberMuteEvent") == 0 || strcmp(tmp, "MemberUnmuteEvent") == 0) {//如果是成员被踢出群（该成员不是Bot）
            strcpy(bot->lately_event[bot->ecount].str_type, tmp);
            qm = parse_qq_group_member(cJSON_GetObjectItem(object, "operator"));
            memcpy(&bot->lately_event[bot->ecount].event.gevent.operat, qm, sizeof(qq_group_member));
            free(qm);
            qm = NULL;
            qm = parse_qq_group_member(cJSON_GetObjectItem(object, "member"));
            memcpy(&bot->lately_event[bot->ecount].event.gevent.qmember, qm, sizeof(qq_group_member));
            item = cJSON_GetObjectItem(object, "durationSeconds");
            if (item != NULL) { bot->lately_event[bot->ecount].event.gevent.duration_seconds = item->valuedouble; }
            else { if (strcmp(tmp, "BotUnmuteEvent") == 0)bot->lately_event[bot->ecount].event.gevent.duration_seconds = -1; }
            free(qm);
            qm = NULL;
            bot->ecount++;
        }
        else if (strcmp(tmp, "GroupRecallEvent") == 0 || strcmp(tmp, "GroupNameChangeEvent") == 0 || strcmp(tmp, "GroupEntranceAnnouncementChangeEvent") == 0 || strcmp(tmp, "GroupMuteAllEvent") == 0 || strcmp(tmp, "GroupAllowAnonymousChatEvent") == 0 || strcmp(tmp, "GroupAllowMemberInviteEvent") == 0) {//如果是好友消息撤回
            strcpy(bot->lately_event[bot->ecount].str_type, tmp);
            qm = parse_qq_group_member(cJSON_GetObjectItem(object, "operator"));
            if (qm) { memcpy(&bot->lately_event[bot->ecount].event.gevent.operat, qm, sizeof(qq_group_member)); free(qm); }
            qg = parse_qq_group(cJSON_GetObjectItem(object, "group"));
            memcpy(&bot->lately_event[bot->ecount].event.gevent.qg, qg, sizeof(qq_group));
            item = cJSON_GetObjectItem(object, "origin");
            if (item && item->type == 4) {
                strcpy(bot->lately_event[bot->ecount].event.gevent.origin, item->valuestring);
                strcpy(bot->lately_event[bot->ecount].event.gevent.current, cJSON_GetObjectItem(object, "current")->valuestring);
            }
            else if (item && item->type <= 1) {
                bot->lately_event[bot->ecount].event.gevent.iorigin = item->valueint;
                bot->lately_event[bot->ecount].event.gevent.icurrent = cJSON_GetObjectItem(object, "current")->valueint;
            }
            else {
                bot->lately_event[bot->ecount].event.gevent.author_id = cJSON_GetObjectItem(object, "authorId")->valuedouble;
                bot->lately_event[bot->ecount].event.gevent.message_id = cJSON_GetObjectItem(object, "messageId")->valuedouble;
                bot->lately_event[bot->ecount].event.gevent.time = cJSON_GetObjectItem(object, "time")->valuedouble;
            }
            free(qg);
            qm = NULL;
            qg = NULL;
            bot->ecount++;
        }
        else if (strcmp(tmp, "NewFriendRequestEvent") == 0 || strcmp(tmp, "MemberJoinRequestEvent") == 0 || strcmp(tmp, "BotInvitedJoinGroupRequestEvent") == 0) {//如果是Bot加入了一个新群
            strcpy(bot->lately_event[bot->ecount].str_type, tmp);
            bot->lately_event[bot->ecount].event.cevent.event_id = cJSON_GetObjectItem(object, "eventId")->valuedouble;
            bot->lately_event[bot->ecount].event.cevent.from_id = cJSON_GetObjectItem(object, "fromId")->valuedouble;
            bot->lately_event[bot->ecount].event.cevent.group_id = cJSON_GetObjectItem(object, "groupId")->valuedouble;
            strcpy(bot->lately_event[bot->ecount].event.cevent.nick, cJSON_GetObjectItem(object, "nick")->valuestring);
            strcpy(bot->lately_event[bot->ecount].event.cevent.message, cJSON_GetObjectItem(object, "message")->valuestring);
            item = cJSON_GetObjectItem(object, "groupName");
            if (item != NULL) {
                strcpy(bot->lately_event[bot->ecount].event.cevent.group_name, item->valuestring);
            }
            bot->ecount++;
        }
        else if (strcmp(tmp, "FriendRecallEvent") == 0) {//如果是好友消息撤回
            strcpy(bot->lately_event[bot->ecount].str_type, tmp);

            bot->ecount++;
        }
        /* message deal with */
        else if (strcmp(tmp, "FriendMessage") == 0 || strcmp(tmp, "StrangerMessage") == 0) {//如果是Bot被踢出一个群
            qq_message* message = parse_qmessage(cJSON_GetObjectItem(object, "messageChain"));
            qq_friend* fend;
            bot->lately_message[bot->mcount] = message;
            strcpy(bot->lately_message[bot->mcount]->stype, tmp);
            sender = cJSON_GetObjectItem(object, "sender");
            fend = parse_qq_friend(sender);
            memcpy(&bot->lately_message[bot->mcount++]->fend, fend, sizeof(qq_friend));
            free(fend);
        }
        else if (strcmp(tmp, "GroupMessage") == 0 || strcmp(tmp, "TempMessage") == 0) {//如果是群消息撤回
            qq_message* message = parse_qmessage(cJSON_GetObjectItem(object, "messageChain"));
            qq_group_member* member;
            bot->lately_message[bot->mcount] = message;
            strcpy(bot->lately_message[bot->mcount]->stype, tmp);
            sender = cJSON_GetObjectItem(object, "sender");
            member = parse_qq_group_member(sender);
            memcpy(&bot->lately_message[bot->mcount++]->member, member, sizeof(qq_group_member));
            free(member);
        }
    }
    cJSON_Delete(root);
    return size;
}

char* _QQ_SPIDER_ message_convert_json(qq_message* msg)
{
    cJSON* root, * item = NULL, * temp = NULL, * ctmp = NULL;
    qq_message* mct = msg, * mitem = NULL, * mtemp = NULL;
    char* str = NULL;
    char * res = NULL;
    int i = 0;
    root = cJSON_CreateArray();
    while (msg != NULL) {
        if (msg->type == QSOURCE) {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Source");
            cJSON_AddNumberToObject(item, "id", msg->message.source.id);
            cJSON_AddNumberToObject(item, "time", msg->message.source.time);
        }
        else if (msg->type == QQUOTE)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Quote");
            cJSON_AddNumberToObject(item, "id", msg->message.quote.id);
            cJSON_AddNumberToObject(item, "groupId", msg->message.quote.group_id);
            cJSON_AddNumberToObject(item, "senderId", msg->message.quote.sender_id);
            cJSON_AddNumberToObject(item, "targetId", msg->message.quote.target_id);
            str = message_convert_json(msg->child);
            cJSON_AddItemToObject(item, "origin", cJSON_Parse(str));
            free(str);
        }
        else if (msg->type == QAT)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "At");
            cJSON_AddNumberToObject(item, "target", msg->message.at.target);
            cJSON_AddStringToObject(item, "type", msg->message.at.display);
        }
        else if (msg->type == QFACE)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Face");
            cJSON_AddNumberToObject(item, "faceId", msg->message.face.id);
            cJSON_AddStringToObject(item, "name", msg->message.face.name);
        }
        else if (msg->type == QPLAIN)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Plain");
            cJSON_AddStringToObject(item, "text", msg->message.curr.text);
        }
        else if (msg->type == QIMAGE || msg->type == QFLASHIMAGE || msg->type == QVOICE)
        {
            item = cJSON_CreateObject();
            switch (msg->type)
            {
            case QIMAGE:cJSON_AddStringToObject(item, "type", "Image"); cJSON_AddStringToObject(item, "imageId", msg->message.image.image_id); break;
            case QFLASHIMAGE:cJSON_AddStringToObject(item, "type", "FlashImage"); cJSON_AddStringToObject(item, "FlashImage", msg->message.image.image_id); break;
            case QVOICE:cJSON_AddStringToObject(item, "type", "Voice"); cJSON_AddStringToObject(item, "voiceId", msg->message.image.image_id); break;
            }
            
            cJSON_AddStringToObject(item, "url", msg->message.image.url);
            cJSON_AddStringToObject(item, "path", msg->message.image.path);
            cJSON_AddStringToObject(item, "base64", msg->message.image.base64 == NULL ? "" : msg->message.image.base64);
        }
        else if (msg->type == QXML )
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Xml");
            cJSON_AddStringToObject(item, "xml", msg->message.curr.text);
        }

        else if (msg->type == QJSON)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Json");
            cJSON_AddStringToObject(item, "json", msg->message.curr.text);
        }
        else if (msg->type == QAPP)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "App");
            cJSON_AddStringToObject(item, "content", msg->message.curr.text);
        }
        else if (msg->type == QPOKE)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Poke");
            cJSON_AddStringToObject(item, "name", msg->message.curr.text);
        }
        else if (msg->type == QDICE)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Dice");
            cJSON_AddNumberToObject(item, "value", msg->message.curr.length);
        }
        else if (msg->type == QMUSICSHARE)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "MusicShare");
            cJSON_AddStringToObject(item, "kind", msg->message.music.kind);
            cJSON_AddStringToObject(item, "title", msg->message.music.title);
            cJSON_AddStringToObject(item, "summary", msg->message.music.summary);
            cJSON_AddStringToObject(item, "jumpUrl", msg->message.music.jump_url);
            cJSON_AddStringToObject(item, "pictureUrl", msg->message.music.picture_url);
            cJSON_AddStringToObject(item, "musicUrl", msg->message.music.music_url);
            cJSON_AddStringToObject(item, "brief", msg->message.music.brief);
        }
        else if (msg->type == QFORWARDMESSAGE)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "Forward");
            temp = cJSON_CreateArray();
            mtemp = msg;
            msg = msg->child;
            for (i = 0; msg->next != NULL ; i++,msg = msg->next)
            {

                ctmp = cJSON_CreateObject();
                cJSON_AddNumberToObject(ctmp, "senderId", msg->message.forward.sender_id);
                cJSON_AddNumberToObject(ctmp, "time", msg->message.forward.time);
                cJSON_AddNumberToObject(ctmp, "sourceId", msg->message.forward.source_id);
                cJSON_AddStringToObject(ctmp, "senderName", msg->message.forward.sender_name);
                str = message_convert_json(msg->child);
                cJSON_AddItemToObject(ctmp,"messageChain",cJSON_Parse(str));
                free(str);
                cJSON_AddItemToArray(temp, ctmp);

            }
            msg = mtemp;
            cJSON_AddItemToObject(item, "nodeList", temp);
        }
        else if (msg->type == QFILE)
        {
            item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "type", "File");
            cJSON_AddStringToObject(item, "id", msg->message.file.id);
            cJSON_AddStringToObject(item, "name", msg->message.file.name);
            cJSON_AddNumberToObject(item, "size", msg->message.file.size);
        }
        cJSON_AddItemToArray(root, item);
        msg = msg->next;
    }
    res = cJSON_Print(root);
    cJSON_Delete(root);
    return res;
}

int _QQ_SPIDER_ destroy_message(qq_message* msg)
{
    qq_message* mitem = NULL, * mtemp = NULL;
    while (msg)
    {
        mitem = msg->next;
        destroy_message(msg->child);
        switch (msg->type)
        {
        case QQUOTE:
        case QFORWARDMESSAGE:
            break;
        case QIMAGE:
            if (msg->message.image.base64)   free(msg->message.image.base64); break;
        case QFILE:
            if (msg->message.file.context)   free(msg->message.file.context); break;
        case QPOKE:
        case QJSON:
        case QXML:
        case QAPP:
        case QPLAIN:
            if (msg->message.curr.text) 
                free(msg->message.curr.text); break;
        default:
            break;
        }
        free(msg);
        msg = mitem;
    }
    msg = NULL;
    return 0;
}

char * _QQ_SPIDER_ convert_code(const char* tocode, const char* fromcode, const char* inbuf)
{
    iconv_t cd;
    char** pin = &inbuf;
    char** pout = NULL;
    char* outbuf = NULL;
    size_t inlen = strlen(inbuf);
    size_t outlen = inlen << 3;
    size_t size = inlen << 3;
    outbuf = malloc(outlen);
    if (outbuf == NULL) return NULL;
    memset(outbuf, 0, outlen);
    pout = &outbuf;
    cd = iconv_open(tocode, fromcode);
    if (cd == -1)
    {
        free(outbuf);
        return NULL;
    }

    int ret = iconv(cd, (const char**)pin, &inlen, pout, &outlen);
    if (errno == EINVAL) printf("不完整的字节\n");
    if (errno == E2BIG) printf("没有剩余空间\n");
    if (errno == EILSEQ) printf("源字串无效字节\n");
    outbuf -= size - outlen;
    if (ret == -1)
    {
        iconv_close(cd);
        free(outbuf);
        return NULL;
    }
    iconv_close(cd);
    return outbuf;
}


qq_message* _QQ_SPIDER_ match_command(const char* command, const char* msg)
{
    qq_message* res = NULL,*note = NULL,*tmp = NULL;
    cJSON *root = NULL;
    
	int error_code = 0;
	PCRE2_SIZE error_offset = 0;
	int nOptions =  0;  
	const char * strOrg2 = msg;
	const char * strRex2 = command;
	pcre2_code *recm;
	pcre2_match_data *match_data;
	int rc2 = 0;
	int start_offset = 0;
	unsigned int match_index = 0;
	PCRE2_SIZE *ovector;
	int i = 0;
	char *substring_start;
	int substring_length;
	char matched[1024];
	 
	 
#ifdef _DEBUG
	printf(" PCRE 2 库  应用实例:\n\n");
	printf("字符串是 : %s\n", strOrg2);
	printf("正则表达式是: %s\n\n\n", strRex2);
#endif
	//going to compile the regular expression pattern, and handle  any errors that are detected
	recm = pcre2_compile((PCRE2_SPTR)strRex2, PCRE2_ZERO_TERMINATED, 0 , &error_code, &error_offset, NULL);
	//Compilation failed: print the error message and exit
	if (recm == NULL)
	{
	    PCRE2_UCHAR buffer[256];
	    pcre2_get_error_message(error_code, buffer, sizeof(buffer));
	    printf("PCRE2 compilation failed at offset %d: %s\n", (int)error_offset,buffer);
	    return NULL;
	}
	// Using this function ensures that the block is exactly the right size for the number of capturing parentheses in the pattern.
	match_data = pcre2_match_data_create_from_pattern(recm, NULL);

	rc2= pcre2_match(recm, (PCRE2_SPTR)strOrg2, strlen(strOrg2), start_offset, 0, match_data, NULL);
	if (rc2 < 0){
	    switch(rc2){
	        case PCRE2_ERROR_NOMATCH: printf("No match\n"); break;
	        /*Handle other special cases if you like */
	        default: printf("Matching error %d\n", rc2); break;
	    }
	    pcre2_match_data_free(match_data);   /* Release memory used for the match */
	    pcre2_code_free(recm);                 /* data and the compiled pattern. */
	    return NULL;
	}
	//going to compile the regular expression pattern, and handle  any errors that are detected
	do{
	    //Match succeded. Get a pointer to the output vector, where string offsets are stored. 
	    ovector = pcre2_get_ovector_pointer(match_data);
        for (i = 0; i < rc2; i++) { 
            substring_start = (char *)strOrg2+ ovector[2*i]; 
            substring_length = ovector[2*i+1] - ovector[2*i]; 
            memset( matched, 0, 1024 ); 
            strncpy( matched, substring_start, substring_length ); 
            root = cJSON_Parse(matched);
            if(root != NULL && res != NULL){
                tmp = parse_qmessage(root);
                note->next = tmp;
                note = note->next;
            }else if(root != NULL && res == NULL){
                note = parse_qmessage_item(root);
                res = note;
            }else{
#ifdef _DEBUG
                printf("current json wrong format!");
#else
                return NULL;
#endif
            }
#ifdef _DEBUG
            printf( "全部匹配结果有  :%s\n",matched ); 
#endif
        } 
        start_offset = ovector[2*(i-1) + 1];
        if ( strOrg2[start_offset] == '\0'){ break; }
	}while ((rc2 = pcre2_match(recm, (PCRE2_SPTR)strOrg2, strlen(strOrg2), start_offset, 0, match_data, NULL)) > 0);
	pcre2_match_data_free(match_data);      /* Release memory used for the match */
	pcre2_code_free(recm);                  /* data and the compiled pattern. */
	return res;
}


#ifdef _DEBUG
static int test(CURL** cd, qq_spider* qq, int time);
int main()
{
    printf("Hello World!\n");
    char command[64] = { 0 };
    char* h = NULL; "你好啊!!";  h = malloc(256);
    h[0] = 204;
    h[1] = 183;
    h[2] = 211;
    h[3] = 234;
    h[4] = 230;
    h[5] = 195;
    h[6] = 65;
    h[7] = 65;
    h[8] = 0;
    //scanf("%s", h);
    char * code = transcoding("utf-8", "gbk", h);
    CURL* curl = init_web_spider();
    qq_spider* qq = NULL;
    char *res;
    qq = init_qq_spider(&curl, "localhost", 4040, "zzer-qqbot", 127726076);
    char* message = transcoding("utf-8", "utf-8", "Hello World!!\n你好啊!!我是你的专属NPC");
    get_friend_list(&curl, qq);
    get_group_list(&curl, qq);
    get_friend_profile(&curl, qq, 1589915708);
    get_member_list(&curl, qq, 548590128);
    res = send_message(&curl, qq, 1589915708, message);
    //match_command("\\b\\w+\\b","I am hk!");
    match_command("\\b\\w+\\d{2}\\b","I12 am23 hk23!");
    
    do{
        printf("continue?...\t\t yes/no\n");
        scanf("%s",command);
    }while(strcmp(command,"yes") != 0 && strcmp(command,"no") != 0);
    
    if(strcmp(command,"yes") == 0)  test(&curl, qq, 10010000);
    free(message);
    //send_post(&curl, "http://localhost:4040/verify", "{\"verifyKey\":\zzer-qqbot\"}", res, NULL,NULL);
    return 0;
}


int test(CURL** cd,qq_spider * qq,int time) {
	int i = 0;
	char mam[32] = { 0 };
    while (true) {
        int ret = unread_message_number(cd, qq);
        printf("ret=%d\n\n\n",ret);
        char* old = get_old_unread_information(cd, qq, 10);
        printf("old=%s\n\n\n",old);
        char* new = get_new_unread_information(cd, qq, 10);
        printf("new=%s\n\n\n",new);
        //parsing_events_information(cd, qq, old);

        printf("qq->mcount==%d\n\n\n", qq->mcount);
        parsing_events_information(new, &qq);
        parsing_events_information(old, &qq);
        printf("qq->mcount==%d\n\n\n", qq->mcount);
        char * mg = NULL;
        if(qq->lately_message != NULL){ 
            printf("Not full!!!\n");
            for(i = 0; i < qq->mcount; i++){
                mg = message_convert_json(qq->lately_message[i]);
                printf("mg=%s\n\n\n", mg);
                free(mg);
                mg = NULL;
            }
        }
        if(qq->lately_message != NULL) {
		for(i = 0; i < qq->mcount; i++)
	       		destroy_message(qq->lately_message[i]);
		free(qq->lately_message);
		qq->lately_message = NULL;
		qq->mcount = 0;
	}
	printf("qq->mcount==%d\n\n\n", qq->mcount);
	
	printf("continue?...  (Y/N)");
	if(mam[0] != NULL && strcmp(mam,"AUTO YES") != 0)	scanf("%s",mam);
	if(strcmp(mam,"YES") == 0){
		continue;
	}else if(strcmp(mam,"AUTO YES") == 0){
		
	}else if(strcmp(mam,"NO") == 0){
		break;
	}
#ifdef _WIN32
        Sleep(time);
#endif
#ifdef __linux__
        usleep(time);
#endif
    }
}

#endif // _DEBUG
// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

char* transcoding(const char* tocode, const char* fromcode, const char* inbuf)
{
    iconv_t cd;
    char** pin = &inbuf;
    char** pout = NULL;
    char* outbuf = NULL;
    size_t inlen = strlen(inbuf); 
    size_t outlen = inlen << 3;
    size_t size = inlen << 3;
    outbuf = malloc(outlen);
    if (outbuf == NULL) return NULL;
    memset(outbuf, 0, outlen);
    pout = &outbuf;
    cd = iconv_open(tocode, fromcode);
    if (cd == -1)
    {
        free(outbuf);
        return NULL;
    }
    
    int ret = iconv(cd, (const char**)pin, &inlen, pout, &outlen);
    if (errno == EINVAL) printf("不完整的字节");
    if (errno == E2BIG) printf("没有剩余空间");
    if (errno == EILSEQ) printf("源字串无效字节");
    outbuf -= size - outlen;
    if (ret == -1)
    {
        iconv_close(cd);
        free(outbuf);
        return NULL;
    }
    iconv_close(cd);
    return outbuf;
}

qq_friend* parse_qq_friend(cJSON* root)
{
    qq_friend* friend = NULL;

    friend = malloc(sizeof(qq_friend));
    if (friend == NULL) return NULL;
    memset(friend, 0, sizeof(qq_friend));
    
    friend->qq_id = cJSON_GetObjectItem(root, "id")->valuedouble;
    strcpy(friend->remark, cJSON_GetObjectItem(root, "remark")->valuestring);
    strcpy(friend->infor.nickname, cJSON_GetObjectItem(root, "nickname")->valuestring);
    
    return friend;
}

qq_group* parse_qq_group(cJSON* root)
{
    qq_group* group = NULL;

    group = malloc(sizeof(qq_group));
    if (group == NULL) { printf("function %s line %d error: NULL", __FUNCTION__, __LINE__); return NULL; }
    memset(group, 0, sizeof(qq_group));

    group->group_id = cJSON_GetObjectItem(root, "id")->valuedouble;
    strcpy(group->name, cJSON_GetObjectItem(root, "name")->valuestring);
    strcpy(group->permission, cJSON_GetObjectItem(root, "permission")->valuestring);

    return group;
}

qq_group_member* parse_qq_group_member(cJSON* root)
{
    qq_group_member* member = NULL;
    cJSON* group = cJSON_GetObjectItem(root, "group");

    if (group == NULL) { printf("function %s line %d error: NULL",__FUNCTION__,__LINE__); return NULL; }
    member = malloc(sizeof(qq_group_member));
    if (member == NULL) return NULL;
    memset(member, 0, sizeof(qq_group_member));

    member->member.qq_id = cJSON_GetObjectItem(root, "id")->valuedouble;
    strcpy(member->member_name, cJSON_GetObjectItem(root, "memberName")->valuestring);
    strcpy(member->special_title, cJSON_GetObjectItem(root, "specialTitle")->valuestring);
    member->join_timestamp = cJSON_GetObjectItem(root, "joinTimestamp")->valuedouble;
    member->last_speak_timestamp = cJSON_GetObjectItem(root, "lastSpeakTimestamp")->valuedouble;
    member->mute_time_remaining = cJSON_GetObjectItem(root, "muteTimeRemaining")->valuedouble;
    member->group_id = cJSON_GetObjectItem(group, "id")->valuedouble;
    strcpy(member->name, cJSON_GetObjectItem(group, "name")->valuestring);
    strcpy(member->permission, cJSON_GetObjectItem(group, "permission")->valuestring);

    return member;
}

qq_message* parse_qmessage_item(cJSON* root)
{    
    qq_message* msg = NULL, * tem = NULL,*head = NULL;
    int size = 0;
    int i = 0,j = 0;
    int itemp = 0;
    cJSON* type = NULL, * item = NULL, *ctemp = NULL;
    /* source */
    cJSON* id = NULL, * time = NULL;
    cJSON* base64;
    
    msg = malloc(sizeof(qq_message));
    if(msg == NULL) return NULL;
    memset(msg, 0, sizeof(qq_message));
    head = msg;

    item = root;
    type = cJSON_GetObjectItem(item, "type");
    printf("-------------\n");
    if (strcmp(type->valuestring, "Source") == 0) {
        msg->type = QSOURCE;
        msg->message.source.id = cJSON_GetObjectItem(item, "id")->valuedouble;
        msg->message.source.time = cJSON_GetObjectItem(item, "time")->valuedouble;
    }
    else if (strcmp(type->valuestring, "Quote") == 0)
    {
        msg->type = QQUOTE;
        msg->message.quote.id = cJSON_GetObjectItem(item, "id")->valuedouble;
        msg->message.quote.group_id = cJSON_GetObjectItem(item, "groupId")->valuedouble;
        msg->message.quote.sender_id = cJSON_GetObjectItem(item, "senderId")->valuedouble;
        msg->message.quote.target_id = cJSON_GetObjectItem(item, "targetId")->valuedouble;
        msg->child = parse_qmessage(cJSON_GetObjectItem(item, "origin"));
    }
    else if (strcmp(type->valuestring, "At") == 0)
    {
        msg->type = QAT;
        msg->message.at.target = cJSON_GetObjectItem(item, "target")->valuedouble;
        strcpy(msg->message.at.display, cJSON_GetObjectItem(item, "display")->valuestring);
    }
    else if (strcmp(type->valuestring, "Face") == 0)
    {
        msg->type = QFACE;
        msg->message.face.id = cJSON_GetObjectItem(item, "faceId")->valuedouble;
        strcpy(msg->message.face.name, cJSON_GetObjectItem(item, "name")->valuestring);
    }
    else if (strcmp(type->valuestring, "Plain") == 0)
    {
        msg->type = QPLAIN;
        msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "text")->valuestring) + 1;
        msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
        if (msg->message.curr.text == NULL) { free(msg); printf("funtion :%s   line: %d malloc failed!!!",__FUNCTION__,__LINE__); return NULL; }
        strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "text")->valuestring);
    }
    else if (strcmp(type->valuestring, "Image") == 0 || strcmp(type->valuestring, "FlashImage") == 0 || strcmp(type->valuestring, "Voice") == 0)
    {
        msg->type = QIMAGE;
        base64 = cJSON_GetObjectItem(item, "base64");
        if (base64->type != 2) {
            msg->message.image.length = strlen(cJSON_GetObjectItem(item, "base64")->valuestring) + 1;
            msg->message.image.base64 = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.image.base64 == NULL) { free(msg); printf("funtion :%s   line: %d malloc failed!!!",__FUNCTION__,__LINE__); return NULL; }
            strcpy(msg->message.image.base64, cJSON_GetObjectItem(item, "base64")->valuestring);
        }
        strcpy(msg->message.image.image_id, cJSON_GetObjectItem(item, "imageId")->valuestring);
        if(cJSON_GetObjectItem(item, "url")->type != 2)    strcpy(msg->message.image.url, cJSON_GetObjectItem(item, "url")->valuestring);
        if(cJSON_GetObjectItem(item, "path")->type != 2)    strcpy(msg->message.image.path, cJSON_GetObjectItem(item, "path")->valuestring);
    }
    else if (strcmp(type->valuestring, "Xml") == 0 )
    {
        msg->type = QXML ;
        msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "xml")->valuestring) + 1;
        msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
        if (msg->message.curr.text == NULL) { free(msg); printf("funtion :%s   line: %d malloc failed!!!",__FUNCTION__,__LINE__); return NULL; }
        strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "xml")->valuestring);
    }

    else if (strcmp(type->valuestring, "Json") == 0)
    {
        msg->type = QJSON ;
        msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "Json")->valuestring) + 1;
        msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
        if (msg->message.curr.text == NULL) { free(msg); printf("funtion :%s   line: %d malloc failed!!!",__FUNCTION__,__LINE__); return NULL; }
        strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "Json")->valuestring);
    }
    else if (strcmp(type->valuestring, "App") == 0 )
    {
        msg->type = QAPP;
        msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "App")->valuestring) + 1;
        msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.curr.text == NULL) { free(msg); printf("funtion :%s   line: %d malloc failed!!!",__FUNCTION__,__LINE__); return NULL; }
        strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "App")->valuestring);
    }
    else if ( strcmp(type->valuestring, "Poke") == 0)
    {
        msg->type = QPOKE;
        msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "Poke")->valuestring) + 1;
        msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
        if (msg->message.curr.text == NULL) { free(msg); printf("funtion :%s   line: %d malloc failed!!!",__FUNCTION__,__LINE__); return NULL; }
        strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "Poke")->valuestring);
    }
    else if (strcmp(type->valuestring, "Dice") == 0)
    {
        msg->type = QDICE;
        msg->message.curr.length = cJSON_GetObjectItem(item, "Dice")->valuedouble;
    }
    else if (strcmp(type->valuestring, "MusicShare") == 0)
    {
        msg->type = QIMAGE;
        strcpy(msg->message.music.kind, cJSON_GetObjectItem(item, "kind")->valuestring);
        strcpy(msg->message.music.title, cJSON_GetObjectItem(item, "title")->valuestring);
        strcpy(msg->message.music.summary, cJSON_GetObjectItem(item, "summary")->valuestring);
        strcpy(msg->message.music.jump_url, cJSON_GetObjectItem(item, "jumpUrl")->valuestring);
        strcpy(msg->message.music.picture_url, cJSON_GetObjectItem(item, "pictureUrl")->valuestring);
        strcpy(msg->message.music.music_url, cJSON_GetObjectItem(item, "musicUrl")->valuestring);
        strcpy(msg->message.music.brief, cJSON_GetObjectItem(item, "brief")->valuestring);
    }
    else if (strcmp(type->valuestring, "Forward") == 0)
    {
        msg->type = QFORWARDMESSAGE;
        itemp = cJSON_GetArraySize(cJSON_GetObjectItem(item, "nodeList"));
        item = cJSON_GetObjectItem(item, "nodeList");
        if (itemp <= 0) { free(msg); printf("funtion :%s   line: %d forward message <= 0!!!",__FUNCTION__,__LINE__); return NULL; }
        msg->child = malloc(sizeof(qq_message));
        if (msg->child == NULL) {
            tem = msg->prev;
            while (msg->prev != NULL)
            {
                if (msg->prev != NULL) tem = msg->prev;
                else break;
                free(msg);
                msg = tem;
            }
            return -1;
        }
        memset(msg->child, 0, sizeof(qq_message));
        tem = msg;
        msg = msg->child;
        msg->prev = tem;
        for (j = 0; j < itemp; j++)
        {
            ctemp = cJSON_GetArrayItem(item, j);
            msg->message.forward.sender_id = cJSON_GetObjectItem(ctemp, "senderId")->valuedouble;
            msg->message.forward.time = cJSON_GetObjectItem(ctemp, "time")->valuedouble;
            msg->message.forward.source_id = cJSON_GetObjectItem(ctemp, "sourceId") == NULL ? 0 : cJSON_GetObjectItem(ctemp, "sourceId")->valuedouble;
            strcpy(msg->message.forward.sender_name, cJSON_GetObjectItem(ctemp, "senderName")->valuestring);
            msg->child = parse_qmessage(cJSON_GetObjectItem(ctemp, "messageChain"));
            if (j != item - 1) {
                msg->next = malloc(sizeof(qq_message));
                if (msg->next == NULL) {
                    tem = msg->prev;
                    while (msg->prev != NULL)
                    {
                        if (msg->prev != NULL) tem = msg->prev;
                        else break;
                        free(msg);
                        msg = tem;
                    }
                    return -1;
                }
                memset(msg->next, 0, sizeof(qq_message));
                msg->next->prev = msg;
                msg = msg->next;
            }
        }
        msg = tem;
    }
    else if (strcmp(type->valuestring, "File") == 0)
    {
        msg->type = QFILE;
        msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "name")->valuestring);
        msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
        if (msg->message.curr.text == NULL) { free(msg); printf("funtion :%s   line: %d malloc failed!!!",__FUNCTION__,__LINE__); return NULL; }
        strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "text")->valuestring);
        msg->message.curr.id = cJSON_GetObjectItem(item, "id")->valuedouble;
        msg->message.curr.size = cJSON_GetObjectItem(item, "size")->valuedouble;
    }
    
    msg = head;
    return msg;
    
}

qq_message* parse_qmessage(cJSON* root)
{
    qq_message* msg = NULL, * tem = NULL,*head = NULL;
    int size = 0;
    int i = 0,j = 0;
    int itemp = 0;
    cJSON* type = NULL, * item = NULL, *ctemp = NULL;
    /* source */
    cJSON* id = NULL, * time = NULL;
    cJSON* base64;

    msg = malloc(sizeof(qq_message));
    if(msg == NULL) return NULL;
    memset(msg, 0, sizeof(qq_message));
    head = msg;

    if (cJSON_IsArray(root) != 1 || (size = cJSON_GetArraySize(root)) <= 0) { free(msg); msg = NULL;  return msg; }
    for (i = 0; i < size; i++)
    {
        item = cJSON_GetArrayItem(root, i);
        type = cJSON_GetObjectItem(item, "type");
        if (strcmp(type->valuestring, "Source") == 0) {
            msg->type = QSOURCE;
            msg->message.source.id = cJSON_GetObjectItem(item, "id")->valuedouble;
            msg->message.source.time = cJSON_GetObjectItem(item, "time")->valuedouble;
        }
        else if (strcmp(type->valuestring, "Quote") == 0)
        {
            msg->type = QQUOTE;
            msg->message.quote.id = cJSON_GetObjectItem(item, "id")->valuedouble;
            msg->message.quote.group_id = cJSON_GetObjectItem(item, "groupId")->valuedouble;
            msg->message.quote.sender_id = cJSON_GetObjectItem(item, "senderId")->valuedouble;
            msg->message.quote.target_id = cJSON_GetObjectItem(item, "targetId")->valuedouble;
            msg->child = parse_qmessage(cJSON_GetObjectItem(item, "origin"));
        }
        else if (strcmp(type->valuestring, "At") == 0)
        {
            msg->type = QAT;
            msg->message.at.target = cJSON_GetObjectItem(item, "target")->valuedouble;
            strcpy(msg->message.at.display, cJSON_GetObjectItem(item, "display")->valuestring);
        }
        else if (strcmp(type->valuestring, "Face") == 0)
        {
            msg->type = QFACE;
            msg->message.face.id = cJSON_GetObjectItem(item, "faceId")->valuedouble;
            strcpy(msg->message.face.name, cJSON_GetObjectItem(item, "name")->valuestring);
        }
        else if (strcmp(type->valuestring, "Plain") == 0)
        {
            msg->type = QPLAIN;
            msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "text")->valuestring) + 1;
            msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.curr.text == NULL) break;
            strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "text")->valuestring);
        }
        else if (strcmp(type->valuestring, "Image") == 0 || strcmp(type->valuestring, "FlashImage") == 0 || strcmp(type->valuestring, "Voice") == 0)
        {
            msg->type = QIMAGE;
            base64 = cJSON_GetObjectItem(item, "base64");
            if (base64->type != 2) {
                msg->message.image.length = strlen(cJSON_GetObjectItem(item, "base64")->valuestring) + 1;
                msg->message.image.base64 = malloc(msg->message.curr.length * sizeof(char));
                if (msg->message.image.base64 == NULL) break;
                strcpy(msg->message.image.base64, cJSON_GetObjectItem(item, "base64")->valuestring);
            }
            strcpy(msg->message.image.image_id, cJSON_GetObjectItem(item, "imageId")->valuestring);
            if(cJSON_GetObjectItem(item, "url")->type != 2)    strcpy(msg->message.image.url, cJSON_GetObjectItem(item, "url")->valuestring);
            if(cJSON_GetObjectItem(item, "path")->type != 2)    strcpy(msg->message.image.path, cJSON_GetObjectItem(item, "path")->valuestring);
        }
        else if (strcmp(type->valuestring, "Xml") == 0 )
        {
            msg->type = QXML ;
            msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "xml")->valuestring) + 1;
            msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.curr.text == NULL) break;
            strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "xml")->valuestring);
        }

        else if (strcmp(type->valuestring, "Json") == 0)
        {
            msg->type = QJSON ;
            msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "Json")->valuestring) + 1;
            msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.curr.text == NULL) break;
            strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "Json")->valuestring);
        }
        else if (strcmp(type->valuestring, "App") == 0 )
        {
            msg->type = QAPP;
            msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "App")->valuestring) + 1;
            msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.curr.text == NULL) break;
            strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "App")->valuestring);
        }
        else if ( strcmp(type->valuestring, "Poke") == 0)
        {
            msg->type = QPOKE;
            msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "Poke")->valuestring) + 1;
            msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.curr.text == NULL) break;
            strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "Poke")->valuestring);
        }
        else if (strcmp(type->valuestring, "Dice") == 0)
        {
            msg->type = QDICE;
            msg->message.curr.length = cJSON_GetObjectItem(item, "Dice")->valuedouble;
        }
        else if (strcmp(type->valuestring, "MusicShare") == 0)
        {
            msg->type = QIMAGE;
            strcpy(msg->message.music.kind, cJSON_GetObjectItem(item, "kind")->valuestring);
            strcpy(msg->message.music.title, cJSON_GetObjectItem(item, "title")->valuestring);
            strcpy(msg->message.music.summary, cJSON_GetObjectItem(item, "summary")->valuestring);
            strcpy(msg->message.music.jump_url, cJSON_GetObjectItem(item, "jumpUrl")->valuestring);
            strcpy(msg->message.music.picture_url, cJSON_GetObjectItem(item, "pictureUrl")->valuestring);
            strcpy(msg->message.music.music_url, cJSON_GetObjectItem(item, "musicUrl")->valuestring);
            strcpy(msg->message.music.brief, cJSON_GetObjectItem(item, "brief")->valuestring);
        }
        else if (strcmp(type->valuestring, "Forward") == 0)
        {
            msg->type = QFORWARDMESSAGE;
            itemp = cJSON_GetArraySize(cJSON_GetObjectItem(item, "nodeList"));
            item = cJSON_GetObjectItem(item, "nodeList");
            if (itemp <= 0) break;
            msg->child = malloc(sizeof(qq_message));
            if (msg->child == NULL) {
                tem = msg->prev;
                while (msg->prev != NULL)
                {
                    if (msg->prev != NULL) tem = msg->prev;
                    else break;
                    free(msg);
                    msg = tem;
                }
                return -1;
            }
            memset(msg->child, 0, sizeof(qq_message));
            tem = msg;
            msg = msg->child;
            msg->prev = tem;
            for (j = 0; j < itemp; j++)
            {
                ctemp = cJSON_GetArrayItem(item, j);
                msg->message.forward.sender_id = cJSON_GetObjectItem(ctemp, "senderId")->valuedouble;
                msg->message.forward.time = cJSON_GetObjectItem(ctemp, "time")->valuedouble;
                msg->message.forward.source_id = cJSON_GetObjectItem(ctemp, "sourceId") == NULL ? 0 : cJSON_GetObjectItem(ctemp, "sourceId")->valuedouble;
                strcpy(msg->message.forward.sender_name, cJSON_GetObjectItem(ctemp, "senderName")->valuestring);
                msg->child = parse_qmessage(cJSON_GetObjectItem(ctemp, "messageChain"));
                if (j != item - 1) {
                    msg->next = malloc(sizeof(qq_message));
                    if (msg->next == NULL) {
                        tem = msg->prev;
                        while (msg->prev != NULL)
                        {
                            if (msg->prev != NULL) tem = msg->prev;
                            else break;
                            free(msg);
                            msg = tem;
                        }
                        return -1;
                    }
                    memset(msg->next, 0, sizeof(qq_message));
                    msg->next->prev = msg;
                    msg = msg->next;
                }
            }
            msg = tem;
        }
        else if (strcmp(type->valuestring, "File") == 0)
        {
            msg->type = QFILE;
            msg->message.curr.length = strlen(cJSON_GetObjectItem(item, "name")->valuestring);
            msg->message.curr.text = malloc(msg->message.curr.length * sizeof(char));
            if (msg->message.curr.text == NULL) break;
            strcpy(msg->message.curr.text, cJSON_GetObjectItem(item, "text")->valuestring);
            msg->message.curr.id = cJSON_GetObjectItem(item, "id")->valuedouble;
            msg->message.curr.size = cJSON_GetObjectItem(item, "size")->valuedouble;
        }
        if (i != size - 1) {
            msg->next = malloc(sizeof(qq_message));
            if (msg->next == NULL) {
                tem = msg->prev;
                while (msg->prev != NULL)
                {
                    if (msg->prev != NULL) tem = msg->prev;
                    else break;
                    free(msg);
                    msg = tem;
                }
                return -1;
            }
            memset(msg->next, 0, sizeof(qq_message));
            msg->next->prev = msg;
            msg = msg->next;
        }
        else {
            break;
        }
    }
    msg = head;
    return msg;
}
//
//char* parsing_events_information(CURL **curl,qq_spider * qq,const char* data)
//{
//    cJSON* root = NULL,*object = NULL,*sender = NULL,*messageChain = NULL,*message = NULL,*group = NULL;
//    int size = 0;
//    int i = 0;
//    int j = 0;
//    char m[512] = { 0 };
//    char* tmp;
//    root = cJSON_Parse(data);
//    if (root == NULL) return NULL;
//    size = cJSON_GetArraySize(root);
//    for ( i = 0; i < size; i++)
//    {
//        object = cJSON_GetArrayItem(root, i);
//        tmp = cJSON_GetObjectItem(object, "type")->valuestring;
//        if (strcmp(tmp, "GroupMessage") == 0) {
//            sender = cJSON_GetObjectItem(object, "sender");
//            messageChain = cJSON_GetObjectItem(object, "messageChain");
//            if (sender->child->valueint == 1589915708 || strcmp(cJSON_GetObjectItem(sender, "permission")->valuestring, "MEMBER") == 0) {
//                for ( j = 0; j < cJSON_GetArraySize(messageChain); j++)
//                {
//                    message = cJSON_GetArrayItem(messageChain, j);
//                    if (strcmp(cJSON_GetObjectItem(message, "type")->valuestring, "Plain")==0) {
//                        tmp = cJSON_GetObjectItem(message, "text")->valuestring;
//                        if (strstr(tmp, "command") != NULL) {
//                            strcpy(m, "对不起哦,暂时还没有开发群命令哦."); 
//                            if (strcmp(cJSON_GetObjectItem(sender, "group")->child->next->valuestring, "test") == 0) {
//                                strcpy(m, "命令        说明  派发任务!  发送你要派发的任务,任务发送完毕回复over.");
//                            }
//                        }
//                        if (strstr(tmp, "Hello") != NULL) { strcpy(m, "Hello!!  "); strcat(m, cJSON_GetObjectItem(sender, "memberName")->valuestring); }
//                    }
//                }
//                if (m[0] != NULL) {
//                    tmp = transcoding("utf-8", "GBK", m);
//                    send_group_message(curl, qq, cJSON_GetObjectItem(sender, "group")->child->valueint, tmp);
//                    free(tmp);
//                }
//            }
//        }
//    }
//    cJSON_Delete(root);
//    return NULL;
//}

